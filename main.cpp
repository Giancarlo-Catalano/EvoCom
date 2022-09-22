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

#if 0 //checking BlockReport
    Block block;
    for (int i=0;i<(1ULL<<12);i++)
        block.push_back((i*i)%256);

    LOG("The block is ", containerToString(block, 10));
    GC::BlockReport br(block);
    LOG(br.to_string());
#endif


#if 0 //checking FileBitWriter
    std::string fileName = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/loremIpsum.txt";
    std::ofstream outStream(fileName);
    GC::FileBitWriter bw(outStream);


    for (size_t i=0;i<2;i++) {
        //* in ascii is 42 = 00101010
        const std::array<bool, 8> bits = {0, 0, 1, 1, 1, 0, 1, 0};
        for (auto bit: bits) bw.pushBit(bit);
    }

    bw.forceLast();

    //bw.forceLast();
    outStream.close();
    dumpFile(fileName);
#endif


#if 0 //checking Huffman Compression
    std::string fileName = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/loremIpsum.txt";
    Block block;
    for (size_t i=0;i<(1<<12);i++)
        block.push_back(i/((i>>4)+1));

    LOG("The block to be encoded is ", containerToString(block));
    LOG("it has the following properties:", GC::BlockReport(block).to_string());
    GC::RunLengthCompression hc;
    LOG("The compression to be applied is ", hc.to_string());

    std::ofstream outStream(fileName);
    GC::FileBitWriter writer(outStream);
    hc.compress(block, writer);
    writer.forceLast();
    outStream.close();
    dumpFile(fileName);



    LOG("and then we decompress");
    std::ifstream inStream(fileName);
    GC::FileBitReader reader(inStream);
    Block revertedBlock = hc.decompress(reader);


    LOG("The reverted block is", containerToString(revertedBlock));
    LOG("The original was   ..", containerToString(block));
#endif


#if 1 //checking compression
    std::string fileToBeCompressed = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/fencingLogoSimple.png";
    std::string compressedFile = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/compressed.gac";
    std::string decompressedFile = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/uncompressed.png";

    //LOG("The compressor has the following codes:", GC::SimpleCompressor().to_string());

    GC::SimpleCompressor::compress(fileToBeCompressed, compressedFile);
    LOG("The compressedFile has size ", getFileSize(compressedFile));

    GC::SimpleCompressor::decompress(compressedFile, decompressedFile);

    LOG("In short: ", getFileSize(fileToBeCompressed), "bytes -> ", getFileSize(compressedFile), "bytes");



#endif

    return 0;
}
