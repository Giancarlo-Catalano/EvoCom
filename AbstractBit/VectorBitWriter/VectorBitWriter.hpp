//
// Created by gian on 27/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP

#include "../AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    /** Implementation of AbstractBitWriter to write onto vectors of booleans
     * Mainly used for testing, but also useful to see a minimal implementation of AbstractBitWriter
     */
    class VectorBitWriter : public AbstractBitWriter {
    private: //members
        std::vector<bool> savedBits;
    public:
        VectorBitWriter() : savedBits() {
        }

        virtual void pushBit(const bool b) override {
            savedBits.push_back(b);
        }

        /**
         * Used to get the result of out the class
         * @return
         */
        std::vector<bool> getVectorOfBits() {
            return savedBits;
        }

        /** helper function used for testing, shows how the bits would be grouped together if this had been written onto a file
         * NOTE: this is not implemented elegantly, and is also never used.
         * @return the vector of bytes
         */
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


        /** used so that this can be compatible with the AbstractBitWriter interface, and it mimics what would be done in a file
         * This is not really useful other than allowing compatibility with functions written for AbstractBitWriter.
         */
        void writeLastByte() override {
            const size_t amountOfBits = savedBits.size();
            const size_t roundToBytes = greaterMultipleOf(amountOfBits, 8);
            const size_t bitsToAdd = roundToBytes-amountOfBits;
            repeat(bitsToAdd, [&](){ pushBit(0);});
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP
