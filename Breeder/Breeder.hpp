//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#define DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#include "../Utilities/utilities.hpp"
#include "../names.hpp"
#include "../Individual/Individual.hpp"
#include "../Random/RandomChance.hpp"
#include "../Random/RandomElement.hpp"
#include <sstream>


namespace GC {

    class Breeder {
    public: //types
        using TList = Individual::TList;

    private:
        //for mutation
        RandomChance randomChanceOfMutation;
        RandomElement<TCode> randomTCode;
        RandomElement<CCode> randomCCode;

        //for crossover
        FairCoin crossoverFirstChooser;
        RandomInt<size_t> startOfCrossoverChooser;
        RandomChance chooseCCodeCrossover;

    public:


        Breeder(const Chance mutationChance, const Chance chanceOfCCodeCrossover) :
            randomChanceOfMutation(mutationChance),
            randomTCode(availableTCodes),
            randomCCode(availableCCodes),
            crossoverFirstChooser(),
            startOfCrossoverChooser(0, Individual::TListLength),
            chooseCCodeCrossover(chanceOfCCodeCrossover){};


        Individual mutate(const Individual& individual) {
            //LOG("Mutating an individual");
            Individual child = individual;
            for (TCode& tCode : child.getTList())
                randomChanceOfMutation.doWithChance([&]{randomTCode.assignRandomValue(tCode);});

            randomCCode.assignRandomValue(child.getCCode());
            return child;
        }

        Individual crossover(const Individual& A, const Individual& B) {
            //LOG("Crossing individuals");
            bool AGoesFirst = crossoverFirstChooser.flip();

            Individual child = (AGoesFirst ? A : B);
            const Individual& alternative = (AGoesFirst ? B : A);
            size_t startOfCrossover = startOfCrossoverChooser();
            for (size_t index = startOfCrossover;index<Individual::TListLength;index++)
                    child.copyTCodeFrom(index, alternative);

            chooseCCodeCrossover.doWithChance([&]{child.copyCCodeFrom(alternative);});
            return child;
        }


        std::string to_string() {
            std::stringstream ss;
            ss<<"Breeder {";
            ss<<"MutationChance:"<<std::setprecision(2)<<randomChanceOfMutation.getChance();
            ss<<"}";
            return ss.str();
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BREEDER_HPP
