//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RUNLENGTHCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_RUNLENGTHCOMPRESSION_HPP

#include "../Compression.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../names.hpp"

namespace GC {

    class RunLengthCompression : public Compression {
    public:

        using RunLength = size_t;
        using RLPair = std::pair<Unit, RunLength>;

        std::string to_string() const{
            return "{RunLengthCompression}";
        }

        Bits bitsOfRLPair(const RLPair& pair) const {
            ASSERT_NOT_EQUALS(pair.second, 0);
            Bits result = FileBitWriter::getAmountBits(pair.first, bitsInType<Unit>());
            concatenate(result, FileBitWriter::getRiceEncodedBits(pair.second-1));
            return result;
        }

        static std::vector<std::pair<Unit, RunLength>> getRLPairs(const Block& block) {
            if (block.empty())
                return std::vector<RLPair>();

            std::vector<std::pair<Unit, RunLength>> result;
            Unit repeatingCharacter = block[0];
            RunLength runLength = 1;

            auto addPair = [&]() {
                result.push_back({repeatingCharacter, runLength});
            };
            auto startOfNewRun = [&](const Unit newUnit) {repeatingCharacter = newUnit, runLength = 1;};

            for (size_t i=1;i<block.size();i++) {
                Unit currentUnit = block[i];
                if (currentUnit == repeatingCharacter)
                    runLength++;
                else {
                    addPair();
                    startOfNewRun(currentUnit);
                }
            }
            addPair();
            return result;
        }

        Bits compressIntoBits(const Block& block) const {
            std::vector<RLPair> pairs = getRLPairs(block);
            Bits result = FileBitWriter::getRiceEncodedBits(pairs.size());
            for (const auto& item: pairs) concatenate(result, bitsOfRLPair(item));
            return result;
        }

        static RLPair readPair(FileBitReader& reader) {
            return {reader.readAmount(bitsInType<Unit>()), reader.readRiceEncoded()+1};
        }

        static std::vector<RLPair> readPairs(size_t expectedAmount, FileBitReader& reader) {
            std::vector<RLPair> result;
            auto decodeAndPush = [&](){
                result.push_back(readPair(reader));
            };

            repeat(expectedAmount, decodeAndPush);
            return result;
        }

        static Block unpackRLPairs(std::vector<RLPair>& pairs) {
            Block result;
            auto unpackRLPair = [&](RLPair rl) {
                repeat(rl.second, [&](){result.push_back(rl.first);});
            };

            for (const auto& item : pairs) unpackRLPair(item);
            return result;
        }

        Block decompress(FileBitReader& reader) const{
            Block result;
            size_t expectedVectorSize = reader.readRiceEncoded();
            LOG("read that there are ", expectedVectorSize, "RL pairs");
            std::vector<RLPair> pairs = readPairs(expectedVectorSize, reader);
            LOG("The decoded pairs are");
            LOG("The RL pairs are:");
            for (auto item: pairs) {
                LOG_NONEWLINE_NOSPACES("{", ((int)item.first), "; ", item.second, "},");
            }
            LOG("");
            return unpackRLPairs(pairs);
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RUNLENGTHCOMPRESSION_HPP
