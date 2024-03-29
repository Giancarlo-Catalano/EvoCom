//
// Created by gian on 21/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP

#include "../Compression.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"

namespace GC {

    class IdentityCompression : public Compression {
    public:

        std::string to_string() const {return "{IdentityCompression}";}

        void compress(const Block& block, AbstractBitWriter& writer) const {
            writer.writeRiceEncoded(block.size());
            for (const auto& unit: block) writer.writeByte(unit);
        }

        Block decompress(AbstractBitReader& reader) const {
            Block result;
            auto readAndAppendUnit = [&]() {
                result.push_back(reader.readAmountOfBits(bitsInType<Unit>()));
            };

            size_t expectedAmount = reader.readRiceEncoded();

            repeat(expectedAmount, readAndAppendUnit);
            return result;
        }




    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP
