//
// Created by gian on 01/01/23.
//

#ifndef EVOCOM_VECTORBITREADER_HPP
#define EVOCOM_VECTORBITREADER_HPP

#include "../AbstractBitReader/AbstractBitReader.hpp"

namespace GC {

    class VectorBitReader: public AbstractBitReader{
        using BoolVec = std::vector<bool>;
    private:
        size_t currentIndex;
        const BoolVec boolVector;
    public:
        VectorBitReader(const BoolVec& boolVec) :
        currentIndex(0),
        boolVector(boolVec){
        }

        virtual bool readBit() override {
            ASSERT_LESS_EQ(currentIndex, boolVector.size()-1);
            return boolVector[currentIndex++];
        }
    };

} // GC

#endif //EVOCOM_VECTORBITREADER_HPP
