//
// Created by gian on 29/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RANDOMINT_HPP
#define DISS_SIMPLEPROTOTYPE_RANDOMINT_HPP
#include <random>

namespace GC {

    template <class I>
    class RandomInt {
    private:
        std::random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<I> distr;

    public:
        RandomInt(const I min, const I max) :
            rd(),
            gen(rd()),
            distr(min, max){
        }

        RandomInt() {
            RandomInt(0, 1);
        }

        //mutates it
        I chooseInRange(const I min, const I max) {
            setBounds(min, max);
            return choose();
        }

        I choose() {
            return distr(gen);
        }

        I operator()(){
            return choose();
        }

        void setBounds(const I min,const I max) {
            distr = std::uniform_int_distribution<I>(min, max);
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RANDOMINT_HPP
