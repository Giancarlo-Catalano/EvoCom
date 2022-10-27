//
// Created by gian on 09/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
#define DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
#include "../../Utilities/utilities.hpp"
#include "../PseudoFitness/PseudoFitness.hpp"
#include "../Individual/Individual.hpp"
#include "../../Random/RandomChance.hpp"
#include <sstream>
#include <iomanip>

namespace GC {

    class Evaluator {
    public:
        using FitnessScore = PseudoFitness::FitnessScore;
        using Reliability = PseudoFitness::Reliability;
        using Similarity = PseudoFitness::Similarity;
        using FitnessFunction = std::function<FitnessScore(Individual)>;

    private:
        Reliability reliabilityThreshold;
        mutable RandomChance randomEvaluationChooser;
        FitnessFunction fitnessFunction;

        Similarity getSimilarity(const Individual& A, const Individual& B) const {
            return 1-A.distanceFrom(B);
        }

        void setFitnessScore(Individual& I, const FitnessScore f) const {
            I.getPseudoFitness().setFitnessScore(f);
        }

        void setReliability(Individual& I, const Reliability r) const {
            I.getPseudoFitness().setReliability(r);
        }

        FitnessScore combineFitnesses(const FitnessScore fA, const FitnessScore fB, const Reliability rA, const Reliability rB, Similarity sA, const Similarity sB) {
            return ((sA*rA*fA)+(sB*rB*fB))/(sA*rA + sB*rB);
        }

        Reliability combineReliabilities(const Reliability rA, const Reliability rB, const Similarity sA, const Similarity sB) {
            return (square(sA*rA)+ square(sB*rB))/(sA*rA+sB*rB);
        }

        void assignInheritedFitnessToChild(Individual& child, const Individual& A, const Individual& B) {
            FitnessScore fA = A.getFitness();
            FitnessScore fB = B.getFitness();
            Reliability rA = A.getFitnessReliability();
            Reliability rB = B.getFitnessReliability();
            Similarity sA = getSimilarity(child, A);
            Similarity sB = getSimilarity(child, B);

            setFitnessScore(child, combineFitnesses(fA, fB, rA, rB, sA, sB));
            setReliability(child, combineReliabilities(rA, rB, sA, sB));
        }

        bool reliabilityTooLow(const Individual& I) const {
            return I.getFitnessReliability() < reliabilityThreshold;
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
