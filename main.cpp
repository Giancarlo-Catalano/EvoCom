#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "Dependencies/nlohmann/json.hpp"

#include <fstream>

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


#if 0 //testing the distribution distance
    Block A;
    Block B;

    for (int i=0;i<3000;i++) {
        A.push_back();
        B.push_back(1);
    }


    double distance = GC::BlockReport::distributionDistance(A, B);
    LOG("The distance is", distance);


#endif
}
