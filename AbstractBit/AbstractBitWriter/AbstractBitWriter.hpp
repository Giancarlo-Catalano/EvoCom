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

        AbstractBitWriter() {}

        virtual void pushBit(bool b) = 0;

        virtual void writeVector(const std::vector<bool>& vec);
        virtual void writeAmount(const size_t value, const BitAmount amountOfBits);
        virtual void writeUnary(const size_t value);
        virtual void writeRiceEncoded(const size_t value);



    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_ABSTRACTBITWRITER_HPP
