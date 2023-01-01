//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP

#include "../AbstractBitReader/AbstractBitReader.hpp"
#include "../../Utilities/utilities.hpp"
#include <fstream>

namespace GC {

    class FileBitReader : public AbstractBitReader {
        using BitBufferHolder = char;
        using ReadingStream = std::istream;
        static const size_t sizeOfBuffer = sizeof(BitBufferHolder) * 8;


        BitBufferHolder bitBuffer = 0;
        size_t readSoFar = 0;
        ReadingStream &inStream;
        size_t bitSizeOfBuffer = sizeof(BitBufferHolder) * 8;

    public:
        FileBitReader(ReadingStream &_inStream) : inStream(_inStream) { requestNew(); };
        virtual bool readBit() {
            if (readSoFar >= bitSizeOfBuffer)
                requestNew();
            return readBit_unsafe();
        }

        bool hasMoreToRead() {
            return !inStream.eof();
        }
    private:

        void requestNew() {
            inStream.read(&bitBuffer, sizeof(bitBuffer));
            readSoFar = 0;
        }

        bool readBit_unsafe() {
            bool result = (bitBuffer >> (bitSizeOfBuffer - 1 - readSoFar)) & 1;
            readSoFar++;
            return result;
        }


    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP
