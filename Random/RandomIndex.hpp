//
// Created by gian on 30/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_RANDOMINDEX_HPP
#define DISS_SIMPLEPROTOTYPE_RANDOMINDEX_HPP
#include <random>
#include "RandomInt.hpp"
#include "../Utilities/utilities.hpp"

namespace GC {

    class RandomIndex {
    private:
        RandomInt<size_t> intGenerator;
    public:

        RandomIndex() : intGenerator() {};

        template <class L>
        size_t pick(const L& l) {
            ASSERT_NOT_EMPTY(l);
            intGenerator.setBounds(0, l.size()-1);
            return intGenerator.choose();
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_RANDOMINDEX_HPP
