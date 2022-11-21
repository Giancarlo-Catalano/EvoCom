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


namespace GC {
    void EvolutionaryFileCompressor::compress_fixedSizeBlocks(const EvolutionaryFileCompressor::FileName &fileToCompress,
                                                              const EvolutionaryFileCompressor::FileName &outputFile) {

            size_t originalFileSize = getFileSize(fileToCompress);
            LOG("the file has size", originalFileSize);
            LOG("the file is ", fileToCompress);

            if (originalFileSize < 2) {
                //LOG("I refuse to compress such a small file");
                return;
            }

            const size_t blockAmount = (originalFileSize < blockSize ? 1 : originalFileSize/blockSize);
            const size_t sizeOfLastBlock = (originalFileSize < blockSize ? originalFileSize : blockSize+(originalFileSize%blockSize));
            LOG_NOSPACES("Each block has size ", blockSize, ", meaning that there will be ", blockAmount, " blocks, with the last one having size ", sizeOfLastBlock);

            std::ifstream inStream(fileToCompress);
            FileBitReader reader(inStream);

            std::ofstream outStream(outputFile);
            FileBitWriter writer(outStream);

            size_t amountOfBlocksProcessed = 0;
            auto readBlockAndCompressOnFile = [&](size_t sizeOfBlock, const bool isLast) {
                Block block = readBlock(sizeOfBlock, reader);
                //LOG("Evolving the best individual");
                Individual bestIndividual = evolveBestIndividualForBlock(block);
                LOG("(", amountOfBlocksProcessed+1, "/", blockAmount, ") For this block, the best individual is", bestIndividual.to_string());
                encodeIndividual(bestIndividual, writer);
                applyIndividual(bestIndividual, block, writer);
                writer.pushBit(isLast); //signifies whether there are more blocks
                amountOfBlocksProcessed++;
            };

            //start of actual compression
            LOG("There are", blockAmount, "blocks to be encoded");
            repeat(blockAmount-1, [&](){readBlockAndCompressOnFile(blockSize, true);});
            readBlockAndCompressOnFile(sizeOfLastBlock, false);

            writer.forceLast();
    }

    void EvolutionaryFileCompressor::compress_variableSize(const EvolutionaryFileCompressor::FileName &fileToCompress,
                                                              const EvolutionaryFileCompressor::FileName &outputFile) {

        size_t originalFileSize = getFileSize(fileToCompress);
        LOG("the file has size", originalFileSize);
        LOG("the file is ", fileToCompress);

        if (originalFileSize < 2) {
            LOG("I refuse to compress such a small file");
            return;
        }

        std::ifstream inStream(fileToCompress);
        FileBitReader reader(inStream);

        std::ofstream outStream(outputFile);
        FileBitWriter writer(outStream);


        bool isFirstSegment = true;
        auto compressBlock = [&](const Block& block) {
            LOG("Received a block of size", block.size());
            Individual bestIndividual = evolveBestIndividualForBlock(block);
            LOG("For this block, the best individual is", bestIndividual.to_string());
            if (!isFirstSegment) writer.pushBit(1);  //signifies that the segment before had a segment after it
            isFirstSegment = false;
            encodeIndividual(bestIndividual, writer);
            applyIndividual(bestIndividual, block, writer);

        };

        divideFileInSegments(reader, compressBlock, originalFileSize);
        writer.pushBit(0);
        writer.forceLast();
    }

    Block EvolutionaryFileCompressor::readBlock(size_t size, FileBitReader &reader) {
        Block block;
        auto readUnitAndPush = [&]() {
            block.push_back(reader.readAmountOfBits(bitsInType<Unit>()));
        };

        repeat(size, readUnitAndPush);
        return block;
    }



    void EvolutionaryFileCompressor::applyIndividual(const Individual &individual, const Block &block, AbstractBitWriter& writer) {
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
        applyIndividual(individual, block, counterWriter);
        size_t compressedSize = counterWriter.getCounterValue();
        //a compressed block is a sequence of bits, not necessarly in multiples of 8
                ASSERT_NOT_EQUALS(compressedSize, 0); //would be impossible
        ASSERT_NOT_EQUALS(originalSize, 0);   //would cause errors
        return (double) (compressedSize) / (originalSize);
    }

    Individual EvolutionaryFileCompressor::evolveBestIndividualForBlock(const Block & block) {
        Evolver::EvolutionSettings settings;
        settings.generationCount = 12;
        settings.populationSize = 36;
        settings.chanceOfMutation = 0.05; //usually it's 0.05, but I want to get better results faster.
        settings.usesSimulatedAnnealing = true;
        settings.isElitist = true;
        auto getFitnessOfIndividual = [&](const Individual& i){
            return compressionRatioForIndividualOnBlock(i, block);
        };

        Evolver evolver(settings, getFitnessOfIndividual);
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

    void EvolutionaryFileCompressor::divideFileInSegments(FileBitReader &reader,
                                                     std::function<void(const Block&)> blockHandler,
                                                     const size_t fileSize) {

        const size_t microUnitSize = 64; //bytes
        size_t remaining = fileSize;
        struct BlockAndReport {
            Block block;
            BlockReport report;

            BlockAndReport(const Block& b) : block(b), report(b){};
            BlockAndReport() : block(), report() {};
        };
        using BlockReportDistance = double;
        auto blockMetric = [&](const BlockAndReport& A, const BlockAndReport& B) -> BlockReportDistance {
            double distance = A.report.distanceFrom(B.report);
            //LOG("The distance between ", A.report.to_string(), "and", B.report.to_string(), "is", distance);
            return distance;
        };

        auto stripAwayReportsFromCluster = [&](const std::vector<BlockAndReport>& cluster) -> Block {
            Block result;
            for (const BlockAndReport& bbr : cluster)
                result.insert(result.end(), bbr.block.begin() ,bbr.block.end());

            return result;
        };

        StreamingClusterer<BlockAndReport, BlockReportDistance> clusterer(blockMetric,
                                [&](const std::vector<BlockAndReport>& cluster){
                                    blockHandler(stripAwayReportsFromCluster(cluster));},
                        0.1,
                        2);


        auto readAndPushToClusterer = [&](const size_t bytesToRead) {
            Block newMicroUnit = readBlock(bytesToRead, reader);
            BlockAndReport bbr(newMicroUnit);
            clusterer.pushItem(bbr);
        };

        while (remaining > microUnitSize * 2) {  //this is so that the last micro unit has always size at least microUnitSize
            readAndPushToClusterer(microUnitSize);
            remaining -= microUnitSize;
        }
        readAndPushToClusterer(remaining);
        clusterer.finish();
    }


} // GC