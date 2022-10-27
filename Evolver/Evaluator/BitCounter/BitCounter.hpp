//
// Created by gian on 18/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_BITCOUNTER_HPP
#define DISS_SIMPLEPROTOTYPE_BITCOUNTER_HPP
#include "../../../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"

namespace GC {

    class BitCounter : public AbstractBitWriter {
    private:
        size_t counter = 0;
    public:
        BitCounter():
            counter(0){
        }

        virtual void incrementCounter() { counter++;}
        virtual void increaseCounter(const size_t amount) {counter += amount;}

        virtual size_t getCounterValue() {return counter;}

        virtual void pushBit(bool b) override {incrementCounter();}

        virtual void writeAmount(const size_t value, const size_t amountOfBits) override { increaseCounter(amountOfBits);}
        virtual void writeUnary(const size_t value) override { increaseCounter(value+1);}
        virtual void writeVector(const std::vector<bool>& vec) { increaseCounter(vec.size());}
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BITCOUNTER_HPP
