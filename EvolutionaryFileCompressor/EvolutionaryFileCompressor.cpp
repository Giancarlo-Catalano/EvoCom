//
// Created by gian on 21/09/22.
//

#include "EvolutionaryFileCompressor.hpp"
#include <vector>


#include "../Transformation/Transformations/DeltaTransform.hpp"
#include "../Transformation/Transformations/DeltaXORTransform.hpp"
#include "../Transformation/Transformations/RunLengthTransform.hpp"
#include "../Compression/HuffmanCompression/HuffmanCompression.hpp"
#include "../Compression/RunLengthCompression/RunLengthCompression.hpp"
#include "../Compression/IdentityCompression/IdentityCompression.hpp"
#include "../Evolver/Evolver.hpp"
#include "../Transformation/Transformations/SplitTransform.hpp"
#include "../Transformation/Transformations/StackTransform.hpp"
#include "../Transformation/Transformations/SubtractAverageTransform.hpp"
#include "../Transformation/Transformations/SubtractXORAverageTransform.hpp"
#include "../Transformation/Transformations/StrideTransform.hpp"
#include "../Evolver/Evaluator/BitCounter/BitCounter.hpp"


namespace GC {
    void EvolutionaryFileCompressor::compress(const EvolutionaryFileCompressor::FileName &fileToCompress,
                                              const EvolutionaryFileCompressor::FileName &outputFile) {

            size_t originalFileSize = getFileSize(fileToCompress);

            if (originalFileSize == 0) {
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
            auto readBlockAndCompressOnFile = [&](size_t sizeOfBlock) {
                Block block = readBlock(sizeOfBlock, reader);
                //LOG("Evolving the best individual");
                Individual bestIndividual = evolveBestIndividualForBlock(block);
                LOG("(", amountOfBlocksProcessed, "/", blockAmount, ") For this block, the best individual is", bestIndividual.to_string());
                encodeIndividual(bestIndividual, writer);
                applyIndividual(bestIndividual, block, writer);
                amountOfBlocksProcessed++;
            };

            //start of actual compression
            LOG("There are", blockAmount, "blocks to be encoded");
            writer.writeRiceEncoded(blockAmount);
            repeat(blockAmount-1, [&](){readBlockAndCompressOnFile(blockSize);});
            readBlockAndCompressOnFile(sizeOfLastBlock);

            writer.forceLast();
    }

    Block EvolutionaryFileCompressor::readBlock(size_t size, FileBitReader &reader) {
        Block block;
        auto readUnitAndPush = [&]() {
            block.push_back(reader.readAmount(bitsInType<Unit>()));
        };

        repeat(size, readUnitAndPush);
        return block;
    }

    Block EvolutionaryFileCompressor::applyTransformCode(const EvolutionaryFileCompressor::TransformCode &tc, const Block &block) {
#define GC_APPLY_T_CASE(TRANS, ...) case T_##TRANS : return TRANS(__VA_ARGS__).apply_copy(block)
#define GC_APPLY_T_STRIDE_CASE(NUM) case T_StrideTransform_##NUM : return StrideTransform(NUM).apply_copy(block)
        switch (tc) {
            GC_APPLY_T_CASE(DeltaTransform);
            GC_APPLY_T_CASE(DeltaXORTransform);
            GC_APPLY_T_CASE(RunLengthTransform);
            GC_APPLY_T_CASE(SplitTransform);
            GC_APPLY_T_CASE(StackTransform);
            GC_APPLY_T_STRIDE_CASE(2);
            GC_APPLY_T_STRIDE_CASE(3);
            GC_APPLY_T_STRIDE_CASE(4);
            GC_APPLY_T_CASE(SubtractAverageTransform);
            GC_APPLY_T_CASE(SubtractXORAverageTransform);
            default: return block; //ie do nothing
        }
    }

    void EvolutionaryFileCompressor::applyCompressionCode(const EvolutionaryFileCompressor::CompressionCode &cc, const Block &block, AbstractBitWriter& writer) {
        switch (cc) {
            case C_HuffmanCompression: return HuffmanCompression().compress(block, writer);
            case C_RunLengthCompression: return RunLengthCompression().compress(block, writer);
            default: return IdentityCompression().compress(block, writer);
        }
    }

    void EvolutionaryFileCompressor::applyIndividual(const Individual &individual, const Block &block, AbstractBitWriter& writer) {
        ////LOG("Applying individual ", individual.to_string());
        Block toBeProcessed = block;
        for (auto tCode : individual.tList) toBeProcessed = applyTransformCode(tCode, toBeProcessed);
        applyCompressionCode(individual.cCode, toBeProcessed, writer);
    }


    void EvolutionaryFileCompressor::encodeIndividual(const Individual& individual, AbstractBitWriter& writer){
        auto encodeTransform = [&](const TCode tc) {
            writer.writeAmount(tc, bitSizeForCompressionCode);
        };
        auto encodeCompression = [&](const CCode cc) {
            writer.writeAmount(cc, bitSizeForCompressionCode);
        };

        writer.writeAmount(individual.tList.size(), bitsForAmountOfTransforms);
        for (const auto tCode: individual.tList) encodeTransform(tCode);
        encodeCompression(individual.cCode);
    }

    void EvolutionaryFileCompressor::undoTransformCode(const TransformCode& tc, Block& block) {

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
            default: return; //ie do nothing
        }
    }

    Block EvolutionaryFileCompressor::undoCompressionCode(const EvolutionaryFileCompressor::CompressionCode &cc, FileBitReader& reader) {
        switch (cc) {
            case C_HuffmanCompression: return HuffmanCompression().decompress(reader);
            case C_RunLengthCompression: return RunLengthCompression().decompress(reader);
            default: return IdentityCompression().decompress(reader);
        }
    }
    void EvolutionaryFileCompressor::decompress(const EvolutionaryFileCompressor::FileName &fileToDecompress,
                                                const EvolutionaryFileCompressor::FileName &outputFile) {

        std::ifstream inStream(fileToDecompress);
        std::ofstream outStream(outputFile);

        FileBitReader reader(inStream);
        FileBitWriter writer(outStream);

        auto decodeAndWriteOnFile = [&]() {
            Individual individual = decodeIndividual(reader);
            Block decodedBlock = decodeUsingIndividual(individual, reader);
            writeBlock(decodedBlock, writer);
        };

        const size_t blockAmount = reader.readRiceEncoded();
        LOG("Read that there will be ", blockAmount, "blocks");

        //LOG("starting the decoding of blocks");
        repeat(blockAmount, decodeAndWriteOnFile);
        writer.forceLast();

    }

    Individual EvolutionaryFileCompressor::decodeIndividual(FileBitReader& reader) {
        std::vector<TCode> tList;
        auto addTCode = [&](){
            tList.push_back(static_cast<TCode>(reader.readAmount(bitSizeForTransformCode)));
        };

        auto extractCCode = [&]() -> CCode{
            return static_cast<CCode>(reader.readAmount(bitSizeForCompressionCode));
        };

        size_t amountOfTransforms = reader.readAmount(bitsForAmountOfTransforms);
        LOG("Read that there will be", amountOfTransforms, "transforms");
        repeat(amountOfTransforms, addTCode);
        return Individual(tList, extractCCode());
    }

    Block EvolutionaryFileCompressor::decodeUsingIndividual(const Individual& individual, FileBitReader& reader) {
        Block transformedBlock = undoCompressionCode(individual.cCode, reader);
        std::for_each(individual.tList.rbegin(), individual.tList.rend(), [&](auto tc){
            undoTransformCode(tc, transformedBlock);});
        return transformedBlock;
    }

    void EvolutionaryFileCompressor::writeBlock(Block &block, FileBitWriter &writer) {
        auto writeUnit = [&](const Unit unit) {
            writer.writeAmount(unit, bitsInType<Unit>());
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
        settings.generationCount = 36;
        settings.populationSize = 36;
        settings.chanceOfMutation = 0.1; //usually it's 0.05, but I want to get better results faster.
        auto getFitnessOfIndividual = [&](const Individual& i){
            return compressionRatioForIndividualOnBlock(i, block);
        };

        //Individual identityIndividual;
        std::vector<Individual> hintsForEvolver{/*identityIndividual*/};
        for (auto comp: availableCCodes) {
            Individual temp;
            temp.cCode = comp;
            hintsForEvolver.push_back(temp);
        }

        Evolver evolver(settings, getFitnessOfIndividual, hintsForEvolver);
        Individual bestIndividual = evolver.evolveBest();
        return bestIndividual;
    }

} // GC