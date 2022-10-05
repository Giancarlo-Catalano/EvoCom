//
// Created by gian on 21/09/22.
//

#include "SimpleCompressor.hpp"
#include "../Transformation/Transformations/DeltaTransform.hpp"
#include "../Transformation/Transformations/DeltaXORTransform.hpp"
#include "../Transformation/Transformations/RunLengthTransform.hpp"
#include "../Compression/HuffmanCompression/HuffmanCompression.hpp"
#include "../Compression/RunLengthCompression/RunLengthCompression.hpp"
#include "../Compression/IdentityCompression/IdentityCompression.hpp"
#include "../Evolver/Evolver.hpp"
#include <vector>

namespace GC {
    void SimpleCompressor::compress(const SimpleCompressor::FileName &fileToCompress,
                                    const SimpleCompressor::FileName &outputFile) {

            size_t originalFileSize = getFileSize(fileToCompress);

            if (originalFileSize == 0) {
                //LOG("I refuse to compress such a small file");
                return;
            }

            const size_t blockSize = 256; //for no reason in particular
            const size_t blockAmount = (originalFileSize < blockSize ? 1 : originalFileSize/blockSize);
            const size_t sizeOfLastBlock = (originalFileSize < blockSize ? originalFileSize : blockSize+(originalFileSize%blockSize));
            LOG_NOSPACES("Each block has size ", blockSize, ", meaning that there will be ", blockAmount, " blocks, with the last one having size ", sizeOfLastBlock);

            std::ifstream inStream(fileToCompress);
            FileBitReader reader(inStream);

            std::ofstream outStream(outputFile);
            FileBitWriter writer(outStream);

            auto readBlockAndCompressOnFile = [&](size_t sizeOfBlock) {
                Block block = readBlock(sizeOfBlock, reader);
                Individual bestIndividual = evolveBestIndividualForBlock(block);
                LOG("For this block, the best individual is", bestIndividual.to_string());
                encodeIndividual(bestIndividual, writer);
                encodeUsingIndividual(bestIndividual, block, writer);
            };

            //start of actual compression
            LOG("There are", blockAmount, "blocks to be encoded");
            writer.writeRiceEncoded(blockAmount);
            repeat(blockAmount-1, [&](){readBlockAndCompressOnFile(blockSize);});
            readBlockAndCompressOnFile(sizeOfLastBlock);

            writer.forceLast();
    }

    Block SimpleCompressor::readBlock(size_t size, FileBitReader &reader) {
        Block block;
        auto readUnitAndPush = [&]() {
            block.push_back(reader.readAmount(bitsInType<Unit>()));
        };

        repeat(size, readUnitAndPush);
        return block;
    }

    Block SimpleCompressor::applyTransformCode(const SimpleCompressor::TransformCode &tc, const Block &block) {
        switch (tc) { //TODO add all of them
            case T_DeltaTransform: return DeltaTransform().apply_copy(block);
            case T_DeltaXORTransform: return DeltaXORTransform().apply_copy(block);
            case T_RunLengthTransform: return RunLengthTransform().apply_copy(block);
            default: return block; //ie do nothing
        }
    }

    SimpleCompressor::Bits SimpleCompressor::applyCompressionCode(const SimpleCompressor::CompressionCode &cc,const Block &block) {
        switch (cc) {
            case C_HuffmanCompression: return HuffmanCompression().compressIntoBits(block);
            case C_RunLengthCompression: return RunLengthCompression().compressIntoBits(block);
            default: return IdentityCompression().compressIntoBits(block);
        }
    }

    SimpleCompressor::Bits SimpleCompressor::applyIndividual(const Individual &individual, const Block &block) {
        ////LOG("Applying individual ", individual.to_string());
        Block toBeProcessed = block;
        for (auto tCode : individual.tList) toBeProcessed = applyTransformCode(tCode, toBeProcessed);
        return applyCompressionCode(individual.cCode, toBeProcessed);
    }

    SimpleCompressor::Bits SimpleCompressor::getBinaryRepresentationOfIndividual(const Individual& individual){
        auto getBitsOfTransform = [&](const TCode& tc) {
            return FileBitWriter::getAmountBits(tc, bitSizeForTransformCode);
        };

        auto getBitsOfCompression = [&](const CCode& cc) {
            return FileBitWriter::getAmountBits(cc, bitSizeForCompressionCode);
        };
        Bits result = FileBitWriter::getAmountBits(individual.tList.size(), bitsForAmountOfTransforms);
        for (auto tCode : individual.tList) concatenate(result, getBitsOfTransform(tCode));
        concatenate(result, getBitsOfCompression(individual.cCode));
        return result;
    }

    void SimpleCompressor::encodeIndividual(const Individual& individual, FileBitWriter& writer) {
        Bits individualAsBits = getBinaryRepresentationOfIndividual(individual);
        LOG("Encoding the individual, it is:", containerToString(individualAsBits));
        writer.writeVector(individualAsBits);
    }

    void SimpleCompressor::undoTransformCode(const TransformCode& tc, Block& block) {
        switch (tc) {
            case T_DeltaTransform: DeltaTransform().undo(block); break;
            case T_DeltaXORTransform: DeltaXORTransform().undo(block); break;
            case T_RunLengthTransform: RunLengthTransform().undo(block);break;
            default: return; //ie do nothing
        }
    }

    Block SimpleCompressor::undoCompressionCode(const SimpleCompressor::CompressionCode &cc,FileBitReader& reader) {
        switch (cc) {
            case C_HuffmanCompression: return HuffmanCompression().decompress(reader);
            case C_RunLengthCompression: return RunLengthCompression().decompress(reader);
            default: return IdentityCompression().decompress(reader);
        }
    }

    SimpleCompressor::TransformCode SimpleCompressor::decodeTransformCode(FileBitReader &reader) {
        return static_cast<TransformCode>(reader.readAmount(bitSizeForTransformCode));
    }

    SimpleCompressor::CompressionCode SimpleCompressor::decodeCompressionCode(FileBitReader &reader) {
        return static_cast<CompressionCode>(reader.readAmount(bitSizeForCompressionCode));
    }

    void SimpleCompressor::readBlockAndEncode(size_t size, FileBitReader &reader, FileBitWriter &writer) {
        //LOG("preparing to compress a block");
        Block block = readBlock(size, reader);
        Individual bestIndividual = evolveBestIndividualForBlock(block);
        LOG("For this block, the best individual is", bestIndividual.to_string());
        encodeUsingIndividual(bestIndividual, block, writer);
    }

    void SimpleCompressor::decompress(const SimpleCompressor::FileName &fileToDecompress,
                                      const SimpleCompressor::FileName &outputFile) {

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

    Individual SimpleCompressor::decodeIndividual(FileBitReader& reader) {
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

    Block SimpleCompressor::decodeUsingIndividual(const Individual& individual, FileBitReader& reader) {
        Block transformedBlock = undoCompressionCode(individual.cCode, reader);
        std::for_each(individual.tList.rbegin(), individual.tList.rend(), [&](auto tc){
            undoTransformCode(tc, transformedBlock);});
        return transformedBlock;
    }

    void SimpleCompressor::writeBlock(Block &block, FileBitWriter &writer) {
        auto writeUnit = [&](const Unit unit) {
            writer.writeAmount(unit, bitsInType<Unit>());
        };
        std::for_each(block.begin(), block.end(), writeUnit);
    }

    std::string SimpleCompressor::to_string() {
        return "SimpleCompressor";
    }

    SimpleCompressor::Fitness SimpleCompressor::compressionRatioForIndividualOnBlock(const Individual& individual, const Block& block) {
        size_t originalSize = block.size();
        size_t compressedSize = applyIndividual(individual, block).size() + getBinaryRepresentationOfIndividual(individual).size();
        ASSERT_NOT_EQUALS(compressedSize, 0);
        ASSERT_NOT_EQUALS(originalSize, 0);
        return (double) (compressedSize) / (originalSize);
    }

    Individual SimpleCompressor::evolveBestIndividualForBlock(const Block & block) {
        Evolver::EvolutionSettings settings;
        settings.generationCount = 6;
        settings.populationSize = 4;
        auto getFitnessOfIndividual = [&](const Individual& i){
            return compressionRatioForIndividualOnBlock(i, block);
        };
        Evolver evolver(settings, getFitnessOfIndividual);
        return evolver.evolveBest();
    }

    void SimpleCompressor::encodeUsingIndividual(const Individual& individual, const Block& block, FileBitWriter& writer) {
        writer.writeVector(applyIndividual(individual, block));
    }

} // GC