//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP

#include "../Utilities/utilities.hpp"
#include <fstream>

namespace GC {

    struct FileBitReader {
        using BitBufferHolder = char;
        static const size_t sizeOfBuffer = sizeof(BitBufferHolder) * 8;


        BitBufferHolder bitBuffer = 0;
        size_t readSoFar = 0;
        std::ifstream &inStream;
        size_t bitSizeOfBuffer = sizeof(BitBufferHolder) * 8;

        FileBitReader(std::ifstream &_inStream) : inStream(_inStream) { requestNew(); };

        void requestNew() {
            inStream.read(&bitBuffer, sizeof(bitBuffer));
            readSoFar = 0;
        }

        bool readBit_unsafe() {
            bool result = (bitBuffer >> (bitSizeOfBuffer - 1 - readSoFar)) & 1;
            readSoFar++;
            return result;
        }

        bool readBit() {
            if (readSoFar >= bitSizeOfBuffer)
                requestNew();
            return readBit_unsafe();
        }

        size_t readAmountOfBits(const size_t amountOfBits) {
            size_t result = 0;
            for (size_t i = 0; i < amountOfBits; i++) {
                result <<= 1;
                result |= readBit();
            }
            return result;
        }

        Byte readByte() {
            return readAmountOfBits(8);
        }

        std::vector<bool> readVector(const size_t amount) {
            std::vector<bool> result;
            repeat(amount, [&]() {result.push_back(readBit());});
            return result;
        }

        size_t readUnary() {
            size_t result = 0;
            while (!readBit())
                result++;
            return result;
        }

        size_t readSmallAmount() {
            return readRiceEncoded();
        }

        size_t readRiceEncoded() {
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

#endif //DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP
