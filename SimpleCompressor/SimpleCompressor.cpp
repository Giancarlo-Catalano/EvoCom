//
// Created by gian on 21/09/22.
//

#include "SimpleCompressor.hpp"
#include "../BlockReport/BlockReport.hpp"
#include "../Transformation/Transformations/DeltaTransform.hpp"
#include "../Transformation/Transformations/DeltaXORTransform.hpp"
#include "../Transformation/Transformations/RunLengthTransform.hpp"
#include "../Compression/HuffmanCompression/HuffmanCompression.hpp"
#include "../Compression/RunLengthCompression/RunLengthCompression.hpp"
#include "../Compression/IdentityCompression/IdentityCompression.hpp"
#include <vector>

namespace GC {
    void SimpleCompressor::compress(const SimpleCompressor::FileName &fileToCompress,
                                    const SimpleCompressor::FileName &outputFile) {

            LOG("Attempting to compress the file", fileToCompress);
            size_t originalFileSize = getFileSize(fileToCompress);
            LOG("It has size", originalFileSize);

            if (originalFileSize == 0) {
                LOG("I refuse to compress such a small file");
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

            auto processNormalBlock = [&]() {
                LOG("Processing a normal block");
                readBlockAndEncode(blockSize, reader, writer);
            };

            LOG("And now the blocks will be compressed");
            writer.writeRiceEncoded(blockAmount);
            repeat(blockAmount-1, processNormalBlock);
            readBlockAndEncode(sizeOfLastBlock, reader, writer);

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

    void SimpleCompressor::applyTransformCode(const SimpleCompressor::TransformCode &tc, Block &block) {
        switch (tc) {
            case T_DeltaTransform: DeltaTransform().apply(block); break;
            case T_DeltaXORTransform: DeltaXORTransform().apply(block); break;
            case T_RunLengthTransform: RunLengthTransform().apply(block);break;
            default: return; //ie do nothing
        }
    }

    void SimpleCompressor::undoTransformCode(const TransformCode& tc, Block& block) {
        switch (tc) {
            case T_DeltaTransform: DeltaTransform().undo(block); break;
            case T_DeltaXORTransform: DeltaXORTransform().undo(block); break;
            case T_RunLengthTransform: RunLengthTransform().undo(block);break;
            default: return; //ie do nothing
        }
    }

    void SimpleCompressor::applyCompressionCode(const SimpleCompressor::CompressionCode &cc, Block &block,
                                                FileBitWriter &writer) {
        switch (cc) {
            case C_HuffmanCompression: HuffmanCompression().compress(block, writer);break;
            case C_RunLengthCompression: RunLengthCompression().compress(block, writer);break;
            default: IdentityCompression().compress(block, writer);break;
        }
    }


    Block SimpleCompressor::undoCompressionCode(const SimpleCompressor::CompressionCode &cc,FileBitReader& reader) {
        switch (cc) {
            case C_HuffmanCompression: return HuffmanCompression().decompress(reader);
            case C_RunLengthCompression: return RunLengthCompression().decompress(reader);
            default: return IdentityCompression().decompress(reader);
        }
    }

    void SimpleCompressor::encodeTransformCode(const SimpleCompressor::TransformCode &tc, FileBitWriter &writer) {
        writer.writeAmount(tc, bitSizeForTransformCode);
    }

    void SimpleCompressor::encodeCompressionCode(const SimpleCompressor::CompressionCode &cc, FileBitWriter &writer) {
        writer.writeAmount(cc, bitSizeForCompressionCode);
    }

    SimpleCompressor::TransformCode SimpleCompressor::decodeTransformCode(FileBitReader &reader) {
        return static_cast<TransformCode>(reader.readAmount(bitSizeForTransformCode));
    }

    SimpleCompressor::CompressionCode SimpleCompressor::decodeCompressionCode(FileBitReader &reader) {
        return static_cast<CompressionCode>(reader.readAmount(bitSizeForCompressionCode));
    }

    bool SimpleCompressor::decideWhetherToCompress(const BlockReport &blockReport) {
        return blockReport.frequencyFeatures.maximum <  0.03125;
    }

    SimpleCompressor::CompressionCode SimpleCompressor::decideCompressionCode(const BlockReport &br) {
        if (br.runLengthFeatures.average > 2) return C_RunLengthCompression;
        if (br.frequencyFeatures.maximum >= 0.0625) return C_HuffmanCompression;
        return C_IdentityCompression;
    }

    SimpleCompressor::TransformCode SimpleCompressor::decideTransfomCode(const BlockReport &br) {
        if (br.runLengthFeatures.mode > 2) return T_RunLengthTransform;
        if (br.unitFeatures.minimum > 32) return T_SubtractAverageTransform; //this could be sub minimum
        //if (br.xorAverage != 0) return T_SubtractXORAverageTransform;
        if (br.difference2Features.mode <= 32) return T_DeltaTransform;
        if (br.difference2Features.average > 64) return T_StrideTransform_2;
        if (br.difference3Features.average > 64) return T_StrideTransform_3;
        if (br.difference4Features.average > 64) return T_StrideTransform_4;
        if (br.unitFeatures.average > 128) return T_SubtractAverageTransform;
        if (br.uniqueSymbolsAmount < br.unitFeatures.maximum) return T_StackTransform;
    }

    void SimpleCompressor::readBlockAndEncode(size_t size, FileBitReader &reader, FileBitWriter &writer) {
        LOG("preparing to compress a block");
        Block block = readBlock(size, reader);
        std::vector<TransformCode> appliedTransforms;

        auto encodeAppliedTransforms = [&]() {
            writer.writeAmount(appliedTransforms.size(), bitsForAmountOfTransforms);
            for (const auto code: appliedTransforms) encodeTransformCode(code, writer);
        };


        while (appliedTransforms.size()<maxTransoformsPerBlock) {
            BlockReport blockReport(block);
            if (decideWhetherToCompress(blockReport)) {
                break;
            }

            LOG("Applying a new transform!");
            TransformCode transformCode = decideTransfomCode(blockReport);
            LOG("Decided to apply transform #", transformCode);
            appliedTransforms.push_back(transformCode);
            applyTransformCode(transformCode, block);
        }

        LOG("Time to compress!");
        LOG("First, we write which transforms have been applied");
        encodeAppliedTransforms();
        CompressionCode compressionCode = decideCompressionCode(BlockReport(block));
        LOG("Will compress using compression #", compressionCode);
        encodeCompressionCode(compressionCode, writer);
        LOG("And now we compress");
        applyCompressionCode(compressionCode, block, writer);
    }

    void SimpleCompressor::decompress(const SimpleCompressor::FileName &fileToDecompress,
                                      const SimpleCompressor::FileName &outputFile) {

        std::ifstream inStream(fileToDecompress);
        std::ofstream outStream(outputFile);

        FileBitReader reader(inStream);
        FileBitWriter writer(outStream);

        const size_t blockAmount = reader.readRiceEncoded();
        LOG("Read that there will be ", blockAmount, "blocks");


        auto decodeAndWriteOnFile = [&]() {
            Block decodedBlock = decodeSingleBlock(reader);
            writeBlock(decodedBlock, writer);
        };

        LOG("starting the decoding of blocks");
        repeat(blockAmount, decodeAndWriteOnFile);
        writer.forceLast();

    }

    Block SimpleCompressor::decodeSingleBlock(FileBitReader &reader) {
        std::vector<TransformCode> transformCodes;
        CompressionCode compressionCode;

        auto decodeAndPushTransformCode = [&](){
            transformCodes.push_back(decodeTransformCode(reader));
        };

        size_t amountOfTransformations = reader.readAmount(bitsForAmountOfTransforms);
        repeat(amountOfTransformations, decodeAndPushTransformCode);
        compressionCode = decodeCompressionCode(reader);

        LOG("Read that there are", amountOfTransformations, "transformations:");
        LOG(containerToString(transformCodes));
        LOG("and the compression is #", compressionCode);

        Block decodedBlock = undoCompressionCode(compressionCode, reader);
        std::for_each(transformCodes.rbegin(), transformCodes.rend(), [&](auto tc){undoTransformCode(tc, decodedBlock);});

        return decodedBlock;
    }

    void SimpleCompressor::writeBlock(Block &block, FileBitWriter &writer) {
        auto writeUnit = [&](const Unit unit) {
            writer.writeAmount(unit, bitsInType<Unit>());
        };

        std::for_each(block.begin(), block.end(), writeUnit);
    }

    std::string SimpleCompressor::to_string() {
        std::stringstream ss;
        ss<<"Legend of codes:"<<"\n";
        ss<<T_DeltaTransform<<"=T_DeltaTransform"<<"\n";
        ss<<T_RunLengthTransform<<"=T_RunLengthTransform"<<"\n";
        ss<<T_SubtractAverageTransform<<"=T_SubtractAverageTransform"<<"\n";
        ss<<T_StackTransform<<"=T_StackTransform"<<"\n";
        ss<<T_StrideTransform_2<<"=T_StrideTransform2<<"<<"\n";
        return ss.str();
    }
} // GC