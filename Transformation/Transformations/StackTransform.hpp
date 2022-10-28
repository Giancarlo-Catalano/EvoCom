//
// Created by gian on 28/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_STACKTRANSFORM_HPP
#define DISS_SIMPLEPROTOTYPE_STACKTRANSFORM_HPP

#include <numeric>
#include <forward_list>
#include "../Transformation.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    class StackTransform : public Transformation {
    public:

        using UnitList = std::forward_list<Unit>;
        static const size_t maxValueOfUnit = typeVolume<Unit>();

        StackTransform() {};

        std::string to_string() const {return "{NewStackTransform}";}

        static UnitList getInitialList() {
            UnitList result;
            auto currentEnd = result.before_begin();
            for (size_t i=0;i<maxValueOfUnit;i++) {
                result.emplace_after(currentEnd, i);
                currentEnd++;
            }
            return result;
        }

        static std::string listToString(const UnitList& list) {
            LOG("This unit list is:");
            for (auto item: list) {LOG_NONEWLINE_NOSPACES(item);}
            LOG("");
        }


        /**
         * Finds where the given unit was in the stack, and also updates the stack to have that on top
         * @param u the unit we're looking for
         * @return  where u was in the stack, counting from the top, 0 indexed
         */
        static Unit findInListAndUpdate(const Unit u, UnitList& mainList) {
            Unit where = 0;
            auto beforeIter = mainList.before_begin();
            auto iter = mainList.begin();
            while (true) {//dangerous
                if (*iter == u) break;
                where++;
                beforeIter = iter;
                iter++;
            }
            mainList.erase_after(beforeIter);
            mainList.emplace_front(u);
            return where;
        }

        Block apply_copy(const Block& block) const {
            //stack is initially all the values in the unit, in order, with 0 at the top
            UnitList encodingList = getInitialList();

            Block result(block.size());
            for (size_t i=0;i<result.size();i++)
                result[i] = findInListAndUpdate(block[i], encodingList);
            return result;
        }

        static Unit getNthFromListAndUpdate(const Unit position, UnitList& mainStack) {
            auto beforeIter = mainStack.before_begin();
            repeat(position, [&](){beforeIter++;});
            auto iter = beforeIter; iter++;
            Unit result = *iter;
            mainStack.erase_after(beforeIter);
            mainStack.emplace_front(result);
            return result;
        }

        Block undo_copy(const Block& block) const {
            UnitList encodingList = getInitialList(); //needs to start with the same stack as apply_copy
            Block result(block.size());
            for (size_t i=0;i<result.size();i++)
                result[i] = getNthFromListAndUpdate(block[i], encodingList);
            return result;

        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_STACKTRANSFORM_HPP
