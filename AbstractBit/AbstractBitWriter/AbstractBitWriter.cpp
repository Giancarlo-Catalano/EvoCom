//
// Created by gian on 18/10/22.
//

#include "AbstractBitWriter.hpp"

namespace GC {
    void AbstractBitWriter::writeAmountOfBits(const size_t value, const AbstractBitWriter::BitAmount amountOfBits) {
        ASSERT_GREATER(amountOfBits, 0);
        for (int i = amountOfBits - 1; i >= 0; i--)
            pushBit((value >> i) & 1);
    }

    void AbstractBitWriter::writeUnary(const size_t value) {
        repeat(value, [&](){pushBit(0);});
        pushBit(1);
    }

    void AbstractBitWriter::writeSmallAmount(const size_t value) {
        writeRiceEncoded(value);
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
        writeAmountOfBits(value - getOffset(bitLength), bitLength);
    }

    void AbstractBitWriter::writeVector(const std::vector<bool>& vec) {
        for (const bool b: vec) pushBit(b);
    }

    void AbstractBitWriter::writeByte(const unsigned char value) {
        writeAmountOfBits(value, 8);
    }

    void AbstractBitWriter::writeBigAmount(const size_t value) {
        size_t minimumBitsOccupied = floor_log2(value); //also the position of the leftmost i, counted from the right starting from 0
        size_t clusterSize = 4;
        size_t bitsOccupied = ceil_div(minimumBitsOccupied+1, clusterSize);
        size_t declaredClusterAmount = (bitsOccupied/clusterSize)-1;
        LOG("the declared cluster has cluster class", declaredClusterAmount);
        LOG("the payload size is ", (declaredClusterAmount+1)*clusterSize);

        auto power16 = [&](const size_t n) -> size_t {return 1<<(4*n);};
        auto sumOfPowersOf16 = [&](const size_t n) -> size_t {return (power16(n+1)-1)/15;};
        size_t payloadOffset = sumOfPowersOf16(declaredClusterAmount)-1;
        LOG("the payload offset is ", payloadOffset);
        LOG("the payload will be ", value-payloadOffset);


        writeUnary(declaredClusterAmount);
        writeAmountOfBits(value-payloadOffset, bitsOccupied);
    }
} // GC