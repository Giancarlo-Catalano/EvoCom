#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "BlockReport/BlockReport.hpp"

#include <future>
#include <queue>


int main(int argc, char**argv) {

    /**
     * TODO:: write a decision tree
     * * Make a Node class, which is templated to the class it observes and the variable it checks
     * * It has a function called obtainValue, which takes the T and produces the desidered value
     *          * this is provided from the constructor
*      * It has a threshold of type V used to check against the obverved value
       * It has a method that returns true if observed < threshold
       *
     */



#if 1 //compression
    std::string compressedExtension = "gac";
    std::string sampleFileDirectory = "../SampleFiles/";

    using FileName = std::string;


    GC::EvoComSettings settings(argc, argv);


    if (settings.mode == GC::EvoComSettings::Compress) {
        FileName fileToCompress = settings.inputFile;
        FileName compressedFile = fileToCompress + "." + compressedExtension;


        size_t durationInSeconds = timeFunction([&]() { GC::EvolutionaryFileCompressor::compress(settings); });
        LOG("The compression took", durationInSeconds, "seconds");
        const size_t originalSize = getFileSize(fileToCompress);
        const size_t compressedSize = getFileSize(compressedFile);
        const double ratio = (double)(compressedSize)/ (double)(originalSize);
        LOG("size reduction:", originalSize, "->", compressedSize, ", that is a", (int)(100*(1-ratio)), "% reduction!");
    }
    else {
        LOG("Decompressing!----------------------------------------------");
        std::string compressedFile = settings.inputFile;
        std::string decompressedFile = compressedFile.substr(0, compressedFile.size()-1-compressedExtension.size());
        GC::EvolutionaryFileCompressor::decompress(compressedFile, decompressedFile);
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
}
