//
// Created by gian on 15/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_TRANSFORMATION_HPP
#define DISS_SIMPLEPROTOTYPE_TRANSFORMATION_HPP

#include <string>
#include "../Utilities/utilities.hpp"
#include "../names.hpp"

namespace GC {

    class Transformation {
    public:

        virtual std::string to_string() const = 0;

        virtual Block apply_copy(const Block& block) const = 0;

        virtual void apply(Block& block) const {
            Block newBlock = apply_copy(block);
            block.swap(newBlock);
        }

        virtual Block undo_copy(const Block& block) const = 0;

        virtual void undo(Block& block) const {
            Block undoneBlock = undo_copy(block);
            block.swap(undoneBlock);
        }

    protected:
        Transformation(){}
    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_TRANSFORMATION_HPP
