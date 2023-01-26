//
// Created by gian on 21/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_ANSCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_ANSCOMPRESSION_HPP

#include "../Compression.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../Evolver/Evaluator/BitCounter/BitCounter.hpp"
#include "../../BlockReport/BlockReport.hpp"

namespace GC {

    class ANSCompression : public Compression {
    public: //types

        using Frequencies = GC::BlockReport::Frequencies; //this is an array of 256 doubles
        static constexpr size_t byteValues = 256;
        static const size_t rangeExponent = 4;

        struct QuantizedFrequencies {
            size_t exponentOfDenominator;
            std::array<size_t, byteValues> frequencies;

            QuantizedFrequencies() {};

            static double getMinimumNonZeroFrequency(const Frequencies& freqs) {
                double currentMin = 1.0; //they're at most 1
                std::for_each(freqs.begin(), freqs.end(), [&](const double item) {
                    if (item != 0.0 && item<currentMin) {
                        currentMin = item;
                    }
                });
                return currentMin;

            }


            QuantizedFrequencies(const Frequencies& freqs) {
                exponentOfDenominator = rangeExponent; //i chose this arbitrarly
                LOG("it will use denominator ", exponentOfDenominator);
                const size_t denominator = 1ULL<<exponentOfDenominator;
                auto scaleByDenominator = [&](const double frequency) -> size_t {
                    const size_t result = std::floor(frequency*((double)denominator));
                    if (frequency != 0.0 && result == 0) return 1;
                    return result; //it can't be 0, because the range would become empty for a symbol that is present
                };

                for (size_t i=0;i<byteValues;i++)
                    frequencies[i] = scaleByDenominator(freqs[i]);

                fixExcessiveRangeIfNecessary();
            }

            void fixExcessiveRangeIfNecessary() {
                auto decreaseFrequency = [&]() { //does not introduce zeros
                    size_t* maxFreq = std::max_element(frequencies.begin(), frequencies.end());
                    if (*maxFreq > 1) (*maxFreq)--;
                    else {
                        exponentOfDenominator++; //panic and increase the available range
                    }
                };

                const size_t occupied = std::accumulate(frequencies.begin(), frequencies.end(), 0);
                const size_t available = 1ULL<<exponentOfDenominator;

                auto getMinimumFittingExponent = [&]() {
                    return ceil_log2(occupied);
                };

                if (occupied > available) {
                    exponentOfDenominator = getMinimumFittingExponent();
                }
            }

            std::string to_string() const {
                Logger logger;
                logger.addVar("exponent", exponentOfDenominator);
                logger.addNamedListFromCollection("frequencies", frequencies, 6);
                return logger.end();
            }

            void encodeIntoWriter(AbstractBitWriter& writer) const {
                size_t lastUnit = 0;
                auto encodeFrequency = [&](const Unit which, const size_t freq) {
                    if (freq == 0) return;
                    const size_t diff_from_last_unit = which-lastUnit;
                    LOG("Writing the difference", diff_from_last_unit);
                    writer.writeSmallAmount(diff_from_last_unit);
                    writer.writeSmallAmount(freq);
                    lastUnit = which;
                };
                auto encodeExponent = [&]() {
                    writer.writeSmallAmount(exponentOfDenominator);
                };

                encodeExponent();
                for (size_t i=0;i<256;i++)
                    encodeFrequency(i, frequencies[i]);
                writer.writeSmallAmount(256-lastUnit);
            }

            QuantizedFrequencies(AbstractBitReader& reader) :
                frequencies{0}{

                exponentOfDenominator = reader.readSmallAmount();
                LOG("Read that the exponent is ", exponentOfDenominator);

                size_t lastUnit = 0;
                auto addFrequency = [&]() -> bool{
                    const size_t deltaFromLast = reader.readSmallAmount();
                    lastUnit+=deltaFromLast;
                    LOG("Read that the delta is ", deltaFromLast, "and the lastUnit is now", (int)lastUnit);
                    if (lastUnit > 255) return false;
                    const size_t freq = reader.readSmallAmount(); //note that it only reads this if it's not the last freq
                    frequencies[lastUnit] = freq;
                    LOG("read that the frequency is", freq);
                    return true;
                };

                while (addFrequency()){};
            }
        };

        struct CQF { //stands for cumulative quantized frequencies
            size_t exponentOfDenominator;
            std::array<size_t, byteValues+1> cumulativeFrequencies; //the +1 is because adjacent values will represent the range allocated to each unit

            CQF(const QuantizedFrequencies& qf):
                exponentOfDenominator(qf.exponentOfDenominator)
            {
                size_t sumSoFar = 0;
                for (size_t i=0;i<=byteValues;i++) {
                    cumulativeFrequencies[i] = sumSoFar;
                    sumSoFar += qf.frequencies[i];
                }
                cumulativeFrequencies[byteValues] = (1ULL<<exponentOfDenominator);
            };

            std::string to_string() const {
                Logger logger;
                logger.addVar("exponent", exponentOfDenominator);
                logger.addNamedListFromCollection("frequencies", cumulativeFrequencies, 6);
                return logger.end();
            }

        };

        struct StateRange {
            size_t startNumerator;
            size_t endNumerator;
            size_t exponentOfDenominator;

            StateRange(const size_t startNumerator, const size_t endNumerator, const size_t exponentOfDenominator) :
                    startNumerator(startNumerator),
                    endNumerator(endNumerator),
                    exponentOfDenominator(exponentOfDenominator)
            {

            }

            StateRange():  //sets it to [0, 1)
                    startNumerator(0),
                    endNumerator(1),
                    exponentOfDenominator(0){

            }

            bool isContainedByLeftSide() const {  //assumes end >= start
                if (exponentOfDenominator == 0) return false;
                return endNumerator <= (1<<(exponentOfDenominator-1));
            }

            bool isContainedByRightSide() const { //assumes end >= start
                if (exponentOfDenominator == 0) return false;
                return startNumerator >= (1<<(exponentOfDenominator-1));
            }

            void expandToRight() {
                LOG("Expanding right, the denExp is", exponentOfDenominator);
                ASSERT(exponentOfDenominator > 0);
                exponentOfDenominator --;
            }

            void expandToLeft() {
                LOG("Expanding left, the denExp is", exponentOfDenominator);
                ASSERT(exponentOfDenominator > 0);
                exponentOfDenominator--;
                const size_t numeratorOfOne = 1ULL<<exponentOfDenominator;
                startNumerator -= numeratorOfOne;
                endNumerator -= numeratorOfOne;
            }

            /**
             * Checks if the range can be renormalised, and if so the normalisation gets emitted into the writer
             * @param writer where to emit the renorm, 0 if on left side, 1 on right side
             * @return
             */
            void renormalizeAndEmit(AbstractBitWriter &writer) {
                ASSERT(exponentOfDenominator<32);
                auto renormalizeOnce = [&]() -> bool {
                    LOG("\t called renormalise on ", to_string());
                    if (isContainedByLeftSide()) {
                        LOG("\t contained in the left side");
                        writer.pushBit(0);
                        expandToRight();
                        return true;
                    } else if (isContainedByRightSide()) {
                        LOG("\t contained in the right side");
                        writer.pushBit(1);
                        expandToLeft();
                        return true;
                    }
                    LOG("\t contained in neither side");
                    return false;
                };


                while (renormalizeOnce()){};
                simplifyFraction();

            }

            void halfLengthLeft(){ //assumes that it's not contained by either side
                adaptToDenominator(exponentOfDenominator+1);
                const size_t middleNumerator = (startNumerator+endNumerator)/2;
                endNumerator = middleNumerator;

            }

            void halfLengthRight() {
                adaptToDenominator(exponentOfDenominator+1);
                const size_t middleNumerator = (startNumerator+endNumerator)/2;
                startNumerator = middleNumerator;
            }

            void intersectWith(const StateRange& other) { //assumes there is an intersection
                StateRange tempOther = other;
                putOnSameDenominator(tempOther);
                startNumerator = std::max(startNumerator, other.startNumerator);
                endNumerator = std::min(endNumerator, other.endNumerator);
            }

            void adaptToDenominator(const size_t newDenomExponent) { //assumes newDecomExponent >= exponentOfDenominator
                ASSERT(newDenomExponent >= exponentOfDenominator);
                const size_t differenceInExp = newDenomExponent - exponentOfDenominator;
                const size_t ratio = 1ULL << differenceInExp;
                startNumerator *= ratio;
                endNumerator *= ratio;
                exponentOfDenominator = newDenomExponent;    //technically it should be exponentOfDenominator += differenceInExp
            }

            void multiply(const size_t other_numerator, const size_t other_exponentOfDenominator) {
                startNumerator*=other_numerator;
                endNumerator*=other_numerator;

                exponentOfDenominator += other_exponentOfDenominator;
            }

            void putOnSameDenominator(StateRange& other) {
                const size_t exponentOfLCD = std::max(other.exponentOfDenominator, exponentOfDenominator);
                adaptToDenominator(exponentOfLCD);
                other.adaptToDenominator(exponentOfLCD);
            }

            void applySubRange(const StateRange& symbolRange) {
                //the range length is represented by (endNumerator-startNumerator)/2^(exponentOfDenominator)
                const size_t numeratorOfLength = endNumerator-startNumerator;
                const size_t denominatorOfLength = exponentOfDenominator;
                StateRange temp_other = symbolRange;
                temp_other.multiply(numeratorOfLength, denominatorOfLength);
                putOnSameDenominator(temp_other);
                const size_t startBefore = startNumerator;
                startNumerator = startBefore + temp_other.startNumerator;
                endNumerator = startBefore + temp_other.endNumerator;
                simplifyFraction();
            }

            size_t getDenominator() const {
                return 1ULL << exponentOfDenominator;
            }

            bool isRangeFromZeroToOne() {
                auto isStartAtZero = [&]() {return startNumerator == 0;};
                auto isEndAtOne = [&]() {return endNumerator == getDenominator();};
                return isStartAtZero() && isEndAtOne();
            }

            bool isMostlyOnLeftSide() { //assumes that it's not contained completely on the left or the right
                if (isRangeFromZeroToOne())
                    return true;
                const size_t numeratorOfHalf = getDenominator()>>1;
                const size_t distanceFromStartToHalf = numeratorOfHalf-startNumerator;
                const size_t distanceFromHalfToEnd = endNumerator - numeratorOfHalf;

                return distanceFromStartToHalf >= distanceFromHalfToEnd;
            }

            void setStartToZero() {
                startNumerator = 0;
            }

            void setEndToOne() {
                endNumerator = getDenominator();
            }

            void emitApproximatedRange(AbstractBitWriter& writer) {
                LOG("Called emitApproximatedRange");
                while (!isRangeFromZeroToOne()) {
                    renormalizeAndEmit(writer);
                    LOG("range is still not [0,1]");
                    //should be normalised already
                    if (isMostlyOnLeftSide()) {
                        LOG("the range is mostly on the left side");
                        expandToRight();
                        setEndToOne();
                        writer.pushBit(0);
                    } else { //has to be on the right side
                        LOG("the range is mostly on the right side");
                        expandToLeft();
                        setStartToZero();
                        writer.pushBit(1);
                    }
                    LOG("the range is now ", to_string());
                    //renormalizeAndEmit(writer);
                }
            }

            std::string to_string() const {
                std::stringstream ss;
                ss<<"{start:"<<startNumerator<<", end:"<<endNumerator<<", denExp:"<<exponentOfDenominator<<"}";
                return ss.str();
            }

            std::string to_string_double() const {
                Logger logger;
                double start = (double)startNumerator/(double)getDenominator();
                double end = (double)endNumerator/(double)getDenominator();
                logger.addVar("start", start);
                logger.addVar("end", end);
                return logger.end();
            }
            bool simplifyFraction() {
                LOG("Simplifying");
                auto isEven = [](const size_t x){return x%2==0;};
                auto reduceFraction = [&](){
                    startNumerator/=2;
                    endNumerator/=2;
                    exponentOfDenominator--;
                };

                auto isReducible = [&](){
                    return isEven(startNumerator) && isEven(endNumerator)
                           && (!(startNumerator==0 && endNumerator==0)) && (exponentOfDenominator != 0);
                };
                while (isReducible()){ reduceFraction(); }
            }

            bool isWithinRange(const StateRange& other) const {
                StateRange tempOther = other;
                StateRange tempSelf = *this;
                tempSelf.putOnSameDenominator(tempOther);
                return ((tempSelf.startNumerator>=tempOther.startNumerator) && (tempSelf.endNumerator<=tempOther.endNumerator));
            }
        };

        struct RangeEncoder {
            //types

            StateRange currentRange;
            const CQF& cqf;
            AbstractBitWriter& writer;
            bool justFlushed = false;

            void resetRange() {
                currentRange.startNumerator = 0;
                currentRange.endNumerator = 1;
                currentRange.exponentOfDenominator = 0;
            }


            RangeEncoder(const CQF& cqf, AbstractBitWriter& writer):
                    cqf(cqf),
                    writer(writer),
                    currentRange(){
                resetRange();
            }

            StateRange getRangeOfSymbol(const Unit symbol) const {
                return {cqf.cumulativeFrequencies[symbol],
                        cqf.cumulativeFrequencies[symbol+1],
                        cqf.exponentOfDenominator};
            }

            void addSymbol(const Unit symbol) {
                justFlushed = false;
                LOG("adding the symbol", ((unsigned int)symbol));
                const StateRange symbolRange = getRangeOfSymbol(symbol);
                LOG("The range of the symbol is", symbolRange.to_string());
                currentRange.applySubRange(symbolRange);
                LOG("After applying the range, the current range is ", currentRange.to_string());
                currentRange.renormalizeAndEmit(writer);
                LOG("After renormalising, the current range is ", currentRange.to_string());
                LOG("Before adding a new symbol, the range is", currentRange.to_string_double());

                const size_t worstCaseNextDenominator = currentRange.exponentOfDenominator*2+cqf.exponentOfDenominator;
                if (worstCaseNextDenominator >= 32) {
                    LOG("Reached the limit, flushing");
                    currentRange.emitApproximatedRange(writer);
                    resetRange();
                    justFlushed = true;
                }
            }

            void finish() {
                if (!justFlushed) currentRange.emitApproximatedRange(writer);
            }

        };

        struct RangeDecoder {
            BitCounter dummyWriter;
            RangeEncoder dummyEncoder;
            AbstractBitReader& reader;

            StateRange stateFromInput;
            StateRange currentRange;

            RangeDecoder(const CQF& cqf, AbstractBitReader& reader) :
                dummyWriter(),
                reader(reader),
                dummyEncoder(cqf, dummyWriter),
                stateFromInput(),
                currentRange(){
            }

            void increaseRangeInformation() {
                LOG("Increasing the information content");
                bool nextBit = reader.readBit();
                LOG("read the next bit being ", nextBit);
                if (nextBit)
                    stateFromInput.halfLengthRight();
                else
                    stateFromInput.halfLengthLeft();

                LOG("After increasing information, the informationRange is", stateFromInput.to_string());
                LOG("And the state range is ", currentRange.to_string());
            }

            std::optional<Unit> extractSymbolFromCurrentRange() const {
                LOG("Extracting a symbol from the range");
                for (size_t i = 0;i<256;i++){
                    const StateRange symbolRange = dummyEncoder.getRangeOfSymbol(i);
                    StateRange wouldBeRange = currentRange;
                    LOG("#1:wouldBeRange=", wouldBeRange.to_string());
                    LOG("#2:symbolRange=", symbolRange.to_string());
                    wouldBeRange.applySubRange(symbolRange);
                    LOG("#3:wouldBeRange=", wouldBeRange.to_string());
                    wouldBeRange.simplifyFraction();
                    LOG("#4:wouldBeRange=", wouldBeRange.to_string());


                    if (stateFromInput.isWithinRange(wouldBeRange)) {
                        LOG("Found that the range is completely contained within", i, "'s range:", symbolRange.to_string());
                        return i;
                    }
                }
                LOG("No fitting range found, returning {}");
                return {};
            }

            void resetState() {
                LOG("Resetting the state");
                currentRange = StateRange(); //resets it to 0, 1
                stateFromInput = StateRange();
            }

            Unit getSymbol() {
                LOG("Requested a symbol");
                std::optional<Unit> nextSymbol = extractSymbolFromCurrentRange();

                while (!nextSymbol.has_value()) {
                    increaseRangeInformation();
                    nextSymbol = extractSymbolFromCurrentRange();
                    if (nextSymbol.has_value()) {
                        LOG("nextSymbol has value", (int)nextSymbol.value());
                    }
                    else {
                        LOG("no nextSymbol");
                    }
                }

                const Unit symbol = nextSymbol.value();
                LOG("The symbol is", (int) symbol);
                //dummyEncoder.addSymbol(symbol);
                currentRange.applySubRange(dummyEncoder.getRangeOfSymbol(symbol));
                if (currentRange.exponentOfDenominator*2+dummyEncoder.cqf.exponentOfDenominator) {LOG("FLUSHED!");resetState();};

                return symbol;
            }
        };



    public:

        std::string to_string() const {return "{ANSCompression}";}


        void compress(const Block& block, AbstractBitWriter& writer) const {
            const Frequencies freqs = GC::BlockReport::getFrequencyArray(block);
            const QuantizedFrequencies quantizedFrequencies(freqs);
            const CQF cqf(quantizedFrequencies);

            LOG("The quantizedFrequencies are", quantizedFrequencies.to_string());
            LOG("The cqf are", cqf.to_string());






            //ending
#if 1
            LOG("Encoding the frequencies");
            quantizedFrequencies.encodeIntoWriter(writer);
#endif
            LOG("Writing the block length");
            writer.writeSmallAmount(block.size());
            LOG("Constructing the RangeEncoder");
            RangeEncoder rangeEncoder(cqf, writer);
            LOG("Encoding each of the symbols");
            std::for_each(block.begin(), block.end(), [&](const Unit symbol){rangeEncoder.addSymbol(symbol);});
#if 1
            LOG("Pushing the last data");
            rangeEncoder.finish();
#endif
        }

        Block decompress(AbstractBitReader& reader) const {
            QuantizedFrequencies qf(reader);
            CQF cqf(qf);

            LOG("The qf are", qf.to_string());
            LOG("the cqf are", cqf.to_string());

            const size_t amountOfSymbols = reader.readSmallAmount();
            LOG("We expect", amountOfSymbols, "symbols");

            RangeDecoder decoder(cqf, reader);
            Block result;

            LOG("Starting to decode");
            repeat(amountOfSymbols, [&](){result.push_back(decoder.getSymbol());});
            return result;
        }




    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP
