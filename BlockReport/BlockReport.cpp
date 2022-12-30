//
// Created by gian on 15/09/22.
//

#include "BlockReport.hpp"
#include "../Utilities/Logger/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <set>

namespace GC {
#define ASSERT_BLOCK_NOT_EMPTY() ASSERT_NOT_EMPTY(block)

    //I'm gutting this!!!

    BlockReport::BlockReport(const Block &block):
            size(block.size()),
            unitFeatures(block),
            deltaFeatures(getDeltaArray(block)),
            frequencyFeatures(getFrequencyArray(block))

    {
        ASSERT_GREATER(block.size(), 1);
    }

    void BlockReport::log(Logger& logger) const {
        auto pushStats = [&](const std::string& statsName, const  StatisticalFeatures& sf) {
            logger.beginObject(statsName);
            logger.addVar("avg", sf.average);
            logger.addVar("stdev", sf.standardDeviation);
            logger.addVar("min", sf.minimum);
            logger.addVar("firstQ", sf.firstQuantile);
            logger.addVar("median", sf.median);
            logger.addVar("thirdQ", sf.thirdQuantile);
            logger.addVar("max", sf.maximum);
            logger.endObject();
        };
        logger.beginObject("BlockReport");

        logger.addVar("Size", size);
        pushStats("unitFeatures", unitFeatures);
        pushStats("deltaFeatures", deltaFeatures);
        pushStats("frequencyFeatures", frequencyFeatures);

        logger.endObject(); //ends BlockReport
    }

    std::string BlockReport::to_string() const{
        Logger logger;
        log(logger);
        return logger.end();
    }

    BlockReport::Frequencies BlockReport::getFrequencyArray(const Block &block) {
        Frequencies result;
        size_t blockSize = block.size();
        for (auto& item: result) item = 0;
        auto updateFreq = [&](const Unit unit){result[unit]+=1.0;};
        std::for_each(block.begin(), block.end(), updateFreq);
        auto normalize = [&](Frequency& freq){freq /= blockSize;};
        std::for_each(result.begin(), result.end(), normalize);
        return result;
    }

    double BlockReport::distanceFrom(const BlockReport& other) const {

        auto simpleDistance = [&](const auto first, const auto second, const auto min, const auto max) -> double {
            return (double) safeAbsDifference(first, second) / (max-min);
        };
        return 0;

    }

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

    std::vector<int> BlockReport::getDeltaArray(const Block &block) {
        std::vector<int> result(block.size()-1);
        for (size_t i=0;i<result.size();i++) {
            result[i]=block[i+1]-block[i];
        }
        return result;
    }

    double BlockReport::differentialSampleDistance(const Block &A, const Block &B) {

        const size_t defaultSampleSize = 64;
        const size_t sampleSize = std::min({defaultSampleSize, A.size(), B.size()});

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

        const double headerDistance = distanceInSubRange(0, 0, sampleSize);
        const double tailDistance = distanceInSubRange(leftoverInA, leftoverInB, sampleSize);
        const double bodyDistance = distanceInSubRange(leftoverInA/2, leftoverInB/2, sampleSize);

        //LOG("headerDistance =", headerDistance);
        //LOG("bodyDistance =", bodyDistance);
        //LOG("tailDistance =", tailDistance);

        return std::min({headerDistance, tailDistance, bodyDistance});



    }
} // GC