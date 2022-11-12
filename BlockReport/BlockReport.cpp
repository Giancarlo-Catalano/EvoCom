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
            runLengthFeatures(StatisticalFeatures<RunLength>(getRunLengths(block))),
            uniqueSymbolsAmount(getUniqueSymbolsAmount(block))
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

    std::string BlockReport::to_string() {
        std::stringstream ss;
        ss<<"BlockReport {";
        ss<<"size:"<<size<<",\n ";
        ss<<"UnitFeatures:"<<unitFeatures.to_string()<<",\n ";
        ss<<"FrequencyFeatures:"<<frequencyFeatures.to_string()<<",\n ";
        ss<<"Difference2Features:"<<difference2Features.to_string()<<",\n ";
        ss<<"Difference3Features:"<<difference3Features.to_string()<<",\n ";
        ss<<"Difference4Features:"<<difference4Features.to_string()<<",\n ";
        ss<<"RunLengthFeatures:"<<runLengthFeatures.to_string()<<",\n ";
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
} // GC