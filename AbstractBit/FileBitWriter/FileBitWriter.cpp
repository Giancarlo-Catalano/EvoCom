//
// Created by gian on 17/09/22.
//

#include "FileBitWriter.hpp"

namespace GC {
    void FileBitWriter::reset() {
        bitBuffer = 0;
        occupied = 0;
    }

    void FileBitWriter::pushBit_unsafe(const bool b) {
        //bits are inserted from the right
        bitBuffer <<= 1;
        bitBuffer |= b;
        occupied++;
    }

    void FileBitWriter::pushBit(const bool b) {
        if (occupied == sizeOfBuffer) {
            outStream.put(bitBuffer);
            reset();
        }
        pushBit_unsafe(b);
    }

    void FileBitWriter::writeLastByte() {
        if (occupied != 0) {
            outStream.put(bitBuffer << (sizeOfBuffer - occupied));
        }

    }
} // GC