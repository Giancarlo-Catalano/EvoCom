#include <iostream>
#include "Utilities/utilities.hpp"
#include "Individual/Individual.hpp"
#include "Random/RandomInt.hpp"
#include "Random/RandomElement.hpp"
#include "Random/RandomChance.hpp"
#include "Breeder/Breeder.hpp"
#include "Selector/Selector.hpp"
#include "Evolver/Evolver.hpp"


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


#if 1 //breeder tests

    GC::RandomIndividual randomIndividualMaker;
    GC::Individual Adam = randomIndividualMaker.makeIndividual();
    GC::Individual Eve = randomIndividualMaker.makeIndividual();

    GC::Breeder breeder(0.3, 0.5);
    LOG("initially Adam =", Adam.to_string(), "Eve =", Eve.to_string());
    LOG("the breeder is", breeder.to_string());

    std::vector<GC::Individual> population {Adam, Eve};
    auto makeNewIndividual = [&](){
        GC::RandomElement<GC::Individual> parentChooser(population);
        population.push_back(breeder.mutate(breeder.crossover(parentChooser(), parentChooser())));
    };

    size_t incestAmount = 300;
    repeat(incestAmount, makeNewIndividual);

    LOG("At the end of this morally ambiguous process, the population is ");
    for (auto ind : population) {
        LOG(ind.to_string());
    }

    GC::Individual GodsImage;
    //TODO make the fitness function that measures the distance from God's image
    auto pseudoFitnessFunction = [&](const GC::Individual& ind) {
        return 1.0-ind.similarityWith(GodsImage);
    };

    using Sel = GC::Selector;

    auto generousTournamentSelection = Sel::TournamentSelection(0.5);
    GC::Selector selector(Sel::SelectionKind(generousTournamentSelection), pseudoFitnessFunction);
    selector.preparePool(population);
    LOG("The population has been preapared:");
    selector.LOGPool();

    LOG("then we select a few individuals");
    auto selectAndShow = [&]() {
        auto ind = selector.select();
        LOG("selected ", ind.to_string());
    };

    LOG("The target individual is", GodsImage.to_string());
    repeat(10, selectAndShow);


#endif

    return 0;
}
