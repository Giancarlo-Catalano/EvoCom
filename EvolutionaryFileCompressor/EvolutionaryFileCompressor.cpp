//
// Created by gian on 21/09/22.
//

#include "EvolutionaryFileCompressor.hpp"
#include <vector>
#include "../Utilities/StreamingClusterer/StreamingClusterer.hpp"


#include "../Transformation/Transformations/DeltaTransform.hpp"
#include "../Transformation/Transformations/DeltaXORTransform.hpp"
#include "../Transformation/Transformations/RunLengthTransform.hpp"
#include "../Compression/HuffmanCompression/HuffmanCompression.hpp"
#include "../Compression/RunLengthCompression/RunLengthCompression.hpp"
#include "../Compression/IdentityCompression/IdentityCompression.hpp"
#include "../Evolver/Evolver.hpp"
#include "../Transformation/Transformations/SplitTransform.hpp"
#include "../Transformation/Transformations/SubtractAverageTransform.hpp"
#include "../Transformation/Transformations/SubtractXORAverageTransform.hpp"
#include "../Transformation/Transformations/StrideTransform.hpp"
#include "../Evolver/Evaluator/BitCounter/BitCounter.hpp"
#include "../Compression/NRLCompression/NRLCompression.hpp"
#include "../Transformation/Transformations/StackTransform.hpp"
#include "../Transformation/Transformations/IdentityTransform.hpp"
#include "../Compression/SmallValueCompression/SmallValueCompression.hpp"
#include "../Transformation/Transformations/LempelZivWelchTransform.hpp"

#include <future>
#include <queue>

namespace GC {


    void EvolutionaryFileCompressor::compress(const EvoComSettings& settings) {

        LOG("Compressing using the following settings:");
        LOG(settings.to_string());

        size_t originalFileSize = getFileSize(settings.inputFile);
        std::string outputFile = settings.inputFile+".gac";
        LOG("the file has size", originalFileSize);

        if (originalFileSize < 2) { LOG("I refuse to compress such a small file"); return; }

        std::ifstream inStream(settings.inputFile);
        FileBitReader reader(inStream);

        std::ofstream outStream(outputFile);
        FileBitWriter writer(outStream);

        if (!inStream || !outStream) {LOG("There was a problem when opening the files"); return;}

        if (settings.async)
            compressToStreamsAsync(reader, writer, originalFileSize, settings);
        else
            compressToStreamsSequentially(reader, writer, originalFileSize, settings);
    }


    void EvolutionaryFileCompressor::compressToStreamsSequentially(FileBitReader& reader, FileBitWriter& writer, const size_t originalFileSize, const EvoComSettings& settings) {
        bool isFirstSegment = true;
        Evolver::EvolutionSettings evoSettings(settings);
        auto compressBlock = [&](const Block& block) {
            LOG("Received a block of size", block.size());
            Individual bestIndividual = evolveBestIndividualForBlock(block, evoSettings);
            LOG("For this block, the best individual is", bestIndividual.to_string());
            if (!isFirstSegment) writer.pushBit(1);  //signifies that the segment before had a segment after it
            isFirstSegment = false;
            encodeIndividual(bestIndividual, writer);
            compressBlockUsingRecipe(bestIndividual, block, writer);

        };

        if (settings.segmentationMethod == EvoComSettings::Clustered)
            clusterFileInSegments(reader, compressBlock, originalFileSize, settings);
        else
            processFileAsFixedSegments(reader, compressBlock, originalFileSize, settings);

        writer.pushBit(0);
        writer.forceLast();
    }

    void EvolutionaryFileCompressor::compressToStreamsAsync(FileBitReader& reader, FileBitWriter& writer, const size_t originalFileSize, const EvoComSettings& settings) {
        using Job = std::pair<Block, std::future<Individual>>;
        using JobQueue = std::queue<Job>;

        JobQueue jobQueue;
        Evolver::EvolutionSettings evoSettings(settings);

        auto passBlockToJobQueue = [&](const Block& block) {
            //LOG("Received the block (size", block.size(), "), passing it to the queue");
            jobQueue.emplace(block, std::async(
                    std::launch::async,
                    &EvolutionaryFileCompressor::evolveBestIndividualForBlock,
                    block,
                    evoSettings));
        };

        bool isFirstSegment = true;
        size_t processedSoFar = 0;
        auto compressBlock = [&](const Block& block, const Individual& recipe) {
            processedSoFar += block.size();
            size_t progress = 100.0*(double)(processedSoFar) / (double)originalFileSize;
            LOG_NOSPACES("(Progress ", progress, "%) Received the block (size ", block.size(), "), and the recipe ", recipe.to_string());
            if (!isFirstSegment) writer.pushBit(1);  //signifies that the segment before had a segment after it
            isFirstSegment = false;
            encodeIndividual(recipe, writer);
            compressBlockUsingRecipe(recipe, block, writer);
        };

        if (settings.segmentationMethod == EvoComSettings::Clustered)
            clusterFileInSegments(reader, passBlockToJobQueue, originalFileSize, settings);
        else
            processFileAsFixedSegments(reader, passBlockToJobQueue, originalFileSize, settings);

        LOG("Starting to process the queue!");

        while (!jobQueue.empty()) {
            //LOG("waiting for the future");
            jobQueue.front().second.wait();

            //LOG("acquiring the block and the future");
            Individual recipe = jobQueue.front().second.get();
            Block block = jobQueue.front().first;
            compressBlock(block, recipe);

            jobQueue.pop(); //very important!!
        }

        LOG("all done!");

        writer.pushBit(0);
        writer.forceLast();
    }

    void EvolutionaryFileCompressor::processFileAsFixedSegments(FileBitReader& reader, std::function<void(const Block&)> blockHandler,
                                                                const size_t fileSize, const EvoComSettings& settings) {

        LOG("Processing the file as fixed segments of size", settings.fixedSegmentSize);
        size_t remaining = fileSize;
        const size_t blockSize = settings.fixedSegmentSize;
        while (remaining > blockSize * 2) {
            const Block block = readBlock(settings.fixedSegmentSize, reader);
            blockHandler(block);
            remaining -= blockSize;
        }
        const Block finalBlock = readBlock(remaining, reader);
        blockHandler(finalBlock);
    }

    Block EvolutionaryFileCompressor::readBlock(size_t size, FileBitReader &reader) {
        Block block;
        auto readUnitAndPush = [&]() {
            block.push_back(reader.readAmountOfBits(bitsInType<Unit>()));
        };

        repeat(size, readUnitAndPush);
        return block;
    }



    void EvolutionaryFileCompressor::compressBlockUsingRecipe(const Individual &individual, const Block &block, AbstractBitWriter& writer) {
        ////LOG("Applying individual ", individual.to_string());
        Block toBeProcessed = block;
        for (auto tCode : individual.tList) applyTransformCode(tCode, toBeProcessed);
        applyCompressionCode(individual.cCode, toBeProcessed, writer);
    }


    void EvolutionaryFileCompressor::encodeIndividual(const Individual& individual, AbstractBitWriter& writer){
        auto encodeTransform = [&](const TCode tc) {
            writer.writeAmountOfBits(tc, bitSizeForCompressionCode);
        };
        auto encodeCompression = [&](const CCode cc) {
            writer.writeAmountOfBits(cc, bitSizeForCompressionCode);
        };

        writer.writeAmountOfBits(individual.tList.size(), bitsForAmountOfTransforms);
        for (const auto tCode: individual.tList) encodeTransform(tCode);
        encodeCompression(individual.cCode);
    }


    void EvolutionaryFileCompressor::decompress(const EvolutionaryFileCompressor::FileName &fileToDecompress,
                                                const EvolutionaryFileCompressor::FileName &outputFile) {

        std::ifstream inStream(fileToDecompress);
        std::ofstream outStream(outputFile);

        FileBitReader reader(inStream);
        FileBitWriter writer(outStream);

        auto decodeAndWriteOnFile = [&]() {
            LOG("Decompressing new block");
            Individual individual = decodeIndividual(reader);
            LOG("Extracted that the recipe for the next block is ", individual.to_string());
            Block decodedBlock = decodeUsingIndividual(individual, reader);
            writeBlock(decodedBlock, writer);
        };

        //LOG("starting the decoding of blocks");

        auto thereAreMoreSegments = [&]() -> bool {
            LOG("Are there more segments?");
            bool morePresent = reader.readBit();
            LOG("Morepresent =", morePresent);
            return morePresent;
        };
        do {
            decodeAndWriteOnFile();
        } while (thereAreMoreSegments()); //if there is a bit following a segment, there is another segment to be decoded

        writer.forceLast();

    }

    Individual EvolutionaryFileCompressor::decodeIndividual(FileBitReader& reader) {
        std::vector<TCode> tList;
        auto addTCode = [&](){
            tList.push_back(static_cast<TCode>(reader.readAmountOfBits(bitSizeForTransformCode)));
        };

        auto extractCCode = [&]() -> CCode{
            return static_cast<CCode>(reader.readAmountOfBits(bitSizeForCompressionCode));
        };

        size_t amountOfTransforms = reader.readAmountOfBits(bitsForAmountOfTransforms);
        repeat(amountOfTransforms, addTCode);
        return Individual(tList, extractCCode());
    }

    Block EvolutionaryFileCompressor::decodeUsingIndividual(const Individual& individual, FileBitReader& reader) {
        Block transformedBlock = undoCompressionCode(individual.cCode, reader);
        LOG("Undone the compression successfully, now we undo the transformations");
        std::for_each(individual.tList.rbegin(), individual.tList.rend(), [&](auto tc){
            undoTransformCode(tc, transformedBlock);});
        return transformedBlock;
    }

    void EvolutionaryFileCompressor::writeBlock(Block &block, FileBitWriter &writer) {
        auto writeUnit = [&](const Unit unit) {
            writer.writeAmountOfBits(unit, bitsInType<Unit>());
        };
        std::for_each(block.begin(), block.end(), writeUnit);
    }

    std::string EvolutionaryFileCompressor::to_string() {
        return "SimpleCompressor";
    }

    EvolutionaryFileCompressor::Fitness EvolutionaryFileCompressor::compressionRatioForIndividualOnBlock(const Individual& individual, const Block& block) {
        size_t originalSize = block.size()*8;

        BitCounter counterWriter;
        encodeIndividual(individual, counterWriter);
        compressBlockUsingRecipe(individual, block, counterWriter);
        size_t compressedSize = counterWriter.getCounterValue();
        //a compressed block is a sequence of bits, not necessarly in multiples of 8
                ASSERT_NOT_EQUALS(compressedSize, 0); //would be impossible
        ASSERT_NOT_EQUALS(originalSize, 0);   //would cause errors
        return (double) (compressedSize) / (originalSize);
    }

    Individual EvolutionaryFileCompressor::evolveBestIndividualForBlock(const Block & block, const Evolver::EvolutionSettings& evoSettings) {
        auto getFitnessOfIndividual = [&](const Individual& i){
            return compressionRatioForIndividualOnBlock(i, block);
        };

        Evolver evolver(evoSettings, getFitnessOfIndividual);
        Individual bestIndividual = evolver.evolveBest();
        return bestIndividual;
    }

    void EvolutionaryFileCompressor::applyCompressionCode(const EvolutionaryFileCompressor::CompressionCode &cc, const Block &block, AbstractBitWriter& writer) {
        switch (cc) {
            case C_IdentityCompression: return IdentityCompression().compress(block, writer);
            case C_HuffmanCompression: return HuffmanCompression().compress(block, writer);
            case C_RunLengthCompression: return NRLCompression().compress(block, writer);
            case C_SmallValueCompression: return SmallValueCompression().compress(block, writer);

        }
    }

    void EvolutionaryFileCompressor::applyTransformCode(const EvolutionaryFileCompressor::TransformCode &tc,
                                                        Block &block) {
#define GC_APPLY_T_CASE_X(TRANS, ...) case T_##TRANS : TRANS(__VA_ARGS__).apply(block);break
#define GC_APPLY_T_STRIDE_CASE_X(NUM) case T_StrideTransform_##NUM : StrideTransform(NUM).apply(block); break;
        switch (tc) {
            GC_APPLY_T_CASE_X(DeltaTransform);
            GC_APPLY_T_CASE_X(DeltaXORTransform);
            GC_APPLY_T_CASE_X(RunLengthTransform);
            GC_APPLY_T_CASE_X(StackTransform);
            GC_APPLY_T_CASE_X(SplitTransform);
            GC_APPLY_T_STRIDE_CASE_X(2);
            GC_APPLY_T_STRIDE_CASE_X(3);
            GC_APPLY_T_STRIDE_CASE_X(4);
            GC_APPLY_T_CASE_X(SubtractAverageTransform);
            GC_APPLY_T_CASE_X(SubtractXORAverageTransform);
            GC_APPLY_T_CASE_X(IdentityTransform);
            GC_APPLY_T_CASE_X(LempelZivWelchTransform);
        }

    }

    void EvolutionaryFileCompressor::undoTransformCode(const TransformCode& tc, Block& block) {
        LOG("undoing transform", Individual::TCode_as_string(tc), ", size was", block.size());

#define GC_UNDO_T_CASE(TRANS, ...) case T_##TRANS : TRANS(__VA_ARGS__).undo(block);break;
#define GC_UNDO_T_STRIDE_CASE(NUM) case T_StrideTransform_##NUM : StrideTransform(NUM).undo(block);break;
        switch (tc) {
            GC_UNDO_T_CASE(DeltaTransform);
            GC_UNDO_T_CASE(DeltaXORTransform);
            GC_UNDO_T_CASE(RunLengthTransform);
            GC_UNDO_T_CASE(SplitTransform);
            GC_UNDO_T_CASE(StackTransform);
            GC_UNDO_T_STRIDE_CASE(2);
            GC_UNDO_T_STRIDE_CASE(3);
            GC_UNDO_T_STRIDE_CASE(4);
            GC_UNDO_T_CASE(SubtractAverageTransform);
            GC_UNDO_T_CASE(SubtractXORAverageTransform);
            GC_UNDO_T_CASE(IdentityTransform);
            GC_UNDO_T_CASE(LempelZivWelchTransform);
        }
        LOG("The new block size is", block.size());
    }

    Block EvolutionaryFileCompressor::undoCompressionCode(const EvolutionaryFileCompressor::CompressionCode &cc, FileBitReader& reader) {
        switch (cc) {
            case C_IdentityCompression: return IdentityCompression().decompress(reader);
            case C_HuffmanCompression: return HuffmanCompression().decompress(reader);
            case C_RunLengthCompression: return NRLCompression().decompress(reader);
            case C_SmallValueCompression: return SmallValueCompression().decompress(reader);
        }
    }

    void EvolutionaryFileCompressor::clusterFileInSegments(FileBitReader &reader,
                                                           std::function<void(const Block&)> blockHandler,
                                                           const size_t fileSize, const EvoComSettings& settings) {

        LOG("Compressing in clusters");

        const size_t microUnitSize = 256; //bytes
        size_t remaining = fileSize;

        auto joinBlocks = [&](const std::vector<Block>& cluster) -> Block {
            Block result;
            for (const Block& block : cluster)
                result.insert(result.end(), block.begin(), block.end());

            return result;
        };

        StreamingClusterer<Block, double> clusterer(differentialSampleDistance,
                                [&](const std::vector<Block>& cluster){
                                    blockHandler(joinBlocks(cluster));},
                        settings.clusteredSegmentThreshold,
                        settings.clusteredSegmentCooldown);


        while (remaining > microUnitSize * 2) {  //this is so that the last micro unit has always size at least microUnitSize
            clusterer.pushItem(readBlock(microUnitSize, reader));
            remaining -= microUnitSize;
        }
        clusterer.pushItem(readBlock(remaining, reader));
        clusterer.finish();
    }


} // GC