//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
#define DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP

#include <variant>
#include "../SimpleCompressor/SimpleCompressor.hpp"

namespace GC {

    class Individual {

    public: //types
        using TCode = SimpleCompressor::TransformCode;
        const size_t MaxTransformationsPerBlock = 6;
        using CCode = SimpleCompressor::CompressionCode;

        using TList = std::vector<TCode>;

        struct FixedLengthBlocksSize { size_t blockSize; };
        struct SimilarityBasedBlocks {
            const size_t atomicBlockSize;
            double thresholdForSimilarity; //will be compared against BlockReport.getDistanceFrom(..)
        };

        enum IntegerInterpretationCritera {
            OneByte,
            TwoBytes
        };

        using BlockDelimitingCriteria = std::variant<FixedLengthBlocksSize, SimilarityBasedBlocks>;

        using TAmountPenalty = size_t;

    public:
        BlockDelimitingCriteria blockDelimitingCriteria;
        IntegerInterpretationCritera integerInterpretationCritera;
        TList tList;
        CCode cCode;
        TAmountPenalty tAmountPenalty;



    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_INDIVIDUAL_HPP
