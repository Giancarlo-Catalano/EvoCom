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

    /** class used to obtain stastitical data regarding a segment, here also called a Block
     * This class has been changed many times, and slowly became almost obsolete.
     * A lot of code is sub-optimal in both performance and quality, forgive me.
     * The main purpose of this class is static functions such as getFrequencyArray
     */
    class BlockReport {
    public: //types
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
        void log(Logger& log) const; //to convert it into a JSON friendly format



    public: //members
        size_t size;
        double entropy;

        //the following are slow to compute and often unused
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

        static double distributionDistance(const Block& A, const Block& B);


        static double getEntropy(const Frequencies& frequencies);

    };
} // GC

#endif //DISS_SIMPLEPROTOTYPE_BLOCKREPORT_HPP
