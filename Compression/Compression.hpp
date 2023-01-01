//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP

#include "../names.hpp"
#include <fstream>
#include "../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"
#include "../names.hpp"
#include "../AbstractBit/AbstractBitReader/AbstractBitReader.hpp"

namespace GC {

    class Compression {
    public:
        virtual void compress(const Block& block, AbstractBitWriter& writer){};

        virtual Block decompress(AbstractBitReader& reader){};

        virtual std::string to_string() const = 0;
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP
