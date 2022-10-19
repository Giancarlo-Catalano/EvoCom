//
// Created by gian on 19/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RUNNINGAVERAGE_HPP
#define DISS_SIMPLEPROTOTYPE_RUNNINGAVERAGE_HPP

namespace GC {

    template <class T>
    struct RunningAverage {
    public:
        double weightOfNew;
        bool needsFirstValue = true;
        T currentAverage;
        double deviation = 0;

        RunningAverage(const double weightOfNew): weightOfNew(weightOfNew){};
        RunningAverage(): weightOfNew(0.25){};

        void registerNewValue(const T newValue) {
            if (needsFirstValue) {
                currentAverage = newValue;
                needsFirstValue = false;
            }
            else {
                if (currentAverage != 0) {
                    double newDeviation = (newValue - currentAverage)/currentAverage;
                    deviation = (1.0-weightOfNew)*deviation + weightOfNew*newDeviation;
                }
                currentAverage = (1.0-weightOfNew)*(double)currentAverage + weightOfNew*(double)newValue;
            }
        }

        T getAverage() const {return currentAverage;}

        double getDeviation() const {
            return deviation;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RUNNINGAVERAGE_HPP
