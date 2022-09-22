//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SUBTRACTXORAVERAGETRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_SUBTRACTXORAVERAGETRANSFORM_HPP

#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../BlockReport/BlockReport.hpp"

namespace GC {

    class SubtractXORAverageTransform : public Transformation{ //Note that is doesn't "subtract", it xors the xoraverage with the units, but the name XORXORAverageTransform is silly
    public:
        std::string to_string() const {return "{DeltaXORTransform}";}

        Block apply_copy(const Block& block) const {
            Unit xorAverage = BlockReport::getXorAverage(block);
            Block result;
            result.push_back(xorAverage);
            for (const auto& unit : block)
                result.push_back(unit ^ xorAverage);
            return result;
        }

        Block undo_copy(const Block& block) const {
            ASSERT_NOT_EMPTY(block);
            Unit xorAverage = block[0];
            Block result(block.size()-1);
            for (size_t i=0;i<result.size();i++)
                result[i] = block[i+1]^xorAverage;
            return result;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SUBTRACTXORAVERAGETRANSFORM_HPP
