//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP

#include "../names.hpp"
#include <fstream>
#include "../FileBitWriter/FileBitWriter.hpp"
#include "../FileBitReader/FileBitReader.hpp"
#include "../names.hpp"

namespace GC {

    class Compression {
    public:

        virtual Bits compressIntoBits(const Block&) const = 0;
        virtual void compress(const Block& block, FileBitWriter& writer) {
            Bits bits = compressIntoBits(block);
            writer.writeVector(bits);
        }

        virtual Block decompress(FileBitReader& reader) const = 0;

        virtual std::string to_string() const = 0;
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_COMPRESSION_HPP
