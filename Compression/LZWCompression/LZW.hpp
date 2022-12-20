//
// Created by gian on 15/12/22.
//

#ifndef EVOCOM_LZW_HPP
#define EVOCOM_LZW_HPP

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

namespace JP {


/// Type used to store and retrieve codes.
    using CodeType = std::uint16_t;

    namespace globals {

/// Dictionary Maximum Size (when reached, the dictionary will be reset)
        const CodeType dms{std::numeric_limits<CodeType>::max()};

    } // namespace globals

///
/// @brief Encoder's custom dictionary type.
///
    class EncoderDictionary {

        ///
        /// @brief Binary search tree node.
        ///
        struct Node {

            ///
            /// @brief Default constructor.
            /// @param c    byte that the Node will contain
            ///
            explicit Node(char c) : first(globals::dms), c(c), left(globals::dms), right(globals::dms) {
            }

            CodeType first;  ///< Code of first child string.
            char c;      ///< Byte.
            CodeType left;   ///< Code of child node with byte < `c`.
            CodeType right;  ///< Code of child node with byte > `c`.
        };

    public:

        ///
        /// @brief Default constructor.
        /// @details It builds the `initials` cheat sheet.
        ///
        EncoderDictionary() {
            const long int minc = std::numeric_limits<char>::min();
            const long int maxc = std::numeric_limits<char>::max();
            CodeType k{0};

            for (long int c = minc; c <= maxc; ++c)
                initials[static_cast<unsigned char> (c)] = k++;

            vn.reserve(globals::dms);
            reset();
        }

        ///
        /// @brief Resets dictionary to its initial contents.
        /// @see `EncoderDictionary::EncoderDictionary()`
        ///
        void reset() {
            vn.clear();

            const long int minc = std::numeric_limits<char>::min();
            const long int maxc = std::numeric_limits<char>::max();

            for (long int c = minc; c <= maxc; ++c) {
                vn.push_back(Node(c));
            }
        }

        ///
        /// @brief Searches for a pair (`i`, `c`) and inserts the pair if it wasn't found.
        /// @param i                code to search for
        /// @param c                attached byte to search for
        /// @returns The index of the pair, if it was found.
        /// @retval globals::dms    if the pair wasn't found
        ///
        CodeType search_and_insert(CodeType i, char c) {
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
        CodeType search_initials(char c) const {
            CodeType result = initials[static_cast<unsigned char> (c)];
            return result;
        }

    private:

        /// Vector of nodes on top of which the binary search tree is implemented.
        std::vector<Node> vn;

        /// Cheat sheet for mapping one-byte strings to their codes.
        std::array<CodeType, 1u << CHAR_BIT> initials;
    };
}

#endif //EVOCOM_LZW_HPP
