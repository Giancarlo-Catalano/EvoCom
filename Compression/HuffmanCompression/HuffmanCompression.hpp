//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_HUFFMANCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_HUFFMANCOMPRESSION_HPP


#include "../../HuffmanCoder/HuffmanCoder.hpp"
#include "../Compression.hpp"
#include "../../BlockReport/BlockReport.hpp"
#include "../../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"
#include <sstream>
#include <cmath>


namespace GC {

    class HuffmanCompression : public Compression{

    private:
        using Symbol = Unit;
        using Weight = size_t;

        static const size_t frequencyGroupAmount = 16;  //instead of recording the frequency of each possible value, we store frequencies in groups
        static const size_t howManyFrequenciesPerGroup = typeVolume<Symbol>()/frequencyGroupAmount;
        using SmallFrequencyReport = std::array<Weight, frequencyGroupAmount>;
        static const size_t bitSizeOfFrequency = 3;
        static const size_t maxFrequencyEncodable = (1ULL<<bitSizeOfFrequency)-1;

        static size_t indexStartOfGroup(const size_t whichGroup){return whichGroup*howManyFrequenciesPerGroup;};

        static SmallFrequencyReport getSmallFrequencyReport(const Block& block) {
            auto normalFrequencies = GC::BlockReport::getFrequencyArray(block);
            StatisticalFeatures<BlockReport::Frequency> frequencyFeatures(normalFrequencies);
            auto minFreq = frequencyFeatures.minimum;
            auto maxFreq = frequencyFeatures.maximum;

            auto fitFrequencyBetween0AndEncodable = [&](const auto freq) -> Weight{
                if (maxFreq != minFreq)
                    return ((freq-minFreq)*maxFrequencyEncodable/(maxFreq-minFreq))+1;
                else return 1;
            };

            
            auto getGreatestInGroup = [&](const size_t whichGroup) {
                size_t greatestFreqInGroup = 0;
                for (size_t i=indexStartOfGroup(whichGroup); i<indexStartOfGroup(whichGroup+1); i++) {
                    size_t fittedFrequency = fitFrequencyBetween0AndEncodable(normalFrequencies[i]);
                    greatestFreqInGroup = std::max(greatestFreqInGroup, fittedFrequency);
                }

                return greatestFreqInGroup;
            };

            SmallFrequencyReport result;
            for (size_t i=0;i<frequencyGroupAmount;i++)
                result[i] = getGreatestInGroup(i);

            return result;
        }

        static std::vector<std::pair<Symbol, Weight>> expandSmallFrequencyReport(const SmallFrequencyReport& sfr){
            std::vector<std::pair<Symbol, Weight>> result;
            auto addExpandedGroup = [&](const size_t whichGroup) {
                for (size_t i=0;i<howManyFrequenciesPerGroup;i++)
                    result.push_back({indexStartOfGroup(whichGroup)+i, sfr[whichGroup]});
            };

            for (size_t i=0;i<frequencyGroupAmount;i++)
                addExpandedGroup(i);
            return result;
        }




    public:
        HuffmanCompression() = default;

        std::string to_string() const override {
            return "{HuffmanCompression}";
        }

        void compress(const Block& block, AbstractBitWriter& writer) const {
            SmallFrequencyReport smallFrequencyReport = getSmallFrequencyReport(block);
            HuffmanCoder<Symbol, Weight> huffmanCoder(expandSmallFrequencyReport(smallFrequencyReport));

            auto encodeWeight = [&](const Weight w) {
                return writer.writeAmountOfBits(w - 1, bitSizeOfFrequency);
            };

            auto encodeSmallFrequencyReport = [&]() {
                for (const auto& weight : smallFrequencyReport) encodeWeight(weight);
            };
            Bits result;
            HuffmanCoder<Symbol, Weight>::Encoder encoder = huffmanCoder.getEncoder(
                    [&](const std::vector<bool>& vec){writer.writeVector(vec);});

            encodeSmallFrequencyReport();
            writer.writeRiceEncoded(block.size());
            encoder.encodeAll(block);
        }

        Block decompress(FileBitReader& reader) override {
            auto readSingleWeight = [&]() -> Weight {
                return reader.readAmountOfBytes(bitSizeOfFrequency) + 1;
            };

            auto readSmallFrequencyReport = [&]() -> SmallFrequencyReport {
                SmallFrequencyReport sfr;
                for (size_t i=0;i<frequencyGroupAmount;i++) {
                    sfr[i] = readSingleWeight();
                }
                return sfr;
            };

            auto readBlockSize = [&]() -> size_t {
                return reader.readRiceEncoded();
            };




            SmallFrequencyReport sfr = readSmallFrequencyReport();
            LOG("read a small frequency report:", containerToString(sfr));
            HuffmanCoder huffmanCoder(expandSmallFrequencyReport(sfr));

            size_t expectedBlockSize = readBlockSize();
            LOG("expecting a block of size", expectedBlockSize);

            Block result;
            auto pushToResult = [&](const Symbol s) {
                result.push_back(s);
            };

            auto getBitFromReader = [&]() -> bool {return reader.readBit();};

            auto decoder = huffmanCoder.getDecoder(pushToResult, getBitFromReader);
            decoder.decodeAmountOfSymbols(expectedBlockSize);
            return result;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_HUFFMANCOMPRESSION_HPP
