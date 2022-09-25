//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
#define DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP

#include <variant>
#include <sstream>
#include "../SimpleCompressor/SimpleCompressor.hpp"

namespace GC {

    class Individual {

    public: //types
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

        using TList = std::vector<TCode>;

        struct FixedLengthBlocksSize { size_t blockSize; };
        struct SimilarityBasedBlocks {
            const size_t atomicBlockSize = 64;
            double thresholdForSimilarity; //will be compared against BlockReport.getDistanceFrom(..)
        };

        using BlockDelimitingCriteria = std::variant<FixedLengthBlocksSize, SimilarityBasedBlocks>;
        enum IntegerInterpretationCriteria {OneByte,TwoBytes};

    public:
        BlockDelimitingCriteria blockDelimitingCriteria;
        IntegerInterpretationCriteria integerInterpretationCriteria;
        TList tList;
        CCode cCode;


        Individual() :
            blockDelimitingCriteria(FixedLengthBlocksSize{256}),
            integerInterpretationCriteria(OneByte),
            tList(),
            cCode(C_IdentityCompression) {}


        bool hasFixedLengthBlockSize() {return holds_alternative<FixedLengthBlocksSize>(blockDelimitingCriteria);}
        decltype(FixedLengthBlocksSize::blockSize) getFixedLengthBlockSize() {return std::get<FixedLengthBlocksSize>(blockDelimitingCriteria).blockSize;}
        bool hasSimilarityBasedBlocks() {return !hasFixedLengthBlockSize();}
        decltype(SimilarityBasedBlocks::thresholdForSimilarity) getBlockSimilarityThreshold() {return std::get<SimilarityBasedBlocks>(blockDelimitingCriteria).thresholdForSimilarity;}
        decltype(integerInterpretationCriteria) getIntegerInterpretationCriteria(){return integerInterpretationCriteria;}

        std::string to_string() {
            std::stringstream ss;

            return ss.str();
        }





    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
