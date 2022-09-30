//
// Created by gian on 30/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP
#define DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP

#include <random>
#include "../names.hpp"

namespace GC {

    class RandomDouble {
    private:
        std::random_device randomDevice;
        std::mt19937 generator;
        std::uniform_real_distribution<double> distribution;

    public:
        RandomDouble(double min, double max):
                randomDevice(),
                generator(randomDevice()),
                distribution(min, max){

        }

        double choose() {
            return distribution(randomDevice);
        }

        double operator()() {
            return choose();
        }
    };

    class RandomChance {
        RandomDouble randomDouble;
        Chance chanceOfTrue;
    public:
        RandomChance(const Chance chanceOfTrue):
                randomDouble(0, 1),
                chanceOfTrue(chanceOfTrue){
        }

        bool flip() {
            return chanceOfTrue >= randomDouble();
        }

        template <class Function>
        bool doWithChance(Function func) {
            if (flip()) {
                func();
                return true;
            };
            return false;
        }

    };


    class FairCoin {
    private:
        RandomChance randomChance;
    public:
        FairCoin() :
                randomChance(0.5){
        }

        bool flip() {
            return randomChance.flip();
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP
