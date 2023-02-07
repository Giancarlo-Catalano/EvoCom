//
// Created by gian on 07/02/23.
//

#ifndef EVOCOM_SEGMENTDATA_HPP
#define EVOCOM_SEGMENTDATA_HPP
#include "../Utilities/utilities.hpp"
#include "../BlockReport/BlockReport.hpp"
#include "../Utilities/Logger/Logger.hpp"

namespace GC {

    class SegmentData {
    public:
        const size_t size;
        const double entropy;

    private:
        static double getEntropy(const Block& block) {
            const BlockReport::Frequencies frequencies = BlockReport::getFrequencyArray(block);
            auto addToAcc = [&](const double acc, const double input) -> double {
                return acc-(input!=0? std::log2(input)*input : 0);
            };

            return std::accumulate(frequencies.begin(), frequencies.end(), (double)0, addToAcc);
        }

    public:
        SegmentData(const Block& block):
            size(block.size()),
            entropy(getEntropy(block)){
        }

        void log(Logger& logger) const {
            logger.beginObject("SegmentData");
            logger.addVar("size", size);
            logger.addVar("entropy", entropy);
            logger.endObject();
        }

    };

} // GC

#endif //EVOCOM_SEGMENTDATA_HPP
