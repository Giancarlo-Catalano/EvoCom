//
// Created by gian on 09/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
#define DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
#include "../../Utilities/utilities.hpp"
#include "../PseudoFitness/PseudoFitness.hpp"
#include "../Recipe/Recipe.hpp"
#include "../../Random/RandomChance.hpp"
#include <sstream>
#include <iomanip>

namespace GC {

    class Evaluator {
    public:
        using FitnessScore = PseudoFitness::FitnessScore;
        using Reliability = PseudoFitness::Reliability;
        using Similarity = PseudoFitness::Similarity;
        using FitnessFunction = std::function<FitnessScore(Recipe)>;

    private:
        Reliability reliabilityThreshold; //what is the minimum accepted reliability? (always in [0, 1])
        mutable RandomChance randomEvaluationChooser;
        FitnessFunction fitnessFunction;

        Similarity getSimilarity(const Recipe& A, const Recipe& B) const { //1 means they're identical
            const auto elemsIn = [&](const Recipe& i) {
                return i.getTListLength()+1; //+1 is because there's the compression
            };

            const Similarity distance = (A.distanceFrom(B))/(elemsIn(A)+elemsIn(B));
            return 1-distance;
        }

        void setFitnessScore(Recipe& I, const FitnessScore f) const {
            I.getPseudoFitness().setFitnessScore(f);
        }

        void setReliability(Recipe& I, const Reliability r) const {
            I.getPseudoFitness().setReliability(r);
        }



        FitnessScore combineFitnesses(const FitnessScore fA, const FitnessScore fB, const Reliability rA, const Reliability rB, Similarity sA, const Similarity sB) {
            const auto canUse = [&](const Reliability r) {
                return r > 0;
            };

            if (canUse(rA) || canUse(rB))
                return ((sA*rA*fA)+(sB*rB*fB))/(sA*rA + sB*rB);
            else
                return 2.0; //panic and return a bad fitness;
        }

        Reliability combineReliabilities(const Reliability rA, const Reliability rB, const Similarity sA, const Similarity sB) {
            const auto canUse = [&](const Reliability r, const Similarity s) {return r*s > 0;};
            const bool canUseA = canUse(rA, sA);
            const bool canUseB = canUse(rB, sB);

            if (canUseA || canUseB)
                return (square(sA*rA)+ square(sB*rB))/(sA*rA+sB*rB);
            else
                return 0; //unreliable
        }

        void assignInheritedFitnessToChild(Recipe& child, const Recipe& A, const Recipe& B) {
            FitnessScore fA = A.getFitness();
            FitnessScore fB = B.getFitness();
            Reliability rA = A.getFitnessReliability();
            Reliability rB = B.getFitnessReliability();
            Similarity sA = getSimilarity(child, A);
            Similarity sB = getSimilarity(child, B);
            //LOG("Variables:", fA, fB, rA, rB, sA, sB);

            setFitnessScore(child, combineFitnesses(fA, fB, rA, rB, sA, sB));
            setReliability(child, combineReliabilities(rA, rB, sA, sB));
        }

        bool reliabilityTooLow(const Recipe& I) const {
            //LOG("Checking the reliability, it's", I.getFitnessReliability());
            return I.getFitnessReliability() < reliabilityThreshold;
        }


    public:
        Evaluator(const FitnessFunction fitnessFunction) :
            fitnessFunction(fitnessFunction),
            reliabilityThreshold(0.7),
            randomEvaluationChooser(0.05){
        }

        void decideFitness(Recipe& child, const Recipe& A, const Recipe& B) {
            assignInheritedFitnessToChild(child, A, B);
            //LOG("A's reliability=", A.getFitnessReliability(), "B's reliability=", B.getFitnessReliability());
            if (reliabilityTooLow(child) || randomEvaluationChooser.shouldDo()) {
                forceEvaluation(child);
            }

        }


        void forceEvaluation(Recipe& I) const {
            setFitnessScore(I, fitnessFunction(I));
            setReliability(I, 1.0);
        }





    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_EVALUATOR_HPP
