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
#include "EvoCompressorSettings/EvoComSettings.hpp"
#include "../Evolver/Evolver.hpp"

namespace GC {

    class EvolutionaryFileCompressor {

    public:
        using FileName = std::string;
        using TransformCode = TCode;
        using TList = Individual::TList;
        using CompressionCode = CCode;
        using Fitness = Individual::FitnessScore;

        std::string to_string();


        EvolutionaryFileCompressor() {};
        static void compress(const EvoComSettings &settings);
        static void decompress(const FileName& fileToDecompress, const FileName& outputFile);


        static void generateCompressionData(const EvoComSettings &settings, Logger& logger);


    private:
        static const size_t bitSizeForTransformCode = 4;
        static const size_t bitSizeForCompressionCode = 4;
        static const size_t bitsForAmountOfTransforms = 4;

        static Block readBlock(size_t size, FileBitReader &reader);


    public: //for the purposes of testing
        static void applyTransformCode(const TransformCode &tc, Block &block);

        static void applyCompressionCode(const CompressionCode &cc, const Block &block, AbstractBitWriter& writer);

        static void compressBlockUsingRecipe_DataCollection(const Individual &individual, const Block &block, AbstractBitWriter& writer, Logger& logger);
        static void compressBlockUsingRecipe(const Individual &individual, const Block &block, AbstractBitWriter& writer);

        static void encodeIndividual(const Individual &individual, AbstractBitWriter& writer);

        static void undoTransformCode(const TransformCode &tc, Block &block);

        static Block undoCompressionCode(const CompressionCode &cc, FileBitReader &reader);

    private:

        static TransformCode decodeTransformCode(FileBitReader &reader);

        static CompressionCode decodeCompressionCode(FileBitReader &reader);

        static void readBlockAndEncode(size_t size, FileBitReader &reader, FileBitWriter &writer);

        static Block decodeSingleBlock(FileBitReader &reader);

        static void writeBlock(Block &block, FileBitWriter &writer);

        static Fitness compressionRatioForIndividualOnBlock(const Individual &individual, const Block &block);

        static Individual decodeIndividual(FileBitReader &reader);

        static Block decodeUsingIndividual(const Individual &individual, FileBitReader &reader);

        static void
        clusterFileInSegments(FileBitReader &reader, std::function<void(const Block &)> blockHandler,
                              const size_t fileSize, const EvoComSettings& settings);


        static Individual evolveBestIndividualForBlock(const Block &block, const Evolver::EvolutionSettings& evoSettings);

        static void processFileAsFixedSegments(FileBitReader &reader, std::function<void(const Block &)> blockHandler,
                                        const size_t fileSize, const EvoComSettings &settings);


        static void compressToStreamsSequentially(FileBitReader &reader, FileBitWriter &writer, const size_t originalFileSize,
                                                  const EvoComSettings &settings);

        static void compressToStreamsAsync(FileBitReader &reader, FileBitWriter &writer, const size_t originalFileSize,
                                           const EvoComSettings &settings);

        static void compressToStreamsSequentially_DataCollection(FileBitReader &reader, AbstractBitWriter &writer,
                                                          const size_t originalFileSize, const EvoComSettings &settings,
                                                          Logger &logger);

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVOLUTIONARYFILECOMPRESSOR_HPP
