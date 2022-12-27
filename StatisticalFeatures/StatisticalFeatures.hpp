//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_STATISTICALFEATURES_HPP
#define DISS_SIMPLEPROTOTYPE_STATISTICALFEATURES_HPP

#include "../Utilities/utilities.hpp"
#include "../Utilities/JSONer/JSONer.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

namespace GC {

    class StatisticalFeatures {
        using S = double;


    public:
        S average, standardDeviation;
        S firstQuantile, median, thirdQuantile;
        S minimum, maximum;

    public:

        template <class Container>
        static S getAverage(const Container& c) {
            ASSERT_NOT_EMPTY(c);
            S sum = 0;
            for (const auto& item : c)
                sum += item;
            return (S)(sum / c.size());
        }

        template <class Container>
        static S getStandardDeviation(const Container& c, const S precalculatedAverage) {
            ASSERT_NOT_EMPTY(c);
            auto squared = [](auto n) {return n*n;};
            auto distanceSquaredFromAverage = [&](const S x) {
                return squared(x - precalculatedAverage);
            };

            S sum = 0;
            for (const auto& x :c)
                sum += distanceSquaredFromAverage(x);

            return std::sqrt(sum / precalculatedAverage); //TODO this should be approximated better
        }


        template <class Container> //container of T's
        StatisticalFeatures(const Container& c){
            ASSERT_NOT_EMPTY(c);
            average = getAverage(c);
            standardDeviation = getStandardDeviation(c, average);

            Container sorted = c;
            std::sort(sorted.begin(), sorted.end());
            minimum = sorted[0];
            const size_t lastIndex = sorted.size()-1;
            maximum = sorted[lastIndex];
            const size_t middle = lastIndex/2;
            const size_t firstQuartileIndex = lastIndex/4;
            const size_t thirdQuartileIndex = (lastIndex*3)/4;
            firstQuantile = sorted[firstQuartileIndex];
            median = sorted[middle];
            thirdQuantile = sorted[thirdQuantile];
        }

        std::string to_string() const {
            JSONer js("Stats");
            js.pushVar("avg", average);
            js.pushVar("stdev", standardDeviation);
            js.pushVar("min", minimum);
            js.pushVar("firstQ", firstQuantile);
            js.pushVar("median", median);
            js.pushVar("thirdQ", thirdQuantile);
            js.pushVar("max", maximum);
            return js.end();
        }
    };



} // GC

#endif //DISS_SIMPLEPROTOTYPE_STATISTICALFEATURES_HPP
