//
// Created by gian on 17/09/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_STACKTRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_STACKTRANSFORM_HPP

#include <stack>
#include <numeric>
#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    class StackTransform : public Transformation {
    public:

        using UnitStack = std::stack<Unit>;

        StackTransform() {};

        std::string to_string() const {return "{StackTransform}";}

        static std::stack<Unit> getInitialStack() {
            UnitStack result;
            const size_t unitVolume = typeVolume<Unit>();
            for (Unit unit=unitVolume-1;unit!=0;unit--)
                result.push(unit);
            result.push(0);
            return result;
        }

        static void popInto(UnitStack & source, UnitStack& destination) {
            destination.push(source.top());
            source.pop();
        };

        /**
         * Finds where the given unit was in the stack, and also updates the stack to have that on top
         * @param u a unit
         * @return  where u was in the stack, counting from the top, 0 indexed
         */
        static Unit findInStackAndUpdate(const Unit u, UnitStack& mainStack) {
            UnitStack poppedWhileSearching;
            while (mainStack.top() != u)
                popInto(mainStack, poppedWhileSearching);

            size_t position = poppedWhileSearching.size();
            mainStack.pop();
            while (!poppedWhileSearching.empty())
                popInto(poppedWhileSearching, mainStack);
            mainStack.push(u);
            return position;
        }

        Block apply_copy(const Block& block) const {
            //stack is initially all the values in the unit, in order, with 0 at the top
            UnitStack encodingStack = getInitialStack();

            Block result;
            for (const auto& unit: block)
                result.push_back(findInStackAndUpdate(unit, encodingStack));

            return result;
        }

        static Unit getFromStackAndUpdate(const Unit position, UnitStack& mainStack) {
            UnitStack poppedWhileSearching;
            repeat(position, [&](){ popInto(mainStack, poppedWhileSearching);});
            Unit result = mainStack.top();
            mainStack.pop();
            while (!poppedWhileSearching.empty())
                popInto(poppedWhileSearching, mainStack);
            mainStack.push(result);
            return result;
        }

        Block undo_copy(const Block& block) const {
            UnitStack encodingStack = getInitialStack(); //needs to start with the same stack as apply_copy
            Block result;
            for (const auto& unit: block)
                result.push_back(getFromStackAndUpdate(unit, encodingStack));

            return result;

        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_STACKTRANSFORM_HPP
