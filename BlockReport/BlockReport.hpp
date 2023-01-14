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
        using Frequency = double; //here intented to be normalised, ie 0.5 means that something appeared half of the time
        using Frequencies = std::array<Frequency, AmountOfValues>;
        using Difference = Unit;
        using RunLength = Unit;
        using NormalisedRunLength = double;
    public:
        explicit BlockReport(const Block& block);
        explicit BlockReport();

        std::string to_string() const;
        void log(Logger& log) const;



    public: //structs
        size_t size;
        double entropy;
        StatisticalFeatures unitFeatures;
        StatisticalFeatures frequencyFeatures;
        StatisticalFeatures deltaFeatures;

    public:
        static Frequencies getFrequencyArray(const Block& block);

        static std::vector<int> getDeltaArray(const Block& block);

        static Unit getXorAverage(const Block &block);

        double distanceFrom(const BlockReport& other) const;

        //this is a rough distance metric, which only assumes that the blocks are non-empty
        static double differentialSampleDistance(const Block& A, const Block& B);

        static double getEntropy(const Frequencies& frequencies);

    };
} // GC

#endif //DISS_SIMPLEPROTOTYPE_BLOCKREPORT_HPP
