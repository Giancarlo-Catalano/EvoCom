//
// Created by gian on 21/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_EVOLUTIONARYFILECOMPRESSOR_HPP
#define DISS_SIMPLEPROTOTYPE_EVOLUTIONARYFILECOMPRESSOR_HPP

#include "../names.hpp"
#include "../Utilities/utilities.hpp"
#include "../Transformation/Transformation.hpp"
#include "../Compression/Compression.hpp"
#include "../BlockReport/BlockReport.hpp"
#include "../Evolver/Individual/Individual.hpp"
#include "../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"
#include "../AbstractBit/FileBitWriter/FileBitWriter.hpp"

namespace GC {

    class EvolutionaryFileCompressor {

    public:
        using FileName = std::string;
        using TransformCode = TCode;
        using TList = Individual::TList;
        using CompressionCode = CCode;
        using Bits = std::vector<bool>;
        using Fitness = Individual::FitnessScore;

        static const size_t blockSize = 512; //for no reason in particular

        std::string to_string();


        EvolutionaryFileCompressor() {};
        static void compress(const FileName& fileToCompress, const FileName& outputFile);
        static void decompress(const FileName& fileToDecompress, const FileName& outputFile);

    private:
        static const size_t bitSizeForTransformCode = 4;
        static const size_t bitSizeForCompressionCode = 4;
        static const size_t bitsForAmountOfTransforms = 4;

        static Block readBlock(size_t size, FileBitReader &reader);

        static Block applyTransformCode_copy(const TransformCode &tc, const Block &block);
        static void applyTransformCode(const TransformCode &tc, Block &block);

        static void applyCompressionCode(const CompressionCode &cc, const Block &block, AbstractBitWriter& writer);

        static void applyIndividual(const Individual &individual, const Block &block, AbstractBitWriter& writer);

        static void encodeIndividual(const Individual &individual, AbstractBitWriter& writer);

        static void undoTransformCode(const TransformCode &tc, Block &block);

        static Block undoCompressionCode(const CompressionCode &cc, FileBitReader &reader);

        static TransformCode decodeTransformCode(FileBitReader &reader);

        static CompressionCode decodeCompressionCode(FileBitReader &reader);

        static void readBlockAndEncode(size_t size, FileBitReader &reader, FileBitWriter &writer);

        static Block decodeSingleBlock(FileBitReader &reader);

        static void writeBlock(Block &block, FileBitWriter &writer);

        static Fitness compressionRatioForIndividualOnBlock(const Individual &individual, const Block &block);

        static Individual evolveBestIndividualForBlock(const Block &block);

        static Individual decodeIndividual(FileBitReader &reader);

        static Block decodeUsingIndividual(const Individual &individual, FileBitReader &reader);
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVOLUTIONARYFILECOMPRESSOR_HPP
