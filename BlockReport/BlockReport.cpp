//
// Created by gian on 15/09/22.
//

#include "BlockReport.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <set>

namespace GC {
#define ASSERT_BLOCK_NOT_EMPTY() ASSERT_NOT_EMPTY(block)

    BlockReport::BlockReport(const Block &block):
            size(getSize(block)),
            unitFeatures(StatisticalFeatures<Unit>(block)),
            frequencyFeatures(StatisticalFeatures<Frequency>(getFrequencyArray(block))),
            difference2Features(StatisticalFeatures<Difference>(getDeltas(block, 2))),
            difference3Features(StatisticalFeatures<Difference>(getDeltas(block, 3))),
            difference4Features(StatisticalFeatures<Difference>(getDeltas(block, 4))),
            normalisedRunLengthFeatures(StatisticalFeatures<NormalisedRunLength>(getNormalisedRunLengths(block))),
            uniqueSymbolsAmount(getUniqueSymbolsAmount(block))
    {
        ASSERT_GREATER(block.size(), 1);
    }

    //used for placeholder values
    BlockReport::BlockReport() :
        size(0),
        unitFeatures(),
        frequencyFeatures(),
        difference2Features(),
        difference3Features(),
        difference4Features(),
        normalisedRunLengthFeatures(),
        uniqueSymbolsAmount(0)
    {

    }

    decltype(BlockReport::size) BlockReport::getSize(const Block &block) {
        return block.size();
    }

    std::vector<BlockReport::RunLength> BlockReport::getRunLengths(const Block &block) {
        ASSERT_BLOCK_NOT_EMPTY();
        std::vector<RunLength> runLengths = {1};
        for (size_t i=0;i<block.size()-1;i++) {
            if (block[i]==block[i+1])
                runLengths[runLengths.size()-1]++;
            else
                runLengths.push_back(1);
        }
        return runLengths;
    }

    std::vector<BlockReport::NormalisedRunLength> BlockReport::getNormalisedRunLengths(const Block& block) {
        const size_t totalSize = block.size();
        if (totalSize == 0)
            return {};
        else {
            std::vector<RunLength> runLengths = getRunLengths(block);
            std::vector<NormalisedRunLength> normRunLengths(runLengths.size());
            auto normalise = [&](size_t runLength) -> double {return ((double)runLength)/((double)totalSize);};
            std::transform(runLengths.begin(), runLengths.end(), normRunLengths.begin(), normalise);
            return normRunLengths;
        }
    }


    std::string BlockReport::to_string() const{
        std::stringstream ss;
        ss<<"BlockReport {";
        ss<<"size:"<<size<<",\n ";
        ss<<"UnitFeatures:"<<unitFeatures.to_string()<<",\n ";
        ss<<"FrequencyFeatures:"<<frequencyFeatures.to_string()<<",\n ";
        ss<<"Difference2Features:"<<difference2Features.to_string()<<",\n ";
        ss<<"Difference3Features:"<<difference3Features.to_string()<<",\n ";
        ss<<"Difference4Features:"<<difference4Features.to_string()<<",\n ";
        ss<<"RunLengthFeatures:"<<normalisedRunLengthFeatures.to_string()<<",\n ";
        ss<<"UniqueSymbolAmount:"<<uniqueSymbolsAmount<<"\n";
        ss<<"}";
        return ss.str();
    }

    decltype(BlockReport::uniqueSymbolsAmount) BlockReport::getUniqueSymbolsAmount(const Block &block) {
        return std::set<Unit>{block.begin(), block.end()}.size();
    }

    BlockReport::Frequencies BlockReport::getFrequencyArray(const Block &block) {
        Frequencies result;
        size_t blockSize = getSize(block);
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

        auto unit_metric = [&](const auto first, const auto second) { return simpleDistance(first, second, 0, 256); };

        auto frequency_metric = [&](const auto first, const auto second) { return simpleDistance(first, second, 0.0, 1.0); };

        auto runLength_normalised_metric = [&](const auto first, const auto second) { return simpleDistance(first, second, 0.0, 1.0);};

        auto symbolCount_metric = [&](const auto first, const auto second) {return simpleDistance(first, second, 0, 256);};


#define GC_BR_DISTANCE_OF_FIELD(field, metric) field.distanceFrom(other.field, metric)
        return std::min({GC_BR_DISTANCE_OF_FIELD(unitFeatures, unit_metric),
                         GC_BR_DISTANCE_OF_FIELD(frequencyFeatures, frequency_metric),
                         GC_BR_DISTANCE_OF_FIELD(normalisedRunLengthFeatures, runLength_normalised_metric),
                         GC_BR_DISTANCE_OF_FIELD(difference2Features, unit_metric),
                         GC_BR_DISTANCE_OF_FIELD(difference3Features, unit_metric),
                         GC_BR_DISTANCE_OF_FIELD(difference4Features, unit_metric),
                         symbolCount_metric(uniqueSymbolsAmount, other.uniqueSymbolsAmount)});

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
            const size_t halfSize = getSize(block)/2;
            return oneCounts[pos] > halfSize;
        };

        std::for_each(block.begin(), block.end(), registerOnes);
        Unit result = 0;
        for (size_t i=0;i<bitsInUnit;i++) {
            result |= getAverageBit(i)<<i;
        }
        return result;
    }

    //this is a rough distance metric, which only assumes that the blocks are non-empty
    double differentialSampleDistance(const Block& A, const Block& B) {

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