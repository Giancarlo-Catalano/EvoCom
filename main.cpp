#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "Dependencies/nlohmann/json.hpp"

#include <fstream>


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

#if 1 //testing JSON
    std::ifstream readingStream("/home/gian/CLionProjects/EvoCom/SampleFiles/output.json");
    json data = json::parse(readingStream);
    std::string mode = data.at("Settings").at("mode");
    LOG("The mode is ", mode);

#endif

}
