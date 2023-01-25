#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "BlockReport/BlockReport.hpp"
#include "Transformation/Transformations/BurrowsWheelerTransform.hpp"
#include "Utilities/Logger/Logger.hpp"
#include "Transformation/Transformations/SubMinAdaptiveTransform.hpp"
#include "Compression/ANSCompression/ANSCompression.hpp"

#include <future>
#include <optional>
#include <queue>
#include <numeric>
#include <iterator>


int main(int argc, char**argv) {


#if 0 //normal application behaviour
    const std::string compressedExtension = "gac";
    using FileName = std::string;
    GC::EvoComSettings settings(argc, argv);


    if (settings.mode == GC::EvoComSettings::Compress) {
        FileName fileToCompress = settings.inputFile;
        FileName compressedFile = fileToCompress + "." + compressedExtension;


        size_t durationInSeconds = timeFunction([&]() {
            GC::EvolutionaryFileCompressor::compress(settings); })/1000;
        LOG("The compression took", durationInSeconds, "seconds");
        const size_t originalSize = getFileSize(fileToCompress);
        const size_t compressedSize = getFileSize(compressedFile);
        const double ratio = (double)(compressedSize)/ (double)(originalSize);
        LOG("size reduction:", originalSize, "->", compressedSize, ", that is a", (int)(100*(1-ratio)), "% reduction!");
    }
    else if (settings.mode == GC::EvoComSettings::Decompress) {
        LOG("Decompressing!----------------------------------------------");
        std::string compressedFile = settings.inputFile;
        std::string decompressedFile = compressedFile.substr(0, compressedFile.size()-1-compressedExtension.size());
        GC::EvolutionaryFileCompressor::decompress(compressedFile, decompressedFile);
    }
    else if (settings.mode == GC::EvoComSettings::CompressionDataCollection) {
        GC::Logger logger;
        GC::EvolutionaryFileCompressor::generateCompressionData(settings, logger);
        LOG(logger.end());
    }

#endif



#if 0 //futures
    using Individual = GC::Individual;
    using Job = std::pair<Block, std::future<Individual>>;
    using JobQueue = std::queue<Job>;

    LOG("Creating the job queue");
    JobQueue jobQueue;

    auto getRecipeForBlock = [&](const Block& block, const GC::Evolver::EvolutionSettings) -> Individual {
        return Individual();
    };

    GC::Evolver::EvolutionSettings settings;

    auto passBlockToJobQueue = [&](const Block& block) {
        LOG("Received the block", containerToString(block), ", passing it to the queue");
        jobQueue.emplace(block, std::async(
                std::launch::async,
                getRecipeForBlock,
                block,
                settings));
    };

    bool isFirst = true;
    auto compressBlockUsingRecipe = [&](const Block& block, const Individual& recipe) {
        LOG("Received the block", containerToString(block), "and the recipe", recipe.to_string());
        LOG("isFirst =", isFirst);
        if (!isFirst) {
            LOG("I would have written a 1 to indicate that another one is coming!");
        }
        isFirst = false;
        LOG("Here pretend i'm writing to a file");
    };

    Block a = {1, 1, 1};
    Block b = {2, 2, 2};
    Block c = {3, 3, 3};

    passBlockToJobQueue(a);
    passBlockToJobQueue(b);
    passBlockToJobQueue(c);


    LOG("started to process the queue!");

    while (!jobQueue.empty()) {
        LOG("waiting for the future");
        jobQueue.front().second.wait();

        LOG("acquiring the block and the future");
        Individual recipe = jobQueue.front().second.get();
        Block block = jobQueue.front().first;
        compressBlockUsingRecipe(block, recipe);
        LOG("processed the block ", containerToString(block));

        jobQueue.pop(); //very important!!
    }

    LOG("all done!");

#endif

#if 0 //BWT
    Block block = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Block transformed = GC::BurrowsWheelerTransform().apply_copy(block);
    LOG("The transformed block is", containerToString(transformed));
    Block undone = GC::BurrowsWheelerTransform().undo_copy(transformed);
    LOG("The undone block is", containerToString(undone));
#endif

#if 0 //Logger
    GC::Logger logger;
    logger.addVar("Head", "hello");
    logger.beginList("List");
    logger.addListItem(1);
    logger.addListItem(6);
    logger.endList();
    logger.beginObject("Object");
    logger.addVar("Properties", "Lacking");
    logger.addVar("Other", 4);
    logger.endObject();


    LOG(logger.end());
#endif

#if 0 //trying to read from a file
    std::ifstream inputFile("../SampleFiles/lines.txt");
    if (!inputFile)  {
        LOG("There was an error opening the file");
        return 1;
    }

    std::string temp;
    std::vector<std::string> lines;
    while (getline(inputFile, temp)) {
        lines.push_back(temp);
    }

    LOG("There were", lines.size(), "lines in total:");
    for (const auto item: lines) {
        LOG("Item: ", item);
    }
#endif

#if 0 //testing MaxMin Transform
    auto logBlock = [&](const Block& block) {
        std::for_each(block.begin(), block.end(), [&](const Unit unit){ LOG_NONEWLINE((unsigned int)unit, ",");});
        LOG("");
    };

    Block testBlock = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
    Block result = GC::SubMinAdaptiveTransform().apply_copy(testBlock);
    Block undone = GC::SubMinAdaptiveTransform().undo_copy(result);
    LOG("the original is \n");
    logBlock(testBlock);
    LOG("the transformed is \n");
    logBlock(result);
    LOG("The undone is \n");
    logBlock(undone);
#endif

#if 0 //testing the entropy
    auto getEntropy = [&](const std::vector<double> &frequencies) -> double {
        auto addToAcc = [](const double acc, const double input) -> double {
            return acc-(input!=0? std::log2(input)*input : 0);
        };
        return std::accumulate(frequencies.begin(), frequencies.end(), (double)0.0, addToAcc);
    };

    std::vector<double> elems = {0, 0.5, 0.5};
    double entropy = getEntropy(elems);
    LOG("the entropy is ", entropy);
#endif

#if 1 //testing the ANS encoder
    auto logBlock = [&](const Block& block) {
        std::for_each(block.begin(), block.end(), [&](const Unit unit){ LOG_NONEWLINE((unsigned int)unit, ",");});
        LOG("");
    };

    auto logBoolVec = [&](const std::vector<bool>& boolvec) {
        std::for_each(boolvec.begin(), boolvec.end(), [&](const bool b) { LOG_NONEWLINE(b, " ");});
        LOG("");
    };

    Block testBlock = {0, 0, 2, 2, 2, 2, 2,2, 3, 2, 3, 3, 3, 3,3, 5, 3, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 3, 5, 2, 5, 3, 5, 3, 5, 6, 3, 5, 2, 2, 2, 2, 2, 2, 4, 1, 2, 2, 2, 1, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

    GC::VectorBitWriter writer;
    GC::ANSCompression().compress(testBlock, writer);
    std::vector<bool> result = writer.getVectorOfBits();

    LOG("the compressed bool vector is ");
    logBoolVec(result);
    LOG("it has length", result.size());

#endif

}
