//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP

#include "../AbstractBitReader/AbstractBitReader.hpp"
#include "../../Utilities/utilities.hpp"
#include <fstream>

namespace GC {

    /** This is an implementation of AbstractBitReader, and it provides an abstraction to read bits from files
     * A bit is read using the function readBit()
     */
    class FileBitReader : public AbstractBitReader {

    private: //types
        using BitBufferHolder = char;
        using ReadingStream = std::istream;
        static const size_t sizeOfBuffer = sizeof(BitBufferHolder) * 8;


    private: //members
        //the class reads $sizeOfBuffer amount of bits at a time, and stores them in the bitbuffer
        BitBufferHolder bitBuffer = 0;
        size_t readSoFar = 0; //keeps track of how many bits have been read from the buffer
        ReadingStream &inStream; //source of the reading
        const size_t bitSizeOfBuffer = sizeof(BitBufferHolder) * 8;

    public:
        FileBitReader(ReadingStream &_inStream) : inStream(_inStream) { requestNew(); };

        /**
         * Every time this function is called, a successive bit is returned from the file
         * NOTE: bits are obtained from the higher-value part of the bite first, ie big endian
         * @return the read bit
         */
        virtual bool readBit() {
            if (readSoFar >= bitSizeOfBuffer)
                requestNew();
            return readBit_unsafe();
        }

        /**
         * Used to detect when the entirety of the stream has been read
         * (it's not used because it's easier to keep track of the entire size of the file
         * @return true if there's still some bits to read, false if the last bit returned by readBit is the last bit in the file
         */
        bool hasMoreToRead() {
            return !inStream.eof();
        }
    private:

        /**
         * Used to read a new buffer
         */
        void requestNew() {
            inStream.read(&bitBuffer, sizeof(bitBuffer));
            readSoFar = 0;
        }

        /**
         * Reads a bit from the buffer, based on what has been read so far
         * Does not mutate the buffer, only readSoFar
         * @return the bool at position readSoFar (from left) in the buffer
         */
        bool readBit_unsafe() {
            bool result = (bitBuffer >> (bitSizeOfBuffer - 1 - readSoFar)) & 1;
            readSoFar++;
            return result;
        }


    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_FILEBITREADER_HPP
