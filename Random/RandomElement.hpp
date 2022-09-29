//
// Created by gian on 29/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RANDOMELEMENT_HPP
#define DISS_SIMPLEPROTOTYPE_RANDOMELEMENT_HPP

#include <initializer_list>
#include <vector>
#include <random>
#include "RandomInt.hpp"
#include "../Utilities/utilities.hpp"

namespace GC {

    template <class T>
    class RandomElement {
        std::vector<T> elements;
        RandomInt<size_t> intGenerator;

    public:
        RandomElement(std::initializer_list<T> initList) :
            elements(initList),
            intGenerator(0, initList.size()-1)
        {};


        T choose() {
            return elements[intGenerator()];
        }

        T operator()() {
            return choose();
        }

        void assignRandomValue(T& toBeChanged) {
            toBeChanged = choose();
        }
    };




} // GC

#endif //DISS_SIMPLEPROTOTYPE_RANDOMELEMENT_HPP
