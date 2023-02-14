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

    /** The compression interface provides a common interface for all of the implementations of other compressions.
     * The minimum requirements are: an empty constructor, compress(const Block&, Writer&), decompress(const& reader) Block, and to_string();
     *
     * To_string is now unused, but was originally for debug purposes.
     */
    class Compression {
    public:
        virtual void compress(const Block& block, AbstractBitWriter& writer){};

        virtual Block decompress(AbstractBitReader& reader){return {};};

        virtual std::string to_string() const = 0;
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP
