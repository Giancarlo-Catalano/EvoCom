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
        const I min, max;
        std::uniform_int_distribution<I> distr;

    public:
        RandomInt(I min, I max) :
            rd(),
            gen(rd()),
            min(min),
            max(max),
            distr(min, max){
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
