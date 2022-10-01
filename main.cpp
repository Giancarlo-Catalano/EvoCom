#include <iostream>
#include "Utilities/utilities.hpp"
#include "Individual/Individual.hpp"
#include "Random/RandomInt.hpp"
#include "Random/RandomElement.hpp"
#include "Random/RandomChance.hpp"
#include "Breeder/Breeder.hpp"


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



#if 0 //compression
    std::string fileToBeCompressed = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/fencingLogoSimple.png";
    std::string compressedFile = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/compressed.gac";
    std::string decompressedFile = "/home/gian/CLionProjects/Diss_SimplePrototype/SampleFiles/decompressedFencingLogoSimple.png";

    GC::SimpleCompressor::compress(fileToBeCompressed, compressedFile);
    LOG("The compressedFile has size ", getFileSize(compressedFile));

    GC::SimpleCompressor::decompress(compressedFile, decompressedFile);

    LOG("In short: ", getFileSize(fileToBeCompressed), "bytes -> ", getFileSize(compressedFile), "bytes");
#endif

#if 0 // individuals
    GC::Individual ind;
    LOG(ind.to_string());
    ind.integerInterpretationCriteria = GC::Individual::TwoBytes;
    ind.blockDelimitingCriteria = GC::Individual::SimilarityBasedBlocks(0.5);
    ind.setTItem(GC::Individual::T_RunLengthTransform, 2);
    ind.cCode = GC::Individual::C_HuffmanCompression;

    LOG(ind.to_string());
#endif

#if 0 //randomness tests
    GC::RandomInt intGen(0, 6);
    auto showRandom = [&]() {
        LOG("Generated ", intGen());
    };

    repeat(30, showRandom);


    enum Colour {
        Purple,Red,Orange,Yellow,Green,Blue
    };
    GC::RandomElement<Colour> randColour(std::vector<Colour>{Purple, Red, Orange,Yellow,Green,Blue});

    auto colour_to_string = [&](const Colour c) -> std::string {
        std::vector<std::string> asStrings = {"Purple", "Red", "Orange", "Yellow", "Green", "Blue"};
        return asStrings[static_cast<size_t>(c)];
    };

    auto showRandColour = [&]() {
        LOG("Random colour:", colour_to_string(randColour()));
    };

    repeat(20, showRandColour);



    GC::RandomChance randChance(0.9);
    for (size_t i=0;i<30;i++) {
        auto maybeShowI = [&] {
            randChance.doWithChance([&](){LOG("i =", i, "succeeded!");});
        };
        maybeShowI();
    }
#endif


#if 0 //breeder tests
    GC::Individual A;
    GC::Individual B;

    GC::Breeder breeder(0.5, 0.5);
    LOG("initially A =", A.to_string(), "B =", B.to_string());
    auto fuckemup= [&](){
        LOG("A =", A.to_string(), "B =", B.to_string());
        A = breeder.mutate(A);
        B = breeder.mutate(breeder.crossover(A, B));
    };

    repeat(10, fuckemup);
#endif

    return 0;
}
