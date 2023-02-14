//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP

#include "../AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../names.hpp"
#include <fstream>

namespace GC {

    /**
     * Dual of FileBitReader, used to write bits into a file sequentially.
     * Its main service is pushBit(bool), and all fo the functions implemented from it.
     * Implements AbstractBitWriter by holding a buffer, writing bits to it and then writing the buffer when necessary
     * A good reference to understand the implementation is FileBitReader
     */
    class FileBitWriter: public AbstractBitWriter {
    private: //types
        using BitBufferHolder = Byte;
        static const size_t sizeOfBuffer = bitsInType<BitBufferHolder>();

    private: //members
        BitBufferHolder bitBuffer = 0;
        size_t occupied = 0;
        std::ofstream &outStream;

    public:

        FileBitWriter(std::ofstream &_outStream) : outStream(_outStream), occupied(0) {
        };

        void reset();

        void pushBit_unsafe(const bool b);

        virtual void pushBit(const bool b) override;

        void writeLastByte() override;
    };
} // GC

#endif //DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
