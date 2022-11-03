//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#define DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#include "../../Utilities/utilities.hpp"
#include "../../names.hpp"
#include "../Individual/Individual.hpp"
#include "../../Random/RandomChance.hpp"
#include "../../Random/RandomElement.hpp"
#include "../../Random/RandomIndex.hpp"
#include <sstream>


namespace GC {

    class Breeder {
    public: //types
        using TList = Individual::TList;
        using Index = size_t;
        using CrossoverBounds = std::pair<Index, Index>;


    private:
        //for mutation
        RandomChance randomChanceOfMutation;
        FairCoin addOrRemoveElement;
        RandomElement<TCode> randomTCode;
        RandomElement<CCode> randomCCode;

        //for crossover
        RandomInt<Index> randomIndexChooser;
        RandomChance chooseCCodeCrossover;

    public:


        Breeder(const Chance mutationChance, const Chance chanceOfCCodeCrossover) :
            randomChanceOfMutation(mutationChance),
            addOrRemoveElement(),
            randomTCode(availableTCodes),
            randomCCode(availableCCodes),
            chooseCCodeCrossover(chanceOfCCodeCrossover){};


        void addRandomElement(TList& tList) {
            Index position = randomIndexChooser.chooseInRange(0, tList.size());
            tList.insert(tList.begin()+position, randomTCode.choose());
        }

        void removeRandomElement(TList& tList) {
            Index position = randomIndexChooser.chooseInRange(0, tList.size()-1);
            tList.erase(tList.begin()+position);
        }


        Individual mutate(const Individual& individual) {
            ////LOG("Mutating an individual");
            Individual child = individual;
            for (TCode& tCode : child.getTList())
                randomChanceOfMutation.doWithChance([&]{randomTCode.assignRandomValue(tCode);});

            randomCCode.assignRandomValue(child.getCCode());

            auto mutateLength = [&](Individual& ind) {

                if (addOrRemoveElement.flip() && ind.hasSpaceForMoreMutations())
                    addRandomElement(ind.getTList());
                if (addOrRemoveElement.flip() && ind.canRemoveMoreMutations())
                    removeRandomElement(ind.getTList());
            };

            randomChanceOfMutation.doWithChance([&](){mutateLength(child);});
            return child;
        }


        /**
         * Note that an empty selection is possible, and in that case the bounds should be (a, a)
         * If we wanted to have all the elements in a list of size s, it would be (0, s)
         * If we wanted to include only the first element, it would be (0, 1)
         * If we wanted to include the last element, it would be (s-1, s)
         * In case this is confusing, I think of the bounds as the "edges between cells in the lists"
         *    | a | b | c | d | e |
         *    0   1   2   3   4   5
         *
         *    It makes handling empty lists much easier
         *    This also assumes that the bounds are in the right direction
         * @tparam L the kind of collection that will be crossed over
         * @param X
         * @param Y
         * @param boundsX (index of the first element to be included, index of the last element to be included)
         * @param boundsY same as the other bounds
         * @return the list X, but the range of items in [boundsX] has been replaced with the items in [rangeY] of Y
         */
        template <class L>
        static L crossoverLists(const L& X, const L& Y, const CrossoverBounds& boundsX, const CrossoverBounds& boundsY) {
            //LOG("Calling CrossoverLists");

            L result;
            auto addFromBounds = [&](const L& collection, const Index start, const Index afterEnd) {
                //LOG("Calling addFromBounds(", start,", ", afterEnd, ")");
                std::copy(collection.begin()+start, collection.begin()+afterEnd, std::back_inserter(result));
            };

            addFromBounds(X, 0, boundsX.first);
            addFromBounds(Y, boundsY.first, boundsY.second);
            addFromBounds(X, boundsX.second, X.size());
            //LOG("Ending CrossoverLists");
            return result;
        }

        Individual crossover(const Individual& A, const Individual& B) {
            //LOG("Requested to cross over", A.to_string(), " and ", B.to_string());
            auto chooseIndex = [&](const Individual& X) {
                return randomIndexChooser.chooseInRange(0, X.readTList().size()); //note that the size is also a possible option
            };
            auto orderBounds = [&](CrossoverBounds& b) {
                b = {std::min(b.first, b.second), std::max(b.first, b.second)};
            };

            CrossoverBounds boundsInA = {chooseIndex(A), chooseIndex(A)};
            orderBounds(boundsInA);

            const size_t Blen = B.getTListLength();
            const size_t AsContribution = A.getTListLength()-(boundsInA.second-boundsInA.first);
            const Index startInB = randomIndexChooser.chooseInRange(0, Blen);
            const size_t maxSizeOfChunkInB = std::min(Blen-startInB, Individual::MaxTListLength-AsContribution);
            const size_t minSizeOfChunkInB = 0;
            const size_t randomSize = randomIndexChooser.chooseInRange(minSizeOfChunkInB, maxSizeOfChunkInB);
            CrossoverBounds boundsInB = {startInB, startInB+randomSize};
            //this was all to prevent the new child from exceeding the list size requirements

            //LOG_NOSPACES("BoundsInA = {", boundsInA.first, ",", boundsInA.second, "}, BoundsInB = {", boundsInB.first, ", ", boundsInB.second, "}");

            TList newTList = crossoverLists(A.readTList(), B.readTList(), boundsInA, boundsInB);
            CCode newCCode = chooseCCodeCrossover.choose() ? A.readCCode() : B.readCCode();

            return Individual(newTList, newCCode);
        }


        std::string to_string() {
            std::stringstream ss;
            ss<<"Breeder {";
            ss<<"MutationChance:"<<std::setprecision(2)<<randomChanceOfMutation.getChance();
            ss<<"}";
            return ss.str();
        }

        Chance getMutationRate() const {
            return randomChanceOfMutation.getChance();
        }

        void setMutationRate(const Chance newMutationRate) {
            randomChanceOfMutation.setChance(newMutationRate);
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BREEDER_HPP
