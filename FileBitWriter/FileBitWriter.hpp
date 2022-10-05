//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP

#include "../Utilities/utilities.hpp"
#include "../names.hpp"
#include <fstream>

namespace GC {

    struct FileBitWriter {
        using BitBufferHolder = Byte;
        static const size_t sizeOfBuffer = bitsInType<BitBufferHolder>();

        BitBufferHolder bitBuffer = 0;
        size_t occupied = 0;
        std::ofstream &outStream;

        size_t writtenBuffers = 0;
        FileBitWriter(std::ofstream &_outStream) : outStream(_outStream), occupied(0) {
        };

        void reset() {
            bitBuffer = 0;
            occupied = 0;
        }

        void pushBit_unsafe(const bool b) {
            //bits are inserted from the right
            bitBuffer <<= 1;
            bitBuffer |= b;
            occupied++;
        }

        void pushBit(const bool b) {
            if (occupied == sizeOfBuffer) {
                outStream.put(bitBuffer);
                writtenBuffers++;
                reset();
            }
            pushBit_unsafe(b);
        }

        void writeAmount(std::size_t amount, std::size_t amountOfBits) {
            writeVector(getAmountBits(amount, amountOfBits));
        }

        static Bits getAmountBits(std::size_t amount, std::size_t amountOfBits) {
            Bits result;
            ASSERT_GREATER(amountOfBits, 0);
            for (int i = amountOfBits - 1; i >= 0; i--)
                result.push_back((amount >> i) & 1);
            return result;
        }



        void forceLast() {
            if (occupied != 0) {
                outStream.put(bitBuffer << (sizeOfBuffer - occupied));
                writtenBuffers++;
            }

        }

        size_t getWrittenBufferAmount() {
            return writtenBuffers;
        }

        void writeVector(const std::vector<bool>& vec) {
            std::for_each(vec.begin(), vec.end(), [&](const bool b){pushBit(b);});
        }

        void writeUnary(const size_t input) {
            writeVector(getUnaryBits(input));
        }

        static Bits getUnaryBits(const size_t input) {
            Bits result;
            repeat(input, [&](){ result.push_back(0);});
            result.push_back(1);
            return result;
        }

        void writeRiceEncoded(size_t input) {
            writeVector(getRiceEncodedBits(input));
        }

        static Bits getRiceEncodedBits(size_t input) {
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

            size_t bitLength = getFutureBitLength(input);
            Bits unaryLengthHint = getUnaryBits((bitLength/2)-1);
            Bits amountHint = getAmountBits(input-getOffset(bitLength), bitLength);
            Bits result;
            result.insert(result.end(), unaryLengthHint.begin(), unaryLengthHint.end());
            result.insert(result.end(), amountHint.begin(), amountHint.end());
            return result;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
