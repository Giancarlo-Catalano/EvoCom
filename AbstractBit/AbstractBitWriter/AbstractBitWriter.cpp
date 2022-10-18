//
// Created by gian on 18/10/22.
//

#include "AbstractBitWriter.hpp"

namespace GC {
    void AbstractBitWriter::writeAmount(const size_t value, const AbstractBitWriter::BitAmount amountOfBits) {
        ASSERT_GREATER(amountOfBits, 0);
        for (int i = amountOfBits - 1; i >= 0; i--)
            pushBit((value >> i) & 1);
    }

    void AbstractBitWriter::writeUnary(const size_t value) {
        repeat(value, [&](){pushBit(0);});
        pushBit(1);
    }

    void AbstractBitWriter::writeRiceEncoded(const size_t value) {
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

    void AbstractBitWriter::writeVector(const std::vector<bool>& vec) {
        for (const bool b: vec) pushBit(b);
    }
} // GC