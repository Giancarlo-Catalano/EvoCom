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

        virtual void writeAmountOfBits(const size_t value, const size_t amountOfBits) override { increaseCounter(amountOfBits);}
        virtual void writeUnary(const size_t value) override { increaseCounter(value+1);}
        virtual void writeVector(const std::vector<bool>& vec) { increaseCounter(vec.size());}
        virtual void writeRiceEncoded(const size_t value) {
            auto getFutureBitLength = [&](const size_t n) {
                auto log4 = [&](const size_t x) { return floor_log2(x)/2; };
                return log4((n+2)*3)*2;
            };

            size_t bitLength = getFutureBitLength(value);
            increaseCounter((bitLength/2)+bitLength);
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BITCOUNTER_HPP
