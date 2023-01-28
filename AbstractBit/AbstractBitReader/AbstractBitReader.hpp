//
// Created by gian on 01/01/23.
//

#ifndef EVOCOM_ABSTRACTBITREADER_HPP
#define EVOCOM_ABSTRACTBITREADER_HPP

#include "../../Utilities/utilities.hpp"

namespace GC {

    /**
     * This class is an interface which provides a mechanic to read bits from an undisclosed source.
     * It is the dual of AbstractBitWriter, and for every behavior in this class there is a symmetry in AbstractBitReader.
     * The main usage of this class is reader.readBit(), .readUnary(), .readSmallAmount(), readAmountOfBits()
     */
    class AbstractBitReader {
    public: //minimum requirements
        virtual bool readBit() = 0;

    public: //functions implemented on top of readBit
        virtual size_t readAmountOfBits(const size_t amountOfBits);

        virtual Byte readByte();

        virtual std::vector<bool> readVector(const size_t amount);

        virtual size_t readUnary();

        virtual size_t readSmallAmount();

        virtual size_t readRiceEncoded();

    };

} // GC

#endif //EVOCOM_ABSTRACTBITREADER_HPP
