//
// Created by gian on 30/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP
#define DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP

#include <random>
#include <functional>
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

        bool choose() {
            return chanceOfTrue >= randomDouble();
        }

        bool shouldDo() {
            return choose();
        }

        template <class Function>
        bool doWithChance(Function func) {
            if (choose()) {
                func();
                return true;
            };
            return false;
        }


        bool doWithChanceOrElse(std::function<void()> ifTrue, std::function<void()> ifFalse) {
            bool isTrue = choose();
            if (isTrue) ifTrue();
            else ifFalse();
            return isTrue;
        }

        Chance getChance() const { return chanceOfTrue;}

        void setChance(const Chance newChance) {
            chanceOfTrue = newChance;

        }
    };


    class FairCoin : public RandomChance{
    private:
    public:
        FairCoin() :
                RandomChance(0.5) {
        }

        bool flip() {return choose();}
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RANDOMCHANCE_HPP
