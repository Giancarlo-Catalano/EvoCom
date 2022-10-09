//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SELECTOR_HPP
#define DISS_SIMPLEPROTOTYPE_SELECTOR_HPP
#include <vector>
#include "../Individual/Individual.hpp"
#include "../Random/RandomElement.hpp"
#include "../Utilities/utilities.hpp"
#include "../Evaluator/Evaluator.hpp"
#include "../names.hpp"
#include <sstream>

namespace GC {

    /**
     * This class will hold a pool of individuals, and will be used to select them, based on their fitness
     * It does not care about how the fitness is stored or calculated, and just assumes that the individuals will always have a fitness score precalculated and embedded in them
     */
    class Selector {
    public:
        using Fitness = PseudoFitness::FitnessScore;
        using FitnessFunction = Evaluator::FitnessFunction;

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

        RandomElement<Individual> randomIndividualChooser;

    public:

        Selector(const SelectionKind& selectionKind) :
            selectionKind(selectionKind)
            {};

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
            //LOG("Preparing the population");
            if (isTournamentSelection()) {
                //LOG("(Using tournament selection)");
                //there's nothing special to do...
                pool = totalPopulation;
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
            ////LOG("proportion is ", getTournamentProportion(), ", will select", howManyToSelect, "individuals, from the pool of size", pool.size());
            repeat(howManyToSelect, addRandomIndividual);


            auto getFitness = [&](const Individual& i) -> Fitness {return i.getFitness();};
            return getMinimumBy(tournament, getFitness);
        }


        Individual select() {
            //LOG("Selector is selecting");
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
