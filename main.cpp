#include <iostream>
#include "Utilities/utilities.hpp"
#include "SimpleCompressor/SimpleCompressor.hpp"


int main() {


    /**
     * TODO:: write a decision tree
     * * Make a Node class, which is templated to the class it observes and the variable it checks
     * * It has a function called obtainValue, which takes the T and produces the desidered value
     *          * this is provided from the constructor
*      * It has a threshold of type V used to check against the obverved value
       * It has a method that returns true if observed < threshold
       *
     */

    std::string fileToBeCompressed = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/fencingLogoSimple.png";
    std::string compressedFile = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/compressed.gac";
    std::string decompressedFile = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/decompressedFencingLogoSimple.png";

    GC::SimpleCompressor::compress(fileToBeCompressed, compressedFile);
    LOG("The compressedFile has size ", getFileSize(compressedFile));

    GC::SimpleCompressor::decompress(compressedFile, decompressedFile);

    LOG("In short: ", getFileSize(fileToBeCompressed), "bytes -> ", getFileSize(compressedFile), "bytes");

    return 0;
}
