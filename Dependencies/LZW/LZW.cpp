//
// Created by gian on 15/12/22.
//

#ifndef EVOCOM_LZW_CPP
#define EVOCOM_LZW_CPP

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include "LZW.hpp"

namespace JP {

    //TODO include the original credits and licence header


    EncoderDictionary::EncoderDictionary() {
        const long int minc = std::numeric_limits<char>::min();
        const long int maxc = std::numeric_limits<char>::max();
        CodeType k{0};

        for (long int c = minc; c <= maxc; ++c)
            initials[static_cast<unsigned char> (c)] = k++;

        vn.reserve(globals::dms);
        reset();
    }



        void EncoderDictionary::reset() {
            vn.clear();

            const long int minc = std::numeric_limits<char>::min();
            const long int maxc = std::numeric_limits<char>::max();

            for (long int c = minc; c <= maxc; ++c) {
                vn.push_back(Node(c));
            }
        }

        CodeType EncoderDictionary::search_and_insert(CodeType i, char c) {
            // dictionary's maximum size was reached
            if (vn.size() == globals::dms)
                reset();

            if (i == globals::dms)
                return search_initials(c);

            const CodeType vn_size = vn.size();
            CodeType ci{vn[i].first}; // Current Index

            if (ci != globals::dms) {
                while (true)
                    if (c < vn[ci].c) {
                        if (vn[ci].left == globals::dms) {
                            vn[ci].left = vn_size;
                            break;
                        } else
                            ci = vn[ci].left;
                    } else if (c > vn[ci].c) {
                        if (vn[ci].right == globals::dms) {
                            vn[ci].right = vn_size;
                            break;
                        } else
                            ci = vn[ci].right;
                    } else // c == vn[ci].c
                        return ci;
            } else
                vn[i].first = vn_size;

            vn.push_back(Node(c));
            return globals::dms;
        }

        ///
        /// @brief Fakes a search for byte `c` in the one-byte area of the dictionary.
        /// @param c    byte to search for
        /// @returns The code associated to the searched byte.
        ///
        CodeType EncoderDictionary::search_initials(char c) const {
            CodeType result = initials[static_cast<unsigned char> (c)];
            return result;
        }

}

#endif //EVOCOM_LZW_CPP
