//
// Created by gian on 27/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP

#include "../AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    class VectorBitWriter : public AbstractBitWriter {
    private:
        std::vector<bool> savedBits;
    public:
        VectorBitWriter() : savedBits() {
        }

        virtual void pushBit(const bool b) override {
            savedBits.push_back(b);
        }

        void LOG_INFO() {
            LOG("the pushed bits are:");
            for (size_t i=0;i<savedBits.size();i++) {
                if (i%8 == 0) {
                    LOG_NONEWLINE_NOSPACES("[");
                }
                LOG_NONEWLINE_NOSPACES(savedBits[i]);
                if (i%8 == 7) {
                    LOG_NOSPACES("]");
                }
            }
        }


        std::vector<bool> getVectorOfBits() {
            return savedBits;
        }

        std::vector<Unit> getVectorOfBytes() {
            auto vectorOfBitsToVectorOfBytes = [&](const std::vector<bool>& bitVector) -> std::vector<Unit>{
                std::vector<Unit> result;
                size_t accumulatorUsage = 0;
                Unit accumulator = 0;
                size_t bitsInAccumulator = bitsInType<Unit>();
                for (size_t i=0;i<bitVector.size();i++) {
                    accumulator<<=1;
                    accumulator|=bitVector[i];
                    accumulatorUsage++;
                    if (accumulatorUsage == bitsInAccumulator) {
                        accumulatorUsage = 0;
                        result.push_back(accumulator);
                    }
                }
                if (accumulatorUsage != 0) {
                    result.push_back(accumulator<<(bitsInAccumulator-accumulatorUsage));
                }
                return result;
            };

            return vectorOfBitsToVectorOfBytes(savedBits);
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP
