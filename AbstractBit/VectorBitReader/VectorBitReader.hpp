//
// Created by gian on 01/01/23.
//

#ifndef EVOCOM_VECTORBITREADER_HPP
#define EVOCOM_VECTORBITREADER_HPP

#include "../AbstractBitReader/AbstractBitReader.hpp"

namespace GC {

    /** An implementation of AbstractBitReader, which can read from a vector of booleans
     * This is mainly used for testing purposes, but it's also a good example of how a minimal
     * implementation of AbstractBitReader is like
     */
    class VectorBitReader: public AbstractBitReader{
    private: //types
        using BoolVec = std::vector<bool>;
    private: //members
        size_t currentIndex;
        const BoolVec boolVector;
    public: //services
        VectorBitReader(const BoolVec& boolVec) :
        currentIndex(0),
        boolVector(boolVec){
        }

        //TODO: perhaps a move constructor for boolVec would be more efficient?

        /** Implementation of readBit
         * @return the bool from the source vector
         */
        virtual bool readBit() override {
            ASSERT_LESS_EQ(currentIndex, boolVector.size()-1); //the assertion is useful because when something goes wrong in testing, this will be activated
            return boolVector[currentIndex++];
        }
    };

} // GC

#endif //EVOCOM_VECTORBITREADER_HPP
