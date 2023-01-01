//
// Created by gian on 18/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_ABSTRACTBITWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_ABSTRACTBITWRITER_HPP
#include "../../Utilities/utilities.hpp"


namespace GC {

    class AbstractBitWriter {
    public:
        using BitAmount = size_t;



        virtual void pushBit(bool b) = 0;

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
