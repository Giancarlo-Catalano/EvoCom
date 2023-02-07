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
#include "../AbstractBit/AbstractBitReader/AbstractBitReader.hpp"
#include "EvoCompressorSettings/EvoComSettings.hpp"
#include "../Evolver/Evolver.hpp"
#include "../Evolver/Evaluator/BitCounter/BitCounter.hpp"
#include "../AbstractBit/FileBitReader/FileBitReader.hpp"

namespace GC {

    class EvolutionaryFileCompressor {

    public:
        using FileName = std::string;
        using TransformCode = TCode;
        using TList = Individual::TList;
        using CompressionCode = CCode;
        using Fitness = Individual::FitnessScore;


        EvolutionaryFileCompressor() {};
        static void compress(const EvoComSettings &settings);
        static void decompress(const FileName& fileToDecompress, const FileName& outputFile);


        static void processSingleFileForCompressionDataCollection(const FileName& file, const EvoComSettings& settings, Logger& logger);
        static void generateCompressionData(const EvoComSettings &settings, Logger& logger);


        static void generateEvolverConvergenceData(const EvoComSettings &settings, Logger &logger);

    private:
        static const size_t bitSizeForTransformCode = 4;
        static const size_t bitSizeForCompressionCode = 4;
        static const size_t bitsForAmountOfTransforms = 4;

        static Block readBlock(size_t size, AbstractBitReader &reader);


    public: //for the purposes of testing
        static void applyTransformCode(const TransformCode &tc, Block &block);

        static void applyCompressionCode(const CompressionCode &cc, const Block &block, AbstractBitWriter& writer);

        static void compressBlockUsingRecipe_DataCollection(const Individual &individual, const Block &block, GC::BitCounter &writer, Logger& logger);
        static void compressBlockUsingRecipe(const Individual &individual, const Block &block, AbstractBitWriter& writer);

        static void encodeIndividual(const Individual &individual, AbstractBitWriter& writer);

        static void undoTransformCode(const TransformCode &tc, Block &block);

        static Block undoCompressionCode(const CompressionCode &cc, AbstractBitReader &reader);

    private:

        static TransformCode decodeTransformCode(AbstractBitReader &reader);

        static CompressionCode decodeCompressionCode(AbstractBitReader &reader);

        static void readBlockAndEncode(size_t size, AbstractBitReader &reader, AbstractBitWriter &writer);

        static Block decodeSingleBlock(AbstractBitReader &reader);

        static void writeBlock(const Block &block, AbstractBitWriter &writer);

        static Fitness compressionRatioForIndividualOnBlock(const Individual &individual, const Block &block);

        static Individual decodeIndividual(AbstractBitReader &reader);

        static Block decodeUsingIndividual(const Individual &individual, AbstractBitReader &reader);

        static void
        clusterFileInSegments(AbstractBitReader &reader, std::function<void(const Block &)> blockHandler,
                              const size_t fileSize, const EvoComSettings& settings);


        static Individual evolveBestIndividualForBlock(const Block &block, const Evolver::EvolutionSettings& evoSettings);

        static void processFileAsFixedSegments(AbstractBitReader &reader, const std::function<void(
                const Block &)> &blockHandler,
                                               const size_t fileSize, const EvoComSettings &settings);


        static void compressToStreamsSequentially(AbstractBitReader &reader, AbstractBitWriter &writer, const size_t originalFileSize,
                                                  const EvoComSettings &settings);

        static void compressToStreamsAsync(AbstractBitReader &reader, AbstractBitWriter &writer, const size_t originalFileSize,
                                           const EvoComSettings &settings);

        static void compressToStreamsSequentially_DataCollection(AbstractBitReader &reader, BitCounter &writer,
                                                                 const size_t originalFileSize, const EvoComSettings &settings,
                                                                 Logger &logger);

        static void getEvolverConvergenceData(GC::FileBitReader &reader, const size_t size,
                                              const EvoComSettings &settings, Logger& logger);

        static Individual
        evolveIndividualForBlockAndLogProgress(const Block& block, const Evolver::EvolutionSettings& evoSettings, Logger& logger);
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVOLUTIONARYFILECOMPRESSOR_HPP
