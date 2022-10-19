//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP

#include "../AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../names.hpp"
#include <fstream>

namespace GC {

    class FileBitWriter: public AbstractBitWriter {
    public:
        using BitBufferHolder = Byte;
        static const size_t sizeOfBuffer = bitsInType<BitBufferHolder>();

        BitBufferHolder bitBuffer = 0;
        size_t occupied = 0;
        std::ofstream &outStream;

        FileBitWriter(std::ofstream &_outStream) : outStream(_outStream), occupied(0) {
        };

        void reset();

        void pushBit_unsafe(const bool b);

        virtual void pushBit(const bool b) override;

        void forceLast();

        virtual void writeRiceEncoded(const size_t value) override { //TODO remove this once it all works, as it's implemented in abstractbit already
            auto getFutureBitLength = [&](const size_t n) {
                auto log4 = [&](const size_t x) { return floor_log2(x)/2; };
                return log4((n+2)*3)*2;
            };

            auto getOffset = [&](const size_t bitSize) {
                //return sum of powers of 4
                auto pow4 = [&](auto n) {return 1LL<<(n*2);};
                auto sumPow4 = [&](auto n) {return (pow4(n)-1)/3;};
                return sumPow4(bitSize/2)-1;
            };

            size_t bitLength = getFutureBitLength(value);
            writeUnary((bitLength/2)-1);
            writeAmount(value-getOffset(bitLength), bitLength);
        }
    };
} // GC

#endif //DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
