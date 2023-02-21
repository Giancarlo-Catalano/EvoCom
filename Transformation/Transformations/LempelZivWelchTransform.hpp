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
#include "../../Dependencies/LZW/LZW.hpp"


namespace GC {

    class LempelZivWelchTransform : public Transformation {
        Block compressBlock(const Block &input) const {
            JP::EncoderDictionary ed;
            JP::CodeType i{JP::globals::dms}; // Index
            char c;


            size_t inputIndex = 0;
            auto getCharFromInput = [&]() {
                return input[inputIndex++];
            };

            Block result;
            auto pushCodeToOutput = [&](const JP::CodeType code) {
                result.push_back(((int) ((code >> 8) & 0xff)));
                result.push_back(((char) (code & 0xff)));
            };

            while (inputIndex < input.size()) {
                c = getCharFromInput();
                const JP::CodeType temp{i};

                if ((i = ed.search_and_insert(temp, c)) == JP::globals::dms) {
                    pushCodeToOutput(temp);
                    i = ed.search_initials(c);
                }
            }

            if (i != JP::globals::dms) {
                pushCodeToOutput(i);
            }

            return result;
        }

        Block decompressBlock(const Block &input) const {

            std::vector<std::pair<JP::CodeType, char>> dictionary;

            // "named" lambda function, used to reset the dictionary to its initial contents
            const auto reset_dictionary = [&dictionary] {
                dictionary.clear();
                dictionary.reserve(JP::globals::dms);

                const long int minc = std::numeric_limits<char>::min();
                const long int maxc = std::numeric_limits<char>::max();

                for (long int c = minc; c <= maxc; ++c)
                    dictionary.push_back({JP::globals::dms, static_cast<char> (c)});
            };

            const auto rebuild_string = [&dictionary](JP::CodeType k) -> const std::vector<char> * {
                static std::vector<char> s; // String

                s.clear();

                // the length of a string cannot exceed the dictionary's number of entries
                s.reserve(JP::globals::dms);

                while (k != JP::globals::dms) {
                    s.push_back(dictionary[k].second);
                    k = dictionary[k].first;
                }

                std::reverse(s.begin(), s.end());
                return &s;
            };

            reset_dictionary();

            JP::CodeType i{JP::globals::dms}; // Index
            JP::CodeType k; // Key

            size_t inputIndex = 0;
            auto readCodeFromInput = [&]() -> JP::CodeType {

                char head = input[inputIndex];
                char tail = input[inputIndex + 1];
                JP::CodeType toReturn = (((unsigned char) head) << 8) | (unsigned char) (tail);
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
                if (dictionary.size() == JP::globals::dms)
                    reset_dictionary();

                if (k > dictionary.size())
                    throw std::runtime_error("invalid compressed code");

                const std::vector<char> *s; // String

                if (k == dictionary.size()) {
                    dictionary.push_back({i, rebuild_string(i)->front()});
                    s = rebuild_string(k);
                } else {
                    s = rebuild_string(k);

                    if (i != JP::globals::dms)
                        dictionary.push_back({i, s->front()});
                }

                writeStringToOutput(s);
                i = k;
            }

            return output;
        }

    public:
        std::string to_string() const { return "{LempelZivWelchTransform}";}


        Block apply_copy(const Block &block) const {
            return compressBlock(block);
        }

        Block undo_copy(const Block &block) const {
            return decompressBlock(block);
        }

    };

} // GC

#endif //EVOCOM_LEMPELZIVWELCHTRANSFORM_HPP
