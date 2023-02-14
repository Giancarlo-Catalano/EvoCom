//
// Created by gian on 17/09/22.
//

#include "FileBitWriter.hpp"

namespace GC {
    /**
     * Resets the buffer and the occupied counter, as if no bits had been read
     */
    void FileBitWriter::reset() {
        bitBuffer = 0;
        occupied = 0;
    }


    /**
     * Pushes a bit without checking that there's space in the buffer
     * @param b the value to be written
     */
    void FileBitWriter::pushBit_unsafe(const bool b) {
        //bits are inserted from the right
        bitBuffer <<= 1;
        bitBuffer |= b;
        occupied++;
    }

    /** Implementation of pushBit
     * It writes onto the buffer, and if the buffer becomes full it writes it and resets itself
     * @param b the value to be written
     */
    void FileBitWriter::pushBit(const bool b) {
        if (occupied == sizeOfBuffer) {
            outStream.put(bitBuffer);
            reset();
        }
        pushBit_unsafe(b);
    }

    /**
     * Writes the leftover bits left in the buffer.
     * This is because it only writes a buffer when the previous one is full, so the very last one needs to be pushed manually.
     * It also makes sure that the last byte is padded on the right so that there are no gaps
     */
    void FileBitWriter::writeLastByte() {
        if (occupied != 0) {
            outStream.put(bitBuffer << (sizeOfBuffer - occupied));
        }

    }
} // GC