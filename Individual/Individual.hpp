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
#include "../Utilities/utilities.hpp"
#include "TCodes.hpp"
#include "CCodes.hpp"
#include "../Random/RandomElement.hpp"
#include <optional>

namespace GC {

    class Individual {

    public: //types
        using Fitness = double; //will be a ratio
        using FitnessFunction = std::function<Fitness(Individual)>;
        using FitnessOfIndividualOnBlock = std::function<Fitness(Individual, Block)>;

        static const size_t TListLength = 6;



        using TList = std::array<TCode, TListLength>;
    public: //attributes, these are all public
        TList tList;
        CCode cCode;
        std::optional<Fitness> fitness;

    public: //methods
        Individual() :
            tList(),
            cCode(C_IdentityCompression),
            fitness(std::nullopt){}


        Individual(const TList& tList, const CCode cCode) :
            tList(tList),
            cCode(cCode),
            fitness(std::nullopt){}

        TList& getTList() { return tList;}

        std::string to_string() {
#define GC_PRINT_TCODE_NAMES 0
#define GC_PRINT_CCODE_NAMES 0
            std::stringstream ss;
            ss<<"{";

            auto showTCodeWithNames = [&](const TCode tc) {
                std::vector<std::string> asStrings = {
                        "T_DeltaTransform",
                        "T_DeltaXORTransform",
                        "T_RunLengthTransform",
                        "T_SplitTransform",
                        "T_StackTransform",
                        "T_StrideTransform_2",
                        "T_StrideTransform_3",
                        "T_StrideTransform_4",
                        "T_SubtractAverageTransform",
                        "T_SubtractXORAverageTransform"
                };
                ss << asStrings[static_cast<int>(tc)];
            };

            auto showTCodeWithNumbers = [&](const TCode tc) {
                ss << static_cast<int>(tc);
            };

            auto showTList = [&]() {
                ss<<"TList:{";
                bool isFirst = true;
                std::for_each(tList.begin(), tList.end(), [&](auto t) {
                    if (!isFirst)ss<<", ";
                    isFirst = false;
#if GC_PRINT_TCODE_NAMES ==1
                    showTCodeWithNames(t);
#else
                    showTCodeWithNumbers(t);
#endif

                } );
                ss<<"}";
            };

            auto showCCodeWithNames = [&]() {
                std::vector<std::string> asStrings = {
                        "C_HuffmanCompression",
                        "C_RunLengthCompression",
                        "C_IdentityCompression"
                };
                ss<<"Compr:"<<asStrings[static_cast<int>(cCode)];
            };

            auto showCCodeWithNumbers = [&]() {
                ss << static_cast<int>(cCode);
            };

            auto showFitness = [&]() {
                ss<<"Fitness:";
                if (fitness.has_value())
                    ss<<std::setprecision(2)<<fitness.value();
                else
                    ss<<"Not assessed yet";
            };


            showTList();ss<<", ";
#if GC_PRINT_CCODE_NAMES == 1
            showCCodeWithNames();
#else
            showCCodeWithNumbers();
#endif
            ss << ", ";
            showFitness();

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
            return fitness.has_value();
        }

        Fitness getFitness() const {
            ASSERT(isFitnessAssessed());
            return fitness.value();
        }

        void setFitness(const Fitness newFitness) {
            fitness = newFitness;
        }


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
