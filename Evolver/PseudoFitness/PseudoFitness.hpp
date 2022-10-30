//
// Created by gian on 09/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_PSEUDOFITNESS_HPP
#define DISS_SIMPLEPROTOTYPE_PSEUDOFITNESS_HPP
#include <sstream>
#include <iomanip>
#include <cmath>

namespace GC {

    class PseudoFitness {
    public:
        using FitnessScore = double;  //a compression ratio, compressedSize/originalSize
        using Reliability = double;   //a value from 0 to 1, where 1 means 100% reliable
        using Similarity = double;    //a value from 0 to 1, where 1 means identical

    private:
        FitnessScore fitnessScore;
        Reliability reliability;
    public:

        std::string to_string() const {
            std::stringstream ss;
            ss<<"{PseudoFitness:";
            if (reliability > 0) {
                ss<<(isActualFitness() ? "Actual" : "Approx")<<"="<<std::setprecision(2)<<fitnessScore;
                if (!isActualFitness())
                    ss<<", Reliability="<<std::setprecision(2)<<reliability;
            }
            else {
                ss<<"Irrelevant";
            }
            ss<<"}";

            return ss.str();
        }

        Reliability getReliability() const {return reliability;}
        FitnessScore getFitnessScore() const {return fitnessScore;}
        bool isActualFitness() const {return reliability == 1.0;}


        PseudoFitness():
            fitnessScore(10000), //TODO find an appropriate DOUBLE_MAX
            reliability(0){
        }

        PseudoFitness(const FitnessScore fs) :  //an actual fitness
            fitnessScore(fs),
            reliability(1.0) {
        }

        PseudoFitness(const FitnessScore approxFs, const Reliability r) : //an estimated fitness
            fitnessScore(approxFs),
            reliability(r) {
        }

        void setActualFitness(const FitnessScore fs) {
            fitnessScore = fs;
            reliability = 1;
        }


        void setFitnessScore(const FitnessScore f) {
            fitnessScore = f;
        }

        void setReliability(const Reliability r) {
            reliability = r;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_PSEUDOFITNESS_HPP
