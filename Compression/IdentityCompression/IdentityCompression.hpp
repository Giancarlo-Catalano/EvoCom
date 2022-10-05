//
// Created by gian on 21/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP

#include "../Compression.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    class IdentityCompression {
    public:

        std::string to_string() const {return "{IdentityCompression}";}

        Bits compressIntoBits(const Block& block) const {
            auto bitsOfUnit = [&](const Unit& unit) {
                return FileBitWriter::getAmountBits(unit, bitsInType<Unit>());
            };

            Bits result = FileBitWriter::getRiceEncodedBits(block.size()); //needs to encode its size
            for (const auto& unit: block) concatenate(result, bitsOfUnit(unit));
            return result;
        }

        Block decompress(FileBitReader& reader) const {
            Block result;
            auto readAndAppendUnit = [&]() -> Unit {
                result.push_back(reader.readAmount(bitsInType<Unit>()));
            };

            size_t expectedAmount = reader.readRiceEncoded();

            repeat(expectedAmount, readAndAppendUnit);
            return result;
        }




    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP
