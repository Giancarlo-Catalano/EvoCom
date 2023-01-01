#include <catch2/catch.hpp>
#include "../EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "../AbstractBit/VectorBitWriter/VectorBitWriter.hpp"
#include "../AbstractBit/VectorBitReader/VectorBitReader.hpp"

namespace GC {

    Block applyAndUndoCompression(const CCode ccode, const Block& input) {
        VectorBitWriter writer;
        EvolutionaryFileCompressor::applyCompressionCode(ccode, input, writer);
        const std::vector<bool> compressed = writer.getVectorOfBits();

        VectorBitReader reader(compressed);
        const Block undone = EvolutionaryFileCompressor::undoCompressionCode(ccode, reader);
        return undone;
    }

    bool isInvertedCorrectly(const CCode ccode, const Block& input) {
        return input == applyAndUndoCompression(ccode, input);
    }




    TEST_CASE("Compressions", "[Compressions]") {

        SECTION("Compressions are reversible") {

    #define TEST_ALL_COMPRESSIONS(input) \
        THEN("The Identity Compression is inverted correctly") { \
            CHECK(isInvertedCorrectly(C_IdentityCompression, input)); \
        } \
        THEN("The Huffman Compression is inverted correctly") { \
            CHECK(isInvertedCorrectly(C_HuffmanCompression, input)); \
        } \
        THEN("The Run Length Compression is inverted correctly") { \
            CHECK(isInvertedCorrectly(C_RunLengthCompression, input)); \
        } \
        THEN("The SmallValueCompression is inverted correctly") { \
            CHECK(isInvertedCorrectly(C_SmallValueCompression, input)); \
        } \
        THEN("The LempelZivWelch Compression is inverted correctly") { \
            CHECK(isInvertedCorrectly(C_LZWCompression, input)); \
        }



        WHEN("The input is a block of 2 bytes") {
            const Unit firstValue = GENERATE(0, 1, 6, 128, 255);
            const Unit secondValue = GENERATE(0, 1, 2, 200, 255);
            Block blockOf2 = {firstValue, secondValue};
            TEST_ALL_COMPRESSIONS(blockOf2);
        }

        WHEN("The input is all zeros") {
            const size_t amountOfZeros = GENERATE(3, 4, 20, 100, 256, 500);
            Block allZeros(amountOfZeros, 0);
            TEST_ALL_COMPRESSIONS(allZeros);
        }

        WHEN("The input is all 255") {
            const size_t amountOfFF = GENERATE(3, 4, 20, 100, 256, 500);
            Block allFF(amountOfFF, 255);
            TEST_ALL_COMPRESSIONS(allFF);
        }

        WHEN("The input has more than or exactly 256 values") {
            Block increasingValues;
            Block manyRepetitions;
            Block almostRandomBlock;
            for (size_t i=0;i<256;i++) {
                increasingValues.push_back(i);
                repeat(i+5, [&](){manyRepetitions.push_back(i);});
                repeat((i*i)%5, [&](){almostRandomBlock.push_back((i*i*i)%256);});
            }

            TEST_ALL_COMPRESSIONS(increasingValues);
            TEST_ALL_COMPRESSIONS(manyRepetitions);
            TEST_ALL_COMPRESSIONS(almostRandomBlock);
        }
    }
}}