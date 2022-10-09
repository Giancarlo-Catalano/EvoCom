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
#include "../Individual/Individual.hpp"

namespace GC {

    class SimpleCompressor {

    public:
        using FileName = std::string;
        using TransformCode = TCode;
        using TList = Individual::TList;
        using CompressionCode = CCode;
        using Bits = std::vector<bool>;
        using Fitness = Individual::FitnessScore;

        std::string to_string();


        SimpleCompressor() {};
        static void compress(const FileName& fileToCompress, const FileName& outputFile);
        static void decompress(const FileName& fileToDecompress, const FileName& outputFile);

    private:
        static const size_t bitSizeForTransformCode = 4;
        static const size_t bitSizeForCompressionCode = 4;
        static const size_t bitsForAmountOfTransforms = 4;

        static Block readBlock(size_t size, FileBitReader &reader);

        static Block applyTransformCode(const TransformCode &tc, const Block &block);

        static Bits applyCompressionCode(const CompressionCode &cc, const Block &block);

        static Bits applyIndividual(const Individual &individual, const Block &block);

        static Bits getBinaryRepresentationOfIndividual(const Individual &individual);

        static void encodeIndividual(const Individual &individual, FileBitWriter &writer);

        static void undoTransformCode(const TransformCode &tc, Block &block);

        static Block undoCompressionCode(const CompressionCode &cc, FileBitReader &reader);

        static TransformCode decodeTransformCode(FileBitReader &reader);

        static CompressionCode decodeCompressionCode(FileBitReader &reader);

        static void readBlockAndEncode(size_t size, FileBitReader &reader, FileBitWriter &writer);

        static Block decodeSingleBlock(FileBitReader &reader);

        static void writeBlock(Block &block, FileBitWriter &writer);

        static Fitness compressionRatioForIndividualOnBlock(const Individual &individual, const Block &block);

        static Individual evolveBestIndividualForBlock(const Block &block);

        static void encodeUsingIndividual(const Individual &individual, const Block &block, FileBitWriter &writer);

        static void compressBlockUsingEvolution(const Block &block, FileBitWriter &writer);

        static Individual decodeIndividual(FileBitReader &reader);

        static Block decodeUsingIndividual(const Individual &individual, FileBitReader &reader);
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SIMPLECOMPRESSOR_HPP
