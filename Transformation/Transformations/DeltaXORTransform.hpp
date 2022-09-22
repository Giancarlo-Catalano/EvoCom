//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_DELTAXORTRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_DELTAXORTRANSFORM_HPP

#include "../Transformation.hpp"

namespace GC {
    class DeltaXORTransform : public Transformation {

    public:
        DeltaXORTransform() {};

        std::string to_string() const {
            return "{DeltaXORTransform}";
        }

        Block apply_copy(const Block& block) const {
            ASSERT_NOT_EMPTY(block);
            Block result;
            result.push_back(block[0]);
            auto pushDifference = [&](const size_t index) {
                result.push_back(block[index+1]^block[index]);
            };
            for (int i=0;i<block.size()-1;i++)
                pushDifference(i);
            return result;
        }

        Block undo_copy(const Block& block) const {
            ASSERT_NOT_EMPTY(block);
            Block result;
            Unit toBePushed = block[0];
            result.push_back(toBePushed);
            for (int i=1;i<block.size();i++) {
                toBePushed^=block[i];
                result.push_back(toBePushed);
            }
            return result;
        }
    };
}
#endif //DISS_SIMPLEPROTOTYPE_DELTAXORTRANSFORM_HPP
