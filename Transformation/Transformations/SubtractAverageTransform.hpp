//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SUBTRACTAVERAGETRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_SUBTRACTAVERAGETRANSFORM_HPP

#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../BlockReport/BlockReport.hpp"

namespace GC {

    class SubtractAverageTransform : public Transformation{
    public:
        std::string to_string() const { return "{SubtractAverageTransform}";}

        Block apply_copy(const Block& block) const {
            Unit average = StatisticalFeatures<Unit>(block).average;
            Block result;
            result.push_back(average);
            for (const auto& unit: block)
                result.push_back(unit - average);
            return result;
        }

        Block undo_copy(const Block& block) const {
            ASSERT_NOT_EMPTY(block);
            Unit average = block[0];
            Block result(block.size()-1);
            for (size_t i=0;i<result.size();i++)
                result[i] = block[i+1]+average;
            return result;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SUBTRACTAVERAGETRANSFORM_HPP
