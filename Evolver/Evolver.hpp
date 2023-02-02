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
#include "../EvolutionaryFileCompressor/EvoCompressorSettings/EvoComSettings.hpp"

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
            size_t eliteSize;
            double mutationThreshold;
            double unstabilityThreshold;

            EvolutionSettings() :
                populationSize(40),
                generationCount(100),
                chanceOfMutation(0.05),
                chanceOfCompressionCrossover(0.25),
                tournamentSelectionProportion(0.80),
                usesSimulatedAnnealing(true),
                eliteSize(2),
                mutationThreshold(0.75),
                unstabilityThreshold(0.4){}


            EvolutionSettings(const EvoComSettings& settings) :
                populationSize(settings.population),
                generationCount(settings.generations),
                chanceOfMutation(settings.mutationRate),
                chanceOfCompressionCrossover(settings.compressionCrossoverRate),
                tournamentSelectionProportion(settings.population / settings.tournamentSelectionSize),
                usesSimulatedAnnealing(settings.usesAnnealing),
                eliteSize(settings.eliteSize),
                mutationThreshold(settings.excessiveMutationThreshold),
                unstabilityThreshold(settings.unstabilityThreshold){

            }
        };

        using Fitness = Individual::FitnessScore;
        using FitnessFunction = Evaluator::FitnessFunction;

    private:
        Breeder breeder;
        Selector selector;
        Evaluator evaluator;
        Population population;

        RunningAverage<Fitness> runningAverageFitness{0.5};
        size_t generationCount = 0;
        const Chance initialMutationRate; static const size_t mutationRateLevels = 12;

        size_t populationSize;
        size_t amountOfGenerations;
        bool usesSimulatedAnnealing;
        size_t eliteSize;
        const double unstabilityThreshold;
        const double excessiveMutationThreshold;


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
            eliteSize(settings.eliteSize),
            excessiveMutationThreshold(settings.mutationThreshold),
            unstabilityThreshold(settings.unstabilityThreshold)
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
                excessiveMutationThreshold(settings.mutationThreshold),
                unstabilityThreshold(settings.unstabilityThreshold)
        {
            initialiseHintedPopulation(hint);
        }

        void evolveSingleGeneration() {
            //LOG("The current population is"); for (auto individual: population) LOG(individual.to_string());
            Population children = selector.selectElite(eliteSize, population);

            selector.preparePool(population);
            auto addNewIndividual = [&]() {
                //LOG("Adding a new member to the population");
                Individual parentA = selector.select();
                Individual parentB = selector.select();
                Individual newChild = breeder.mutate(breeder.crossover(parentA, parentB));
                evaluator.decideFitness(newChild, parentA, parentB);
                children.emplace_back(newChild);
            };
            repeat(populationSize - eliteSize, addNewIndividual);
            population = children;

            runningAverageFitness.registerNewValue(getBestOfPopulation(true).getFitness());
            generationCount++;
        }

        void evolveSingleUniqueGeneration() {
            Population elite = selector.selectElite(eliteSize, population);

            selector.preparePool(population);

            auto generateChild = [&]()->Individual {
                const Individual parentA = selector.select();
                const Individual parentB = selector.select();
                Individual newChild = breeder.mutate(breeder.crossover(parentA, parentB));
                evaluator.decideFitness(newChild, parentA, parentB);
                return newChild;
            };
            population = Breeder::generateUnique<Individual>(populationSize, elite, generateChild);

            runningAverageFitness.registerNewValue(getBestOfPopulation(true).getFitness());
            generationCount++;
        }

        bool isStagnating() {
            return std::abs(runningAverageFitness.getDeviation()) < 0.05; //originally it was >= 0, but equality with 0 if iffy, so this works better
        }

        bool isUnstable() {
            return abs(runningAverageFitness.getDeviation()) > 0.4; //TODO decide a more refined value
        }

        void adjustMutationRate() {
            if (isStagnating()) {
                AM_LOG("Detected stagnation, running deviation is ", runningAverageFitness.getDeviation());
                increaseMutationRate();
            }
            else if (breeder.getMutationRate() > initialMutationRate) {
                decreaseMutationRate();
            }
            else if (breeder.getMutationRate() < initialMutationRate) {
                resetMutationRate();
            }

        }

        bool populationIsMature() {
            static const size_t maturityAge = std::min(generationCount, 3UL);
            return generationCount > maturityAge;
        }

        bool mutationIsExtreme() {
            return breeder.getMutationRate() > 0.75; //TODO arbitrary
        }

        void adaptParameters() {
            if (usesSimulatedAnnealing && populationIsMature() && !mutationIsExtreme())
                adjustMutationRate();
        }

        void evolveForGenerations() {
            for (size_t i=0;i<amountOfGenerations;i++) {
                if (mutationIsExtreme()) return;
                evolveSingleUniqueGeneration();
                adaptParameters();
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

        Individual getBestOfPopulation(const bool forceAssessment = true) {
            if (forceAssessment) forcePopulationFitnessAssessment();
            auto getPrecalculatedFitness = [&](const Individual& individual) {
                return individual.getFitness();
            };

            return getMinimumBy(population, getPrecalculatedFitness);
        }


        Individual evolveBest() {
            evolveForGenerations();
            return getBestOfPopulation(true);
        }

        void reset() {
            initialiseRandomPopulation();
        }

        Individual evolveBestAndLogProgress(Logger &logger) {
            size_t generationCounter = 0;

            auto logGenerationData = [&]() {
                const Individual bestIndividual = getBestOfPopulation(true);
                const Fitness bestFitness = bestIndividual.getFitness();

                logger.beginUnnamedObject();
                logger.addVar("Generation", generationCounter++);
                logger.addVar("BestFitness", bestFitness);
                logger.addVar("BestIndividual:", bestIndividual.to_string());
                logger.addVar("Mutation", breeder.getMutationRate());
                logger.addVar("runningAverage", runningAverageFitness.getAverage());
                logger.addVar("deviation", runningAverageFitness.getDeviation());
                logger.endObject(); //ends the unnamed object
            };

            auto evolveForGenerationsAndLog = [&]() { //mimics evolveForGenerations
                for (size_t i=0;i<amountOfGenerations;i++) {
                    if (mutationIsExtreme()) return;
                    evolveSingleUniqueGeneration();
                    logGenerationData();
                    adaptParameters();
                }
            };

            evolveForGenerationsAndLog();

            return getBestOfPopulation(true);
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVOLVER_HPP
