//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
#define DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
#include "Individual/Individual.hpp"
#include "Breeder/Breeder.hpp"
#include "Selector/Selector.hpp"
#include "../Utilities/utilities.hpp"
#include "../names.hpp"
#include "../StatisticalFeatures/RunningAverage.hpp"

#define SHOW_ADAPTIVE_MUTATION 1

#if SHOW_ADAPTIVE_MUTATION == 0
#define AM_LOG(...) LOG(__VA_ARGS__)
#else
#define AM_LOG(...)
#endif

namespace GC {

    class Evolver {
    public: //types
        using Population = std::vector<Individual>;
        struct EvolutionSettings { //this is a convenient way of passing a lot of parameters to the evolver
            size_t populationSize;
            size_t generationCount;
            Chance chanceOfMutation;
            Chance chanceOfCompressionCrossover;
            Proportion tournamentSelectionProportion;
            bool usesSimulatedAnnealing;
            bool isElitist;

            EvolutionSettings() :
                populationSize(40),
                generationCount(100),
                chanceOfMutation(0.05),
                chanceOfCompressionCrossover(0.25),
                tournamentSelectionProportion(0.80),
                usesSimulatedAnnealing(true),
                isElitist(true){}
        };

        using Fitness = Individual::FitnessScore;
        using FitnessFunction = Evaluator::FitnessFunction;

    private:
        Breeder breeder;
        Selector selector;
        Evaluator evaluator;
        Population population;
        RunningAverage<Fitness> runningAverageFitness{};
        size_t generationCount = 0;
        const Chance initialMutationRate; static const size_t mutationRateLevels = 12;

        size_t populationSize;
        size_t amountOfGenerations;
        bool usesSimulatedAnnealing;
        bool isElitist;


    private: //methods
        void initialiseRandomPopulation() {
            RandomIndividual randomIndividualMaker;
            population = std::vector<Individual>();
            auto addRandomIndividual = [&]() {
                population.push_back(randomIndividualMaker.makeIndividual());
            };

            repeat(populationSize, addRandomIndividual);
            forcePopulationFitnessAssessment();
        }

        void initialiseHintedPopulation(const std::vector<Individual>& hint) {
            for (auto hintItem: hint) {
                ASSERT(hintItem.isWithinAcceptedBounds());
            }

            RandomElement<Individual> randomHint(hint);
            RandomIndividual randomIndividualMaker;
            RandomChance chooseIfRandom(0.5);//(1.0/(hint.size()+1));
            population = std::vector<Individual>();

            auto makeIndividual = [&]() -> Individual {
                if (chooseIfRandom.choose()) return randomIndividualMaker.makeIndividual();
                else return randomHint.choose();
            };

            repeat(populationSize, [&](){population.push_back(makeIndividual());});
            forcePopulationFitnessAssessment();
            //LOG("at the end, the population is"); LOGPopulation();
        }


        Chance getMutationRateUnit() const{
            return (1.0 - initialMutationRate) / mutationRateLevels;
        }

#define ASSERT_MUTATION_VALID() ASSERT_WITHIN(breeder.getMutationRate(), 0.0, 1.0);
        void increaseMutationRate() {
            Chance newRate = breeder.getMutationRate()+getMutationRateUnit();
            if (newRate <= 1.0)
                breeder.setMutationRate(newRate);
        }

        void decreaseMutationRate() {
            Chance newRate = breeder.getMutationRate()-getMutationRateUnit();
            if (newRate >= 0.0)
                breeder.setMutationRate(newRate);
        }

        void resetMutationRate() {
            breeder.setMutationRate(initialMutationRate);
        }


    public:
        Evolver(const EvolutionSettings settings, const FitnessFunction fitnessFunction) :
            populationSize(settings.populationSize),
            amountOfGenerations(settings.generationCount),
            evaluator(fitnessFunction),
            breeder(settings.chanceOfMutation, settings.chanceOfCompressionCrossover),
            selector(Selector::SelectionKind(Selector::TournamentSelection(settings.tournamentSelectionProportion))),
            initialMutationRate(settings.chanceOfMutation),
            usesSimulatedAnnealing(settings.usesSimulatedAnnealing),
            isElitist(settings.isElitist)
            {
                initialiseRandomPopulation();
            }


        Evolver(const EvolutionSettings settings, const FitnessFunction fitnessFunction, std::vector<Individual>& hint) :
                populationSize(settings.populationSize),
                amountOfGenerations(settings.generationCount),
                evaluator(fitnessFunction),
                breeder(settings.chanceOfMutation, settings.chanceOfCompressionCrossover),
                selector(Selector::SelectionKind(Selector::TournamentSelection(settings.tournamentSelectionProportion))),
                initialMutationRate(settings.chanceOfMutation),
                usesSimulatedAnnealing(settings.usesSimulatedAnnealing),
                isElitist(settings.isElitist)
        {
            initialiseHintedPopulation(hint);
        }

        void evolveSingleGeneration() {
            //LOG("The current population is"); for (auto individual: population) LOG(individual.to_string());

            Population children;
            if (isElitist)
                children = population;

            selector.preparePool(population);
            auto addNewIndividual = [&]() {
                //LOG("Adding a new member to the population");
                Individual parentA = selector.select();
                Individual parentB = selector.select();
                Individual newChild = breeder.mutate(breeder.crossover(parentA, parentB));
                evaluator.decideFitness(newChild, parentA, parentB);
                children.emplace_back(newChild);
            };
            repeat(populationSize, addNewIndividual);
            if (isElitist) {
                selector.preparePool(children);
                population = selector.selectMany(populationSize);
            }
            else
                population = children;

            runningAverageFitness.registerNewValue(getBestOfPopulation().getFitness());
            generationCount++;
        }

        bool isStagnating() {
            return runningAverageFitness.getDeviation() > -0.0000000000001; //originally it was >= 0, but equality with 0 if iffy, so this works better
        }

        bool isUnstable() {
            return abs(runningAverageFitness.getDeviation()) > 0.4; //TODO decide a more refined value
        }

        void adjustMutationRate() {
            Chance oldMutationRate = breeder.getMutationRate();
            if (isUnstable()) {
                AM_LOG("Detected unstability, running deviation is ", runningAverageFitness.getDeviation());
                decreaseMutationRate();
            }
            else if (isStagnating()) {
                AM_LOG("Detected stagnation, running deviation is ", runningAverageFitness.getDeviation());
                increaseMutationRate();
            }
            else {
                if (breeder.getMutationRate() != initialMutationRate) {
                    AM_LOG("Resetting mutation rate");
                    resetMutationRate();
                }
            }

            if (breeder.getMutationRate() != oldMutationRate) {
                AM_LOG("Mutation rate is now", breeder.getMutationRate());
            }

        }

        bool populationIsMature() {
            return generationCount > amountOfGenerations / 3;
        }

        bool mutationIsExtreme() {
            return breeder.getMutationRate() > 0.75; //TODO arbitrary
        }

        void evolveForGenerations() {
            for (size_t i=0;i<amountOfGenerations;i++) {
                evolveSingleGeneration();
                if (usesSimulatedAnnealing) {
                    if (mutationIsExtreme()) {
                        AM_LOG("Extreme mutation detected, stopping");
                        return;
                    }
                    if (populationIsMature()) adjustMutationRate();
                }
            }
        }

        void forcePopulationFitnessAssessment() {
            std::for_each(population.begin(), population.end(),
                          [&](Individual& i){evaluator.forceEvaluation(i);});
        }

        void LOGPopulation() {
            LOG("The evolver's population is ");
            std::for_each(population.begin(), population.end(), [&](Individual i){LOG(i.to_string());});
            LOG("--------end of evolver population------------------");
        }

        Individual getBestOfPopulation() {
            forcePopulationFitnessAssessment();
            auto getPrecalculatedFitness = [&](const Individual& individual) {
                return individual.getFitness();
            };

            return getMinimumBy(population, getPrecalculatedFitness);
        }


        Individual evolveBest() {
            evolveForGenerations();
            ////LOG("After evolving a population, the result is");
            //LOGPopulation();
            return getBestOfPopulation();
        }

        void reset() {
            initialiseRandomPopulation();
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
