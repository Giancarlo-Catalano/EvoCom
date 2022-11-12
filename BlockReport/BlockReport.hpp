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
    public:
        explicit BlockReport(const Block& block);

        std::string to_string();



    public: //structs
        size_t size;
        StatisticalFeatures<Unit> unitFeatures;
        StatisticalFeatures<Frequency> frequencyFeatures;
        StatisticalFeatures<Difference> difference2Features, difference3Features, difference4Features;
        StatisticalFeatures<RunLength> runLengthFeatures;
        size_t uniqueSymbolsAmount;

    public:
        static decltype(size) getSize(const Block& block);
        static decltype(unitFeatures) getUnitFeatures(const Block& block);
        static decltype(frequencyFeatures) getFrequencyFeatures(const Block& block);
        static decltype(runLengthFeatures) getRunLengthFeatures(const Block& block);
        static decltype(uniqueSymbolsAmount) getUniqueSymbolsAmount(const Block& block);

        static Frequencies getFrequencyArray(const Block& block);

        static std::vector<RunLength> getRunLengths(const Block &block);

        static std::vector<BlockReport::Difference> getDeltas(const Block& block, const size_t Jump) {
            ASSERT_GREATER(block.size(), Jump-1);
            std::vector<Difference> deltas;
            for (size_t i=0;i<block.size()-1;i++)
                deltas.push_back(block[i+(Jump-1)]-block[i]);
            return deltas;
        }


    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BLOCKREPORT_HPP
