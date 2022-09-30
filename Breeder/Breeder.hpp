//
// Created by gian on 25/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#define DISS_SIMPLEPROTOTYPE_BREEDER_HPP
#include "../Utilities/utilities.hpp"
#include "../names.hpp"
#include "../Individual/Individual.hpp"


namespace GC {

    class Breeder {

    public:
        enum CrossOverType {
            Uniform,
            SubSequence,
            SwapEnds
        };

        Breeder(Chance mutationChance, CrossOverType crossOverType);


        Individual mutate(const Individual& individual) {
            std::vector<
        }



    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_BREEDER_HPP
