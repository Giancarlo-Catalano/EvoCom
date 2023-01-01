//
// Created by gian on 30/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_SMALLVALUECOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_SMALLVALUECOMPRESSION_HPP


#include "../Compression.hpp"

namespace GC {

    class SmallValueCompression : public Compression {
    public:

        std::string to_string() const {return "{SmallValueCompression}";}

        void compress(const Block& block, AbstractBitWriter& writer) const {
            writer.writeSmallAmount(block.size());
            for (const auto& unit: block) writer.writeSmallAmount(unit);
        }

        Block decompress(AbstractBitReader& reader) const {
            Block result;
            auto readAndAppendUnit = [&]() -> Unit {
                result.push_back(reader.readSmallAmount());
            };

            size_t expectedAmount = reader.readSmallAmount();
            repeat(expectedAmount, readAndAppendUnit);
            return result;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_SMALLVALUECOMPRESSION_HPP
