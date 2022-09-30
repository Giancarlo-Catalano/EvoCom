//
// Created by gian on 30/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP
#define DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP

#include <random>

namespace GC {

    class RandomChance {
        using Chance = double;
        std::random_device randomDevice;
        std::mt19937 generator;
        std::uniform_real_distribution<double> distribution;

        RandomChance():
                randomDevice(),
                generator(randomDevice()),
                distribution(0, 1){

        }

        template <class Function>
        bool doWithChance(const Chance chanceOfHappening, Function func) {
            if (distribution(randomDevice) <= chanceOfHappening) {
                func();
                return true;
            };
            return false;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP
