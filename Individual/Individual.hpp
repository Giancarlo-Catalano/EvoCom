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
#include "../Utilities/utilities.hpp"

namespace GC {

    class Individual {

    public: //types
        static const size_t TListLength = 6;

        enum TCode {
            T_DeltaTransform,
            T_DeltaXORTransform,
            T_RunLengthTransform,
            T_SplitTransform,
            T_StackTransform,
            T_StrideTransform_2,
            T_StrideTransform_3,
            T_StrideTransform_4,
            T_SubtractAverageTransform,
            T_SubtractXORAverageTransform
        };

        enum CCode {
            C_HuffmanCompression,
            C_RunLengthCompression,
            C_IdentityCompression
        };



        using TList = std::array<TCode, TListLength>;
    public: //attributes, these are all public
        TList tList;
        CCode cCode;

    public: //methods
        Individual() :
            tList(),
            cCode(C_IdentityCompression) {}

        TList& getTList() { return tList;}

        std::string to_string() {
            std::stringstream ss;
            ss<<"{";

            auto showTCode = [&](const TCode tc) {
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

            auto showTList = [&]() {
                ss<<"TList:{";
                bool isFirst = true;
                std::for_each(tList.begin(), tList.end(), [&](auto t) {
                    if (!isFirst)ss<<", ";
                    isFirst = false;
                    showTCode(t);
                } );
                ss<<"}";
            };

            auto showCCode = [&]() {
                std::vector<std::string> asStrings = {
                        "C_HuffmanCompression",
                        "C_RunLengthCompression",
                        "C_IdentityCompression"
                };
                ss<<"Compr:"<<asStrings[static_cast<int>(cCode)];
            };


            showTList();ss<<", ";
            showCCode();
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





    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
