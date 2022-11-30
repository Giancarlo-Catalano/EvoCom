#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "BlockReport/BlockReport.hpp"


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
    FileName fileToCompress = settings.inputFile;
    FileName compressedFile = fileToCompress+"."+compressedExtension;


    GC::EvolutionaryFileCompressor::compress(settings);
    LOG("The compressedFile has size ", getFileSize(compressedFile));

    LOG("Decompressing!----------------------------------------------");
    GC::EvolutionaryFileCompressor::decompress(compressedFile, fileToCompress);

    LOG("In short: ", getFileSize(sampleFileDirectory+fileToCompress), "bytes -> ", getFileSize(compressedFile), "bytes");
#endif

}
