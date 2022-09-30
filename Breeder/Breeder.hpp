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


namespace GC {

    class Breeder {
    public: //types
        using TCode = Individual::TCode;
        using TList = Individual::TList;
        using CCode = Individual::CCode;

        const std::vector<TCode> availableTCodes = {Individual::T_DeltaTransform,
                                                           Individual::T_DeltaXORTransform,
                                                           Individual::T_RunLengthTransform,
                                                           Individual::T_SplitTransform,
                                                           Individual::T_StackTransform,
                                                           Individual::T_StrideTransform_2,
                                                           Individual::T_StrideTransform_3,
                                                           Individual::T_StrideTransform_4,
                                                           Individual::T_SubtractAverageTransform,
                                                           Individual::T_SubtractXORAverageTransform};

        const std::vector<CCode> availableCCodes = {Individual::C_HuffmanCompression,
                                                    Individual::C_IdentityCompression,
                                                    Individual::C_RunLengthCompression};

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
            Individual child = individual;
            for (TCode& tCode : child.getTList())
                randomChanceOfMutation.doWithChance([&]{randomTCode.assignRandomValue(tCode);});

            randomCCode.assignRandomValue(child.getCCode());
            return child;
        }

        Individual crossover(const Individual& A, const Individual& B) {
            bool AGoesFirst = crossoverFirstChooser.flip();

            Individual child = (AGoesFirst ? A : B);
            const Individual& alternative = (AGoesFirst ? B : A);
            size_t startOfCrossover = startOfCrossoverChooser();
            for (size_t index = startOfCrossover;index<Individual::TListLength;index++)
                    child.copyTCodeFrom(index, alternative);

            chooseCCodeCrossover.doWithChance([&]{child.copyCCodeFrom(alternative);});

            return child;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BREEDER_HPP
