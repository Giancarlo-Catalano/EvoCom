//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
#define DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP

#include <variant>
#include <sstream>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include "../../Utilities/utilities.hpp"
#include "TCodes.hpp"
#include "CCodes.hpp"
#include "../../Random/RandomElement.hpp"
#include <optional>
#include "../PseudoFitness/PseudoFitness.hpp"

namespace GC {

    class Individual {

#define GC_PRINT_TCODE_NAMES 1
#define GC_PRINT_CCODE_NAMES 1

    public: //types
        using Fitness = PseudoFitness;
        using FitnessScore = PseudoFitness::FitnessScore;
        static const size_t TListLength = 6;



        using TList = std::array<TCode, TListLength>;
    public: //attributes, these are all public
        TList tList;
        CCode cCode;
        Fitness fitness;

    public: //methods
        Individual() :
            tList(),
            cCode(C_IdentityCompression),
            fitness(){}


        Individual(const TList& tList, const CCode cCode) :
            tList(tList),
            cCode(cCode),
            fitness(){}

        Individual(const std::vector<TCode>& tVector, const CCode cCode) :
            cCode(cCode),
            tList(),
            fitness(){
            ASSERT_EQUALS(tVector.size(), tList.size());
            std::copy(tVector.begin(), tVector.end(), tList.begin());
        }

        TList& getTList() { return tList;}

        static std::string TCode_as_string(const TCode tc) {
            if (GC_PRINT_TCODE_NAMES)
                return TCodesAsStrings[static_cast<int>(tc)];
            else
                return std::to_string(static_cast<int>(tc));
        }

        static std::string CCode_as_string(const CCode cc) {
            if (GC_PRINT_CCODE_NAMES)
                return CCodesAsStrings[static_cast<int>(cc)];
            else
                return std::to_string(static_cast<int>(cc));
        }

        std::string to_string() const{

            std::stringstream ss;
            ss<<"{";

            auto showTList = [&]() {
                ss<<"TList:{";
                bool isFirst = true;
                std::for_each(tList.begin(), tList.end(), [&](auto t) {
                    if (!isFirst)ss<<", ";
                    isFirst = false;
                    ss << TCode_as_string(t);
                } );
                ss<<"}";
            };


            showTList();ss<<", "<<CCode_as_string(cCode);ss<<", ";
            ss<<fitness.to_string();

            return ss.str();
        }


        void setTItem(const TCode tCode, const size_t index) {
            ASSERT(index < tList.size());
            tList[index] = tCode;
        }

        TCode& getTCode(const size_t index) {
            ASSERT(index < tList.size());
            return tList[index];
        }

        CCode& getCCode() {
            return cCode;
        }

        TCode readTCode(const size_t index) const {
            ASSERT(index < tList.size());
            return tList[index];
        }

        CCode readCCode() const {
            return cCode;
        }

        void copyTCodeFrom(const size_t index, const Individual& A) {
            getTCode(index) = A.readTCode(index);
        }

        void copyCCodeFrom(const Individual& A) {
            getCCode() = A.readCCode();
        }

        bool isFitnessAssessed() const{
            return fitness.isActualFitness();
        }

        FitnessScore getFitness() const {
            return fitness.getFitnessScore();
        }

        Fitness& getPseudoFitness() {return fitness;}

        PseudoFitness::Reliability getFitnessReliability() const {return fitness.getReliability();}


        double distanceFrom(const Individual& other) const {
            auto discreteMetric = [&](auto A, auto B) -> double {
                return (double)(A!=B);
            };

            double currentSum = 0.0;
            ASSERT_EQUALS(tList.size(), other.tList.size());
            for (size_t i = 0;i < tList.size();i++)
                currentSum += discreteMetric(readTCode(i), other.readTCode(i));

            currentSum += discreteMetric(readCCode(), other.readCCode());
            return currentSum / (double(tList.size() + 1)); //the average
        }
    };


    class RandomIndividual {
    private:
        RandomElement<TCode> randomTCode{availableTCodes};
        RandomElement<CCode> randomCCode{availableCCodes};
    public:
        RandomIndividual() = default;
        Individual makeIndividual() {
            Individual::TList tList;
            for (TCode& tCode: tList)
                randomTCode.assignRandomValue(tCode);
            CCode cCode = randomCCode.choose();
            return Individual(tList, cCode);
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
