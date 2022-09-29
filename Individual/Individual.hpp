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

        struct FixedLengthBlocksSize { size_t blockSize; };
        struct SimilarityBasedBlocks {
            static const size_t atomicBlockSize = 64;
            double thresholdForSimilarity; //will be compared against BlockReport.getDistanceFrom(..)

            SimilarityBasedBlocks(double threshold) : thresholdForSimilarity(threshold){};
        };

        using BlockDelimitingCriteria = std::variant<FixedLengthBlocksSize, SimilarityBasedBlocks>;
        enum IntegerInterpretationCriteria {OneByte,TwoBytes};

    public: //attributes, these are all public
        BlockDelimitingCriteria blockDelimitingCriteria;
        IntegerInterpretationCriteria integerInterpretationCriteria;
        TList tList;
        CCode cCode;


    public: //methods
        Individual() :
            blockDelimitingCriteria(FixedLengthBlocksSize{256}),
            integerInterpretationCriteria(OneByte),
            tList(),
            cCode(C_IdentityCompression) {}

        //getters
        decltype(blockDelimitingCriteria) getBlockDelimitingCriteria() {return blockDelimitingCriteria;};
        bool hasFixedLengthBlockSize() {return std::holds_alternative<FixedLengthBlocksSize>(blockDelimitingCriteria);}
        decltype(FixedLengthBlocksSize::blockSize) getFixedLengthBlockSize() {return std::get<FixedLengthBlocksSize>(blockDelimitingCriteria).blockSize;}
        bool hasSimilarityBasedBlocks() {return !hasFixedLengthBlockSize();}
        decltype(SimilarityBasedBlocks::thresholdForSimilarity) getBlockSimilarityThreshold() {return std::get<SimilarityBasedBlocks>(blockDelimitingCriteria).thresholdForSimilarity;}
        decltype(integerInterpretationCriteria) getIntegerInterpretationCriteria(){return integerInterpretationCriteria;}
        size_t getByteSizeOfIntegers() {
            if (getIntegerInterpretationCriteria() == OneByte) return 1;
            else return 2;
        }
        decltype(tList) getTList();
        TCode getTItem(const size_t index) {return tList[index];}
        CCode getCCode() {return cCode;};





        std::string to_string() {
            std::stringstream ss;
            ss<<"{";
            auto showBlockDelimitingCriteria = [&]() {
                ss<<"BlockDelimitingCriteria:{";
                if (hasFixedLengthBlockSize())
                    ss<<"FixedLength:"<<getFixedLengthBlockSize();
                else
                    ss<<"SimilarityBased Threshold:"<<getBlockSimilarityThreshold();
                ss<<"}";
            };

            auto showIntegerRepresentationCriteria = [&]() {
                size_t byteSize = getByteSizeOfIntegers();
                ss<<"IntegerInterpretationCriteria: "<<byteSize<<" byte"<<(byteSize == 1 ? "" : "s");
            };;

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
            showCCode();ss<<", ";
            showBlockDelimitingCriteria();ss<<", ";
            showIntegerRepresentationCriteria();

            return ss.str();
        }


        void setTItem(const TCode tCode, const size_t index) {
            ASSERT(index < tList.size());
            tList[index] = tCode;
        }





    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
