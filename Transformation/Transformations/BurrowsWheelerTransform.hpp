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

#include "../../Dependencies/SAIS/sais.h"


//THIS REALLY NEEDS TO BE OPTIMISIZED
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

            auto getPositionInBWT = [&](const Unit unit, const Amount occurrence) -> Index {
                //0 is the first occurrence, 1 is the second, and so on..
                Amount remainingOccurrences = occurrence;
                for (size_t i=0;i<block.size();i++) {
                    if (block[i]==unit) {
                        if (remainingOccurrences == 0)
                            return i;
                        remainingOccurrences--;
                    }
                }
                return -1; //panic
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



        /////////Experimental

        static bool E_less_lexicographic(const Block& block, const Index startA, const Index startB) {
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
        }


        static BlockWithTerminator E_apply(const Block& block) {
            //setting up the sorted sets
            const auto less_lex_second = [&block](const Index startA, const Index startB) -> bool {
                return E_less_lexicographic(block, startA+1, startB+1);
            };

            using CollectionForSameLetter = std::vector<Index>;
            constexpr size_t amountOfFirstChars = 256;
            std::array<CollectionForSameLetter, 256> setsByFirstChar;

            //for (size_t i=0;i<amountOfFirstChars;i++) setsByFirstChar[i] = SecondarySortedSet(less_lex_second);

            for (size_t i=0;i<block.size();i++) setsByFirstChar[block[i]].push_back(i); //block[i] is an unsigned byte

            //taking the last letter of each rotation
            Block result(block.size());
            size_t indexOfLastInserted = 0;
            Index terminator;

            auto addFromRotation = [&](const Index rotStart) {
                if (rotStart == 0) terminator = indexOfLastInserted;
                else result[indexOfLastInserted++] = block[rotStart-1];
            };

            //since we know that the rotation starting with $ is the lowest one, we can add it "manually"
            addFromRotation(block.size());

            //then we can add the sets
            auto addSet = [&](CollectionForSameLetter& sss) {
                std::sort(sss.begin(), sss.end(), less_lex_second);
                std::for_each(sss.begin(), sss.end(), addFromRotation);
            };

            for (size_t i=0;i<amountOfFirstChars;i++)
                addSet(setsByFirstChar[i]);
            return BlockWithTerminator(result, terminator);
        }
    };


    class BurrowsWheelerTransform : public Transformation{
    private:

        static std::vector<Unit> encodeHeader(const size_t terminator) {
            const size_t bitSize = std::max(floor_log2(terminator), (typeof (terminator)) 1);
            const size_t bytesRequired = greaterMultipleOf(bitSize, 7)/7;
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

        Block LEGACY_apply_copy(const Block& block) const {
            BWT_Helper::BlockWithTerminator blockWithTerminator = BWT_Helper::E_apply(block);
            std::vector<Unit> header = encodeHeader(blockWithTerminator.terminatorPosition);
            Block result = header;
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
            const size_t positionOfTerminator = decodeHeader(header);
            const Block body(block.begin()+header.size(), block.end());
            return BWT_Helper::undo(body, positionOfTerminator);
        }


        Block apply_copy(const Block& block) const {
            const size_t blockLength = block.size();
            unsigned char * transformed  = new Unit[blockLength];
            int * temp = new int[blockLength];
            //transformed.reserve(block.size());
            //temp.reserve(block.size());

            int terminatorPosition = sais_bwt(block.data(), transformed, temp, block.size());

            LOG("The terminator is at", terminatorPosition);

            std::vector<Unit> header = encodeHeader(terminatorPosition);
            Block result = header;
            result.insert(result.end(), transformed, transformed + block.size());

            delete[] transformed;
            delete[] temp;
            return result;
        }

    };


} // GC

#endif //EVOCOM_BURROWSWHEELERTRANSFORM_HPP
