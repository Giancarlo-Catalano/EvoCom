//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#define DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#include "../Utilities/utilities.hpp"


namespace GC {

    class Breeder {

    public:
        enum CrossOverType {
            Uniform,
            SubSequence,
            SwapEnds
        };
        Breeder(size_t mutationSize, CrossOverType crossOverType);



    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BREEDER_HPP
