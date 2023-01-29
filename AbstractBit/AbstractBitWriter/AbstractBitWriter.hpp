//
// Created by gian on 18/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_ABSTRACTBITWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_ABSTRACTBITWRITER_HPP
#include "../../Utilities/utilities.hpp"


namespace GC {

    /**
     * This class provides an interface through which a function can emit bits.
     * Importantly, it also provides implementations for encodings such as Rice encoding, unary etc..
     * This allows for a bit-emitting function to be able to write in different ways without having to re-implement the
     * bit emission mechanic.
     * In some cases we might want to write onto a file, whereas sometime we want to output a boolean vector.
     * It's even possible to simply count the bits (see BitCounter).
     */
    class AbstractBitWriter {
    public: //types
        using BitAmount = size_t;  //represents a quantity of bits

    public: //minimum requirements
        virtual void pushBit(bool b) = 0;

    public: //functions implemented on top of pushBit
        AbstractBitWriter() {};
        virtual void writeVector(const std::vector<bool>& vec);
        virtual void writeAmountOfBits(const size_t value, const BitAmount amountOfBits);
        virtual void writeByte(const unsigned char value);
        virtual void writeUnary(const size_t value);
        virtual void writeRiceEncoded(const size_t value);



        void writeSmallAmount(const size_t value);

        void writeBigAmount(const size_t value);

        virtual void writeLastByte() = 0;
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_ABSTRACTBITWRITER_HPP
