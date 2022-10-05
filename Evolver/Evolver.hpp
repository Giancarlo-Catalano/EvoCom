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

        using Fitness = Individual::Fitness;
        using FitnessFunction = Individual::FitnessFunction;

    private:
        Breeder breeder;
        Selector selector;
        Population population;

        size_t populationSize;
        size_t amountOfGenerations;

        FitnessFunction fitnessFunction;


    public:
        Evolver(const EvolutionSettings settings, const FitnessFunction fitnessFunction) :
            populationSize(settings.populationSize),
            amountOfGenerations(settings.generationCount),
            fitnessFunction(fitnessFunction),
            breeder(settings.chanceOfMutation, settings.chanceOfCompressionCrossover),
            selector(Selector::SelectionKind(Selector::TournamentSelection(settings.tournamentSelectionProportion)), fitnessFunction)
            {
                initialiseRandomPopulation();
            }


        Evolver(const EvolutionSettings settings, const FitnessFunction fitnessFunction, std::vector<Individual>& hint) :
                populationSize(settings.populationSize),
                amountOfGenerations(settings.generationCount),
                fitnessFunction(fitnessFunction),
                breeder(settings.chanceOfMutation, settings.chanceOfCompressionCrossover),
                selector(Selector::SelectionKind(Selector::TournamentSelection(settings.tournamentSelectionProportion)), fitnessFunction)
        {
            LOG("Called evolver using the hint!");
            initialiseHintedPopulation(hint);
        }
        void initialiseRandomPopulation() {
            RandomIndividual randomIndividualMaker;
            population = std::vector<Individual>();
            auto addRandomIndividual = [&]() {
                population.push_back(randomIndividualMaker.makeIndividual());
            };

            repeat(populationSize, addRandomIndividual);
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
            //LOG("at the end, the population is"); LOGPopulation();
        }
        void evolveSingleGeneration() {
            //LOG("Starting a new generation");
            Population children;
            selector.preparePool(population);
            auto addNewIndividual = [&]() {
                //LOG("Adding a new member to the population");
                children.emplace_back(breeder.mutate(breeder.crossover(selector.select(), selector.select())));
            };
            repeat(populationSize, addNewIndividual);
            population = children;
        }

        void evolveForGenerations() {
            auto singleGeneration = [&](){evolveSingleGeneration();};
            repeat(amountOfGenerations, singleGeneration);
        }

        void forcePopulationFitnessAssessment() {
            auto forceAssessmentOnSingle = [&](Individual& individual) {
                if (!individual.isFitnessAssessed())
                    individual.setFitness(fitnessFunction(individual));
            };
            std::for_each(population.begin(), population.end(), forceAssessmentOnSingle);
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
