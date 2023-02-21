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
    using CodeType = std::uint16_t;//
    namespace globals {
/// Dictionary Maximum Size (when reached, the dictionary will be reset)
        const CodeType dms{std::numeric_limits<CodeType>::max()};
    } // namespace globals

///
/// @brief Encoder's custom dictionary type.
///
    class EncoderDictionary {

        struct Node {

            explicit Node(char c) : first(globals::dms), c(c), left(globals::dms), right(globals::dms) {
            }

            CodeType first;  ///< Code of first child string.
            char c;      ///< Byte.
            CodeType left;   ///< Code of child node with byte < `c`.
            CodeType right;  ///< Code of child node with byte > `c`.
        };

    public:

        /// @brief Default constructor.
        /// @details It builds the `initials` cheat sheet.
        ///
        EncoderDictionary();

        ///
        /// @brief Resets dictionary to its initial contents.
        /// @see `EncoderDictionary::EncoderDictionary()`
        ///
        void reset();

        ///
        /// @brief Searches for a pair (`i`, `c`) and inserts the pair if it wasn't found.
        /// @param i                code to search for
        /// @param c                attached byte to search for
        /// @returns The index of the pair, if it was found.
        /// @retval globals::dms    if the pair wasn't found
        ///
        CodeType search_and_insert(CodeType i, char c);

        ///
        /// @brief Fakes a search for byte `c` in the one-byte area of the dictionary.
        /// @param c    byte to search for
        /// @returns The code associated to the searched byte.
        ///
        CodeType search_initials(char c) const;

    private:

        /// Vector of nodes on top of which the binary search tree is implemented.
        std::vector<Node> vn;

        /// Cheat sheet for mapping one-byte strings to their codes.
        std::array<CodeType, 1u << CHAR_BIT> initials;
    };
}

#endif //EVOCOM_LZW_HPP
