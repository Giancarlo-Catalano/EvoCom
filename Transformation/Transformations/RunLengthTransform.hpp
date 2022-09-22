//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RUNLENGTHTRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_RUNLENGTHTRANSFORM_HPP

#include "../../Utilities/utilities.hpp"
#include "../Transformation.hpp"

namespace GC {

    class RunLengthTransform : public Transformation{

    public:

        std::string to_string() const {return "{RunLengthTransform}";}

        Block apply_copy(const Block& block) const {
            if (block.empty())
                return block;

            Unit repeatingUnit = block[0];
            Unit runLength = 1;
            const size_t maximumStorableRunLength = typeVolume<Unit>()-1;

            Block result;
            auto pushRLPair = [&]() {
                result.push_back(repeatingUnit);
                result.push_back(runLength);
            };
            for (size_t i=1;i<block.size();i++) {
                Unit currentUnit = block[i];
                if (currentUnit == repeatingUnit) {
                    if (runLength == maximumStorableRunLength)
                        pushRLPair();
                    else
                        runLength++;
                }
                else {
                    pushRLPair();
                    repeatingUnit = currentUnit;
                    runLength = 1;
                }
            }
            pushRLPair();

            return result;
        }


        Block undo_copy(const Block& block) const {
            ASSERT_EQUALS(block.size()%2, 0);
            Block result;
            auto unpackRLPairIntoResult = [&result](const Unit repeatingUnit, const Unit runLength) {
                repeat(runLength, [&result, &repeatingUnit](){result.push_back(repeatingUnit);});
            };

            for (size_t i=0;i<block.size();i+=2)  //note that we're reading in pairs
                unpackRLPairIntoResult(block[i], block[i+1]);

            return result;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RUNLENGTHTRANSFORM_HPP
