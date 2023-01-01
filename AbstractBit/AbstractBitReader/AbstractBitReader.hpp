//
// Created by gian on 01/01/23.
//

#ifndef EVOCOM_ABSTRACTBITREADER_HPP
#define EVOCOM_ABSTRACTBITREADER_HPP

#include "../../Utilities/utilities.hpp"

namespace GC {

    class AbstractBitReader {
    public:
        //minimum requirements
        virtual bool readBit() = 0;



        virtual size_t readAmountOfBits(const size_t amountOfBits) {
            size_t result = 0;
            for (size_t i = 0; i < amountOfBits; i++) {
                result <<= 1;
                result |= readBit();
            }
            return result;
        }

        virtual Byte readByte() {
            return readAmountOfBits(8);
        }

        virtual std::vector<bool> readVector(const size_t amount) {
            std::vector<bool> result;
            repeat(amount, [&]() {result.push_back(readBit());});
            return result;
        }

        virtual size_t readUnary() {
            size_t result = 0;
            while (!readBit())
                result++;
            return result;
        }

        virtual size_t readSmallAmount() {
            return readRiceEncoded();
        }

        virtual size_t readRiceEncoded() {
            size_t bitSizeRead = readUnary();
            size_t bitSize = (bitSizeRead+1)*2;
            auto getOffsetOfBitSize = [&](const size_t bs) {
                auto pow4 = [&](auto n) {return 1LL<<(n*2);};
                auto sumPow4 = [&](auto n) {return (pow4(n)-1)/3;};
                return sumPow4(bitSize/2)-1;   //this is effectively bitSizeRead..
            };
            size_t contents = readAmountOfBits(bitSize);
            return contents+getOffsetOfBitSize(bitSize);
        }

    };

} // GC

#endif //EVOCOM_ABSTRACTBITREADER_HPP
