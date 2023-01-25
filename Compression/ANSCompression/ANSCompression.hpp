//
// Created by gian on 21/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_ANSCOMPRESSION_HPP
#define DISS_SIMPLEPROTOTYPE_ANSCOMPRESSION_HPP

#include "../Compression.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../AbstractBit/AbstractBitWriter/AbstractBitWriter.hpp"
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
                exponentOfDenominator = rangeExponent; //i chose this arbtrarly
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
                if (occupied > available) {
                    const size_t toRemove = occupied - available;
                    repeat(toRemove, decreaseFrequency);
                }
            }

            std::string to_string() const {
                Logger logger;
                logger.addVar("exponent", exponentOfDenominator);
                logger.addNamedListFromCollection("frequencies", frequencies, 6);
                return logger.end();
            }

            void encodeIntoWriter2(AbstractBitWriter& writer) const {
                auto encodeFrequency = [&](const size_t freq){
                    writer.writeAmountOfBits(freq, exponentOfDenominator);
                };
                writer.writeSmallAmount(exponentOfDenominator);
                std::for_each(frequencies.begin(), frequencies.end(), encodeFrequency);
            }

            void encodeIntoWriter(AbstractBitWriter& writer) const {
                Unit lastUnit = 0;
                auto encodeFrequency = [&](const Unit which, const size_t freq) {
                    if (freq == 0) return;
                    const size_t diff_from_last_unit = which-lastUnit;
                    writer.writeSmallAmount(diff_from_last_unit);
                    writer.writeSmallAmount(freq);
                    lastUnit = which;
                };

                for (size_t i=0;i<256;i++)
                    encodeFrequency(i, frequencies[i]);
                writer.writeSmallAmount(256-lastUnit);
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

        struct RangeEncoder {
            //types

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
                    Logger logger;
                    logger.addVar("startNum", startNumerator);
                    logger.addVar("endNum", endNumerator);
                    logger.addVar("denExp", exponentOfDenominator);
                    return logger.end();
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
            };


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

            StateRange getRangeOfSymbol(const Unit symbol) {
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

                const size_t worstCaseNextDenominator = currentRange.exponentOfDenominator*2+rangeExponent;
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

        }




    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_IDENTITYCOMPRESSION_HPP
