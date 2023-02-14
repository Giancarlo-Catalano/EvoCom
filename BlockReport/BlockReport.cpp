//
// Created by gian on 15/09/22.
//

#include "BlockReport.hpp"
#include "../Utilities/Logger/Logger.hpp"
#include <math.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <bitset>

namespace GC {
#define ASSERT_BLOCK_NOT_EMPTY() ASSERT_NOT_EMPTY(block)


    /*
     * Creates a BlockReport instance containing statistical information about the given block
     */
    BlockReport::BlockReport(const Block &block):
            size(block.size()),
            entropy(getEntropy(getFrequencyArray(block)))
#if USING_ALL_STATISTICAL_FEATURES == 1
            unitFeatures(block),
            deltaFeatures(getDeltaArray(block)),
            frequencyFeatures()
#endif
    {
        ASSERT_GREATER(block.size(), 1);
        Frequencies frequencies = getFrequencyArray(block);
        entropy = getEntropy(frequencies);
        frequencyFeatures = StatisticalFeatures(frequencies);
    }

    /* sends a description of itself to the logger
     *  Note that it might not
    */
    void BlockReport::log(Logger& logger) const {
        auto pushStats = [&](const std::string& statsName, const  StatisticalFeatures& sf) {
            logger.beginObject(statsName);
            logger.addVar("avg", sf.average);
            logger.addVar("stdev", sf.standardDeviation);
#if USING_ALL_STATISTICAL_FEATURES == 1
            logger.addVar("min", sf.minimum);
            logger.addVar("firstQ", sf.firstQuantile);
            logger.addVar("median", sf.median);
            logger.addVar("thirdQ", sf.thirdQuantile);
            logger.addVar("max", sf.maximum);
#endif
            logger.endObject();
        };
        logger.beginObject("BlockReport");

        logger.addVar("size", size);
        logger.addVar("entropy", entropy);
        pushStats("unitFeatures", unitFeatures);
        pushStats("deltaFeatures", deltaFeatures);
        pushStats("frequencyFeatures", frequencyFeatures);

        logger.endObject(); //ends BlockReport
    }

    // returns itself in string form
    std::string BlockReport::to_string() const{
        Logger logger;
        log(logger);
        return logger.end();
    }

    /**
     * Returns a list of the frequency of each byte value in the given block
     * NOTE: passing an empty block will crash this.
     * @param block the block to be analysed
     * @return an array of 256 doubles, where result[val] is the frequency of val in the block. NOTE the frequency is the proportion of the value in the input, so it is always in the range [0, 1]
     */
    BlockReport::Frequencies BlockReport::getFrequencyArray(const Block &block) {
        ASSERT_BLOCK_NOT_EMPTY();
        Frequencies result;
        size_t blockSize = block.size();
        for (auto& item: result) item = 0;
        auto updateFreq = [&](const Unit unit){result[unit]+=1.0;};
        std::for_each(block.begin(), block.end(), updateFreq);
        auto normalize = [&](Frequency& freq){freq /= blockSize;};
        std::for_each(result.begin(), result.end(), normalize);
        return result;
    }

    /** returns the "XOR average" of the block
     * The nth bit in the result is the bit value which appears most commonly in the nth bits of the input
     * @param block the block to be analysed, cannot be empty.
     * @return the xor average, a single byte
     */
    Unit BlockReport::getXorAverage(const Block &block) {
        ASSERT_BLOCK_NOT_EMPTY();
        const size_t bitsInUnit = bitsInType<Unit>();
        std::array<size_t, bitsInUnit> oneCounts;

        for (size_t i=0;i<bitsInUnit;i++) {
            oneCounts[i] = 0;
        }

        auto registerOnes = [&oneCounts](const Unit& unit) {
            std::bitset<bitsInUnit> bits(unit);
            for (size_t i = 0; i< bitsInUnit; i++) {
                if (bits[i]) oneCounts[i]++;
            }
        };

        auto getAverageBit = [&oneCounts, &block](const size_t pos) {
            const size_t halfSize = block.size()/2;
            return oneCounts[pos] > halfSize;
        };

        std::for_each(block.begin(), block.end(), registerOnes);
        Unit result = 0;
        for (size_t i=0;i<bitsInUnit;i++) {
            result |= getAverageBit(i)<<i;
        }
        return result;
    }

    /**
     * returns the delta transform of the block.
     * This function is vestigial, and shouldn't be used
     * @param block the block to be converted
     * @return the transformed block
     */
    std::vector<int> BlockReport::getDeltaArray(const Block &block) {
        std::vector<int> result(block.size()-1);
        for (size_t i=0;i<result.size();i++) {
            result[i]=block[i+1]-block[i];
        }
        return result;
    }

    /** Vestigial implementation of a distance metric between blocks
     * Returns a distance in [0, 1] describing how similar two blocks are
     * @param A A block (non-empty)
     * @param B A block (non-empty)
     * @return the distance metric, as a double (0 = identical, 1=very different)
     */
    double BlockReport::differentialSampleDistance(const Block &A, const Block &B) {

        const size_t defaultSampleSize = 64;  //to compare, it takes smaller segments of this size
        const size_t sampleSize = std::min({defaultSampleSize, A.size(), B.size()}); //the sample size might be too big, so this is used instead

        // amount is the length of the segments that will be compared
        auto distanceInSubRange = [&A, &B](auto startA, auto startB, const size_t amount) {
            Byte prevDiff = 0;
            size_t integralValue = 0;
            for (size_t iA=startA, iB=startB;iA<startA+amount;iA++, iB++) {
                const int localDiff = std::abs(A[iA]-B[iB]);
                const int derivative = localDiff-prevDiff;
                integralValue+=(derivative);
                prevDiff = localDiff;
            }

            return (double) (integralValue) / ((double) amount);
            //return (double) (integralValue)/((double) 256*amount);//to normalise it into [0, 1]
        };

        const size_t leftoverInA = A.size()-sampleSize;
        const size_t leftoverInB = B.size()-sampleSize;

        //checks the difference in the header, body and tail
        const double headerDistance = distanceInSubRange(0, 0, sampleSize);
        const double tailDistance = distanceInSubRange(leftoverInA, leftoverInB, sampleSize);
        const double bodyDistance = distanceInSubRange(leftoverInA/2, leftoverInB/2, sampleSize);

        //LOG("headerDistance =", headerDistance);
        //LOG("bodyDistance =", bodyDistance);
        //LOG("tailDistance =", tailDistance);

        //returns the minimum of the sampled distances (we want this to be lenient after all..
        return std::min({headerDistance, tailDistance, bodyDistance});
    }

    /** returns the entropy of the given frequency array
     * NOTE: there is no function to calculate directly from the block because there's usually a frequency list present already,
     * and it's more efficient to pass it to this function rather than recalculaiting it.
     * @param frequencies the kind of frequency array returned by ::getFrequencyArray(block)
     * @return the entropy, as a double
     */
    double BlockReport::getEntropy(const BlockReport::Frequencies &frequencies) {
        auto addToAcc = [&](const double acc, const double input) -> double {
            return acc-(input!=0? std::log2(input)*input : 0);
        };
        return std::accumulate(frequencies.begin(), frequencies.end(), (double)0, addToAcc);
    }

    /** The final distance metric between blocks
     * Simply checks how similar the distribution of values is between blocks
     * NOTE: the order of the byte values doesn't actually matter
     * NOTE: the distance will always be positive, but might exceed 1
     * @param A Non-empty block
     * @param B Non-empty block
     * @return the distance between them (0=very similar, 1=very different)
     */
    double BlockReport::distributionDistance(const Block &A, const Block &B) {
        const Frequencies freqsA = getFrequencyArray(A);
        const Frequencies freqsB = getFrequencyArray(B);

        const auto differenceInFrequency = [&](const Unit which) {
            const Frequency freqA = freqsA.at(which);
            const Frequency freqB = freqsB.at(which);
            return std::sqrt(std::abs(square(freqA)-square(freqB)));
        };

        double result = 0;
        for (size_t u = 0;u<256;u++) result += differenceInFrequency(u);

        return result / 2.0;
    }
} // GC