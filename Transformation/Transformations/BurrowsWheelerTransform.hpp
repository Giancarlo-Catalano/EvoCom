//
// Created by gian on 20/12/22.
//

#ifndef EVOCOM_BURROWSWHEELERTRANSFORM_HPP
#define EVOCOM_BURROWSWHEELERTRANSFORM_HPP

#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>

#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../AbstractBit/VectorBitWriter/VectorBitWriter.hpp"

namespace GC {

    using Index = size_t;
    using Amount = size_t;

    class BWT_Helper {
    public:
        struct BlockWithTerminator {
            const Block block;
            const Index terminatorPosition;

            BlockWithTerminator(const Block& block, const Index terminatorPosition) :
                    block(block), terminatorPosition(terminatorPosition) {}
        };

        static BlockWithTerminator apply(const Block& block) {
            const auto less_lexicographic = [&block](const Index startA, const Index startB) -> bool {
                const size_t blockSize = block.size();
                const auto validIndex = [&blockSize](const Index index) {return index < blockSize;};
                const auto pointsToTerminator = [&blockSize](const Index index) {return index==blockSize;};

                Index fromA = startA;
                Index fromB = startB;
                while (validIndex(fromA) && validIndex(fromB) && (block[fromA] == block[fromB])) {  //progress while the prefixes are the same
                    fromA++;fromB++;
                }
                if (pointsToTerminator(fromA)) return true;       //$ less than everything
                if (pointsToTerminator(fromB)) return false;      //everything greater than $
                return (block[fromA] < block[fromB]);
            };






            std::set<Index, decltype(less_lexicographic)> sorted(less_lexicographic);
            for (size_t i = 0;i<=block.size();i++) sorted.insert(i); //note how block.size is included because the terminator also counts
            LOG("The order of the sorted rotation starts is", containerToString(sorted));

            Block result(block.size());
            size_t indexOfLastInserted = 0;

            Index terminator;
            auto addFromRotation = [&](const Index rotStart) {
                if (rotStart == 0) terminator = indexOfLastInserted;
                else
                    result[indexOfLastInserted++] = block[rotStart-1];
            };

            std::for_each(sorted.begin(), sorted.end(),addFromRotation);

            return BlockWithTerminator(result, terminator);
        }


        static Block undo(const Block& block, const Index terminatorPosition) {
            Block sorted = block;
            std::sort(sorted.begin(), sorted.end());
            std::array<Amount, 256> characterOffsetTable{0}; //I will assume the initial values are all empty

            auto countOccurrencesOfUnits = [&](){
                for (const Unit unit: block) {
                    characterOffsetTable[unit]++;
                }
            };

            countOccurrencesOfUnits();

            auto makeOffsetTableCumulative = [&]() {
                for (size_t i=1;i<characterOffsetTable.size();i++) {
                    characterOffsetTable[i] += characterOffsetTable[i-1];
                }

                for (size_t i=characterOffsetTable.size()-1;i>0;i--) {
                    characterOffsetTable[i] = characterOffsetTable[i-1];
                }
                characterOffsetTable[0] = 0;
            };

            makeOffsetTableCumulative();
            Block result;

            auto findCharacterOffset = [&](const Unit unit) -> Amount{
                return characterOffsetTable[unit];
            };

            auto getPositionInBWT = [&](const Unit unit, const Amount occurrence) -> Index{
                //0 is the first occurrence, 1 is the second, and so on..
                Amount remainingOccurrences = occurrence;
                for (size_t i=0;i<block.size();i++) {
                    if (block[i]==unit) {
                        if (remainingOccurrences == 0)
                            return i;
                        remainingOccurrences--;
                    }
                }
            };

            Index positionInSorted = terminatorPosition;
            while (positionInSorted != 0) {
                Unit characterToBeAdded = sorted[positionInSorted-1];
                result.push_back(characterToBeAdded);
                const Amount whichOccurrenceOfTheChar = positionInSorted - findCharacterOffset(characterToBeAdded) - 1;
                const Index positionInBWT = getPositionInBWT(characterToBeAdded, whichOccurrenceOfTheChar);
                positionInSorted = positionInBWT + (positionInBWT >= terminatorPosition);
            }

            return result;
        }
    };


    class BurrowsWheelerTransform : public Transformation{
    private:

        static size_t greaterMultipleOf(const size_t input, const size_t multipleOf) {
            if (input == 0)
                return 0;
            else
                return (((input-1)/multipleOf)+1)*multipleOf;
        }

        static std::vector<Unit> encodeHeader(const size_t terminator) {
            //LOG"Encoding the header");
            const size_t bitSize = std::max(floor_log2(terminator), 1UL);
            //LOG"The bit size is", bitSize);
            const size_t bytesRequired = greaterMultipleOf(bitSize, 7)/7;
            //LOG"The bytes required are", bytesRequired);

            std::vector<Unit> result;
            for (size_t i=0;i<bytesRequired;i++) {
                const Unit septet = (terminator>>(7*(bytesRequired-1-i)))&0x7f;
                const Unit theresMore = (i!=(bytesRequired-1))<<7;
                result.push_back(septet | theresMore);
            }
            return result;
        };

        static size_t decodeHeader(const std::vector<Unit>& header) {
            size_t result = 0;
            for (const Unit byte : header) {
                result<<=7;
                result|=(byte&0x7f);
            }
            return result;
        }

    public:
        std::string to_string() const { return "{BWTransform}";}
        Block apply_copy(const Block& block) const {
            BWT_Helper::BlockWithTerminator blockWithTerminator = BWT_Helper::apply(block);
            //LOG"The terminator position is", blockWithTerminator.terminatorPosition);
            std::vector<Unit> header = encodeHeader(blockWithTerminator.terminatorPosition);
            Block result = header;
            /*
            LOG_NONEWLINE("Obtained the following header:");
            for (const auto item: header) {
                LOG_NONEWLINE_NOSPACES("[", ((unsigned int)item), "]");
            }*/
            //LOG"");
            result.insert(result.end(), blockWithTerminator.block.begin(), blockWithTerminator.block.end());
            return result;
        }
        Block undo_copy(const Block& block) const {
            auto isEndOfHeader = [&](const Unit byte) -> bool { //a byte is the end of the header if the first bit from the left is 0
                return !(byte>>7);
            };

            std::vector<Unit> header;
            for (const Unit byte: block) {
                header.push_back(byte);
                if (isEndOfHeader(byte))
                    break;
            }
            /*
            LOG_NONEWLINE("Obtained the following header:");
            for (const auto item: header) {
                LOG_NONEWLINE_NOSPACES("[", ((unsigned int)item), "]");
            }*/
            //LOG"");
            const size_t positionOfTerminator = decodeHeader(header);
            //LOG"The position of the terminator is", positionOfTerminator);
            const Block body(block.begin()+header.size(), block.end());

            return BWT_Helper::undo(body, positionOfTerminator);
        }

    };


} // GC

#endif //EVOCOM_BURROWSWHEELERTRANSFORM_HPP
