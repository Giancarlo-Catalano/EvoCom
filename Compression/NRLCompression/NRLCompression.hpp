//
// Created by gian on 27/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_NRLCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_NRLCOMPRESSION_HPP

#include "../Compression.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../names.hpp"
#include "../../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"

namespace GC {

    class NRLCompression : public Compression{
    public: //types
        using Unit = uint8_t;
        using Amount = size_t;
        struct RLPair {
            Unit unit;
            Amount amount;
        };
    private:
        Unit escapeCharacter = 0xff; //very arbitrary

        std::vector<RLPair> getRLPairs(const Block& block) {
            std::vector<RLPair> pairs;
            Unit currentUnit;
            Amount currentAmount;

            auto startNewRun = [&](const Unit startingUnit){
                currentUnit = startingUnit;
                currentAmount = 1;
            };

            auto finishCurrentRun = [&]() {
                pairs.push_back({currentUnit, currentAmount});
            };

            if (block.empty())
                return pairs;

            startNewRun(block[0]);
            for (size_t i=1;i<block.size();i++) {
                const Unit newUnit = block[i];
                if (newUnit == currentUnit)
                    currentAmount++;
                else {
                    finishCurrentRun();
                    startNewRun(newUnit);
                }
            }
            finishCurrentRun();
            return pairs;
        }

        void encodeEscapedPair(const RLPair& rlPair, AbstractBitWriter& writer) {
            writer.writeByte(escapeCharacter);
            writer.writeByte(rlPair.unit);
            writer.writeSmallAmount(rlPair.amount - 1); //the -1 is because it's always going to be at least 1, and lower values tend to be smaller
        }


        void encodeRLPair(const RLPair& pair, AbstractBitWriter& writer) {
            if (pair.amount > 1 || pair.unit == escapeCharacter)
                encodeEscapedPair(pair, writer);
            else
                writer.writeByte(pair.unit);
        }

        RLPair decodeRLPair(FileBitReader& reader) {
            RLPair result;
            Unit firstByte = reader.readByte();
            if (firstByte == escapeCharacter) {
                result.unit = reader.readByte();
                result.amount = reader.readAmount()+1; //the +1 is due to the -1 we did while encoding
            }
            else {
                result.unit = firstByte;
                result.amount = 1;
            }

            return result;
        }

        std::vector<RLPair> decodeRLPairs(FileBitReader& reader) {
            size_t expectedAmount = reader.readAmount();
            std::vector<RLPair> pairs;
            auto decodeAndSavePair = [&](){pairs.push_back(decodeRLPair(reader));};
            repeat(expectedAmount, decodeAndSavePair);
            return pairs;
        }

        Block expressRLPairs(const std::vector<RLPair>& pairs) {
            Block result;
            auto expressPair = [&](const RLPair& rlPair) {
                repeat(rlPair.amount, [&](){result.push_back(rlPair.unit);});
            };
            std::for_each(pairs.begin(), pairs.end(), expressPair);
            return result;
        }

        void log_pairs(const std::vector<RLPair>& pairs) {
            //LOG("The pairs are");
            auto logPair = [&](const RLPair pair) {
                //LOG("Character: ", (uint16_t)pair.unit, ", Amount:", pair.amount);
            };
            std::for_each(pairs.begin(), pairs.end(), logPair);
        }


    public:
        virtual void compress(const Block& block, AbstractBitWriter& writer) {
            std::vector<RLPair> rlPairs = getRLPairs(block);
            log_pairs(rlPairs);
            writer.writeSmallAmount(rlPairs.size());
            for (const auto& rlPair: rlPairs) encodeRLPair(rlPair, writer);
        }

        virtual Block decompress(FileBitReader& reader) {
            std::vector<RLPair> pairs = decodeRLPairs(reader);
            log_pairs(pairs);
            return expressRLPairs(pairs);
        }

        virtual std::string to_string() const {
            return "New Run Length compression";
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_NRLCOMPRESSION_HPP
