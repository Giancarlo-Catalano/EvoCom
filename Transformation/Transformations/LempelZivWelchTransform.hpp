//
// Created by gian on 14/12/22.
//

#ifndef EVOCOM_LEMPELZIVWELCHTRANSFORM_HPP
#define EVOCOM_LEMPELZIVWELCHTRANSFORM_HPP

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
#include "../../Utilities/utilities.hpp"
#include "../Transformation.hpp"

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

///
/// @brief Compresses the contents of `is` and writes the result to `os`.
/// @param [in] is      input stream
/// @param [out] os     output stream
///
    Block compress(const Block &input) {
        EncoderDictionary ed;
        CodeType i{globals::dms}; // Index
        char c;


        size_t inputIndex = 0;
        auto getCharFromInput = [&]() {
            return input[inputIndex++];
        };

        Block result;
        auto pushCodeToOutput = [&](const CodeType code) {
            result.push_back(((int) ((code >> 8) & 0xff)));
            result.push_back(((char) (code & 0xff)));
        };

        while (inputIndex < input.size()) {
            c = getCharFromInput();
            const CodeType temp{i};

            if ((i = ed.search_and_insert(temp, c)) == globals::dms) {
                pushCodeToOutput(temp);
                i = ed.search_initials(c);
            }
        }

        if (i != globals::dms) {
            pushCodeToOutput(i);
        }

        return result;
    }

///
/// @brief Decompresses the contents of `is` and writes the result to `os`.
/// @param [in] is      input stream
/// @param [out] os     output stream
///
    Block decompress(const Block &input) {

        std::vector<std::pair<CodeType, char>> dictionary;

        // "named" lambda function, used to reset the dictionary to its initial contents
        const auto reset_dictionary = [&dictionary] {
            dictionary.clear();
            dictionary.reserve(globals::dms);

            const long int minc = std::numeric_limits<char>::min();
            const long int maxc = std::numeric_limits<char>::max();

            for (long int c = minc; c <= maxc; ++c)
                dictionary.push_back({globals::dms, static_cast<char> (c)});
        };

        const auto rebuild_string = [&dictionary](CodeType k) -> const std::vector<char> * {
            static std::vector<char> s; // String

            s.clear();

            // the length of a string cannot exceed the dictionary's number of entries
            s.reserve(globals::dms);

            while (k != globals::dms) {
                s.push_back(dictionary[k].second);
                k = dictionary[k].first;
            }

            std::reverse(s.begin(), s.end());
            return &s;
        };

        reset_dictionary();

        CodeType i{globals::dms}; // Index
        CodeType k; // Key

        size_t inputIndex = 0;
        auto readCodeFromInput = [&]() -> CodeType {

            char head = input[inputIndex];
            char tail = input[inputIndex + 1];
            CodeType toReturn = (((unsigned char) head) << 8) | (unsigned char) (tail);
            inputIndex += 2;
            return toReturn;
        };

        Block output;
        auto writeCharToOutput = [&](const char toWrite) {
            output.push_back(toWrite);
        };
        auto writeStringToOutput = [&](const std::vector<char> *str) {
            for (size_t i = 0; i < str->size(); i++) {
                writeCharToOutput(str->at(i));
            }

        };

        while (inputIndex < input.size()) {
            k = readCodeFromInput();
            // dictionary's maximum size was reached
            if (dictionary.size() == globals::dms)
                reset_dictionary();

            if (k > dictionary.size())
                throw std::runtime_error("invalid compressed code");

            const std::vector<char> *s; // String

            if (k == dictionary.size()) {
                dictionary.push_back({i, rebuild_string(i)->front()});
                s = rebuild_string(k);
            } else {
                s = rebuild_string(k);

                if (i != globals::dms)
                    dictionary.push_back({i, s->front()});
            }

            writeStringToOutput(s);
            i = k;
        }

        return output;
    }

}//JP

namespace GC {

    class LempelZivWelchTransform : public Transformation{
    public:
        std::string to_string() const { return "{LempelZivWelchTransform}";}
        Block apply_copy(const Block& block) const {
            return JP::compress(block);
        }
        Block undo_copy(const Block& block) const {
            return JP::decompress(block);
        }
    };

} // GC

#endif //EVOCOM_LEMPELZIVWELCHTRANSFORM_HPP
