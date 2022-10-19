//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
#define DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
#include "../Individual/Individual.hpp"
#include "../Breeder/Breeder.hpp"
#include "../Selector/Selector.hpp"
#include "../Utilities/utilities.hpp"
#include "../names.hpp"
#include "../StatisticalFeatures/RunningAverage.hpp"

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

            EvolutionSettings() :
                populationSize(40),
                generationCount(100),
                chanceOfMutation(0.05),
                chanceOfCompressionCrossover(0.25),
                tournamentSelectionProportion(0.80){}
        };

        using Fitness = Individual::FitnessScore;
        using FitnessFunction = Evaluator::FitnessFunction;

    private:
        Breeder breeder;
        Selector selector;
        Evaluator evaluator;
        Population population;
        RunningAverage<Fitness> runningAverageFitness;
        size_t generationCount = 0;
        Chance initialMutationRate;

        size_t populationSize;
        size_t amountOfGenerations;


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
            RandomElement<Individual> randomHint(hint);
            RandomIndividual randomIndividualMaker;
            RandomChance chooseIfRandom(0.5);//(1.0/(hint.size()+1));
            population = std::vector<Individual>();

            auto makeIndividual = [&]() -> Individual {
                if (chooseIfRandom.flip()) return randomIndividualMaker.makeIndividual();
                else return randomHint.choose();
            };

            repeat(populationSize, [&](){population.push_back(makeIndividual());});
            forcePopulationFitnessAssessment();
            //LOG("at the end, the population is"); LOGPopulation();
        }

        void increaseMutationRate() {
            Chance oldMutationRate = breeder.getMutationRate();
            /**
             * the following function needs to have certain properties:
             *  for x in [0, 1], f(x) in [0, 1]
             *  bijective
             *  f(x)>=x
             */
            Chance newMutationRate = (3.0*oldMutationRate + 1.0)*0.25;
            breeder.setMutationRate(newMutationRate);
        }

        void decreaseMutationRate() {
            Chance oldMutationRate = breeder.getMutationRate();
            Chance newMutationRate = ((oldMutationRate*4.0)-1.0)/3.0;
            breeder.setMutationRate(newMutationRate);
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
            runningAverageFitness(),
            initialMutationRate(settings.chanceOfMutation)
            {
                initialiseRandomPopulation();
            }


        Evolver(const EvolutionSettings settings, const FitnessFunction fitnessFunction, std::vector<Individual>& hint) :
                populationSize(settings.populationSize),
                amountOfGenerations(settings.generationCount),
                evaluator(fitnessFunction),
                breeder(settings.chanceOfMutation, settings.chanceOfCompressionCrossover),
                selector(Selector::SelectionKind(Selector::TournamentSelection(settings.tournamentSelectionProportion))),
                runningAverageFitness()
        {
            initialiseHintedPopulation(hint);
        }

        void evolveSingleGeneration() {
            //LOG("Starting a new generation");
            Population children;
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
            population = children;
            runningAverageFitness.registerNewValue(getBestOfPopulation().getFitness());
            generationCount++;
        }

        bool isStagnating() {
            return runningAverageFitness.getDeviation() > -0.001;
        }

        bool isUnstable() {
            return abs(runningAverageFitness.getDeviation()) > 0.4;
        }

        void adjustMutationRate() {
            if (isStagnating()) {
                //LOG("Stagnation detected, increasing the mutation rate, now ", breeder.getMutationRate());
                increaseMutationRate();
            } else {
                //LOG("Situation is stable, resetting mutation rate");
                resetMutationRate();
            }
        }

        bool populationIsMature() {
            return generationCount > amountOfGenerations / 3;
        }

        bool mutationIsExtreme() {
            return breeder.getMutationRate() > 0.75;
        }

        void evolveForGenerations() {
            for (size_t i=0;i<amountOfGenerations;i++) {
                evolveSingleGeneration();
                //LOG("Deviation = ", runningAverageFitness.getDeviation());
                if (populationIsMature()) adjustMutationRate();
                if (mutationIsExtreme()) {/*LOG("Extreme mutation detected, stopping")*/;return;}
            }
        }

        void forcePopulationFitnessAssessment() {
            std::for_each(population.begin(), population.end(),
                          [&](Individual& i){evaluator.forceEvaluation(i);});
        }

        void forceEvaluation(Individual& i) {
            evaluator.forceEvaluation(i);
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


        bool isPoolIsFullyAssessed() {
            auto isAssessed = [&](const Individual& i) {
                return i.getFitnessReliability() > 0.01;
            };

            return std::all_of(population.begin(), population.end(), isAssessed);
        }








    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
