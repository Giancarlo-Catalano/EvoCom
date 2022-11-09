//
// Created by gian on 27/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP
#define DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP

#include "../AbstractBitWriter/AbstractBitWriter.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {

    class VectorBitWriter : public AbstractBitWriter {
    private:
        std::vector<bool> savedBits;
    public:
        VectorBitWriter() : savedBits() {
        }

        virtual void pushBit(const bool b) override {
            savedBits.push_back(b);
        }

        void LOG_INFO() {
            LOG("the pushed bits are:");
            for (size_t i=0;i<savedBits.size();i++) {
                if (i%8 == 0) {
                    LOG_NONEWLINE_NOSPACES("[");
                }
                LOG_NONEWLINE_NOSPACES(savedBits[i]);
                if (i%8 == 7) {
                    LOG_NOSPACES("]");
                }
            }
        }


        std::vector<bool> getVector() {
            return savedBits;
        }

    };

} // GC

#endif //DISS_SIMPLEPROTOTYPE_DEBUGWRITER_HPP
