//
// Created by gian on 09/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
#define DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
#include "../Utilities/utilities.hpp"
#include "../PseudoFitness/PseudoFitness.hpp"
#include "../Individual/Individual.hpp"
#include "../Random/RandomChance.hpp"
#include <sstream>
#include <iomanip>

namespace GC {

    class Evaluator {
    public:
        using FitnessScore = PseudoFitness::FitnessScore;
        using Reliability = PseudoFitness::Reliability;
        using WeightPair = std::pair<double, double>;
        using Similarity = PseudoFitness::Similarity;
        using FitnessFunction = Individual::FitnessFunction;

    private:
        Reliability reliabilityThreshold;
        mutable RandomChance randomEvaluationChooser;
        FitnessFunction fitnessFunction;

        FitnessScore getFitnessScore(const Individual& I) const {return 0;} //fix this
        Reliability getReliability(const Individual& I) const {return 0;} //TODO fix
        Similarity getSimilarity(const Individual& A, const Individual& B) const {
            return 1-A.distanceFrom(B);
        }

        void setReliability(Individual& I, const FitnessScore f) {//TODO fix
        }

        void setFitnessScore(Individual& I, const Reliability r) const {
            //TODO fix
        }

        WeightPair getWeightsOfParents(const Individual& A, const Individual& B, const Individual& child) {
            Reliability rA = getReliability(A);
            Reliability rB = getReliability(B);
            Similarity sA = getSimilarity(child, A);
            Similarity sB = getSimilarity(child, B);

            return {rA*sA, rB*sB};
        }

        FitnessScore combineFitnesses(const FitnessScore fA, const FitnessScore fB, const WeightPair& weights) {
            return fA*weights.first + fB*weights.second;
        }

        Reliability combineReliabilities(const Reliability rA, const Reliability rB, const WeightPair& weights) {
            return rA*weights.first + rB*weights.second;
        }

        void assignInheritedFitnessToChild(Individual& child, const Individual& A, const Individual& B) {
            WeightPair weights = getWeightsOfParents(A, B, child);
            FitnessScore newFitness = combineFitnesses(getFitnessScore(A), getFitnessScore(B), weights);
            Reliability newReliability = combineReliabilities(getReliability(A), getReliability(B), weights);
            setFitnessScore(child, newFitness);
            setReliability(child, newReliability);
        }

        bool reliabilityTooLow(const Individual& I) const {
            return getReliability(I) < reliabilityThreshold;
        }

        void forceEvaluation(Individual& I) const {
            setFitnessScore(I, fitnessFunction(I));
        }


    public:
        Evaluator(const FitnessFunction fitnessFunction) :
            fitnessFunction(fitnessFunction),
            reliabilityThreshold(0.5),
            randomEvaluationChooser(0.05){
        }

        void decideFitness(Individual& child, const Individual& A, const Individual& B) {
            assignInheritedFitnessToChild(child, A, B);
            if (reliabilityTooLow(child) || randomEvaluationChooser.shouldDo())
                forceEvaluation(child);
        }






    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
