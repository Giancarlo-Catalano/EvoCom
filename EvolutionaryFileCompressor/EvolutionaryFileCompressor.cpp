//
// Created by gian on 21/09/22.
//

#include "EvolutionaryFileCompressor.hpp"
#include <vector>
#include "../Utilities/StreamingClusterer/StreamingClusterer.hpp"
#include "../AbstractBit/FileBitWriter/FileBitWriter.hpp"
#include "../SegmentData/SegmentData.hpp"

#include <future>
#include <queue>

namespace GC {


    void EvolutionaryFileCompressor::compress(const EvoComSettings& settings) {
        size_t originalFileSize = getFileSize(settings.inputFile);
        std::string outputFile = settings.inputFile+".gac";
        LOG("The original file size is", originalFileSize);


        if (originalFileSize <= 2) {LOG("ERROR: file too small!"); return;}

        std::ifstream inStream(settings.inputFile);
        FileBitReader reader(inStream);

        std::ofstream outStream(outputFile);
        FileBitWriter writer(outStream);

        if (!inStream || !outStream) {LOG("ERROR: file could not be openend"); return;}

        if (settings.async)
            compressToStreamsAsync(reader, writer, originalFileSize, settings);
        else
            compressToStreamsSequentially(reader, writer, originalFileSize, settings);
    }

    /**
     * This function is used to generate the data about what transformations and compressions worked best for a given block report
     * @param settings
     */
    void EvolutionaryFileCompressor::generateCompressionData(const EvoComSettings& settings, Logger& logger) {
        settings.log(logger);
        logger.beginList("parsingOfFiles");
        auto parseFile = [&](const FileName& file) {
            LOG("Processing file", file);
            logger.beginUnnamedObject();
            double timeInMilliseconds = timeFunction([&](){ processSingleFileForCompressionDataCollection(file, settings, logger);});
            logger.addVar("timeForFile", timeInMilliseconds);
            logger.endObject();
        };

        std::for_each(settings.testSet.begin(), settings.testSet.end(), parseFile);
        logger.endList(); //ends parsing of file
    }

    void EvolutionaryFileCompressor::processSingleFileForCompressionDataCollection(
            const EvolutionaryFileCompressor::FileName &file, const EvoComSettings &settings, Logger &logger) {
        logger.addVar("fileName", file);
        size_t originalFileSize = getFileSize(file);
        logger.addVar("originalFileSize", originalFileSize);

        if (originalFileSize <= 2) {logger.addVar("Error_FileTooSmall", true); logger.endObject(); return;}

        std::ifstream inStream(file);
        FileBitReader reader(inStream);

        BitCounter writer;
        if (!inStream ) {logger.addVar("Error_FileUnopenable", true);  logger.endObject(); return;}

        //ignores async settings
        compressToStreamsSequentially_DataCollection(reader, writer, originalFileSize, settings, logger);

        logger.addVar("FinalFileSize", writer.getAmountOfBytes());
    }



    void EvolutionaryFileCompressor::compressToStreamsSequentially(AbstractBitReader& reader, AbstractBitWriter& writer, const size_t originalFileSize, const EvoComSettings& settings) {
        bool isFirstSegment = true;
        Evolver::EvolutionSettings evoSettings(settings);
        auto compressBlock = [&](const Block& block) {
            LOG("Received a block of size", block.size());
            Individual bestIndividual = evolveBestIndividualForBlock(block, evoSettings);
            LOG("For this block, the best individual is", bestIndividual.to_string());
            if (!isFirstSegment) writer.pushBit(true);  //signifies that the segment before had a segment after it
            isFirstSegment = false;
            encodeIndividual(bestIndividual, writer);
            compressBlockUsingRecipe(bestIndividual, block, writer);

        };

        if (settings.segmentationMethod == EvoComSettings::Clustered)
            clusterFileInSegments(reader, compressBlock, originalFileSize, settings);
        else
            processFileAsFixedSegments(reader, compressBlock, originalFileSize, settings);

        writer.pushBit(false);
        writer.writeLastByte();
    }

    void EvolutionaryFileCompressor::compressToStreamsSequentially_DataCollection(AbstractBitReader &reader,
                                                                                  BitCounter &writer,
                                                                                  const size_t originalFileSize,
                                                                                  const EvoComSettings &settings,
                                                                                  Logger &logger) {
        bool isFirstSegment = true;
        Evolver::EvolutionSettings evoSettings(settings);

        size_t compressedSoFar = 0;
        auto compressBlock = [&](const Block& block) {
#if 1
            compressedSoFar += block.size();
            LOG("Progress:", (double) ((double)compressedSoFar*100)/originalFileSize, "%");
#endif
            const Individual bestIndividual = evolveBestIndividualForBlock(block, evoSettings);
            //LOG("Generated the best individual, now encoding...");
            if (!isFirstSegment) writer.pushBit(true);  //signifies that the segment before had a segment after it
            isFirstSegment = false;
            encodeIndividual(bestIndividual, writer);
            compressBlockUsingRecipe_DataCollection(bestIndividual, block, writer, logger);

        };

        logger.beginList("Reports");
        if (settings.segmentationMethod == EvoComSettings::Clustered)
            clusterFileInSegments(reader, compressBlock, originalFileSize, settings);
        else
            processFileAsFixedSegments(reader, compressBlock, originalFileSize, settings);

        logger.endList();
        writer.pushBit(false);
    }

    void EvolutionaryFileCompressor::compressToStreamsAsync(AbstractBitReader& reader, AbstractBitWriter& writer, const size_t originalFileSize, const EvoComSettings& settings) {
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
        //size_t processedSoFar = 0;
        auto compressBlock = [&](const Block& block, const Individual& recipe) {
            //processedSoFar += block.size();
            //size_t progress = 100.0*(double)(processedSoFar) / (double)originalFileSize;
            //LOG_NOSPACES("(Progress ", progress, "%) Received the block (size ", block.size(), "), and the recipe ", recipe.to_string());
            if (!isFirstSegment) writer.pushBit(true);  //signifies that the segment before had a segment after it
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

        writer.pushBit(false);
        writer.writeLastByte();
    }

    void EvolutionaryFileCompressor::processFileAsFixedSegments(AbstractBitReader& reader,
                                                                const std::function<void(const Block &)> &blockHandler,
                                                                const size_t fileSize, const EvoComSettings& settings) {

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

    Block EvolutionaryFileCompressor::readBlock(size_t size, AbstractBitReader &reader) {
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

    void logBlockAndTransform(const Block& block, const TCode operation, Logger& logger) {
        logger.beginUnnamedObject();
        SegmentData report(block);
        report.log(logger);
        logger.addVar("Transform", Individual::TCode_as_string(operation));
        logger.endObject();
    }

    void logBlockAndCompression(const Block& block, const CCode operation, const size_t compressedSize, Logger& logger) {
        logger.beginObject("FinalCompression");
        SegmentData report(block);
        report.log(logger);
        logger.addVar("Compression", Individual::CCode_as_string(operation));
        logger.addVar("FinalSize", compressedSize);
        logger.endObject();
    }


    void EvolutionaryFileCompressor::compressBlockUsingRecipe_DataCollection(const Individual &individual, const Block &block, BitCounter &writer, Logger& logger) {
        ////LOG("Applying individual ", individual.to_string());
        const size_t writtenBefore = writer.getAmountOfBits();
        logger.beginUnnamedObject();
        logger.beginObject("StartingState");
        const SegmentData initialReport(block);
        initialReport.log(logger);
        logger.endObject();//ends StartingState



        logger.beginList("IntermediateStates");

        Block toBeProcessed = block;
        for (auto tCode : individual.tList) {
            logBlockAndTransform(block, tCode, logger);
            applyTransformCode(tCode, toBeProcessed);
        }
        logger.endList(); //ends IntermediateStates
        applyCompressionCode(individual.cCode, toBeProcessed, writer);

        const size_t writtenAfter = writer.getAmountOfBits();
        const size_t sizeOfCompressed = writtenAfter-writtenBefore;
        logBlockAndCompression(toBeProcessed, individual.cCode, sizeOfCompressed, logger);
        logger.endObject();
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
            const Individual individual = decodeIndividual(reader);
            const Block decodedBlock = decodeUsingIndividual(individual, reader);
            writeBlock(decodedBlock, writer);
        };

        //LOG("starting the decoding of blocks");

        auto thereAreMoreSegments = [&]() -> bool {
            bool morePresent = reader.readBit();
            return morePresent;
        };
        do {
            decodeAndWriteOnFile();
        } while (thereAreMoreSegments()); //if there is a bit following a segment, there is another segment to be decoded

        writer.writeLastByte();

    }

    Individual EvolutionaryFileCompressor::decodeIndividual(AbstractBitReader& reader) {
        std::vector<TCode> tList;
        auto addTCode = [&](){
            tList.push_back(static_cast<TCode>(reader.readAmountOfBits(bitSizeForTransformCode)));
        };

        auto extractCCode = [&]() -> CCode{
            return static_cast<CCode>(reader.readAmountOfBits(bitSizeForCompressionCode));
        };

        size_t amountOfTransforms = reader.readAmountOfBits(bitsForAmountOfTransforms);
        repeat(amountOfTransforms, addTCode);
        return {tList, extractCCode()};
    }

    Block EvolutionaryFileCompressor::decodeUsingIndividual(const Individual& individual, AbstractBitReader& reader) {
        Block transformedBlock = undoCompressionCode(individual.cCode, reader);
        std::for_each(individual.tList.rbegin(), individual.tList.rend(), [&](auto tc){
            undoTransformCode(tc, transformedBlock);});
        return transformedBlock;
    }

    void EvolutionaryFileCompressor::writeBlock(const Block &block, AbstractBitWriter &writer) {
        auto writeUnit = [&](const Unit unit) {
            writer.writeAmountOfBits(unit, bitsInType<Unit>());
        };
        std::for_each(block.begin(), block.end(), writeUnit);
    }


    EvolutionaryFileCompressor::Fitness EvolutionaryFileCompressor::compressionRatioForIndividualOnBlock(const Individual& individual, const Block& block) {
        size_t originalSize = block.size()*8;

        BitCounter counterWriter;
        encodeIndividual(individual, counterWriter);
        compressBlockUsingRecipe(individual, block, counterWriter);
        size_t compressedSize = counterWriter.getAmountOfBits();
        //a compressed block is a sequence of bits, not necessarly in multiples of 8
                ASSERT_NOT_EQUALS(compressedSize, 0); //would be impossible
        ASSERT_NOT_EQUALS(originalSize, 0);   //would cause errors
        return (double) (compressedSize) / (originalSize);
    }

    Individual EvolutionaryFileCompressor::evolveBestIndividualForBlock(const Block & block, const Evolver::EvolutionSettings& evoSettings) {
        //uses a sample of the actual block
        constexpr size_t sampleSize = 1024; //1 KB
        const size_t blockSampleLength = std::min(sampleSize, block.size());
        const Block blockSample(block.begin(), block.begin()+blockSampleLength);
        auto getFitnessOfIndividual = [&](const Individual& i){
            return compressionRatioForIndividualOnBlock(i, blockSample);
        };

        Evolver evolver(evoSettings, getFitnessOfIndividual);
        Individual bestIndividual = evolver.evolveBest();
        return bestIndividual;
    }

    Individual EvolutionaryFileCompressor::evolveIndividualForBlockAndLogProgress(const Block& block, const Evolver::EvolutionSettings& evoSettings, Logger& logger)  { //based on evolveBestIndividual
        auto getFitnessOfIndividual = [&](const Individual& i) -> Fitness {
            return compressionRatioForIndividualOnBlock(i, block);
        };

        Evolver evolver(evoSettings, getFitnessOfIndividual);
        return evolver.evolveBestAndLogProgress(logger);
    }



    void EvolutionaryFileCompressor::clusterFileInSegments(AbstractBitReader &reader,
                                                           std::function<void(const Block&)> blockHandler,
                                                           const size_t fileSize, const EvoComSettings& settings) {

        const size_t microUnitSize = 1024; //bytes
        size_t remaining = fileSize;

        auto joinBlocks = [&](const std::vector<Block>& cluster) -> Block {
            Block result;
            for (const Block& block : cluster)
                result.insert(result.end(), block.begin(), block.end());

            return result;
        };

        StreamingClusterer<Block, double> clusterer(BlockReport::distributionDistance,
                                [&](const std::vector<Block>& cluster){
                                    blockHandler(joinBlocks(cluster));},
                        settings.clusteredSegmentThreshold,
                        settings.clusteredSegmentCooldown,
                        0); //temporary value before I actually implement this properly


        while (remaining > microUnitSize * 2) {  //this is so that the last micro unit has always size at least microUnitSize
            clusterer.pushItem(readBlock(microUnitSize, reader));
            remaining -= microUnitSize;
        }
        clusterer.pushItem(readBlock(remaining, reader));
        clusterer.finish();
    }

    void EvolutionaryFileCompressor::generateEvolverConvergenceData(const EvoComSettings &settings, Logger &logger) {
        size_t originalFileSize = getFileSize(settings.inputFile);

        if (originalFileSize <= 2) {LOG("ERROR: file too small!"); return;}

        std::ifstream inStream(settings.inputFile);
        FileBitReader reader(inStream);

        if (!inStream) {LOG("ERROR: file could not be opened"); return;}

        getEvolverConvergenceData(reader, originalFileSize, settings, logger);
    }



    void
    EvolutionaryFileCompressor::getEvolverConvergenceData(FileBitReader &reader, const size_t originalFileSize,
                                                          const EvoComSettings &settings, Logger& logger) {
        settings.log(logger);
        logger.beginObject("evolverData");

        size_t segmentCounter = 0;
        Evolver::EvolutionSettings evoSettings(settings);
        auto compressBlock = [&](const Block& block) {
            logger.beginList("Segment#"+std::to_string(segmentCounter));
            segmentCounter++;
            evolveIndividualForBlockAndLogProgress(block, evoSettings, logger);  //doesn't use the return value
            logger.endList();
        };

        if (settings.segmentationMethod == EvoComSettings::Clustered)
            clusterFileInSegments(reader, compressBlock, originalFileSize, settings);
        else
            processFileAsFixedSegments(reader, compressBlock, originalFileSize, settings);

        logger.endList();
    }


} // GC