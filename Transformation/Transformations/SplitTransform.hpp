//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SPLITTRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_SPLITTRANSFORM_HPP


#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    class SplitTransform : public Transformation{
    public:

        static const size_t bitsInEachSplit = bitsInType<Unit>()/2;

        std::string to_string() const {
            return "{SplitTransform}";
        }

        Block apply_copy(const Block& block) const {
            Block result;
            auto addSplit = [&result](const Unit u) {
                result.push_back(u>>bitsInEachSplit);
                result.push_back(((Unit)(u<<bitsInEachSplit))>>bitsInEachSplit);
            };
            for (const auto& unit: block)
                addSplit(unit);
            return result;
        }

        Block undo_copy(const Block& block) const {
            Block result;
            auto addJoint = [&result](const Unit first, const Unit second) {
                result.push_back((first<<bitsInEachSplit)|second);
            };
            for (size_t i=0;i<block.size();i+=2)
                addJoint(block[i], block[i+1]);
            return result;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SPLITTRANSFORM_HPP
