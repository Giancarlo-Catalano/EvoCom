//
// Created by gian on 21/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SIMPLECOMPRESSOR_HPP
#define DISS_SIMPLEPROTOTYPE_SIMPLECOMPRESSOR_HPP

#include "../names.hpp"
#include "../Utilities/utilities.hpp"
#include "../Transformation/Transformation.hpp"
#include "../Compression/Compression.hpp"
#include "../BlockReport/BlockReport.hpp"

namespace GC {

    class SimpleCompressor {

    public:
        using FileName = std::string;

        std::string to_string();


        SimpleCompressor() {};
        static void compress(const FileName& fileToCompress, const FileName& outputFile);
        static void decompress(const FileName& fileToDecompress, const FileName& outputFile);

    private:
        static const size_t bitSizeForTransformCode = 4;
        static const size_t bitSizeForCompressionCode = 4;

        static const size_t maxTransoformsPerBlock = 5;
        static const size_t bitsForAmountOfTransforms = 4;

        enum TransformCode {
            T_DeltaTransform,
            T_DeltaXORTransform,
            T_RunLengthTransform,
            T_SplitTransform,
            T_StackTransfrom,
            T_StrideTransform_2,
            T_StrideTransform_3,
            T_StrideTransform_4,
            T_SubtractAverageTransform,
            T_SubtractXORAverageTransform
        };

        enum CompressionCode {
            C_HuffmanCompression,
            C_RunLengthCompression,
            C_IdentityCompression
        };


        static void applyTransformCode(const TransformCode& tc, Block& block);

        static void applyCompressionCode(const CompressionCode& cc, Block& block, FileBitWriter& writer);

        static void encodeTransformCode(const TransformCode& tc, FileBitWriter& writer);

        static void encodeCompressionCode(const CompressionCode& cc, FileBitWriter& writer);

        static TransformCode decodeTransformCode(FileBitReader& reader);

        static CompressionCode decodeCompressionCode(FileBitReader& reader);

        static bool decideWhetherToCompress(const BlockReport& blockReport);

        static CompressionCode decideCompressionCode(const BlockReport& blockReport);

        static TransformCode decideTransfomCode(const BlockReport& br);

        static void readBlockAndEncode(size_t size, FileBitReader &reader, FileBitWriter &writer);

        static Block readBlock(size_t size, FileBitReader &reader);


        static Block decodeSingleBlock(FileBitReader &reader);

        static void undoTransformCode(const TransformCode &tc, Block &block);

        static Block undoCompressionCode(const CompressionCode &cc, FileBitReader &reader);

        static void writeBlock(Block &block, FileBitWriter &writer);
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SIMPLECOMPRESSOR_HPP
