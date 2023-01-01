//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP

#include "../AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../names.hpp"
#include <fstream>

namespace GC {

    class FileBitWriter: public AbstractBitWriter {
    public:
        using BitBufferHolder = Byte;
        static const size_t sizeOfBuffer = bitsInType<BitBufferHolder>();

        BitBufferHolder bitBuffer = 0;
        size_t occupied = 0;
        std::ofstream &outStream;

        FileBitWriter(std::ofstream &_outStream) : outStream(_outStream), occupied(0) {
        };

        void reset();

        void pushBit_unsafe(const bool b);

        virtual void pushBit(const bool b) override;

        void writeLastByte() override;
    };
} // GC

#endif //DISS_SIMPLEPROTOTYPE_FILEBITWRITER_HPP
