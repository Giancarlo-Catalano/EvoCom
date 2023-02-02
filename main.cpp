#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "Dependencies/nlohmann/json.hpp"

#include <fstream>


template <class T, class Generator> //generator is a T generate();
std::vector<T> generateUnique(const size_t howMany, const Generator& generator) {
    std::unordered_set<T> result;
    while (result.size() < howMany)
        result.insert(generator());

    std::vector<T> resultAsVector(result.begin(), result.end());
    return resultAsVector;
}

template <class T, class Generator> //generator is a T generate();
static std::vector<T> generateUnique(const size_t howMany, const std::vector<T>& startingPoint, const Generator& generator) {
    std::unordered_set<T> result(startingPoint.begin(), startingPoint.end());
    while (result.size() < howMany)
        result.insert(generator());

    std::vector<T> resultAsVector(result.begin(), result.end());
    return resultAsVector;
}


int main(int argc, char**argv) {


#if 1 //normal application behaviour
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
    else if (settings.mode == GC::EvoComSettings::EvolverConvergenceDataCollection) {
        GC::Logger logger;
        GC::EvolutionaryFileCompressor::generateEvolverConvergenceData(settings, logger);
        LOG(logger.end());
    }

#endif

#if 1 //testing makeUniqueSet
    GC::RandomInt<size_t> randomInt(0, 200);
    auto generateRandomInt = [&]() -> size_t {
        return randomInt.choose();
    };

    std::vector<size_t> initialSet = {0, 1, 2, 3};

    const std::vector<size_t> items = generateUnique<size_t>(12, initialSet, generateRandomInt);

    LOG(containerToString(items));


#endif
}
