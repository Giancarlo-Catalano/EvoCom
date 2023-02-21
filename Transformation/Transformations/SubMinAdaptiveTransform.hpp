//
// Created by gian on 12/01/23.
//

#ifndef EVOCOM_SUBMINADAPTIVETRANSFORM_HPP
#define EVOCOM_SUBMINADAPTIVETRANSFORM_HPP
#include "../Transformation.hpp"


namespace GC {

    class SubMinAdaptiveTransform : public Transformation{
    private: //types & constants
        using BlockIter = Block::const_iterator;
        static const size_t peekDistance = 3;

        struct SubsegmentBounds {
            const size_t startIndex;
            const size_t endIndex; //excluded,
            //also if endIndex is 0, it means that the segment continues until the end of the block

            SubsegmentBounds(const size_t startIndex, const size_t endIndex) :
                    startIndex(startIndex),
                    endIndex(endIndex) {}
        };

        struct SubsegmentDescriptor {
            const Unit minimum;
            const size_t length;

            SubsegmentDescriptor(const Unit minimum, const size_t length) :
                minimum(minimum),
                length(length){
            }
        };

    public:

        std::string to_string() const override {return "{SubMinAdaptiveTransform}";};

        Block apply_copy(const Block& block) const override {
            Block result;

            auto getIteratorsOfSubsegment = [&](const SubsegmentBounds& bounds) -> std::pair<BlockIter, BlockIter> {
                if (bounds.endIndex != 0)
                    return {block.begin()+bounds.startIndex, block.begin()+bounds.endIndex};
                else
                    return {block.begin()+bounds.startIndex, block.end()};
            };

            auto processHeader = [&](const SubsegmentBounds& bounds) {
                //LOG("Processing the header with bounds", bounds.startIndex, bounds.endIndex);
                const auto [startIterator, endIterator] = getIteratorsOfSubsegment(bounds);
                const Unit minimum = *std::min_element(startIterator, endIterator);
                const Unit length = bounds.endIndex != 0 ? (bounds.endIndex-bounds.startIndex) : 0; // 0 signifies that the subsegment is until the end
                result.push_back(minimum);
                result.push_back(length);

                std::for_each(startIterator, endIterator, [&](const Unit unit) {
                    result.push_back(unit-minimum);});
                //LOG("Done processing this header");
            };

            //LOG("constructing the subsegments");
            std::vector<SubsegmentBounds> subsegments = getSubsegments(block);
            //LOG("Processing the headers");
            std::for_each(subsegments.begin(), subsegments.end(), processHeader);

            //LOG("returning the results");
            return result;
        }


        Block undo_copy(const Block& block) const override {
            Block result;
            size_t index = 0;

            auto readByte = [&]() -> Unit {return block[index++];};
            auto readStarter = [&]() -> SubsegmentDescriptor {
                return {readByte(), readByte()};
            };

            auto processDescriptor = [&](const SubsegmentDescriptor& descriptor) {
                auto emitUndoneValue = [&](){result.push_back(readByte() + descriptor.minimum);};
                if (descriptor.length != 0)
                    repeat(descriptor.length, [&](){emitUndoneValue();});
                else {
                    while (index < block.size())
                        emitUndoneValue();
                }
            };

            while (index<block.size()) {
                SubsegmentDescriptor descriptor = readStarter();
                //LOG("The descriptor is min=", ((int)descriptor.minimum), ", and the length=", ((int)descriptor.length));
                processDescriptor(descriptor);

            }
            return result;
        }

    private://helpers
        size_t getLocalMinimumDifference (const size_t index, const Block& block) const {
            auto min4 = [](const Unit a, const Unit b, const Unit c, const Unit d) -> Unit{
                return std::min(std::min(a, b), std::min(c, d)); //for cache locality
            };

            auto minOfConsecutive = [&](const size_t index) {
                return min4(block[index], block[index+1], block[index+2], block[index+3]);
            };
            const Unit minLeft = minOfConsecutive(index-peekDistance);
            const Unit minRight = minOfConsecutive(index);
            return safeAbsDifference(minLeft, minRight);
        };


        std::vector<SubsegmentBounds> getSubsegments (const Block& block) const {
            if (block.size() < peekDistance*2) {
                const SubsegmentBounds entirety{0, 0};
                return {entirety};
            }

            std::vector<SubsegmentBounds> subsegments;
            size_t currentStart = 0;
            const size_t threshold = 3;
            const size_t minSegmentSize = 4;
            const size_t maxSegmentSize = 255;

            size_t lastDifference = 0;
            for (size_t i=peekDistance;i<block.size()-peekDistance;i++) {
                //LOG("at i=", i, "the minimum distance is ", getLocalMinimumDifference(i, block));
                if (safeAbsDifference(getLocalMinimumDifference(i, block), lastDifference) >= threshold && isInInterval_inclusive(i-currentStart, minSegmentSize, maxSegmentSize)) {
                    subsegments.emplace_back(currentStart, i+1);
                    currentStart = i+1;
                }
            }
            subsegments.emplace_back(currentStart, 0); //means that the segment is until the end
            return subsegments;
        };

    };

} // GC

#endif //EVOCOM_SUBMINADAPTIVETRANSFORM_HPP
