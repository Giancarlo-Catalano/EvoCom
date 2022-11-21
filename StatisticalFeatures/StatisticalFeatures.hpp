//
// Created by gian on 19/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_STATISTICALFEATURES_HPP
#define DISS_SIMPLEPROTOTYPE_STATISTICALFEATURES_HPP

#include "../Utilities/utilities.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

namespace GC {

    template <class T>
    class StatisticalFeatures {

        using SumType = double;

    public:
        T average;
        T mode;
        T standardDeviation;
        T minimum;
        T maximum;

    public:

        template <class Container>
        static T getAverage(const Container& c) {
            ASSERT_NOT_EMPTY(c);
            SumType sum = 0;
            for (const auto& item : c)
                sum += item;
            return (T)(sum/c.size());
        }

        template <class Container>
        static T getMode(const Container& c) {
            ASSERT_NOT_EMPTY(c);
            //TODO implement this in a more efficient way
            size_t middleIndex = c.size()/2;
            Container sorted = c;
            std::sort(sorted.begin(), sorted.end());
            return sorted[middleIndex];
        }


        template <class Container>
        static T getStandardDeviation(const Container& c) {
            ASSERT_NOT_EMPTY(c);
            T localAverage = getAverage(c);
            auto squared = [](auto n) {return n*n;};
            auto distanceSquaredFromAverage = [&](const T x) {
                return squared(x-localAverage);
            };

            SumType sum = 0;
            for (const auto& x :c)
                sum += distanceSquaredFromAverage(x);

            return std::sqrt(sum/localAverage); //TODO this should be approximated better
        }

        template <class Container>
        static std::pair<T, T> getBounds(const Container& c) {
            ASSERT_NOT_EMPTY(c);
            auto iterators = std::minmax_element(c.begin(), c.end());
            T min = *iterators.first;
            T max = *iterators.second;
            return std::pair<T, T>(min, max);
        }
        template <class Container> //container of T's
        StatisticalFeatures(const Container& c):
            average(getAverage(c)),
            mode(getMode(c)),
            standardDeviation(getStandardDeviation(c)){
                auto bounds = getBounds(c);
                minimum = bounds.first;
                maximum = bounds.second;
        }

        StatisticalFeatures() :
            average(0),
            mode(0),
            standardDeviation(0)
        {
        }

        std::string to_string() {
            auto printableValue = [&](auto item) {
                if (sizeof(item) == 1)
                    return std::to_string((size_t)item);
                else return std::to_string(item);
            };
            std::stringstream ss;
            ss<<"StatisticalFeatures {";
            ss<<"average:"<<printableValue(average)<<", ";
            ss<<"mode:"<<printableValue(mode)<<", ";
            ss<<"std.deviation:"<<printableValue(standardDeviation)<<", ";
            ss<<"minimum:"<<printableValue(minimum)<<", ";
            ss<<"maximum:"<<printableValue(maximum);
            ss<<"}";
            return ss.str();
        }


        template <class Metric>
        double distanceFrom(const StatisticalFeatures<T>& other, const Metric metric) const {
#define GC_SF_DISTANCE_OF_FIELD(field) metric(field, other . field)
            return std::min({GC_SF_DISTANCE_OF_FIELD(average),
                            GC_SF_DISTANCE_OF_FIELD(mode),
                            GC_SF_DISTANCE_OF_FIELD(standardDeviation),
                            GC_SF_DISTANCE_OF_FIELD(minimum),
                            GC_SF_DISTANCE_OF_FIELD(maximum)});
        }
    };



} // GC

#endif //DISS_SIMPLEPROTOTYPE_STATISTICALFEATURES_HPP
