//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SELECTOR_HPP
#define DISS_SIMPLEPROTOTYPE_SELECTOR_HPP
#include <vector>
#include "../Individual/Individual.hpp"
#include "../Random/RandomElement.hpp"
#include "../Utilities/utilities.hpp"
#include <sstream>

namespace GC {

    class Selector {
    public:
        using Fitness = Individual::Fitness;
        using FitnessFunction = Individual::FitnessFunction;
        using Proportion = double;

        struct TournamentSelection {
            const Proportion proportionToKeep;

            TournamentSelection(const Proportion proportionToKeep) :
                proportionToKeep(proportionToKeep){}
        };

        struct FitnessProportionateSelection {
            //TODO
        };

        using SelectionKind = std::variant<TournamentSelection, FitnessProportionateSelection>;
    private:
        std::vector<Individual> pool;
        SelectionKind selectionKind;
        FitnessFunction fitnessFunction;

        RandomElement<Individual> randomIndividualChooser;


        void forceFitnessCalculationOnPool() {
            auto forceAssessmentOnSingle = [&](Individual& individual) {
                if (!individual.isFitnessAssessed())
                    individual.setFitness(fitnessFunction(individual));
            };
            std::for_each(pool.begin(), pool.end(), forceAssessmentOnSingle);
        }
    public:

        Selector(const SelectionKind& selectionKind, FitnessFunction fitnessFunction) :
            selectionKind(selectionKind),
            fitnessFunction(fitnessFunction){};

        bool isTournamentSelection() {
            return std::holds_alternative<TournamentSelection>(selectionKind);
        }

        Proportion getTournamentProportion() {
            return std::get<TournamentSelection>(selectionKind).proportionToKeep;
        }

        bool isFitnessProportionateSelection() {
            return std::holds_alternative<FitnessProportionateSelection>(selectionKind);
        }

        std::string to_string() {
            std::stringstream ss;
            ss<<"Selector {";

            auto showSelectionKind = [&](const SelectionKind& sk) {
                if (isTournamentSelection())
                    ss<<"TournamentSelection:Proportion:"<<std::setprecision(2)<<std::get<TournamentSelection>(sk).proportionToKeep;
                else if (isFitnessProportionateSelection())
                    ss<<"FitnessProportionateSelection";
                else {
                    ERROR_NOT_IMPLEMENTED("The requested selection type is not properly implemented..");
                }
            };

            showSelectionKind(selectionKind);
            ss<<"}";
            return ss.str();
        }

        template <class List>
        void preparePool(const List& totalPopulation) {
            if (isTournamentSelection()) {
                //there's nothing special to do...
                pool = totalPopulation;
                forceFitnessCalculationOnPool();
            }
            else if (isFitnessProportionateSelection()) {
                ERROR_NOT_IMPLEMENTED("FitnessProportionateSelection is not implemented yet!");
            }
            else {
                ERROR_NOT_IMPLEMENTED("The requested selection kind hasn't been implemented");
            }
        }

        Individual tournamentSelect() {
            std::vector<Individual> tournament;//TODO: this copies the individuals into the tournament, which is very inefficient, in the future this should just reference them in some way
            randomIndividualChooser.setElementPool(pool);
            auto addRandomIndividual = [&]() {
                tournament.push_back(randomIndividualChooser.choose());
            };

            size_t howManyToSelect = (double)(getTournamentProportion()*(double)pool.size());
            //LOG("proportion is ", getTournamentProportion(), ", will select", howManyToSelect, "individuals, from the pool of size", pool.size());
            repeat(howManyToSelect, addRandomIndividual);


            auto getFitness = [&](const Individual& i) -> Fitness {return i.getFitness();};
            return getMinimumBy(tournament, getFitness);
        }


        Individual select() {
            if (isTournamentSelection())
                return tournamentSelect();
            else {
                ERROR_NOT_IMPLEMENTED("The requested selection kind is not implemented yet!");
            }

        }


        void LOGPool() {
            LOG("The pool in this selector is");
            for (auto ind: pool) {LOG(ind.to_string());}
            LOG("----------end of pool-----------------");
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SELECTOR_HPP
