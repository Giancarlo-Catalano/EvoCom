//
// Created by gian on 15/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_BLOCKREPORT_HPP
#define DISS_SIMPLEPROTOTYPE_BLOCKREPORT_HPP

#include <vector>
#include <array>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "../names.hpp"
#include "../Utilities/utilities.hpp"
#include "../StatisticalFeatures/StatisticalFeatures.hpp"

namespace GC {

    class BlockReport {
    public:
        static const size_t AmountOfValues = typeVolume<Unit>();
        using Frequency = double;
        using Frequencies = std::array<Frequency, AmountOfValues>;
        using Difference = Unit;
        using RunLength = Unit;
        using NormalisedRunLength = double;
    public:
        explicit BlockReport(const Block& block);
        explicit BlockReport();

        std::string to_string() const;



    public: //structs
        size_t size;
        StatisticalFeatures<Unit> unitFeatures;
        StatisticalFeatures<Frequency> frequencyFeatures;
        StatisticalFeatures<Difference> difference2Features, difference3Features, difference4Features;
        StatisticalFeatures<NormalisedRunLength> normalisedRunLengthFeatures;
        size_t uniqueSymbolsAmount;

    public:
        static decltype(size) getSize(const Block& block);
        static decltype(uniqueSymbolsAmount) getUniqueSymbolsAmount(const Block& block);

        static Frequencies getFrequencyArray(const Block& block);

        static std::vector<RunLength> getRunLengths(const Block &block);

        static std::vector<BlockReport::Difference> getDeltas(const Block& block, const size_t Jump) {
            if (block.size() < Jump) {
                //we can be sure that the size is at least 2
                return getDeltas(block, block.size()); //get the biggest delta you can
            }
            std::vector<Difference> deltas;
            for (size_t i=0;i<block.size()-(Jump-1);i++)
                deltas.push_back(block[i+(Jump-1)]-block[i]);
            return deltas;
        }

        static Unit getXorAverage(const Block &block);

        double distanceFrom(const BlockReport& other) const;
        double distanceFrom_Complex(const BlockReport& other) const;



        std::vector<NormalisedRunLength> getNormalisedRunLengths(const Block &block);
    };

    double differentialSampleDistance(const Block& A, const Block& B);

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BLOCKREPORT_HPP
