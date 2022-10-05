//
// Created by gian on 05/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_IDENTITYTRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_IDENTITYTRANSFORM_HPP

#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"
#include "../../BlockReport/BlockReport.hpp"

namespace GC {

    class IdentityTransform : public Transformation{
    public:
        std::string to_string() const { return "{IdentityTransform}";}
        Block apply_copy(const Block& block) const {
            return block;
        }
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_IDENTITYTRANSFORM_HPP
