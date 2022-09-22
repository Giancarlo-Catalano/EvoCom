//
// Created by gian on 15/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_STRIDETRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_STRIDETRANSFORM_HPP
#include "../Transformation.hpp"

namespace GC {

    class StrideTransform : public Transformation {
    public:

        StrideTransform(size_t stride) : stride(stride) { ASSERT_GREATER(stride, 1);};


        const size_t stride;//TODO consider making stride a template parameter

        std::string to_string() const {
            std::stringstream ss;
            ss << "{StrideTransform, stride =" << stride << "}";
            return ss.str();
        }

        Block apply_copy(const Block& block) const {
            //TODO make sure that this works even if block.size() % stride != 0
            std::vector<Block> separations(stride);
            for (size_t index = 0; index < block.size(); index++)
                separations[index%stride].push_back(block[index]);

            Block result;
            auto appendToResult = [&result](const auto& subBlock) {
                result.insert(result.end(), subBlock.begin(), subBlock.end());
            };
            std::for_each(separations.begin(), separations.end(), appendToResult);
            return result;
        }

        Block undo_copy(const Block& block) const {
            auto getSizeOfSeparation = [&](size_t whichSeparation) {
                const size_t blockSize = block.size();
                return (blockSize / stride)+(whichSeparation<(blockSize % stride));
            };

            std::vector<size_t> separationStarts(stride);
            size_t currentSeparationStart = 0;
            for (size_t i = 0;i < stride; i++) {
                separationStarts[i] = currentSeparationStart;
                currentSeparationStart+=getSizeOfSeparation(i);
            }

            Block result;
            auto addFromSeparationIndex = [&](const size_t whichSeparation) {
                result.push_back(block[separationStarts[whichSeparation]++]);
            };

            for (size_t i = 0;i < block.size();i++)
                addFromSeparationIndex(i%stride);

            return result;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_STRIDETRANSFORM_HPP
