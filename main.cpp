#include <iostream>
#include "Utilities/utilities.hpp"
#include "EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "Evolver/Individual/TCodes.hpp"
#include "Evolver/Individual/CCodes.hpp"
#include "Evolver/Breeder/Breeder.hpp"
#include "Utilities/StreamingClusterer/StreamingClusterer.hpp"


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



#if 1 //compression
    using FileAndExtension = std::pair<std::string, std::string>;
    FileAndExtension smallLogo = {"fencingLogoSimple", "png"};
    FileAndExtension bigLogo = {"finishedboot", "png"};
    FileAndExtension beeMovie = {"beeMovieScript", "txt"};
    FileAndExtension cat = {"smoky", "jpeg"};
    FileAndExtension mosaic = {"mosaic", "bmp"};
    FileAndExtension bird = {"bird", "raw"};
    FileAndExtension tiff = {"tiff", "tiff"};
    FileAndExtension peanut = {"peanut", "tiff"};

    std::string compressedExtension = "gac";
    std::string directory = "/home/gian/CLionProjects/EvoCom/SampleFiles/";


    FileAndExtension fileToCompress = peanut;

    std::string fileToBeCompressed = directory+fileToCompress.first+"."+fileToCompress.second;
    std::string compressedFile = directory+fileToCompress.first+"."+compressedExtension;
    std::string decompressedFile = directory+"DECOMPRESSED_"+fileToCompress.first+"."+fileToCompress.second;

    GC::EvolutionaryFileCompressor::compress_variableSize(fileToBeCompressed, compressedFile);
    LOG("The compressedFile has size ", getFileSize(compressedFile));

    LOG("Decompressing!----------------------------------------------");
    GC::EvolutionaryFileCompressor::decompress(compressedFile, decompressedFile);

    LOG("In short: ", getFileSize(fileToBeCompressed), "bytes -> ", getFileSize(compressedFile), "bytes");
#endif

#if 0 // individuals
    GC::Individual a;
    GC::Individual b;
    b.setTItem(GC::T_DeltaTransform, 0);
    b.setTItem(GC::T_RunLengthTransform, 1);

    GC::Individual c;
    c.setTItem(b.readTCode(0), 0);

    a.getPseudoFitness().setReliability(1);
    a.getPseudoFitness().setFitnessScore(0.3);

    b.getPseudoFitness().setReliability(1);
    b.getPseudoFitness().setFitnessScore(0.6);

    LOG("a=", a.to_string());
    LOG("b=", b.to_string());
    LOG("c=", c.to_string());


    GC::Evaluator evaluator([](const GC::Individual& i) {return 666.0;});
    evaluator.decideFitness(c, a, b);
    LOG("After evaluation, c=", c.to_string());

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


#if 0 //breeder & selector tests
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

#if 0 //evolver tests
    GC::Individual GodsImage;
    auto pseudoFitnessFunction = [&](const GC::Individual& ind) {
        return ind.distanceFrom(GodsImage);
    };
    GC::Evolver::EvolutionSettings evolutionSettings;
    evolutionSettings.populationSize = 36;
    evolutionSettings.generationCount = 12;
    GC::Evolver evolver(evolutionSettings, pseudoFitnessFunction);

    auto showBestIndividual = [&]() {
        GC::Individual best = evolver.evolveBest();
        evolver.reset();
        LOG("the best for this attempt is", best.to_string());
    };

    GodsImage.setFitness(pseudoFitnessFunction(GodsImage));
    LOG("God's image is currently", GodsImage.to_string());
    repeat(12, showBestIndividual);
#endif

#if 0 //Transform test
    Block block;
    for (int i=0;i<512;i++) {
        block.push_back(i*i%8);
    }
    LOG("The block is", containerToString(block));


    Block nTransformed = GC::NStackTransform().apply_copy(block);
    Block transformed = GC::StackTransform().apply_copy(block);

    LOG("The new transform is ", containerToString(nTransformed));
    LOG("The old transform is ", containerToString(transformed));

    Block undone = GC::NStackTransform().undo_copy(block);
    LOG("old block is ", containerToString(block));
    LOG("The block is ", containerToString(block));
#endif


#if 0 //Crossover test
    GC::Breeder breeder(0, 0.5);

    GC::Individual A{{GC::T_StackTransform, GC::T_RunLengthTransform, GC::T_StrideTransform_4}, GC::C_SmallValueCompression};
    GC::Individual B{{}, GC::C_IdentityCompression};

    LOG("A =", A.to_string());
    LOG("B =", B.to_string());



    repeat(12, [&](){LOG("child =", breeder.crossover(B, B).to_string());});
#endif


#if 0 //levenshtine distance tests
    LOG("Testing distances");
    enum Colour {
        Purple,
        Red,
        Orange,
        Yellow,
        Green,
        Blue
    };

    std::vector<Colour> A = {Red, Red, Red, Orange, Orange, Orange};
    std::vector<Colour> B = {Purple, Red, Orange, Green, Purple, Orange};

    size_t distance = LevenshteinDistance<Colour, 6>(A, B);
    LOG("the distance is ", distance);
#endif

#if 0 //streaming clusterer
    LOG("Testing the streaming clusterer");
    using T = size_t;
    using Field = size_t;
    using Cluster = std::vector<T>;

    auto metric = [](const T& a, const T& b) -> Field {
        return (a < b ? (b-a) : (a-b));
    };

    auto log_newCluster = [&](const Cluster& cluter) -> void {
        LOG("The new cluster is ", containerToString(cluter));
    };

    LOG("constructing the clusterer");
    GC::StreamingClusterer<T, Field> clusterer(metric, log_newCluster, 10, 2);


    std::vector<T> items = {0, 2, 1, 2, 1, 3, 20, 40, 30, 40, 20, 40, 41, 42, 43, 44, 45, 45, 56, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    LOG("adding the main items");
    std::for_each(items.begin(), items.end(), [&](const T& item){clusterer.pushItem(item);});

    LOG("finishing");
    clusterer.finish();
#endif
    return 0;
}
