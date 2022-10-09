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
        using FitnessFunction = std::function<FitnessScore(Individual)>;

    private:
        Reliability reliabilityThreshold;
        mutable RandomChance randomEvaluationChooser;
        FitnessFunction fitnessFunction;

        FitnessScore getFitnessScore(const Individual& I) const {return I.getFitness();}
        Reliability getReliability(const Individual& I) const {return I.getFitnessReliability();}
        Similarity getSimilarity(const Individual& A, const Individual& B) const {
            return 1-A.distanceFrom(B);
        }

        void setFitnessScore(Individual& I, const FitnessScore f) const {
            I.getPseudoFitness().setFitnessScore(f);
        }

        void setReliability(Individual& I, const Reliability r) const {
            I.getPseudoFitness().setReliability(r);
        }

        WeightPair getWeightsOfParents(const Individual& A, const Individual& B, const Individual& child) {
            Reliability rA = getReliability(A);
            Reliability rB = getReliability(B);
            Similarity sA = getSimilarity(child, A);
            Similarity sB = getSimilarity(child, B);

            auto normalize = [&](const WeightPair& values) -> WeightPair {
                double sum = values.first + values.second;
                if (sum == 0.0) {return values;}
                return {values.first / sum, values.second / sum};
            };

            return normalize({rA*sA, rB*sB}); //so that their sum is 1
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


    public:
        Evaluator(const FitnessFunction fitnessFunction) :
            fitnessFunction(fitnessFunction),
            reliabilityThreshold(0.5),
            randomEvaluationChooser(0.05){
        }

        void decideFitness(Individual& child, const Individual& A, const Individual& B) {
            assignInheritedFitnessToChild(child, A, B);
            if (reliabilityTooLow(child) || randomEvaluationChooser.shouldDo()) {
                forceEvaluation(child);
            }

        }


        void forceEvaluation(Individual& I) const {
            setFitnessScore(I, fitnessFunction(I));
            setReliability(I, 1.0);
        }





    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
