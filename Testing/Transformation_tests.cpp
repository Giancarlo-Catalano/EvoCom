#include <catch2/catch.hpp>
#include "../Transformation/Transformation.hpp"
#include "../Transformation/Transformations/BurrowsWheelerTransform.hpp"
#include "../Transformation/Transformations/DeltaTransform.hpp"
#include "../Transformation/Transformations/DeltaXORTransform.hpp"
#include "../Transformation/Transformations/IdentityTransform.hpp"
#include "../Transformation/Transformations/LempelZivWelchTransform.hpp"
#include "../Transformation/Transformations/RunLengthTransform.hpp"
#include "../Transformation/Transformations/SplitTransform.hpp"
#include "../Transformation/Transformations/StackTransform.hpp"
#include "../Transformation/Transformations/StrideTransform.hpp"
#include "../Transformation/Transformations/SubtractAverageTransform.hpp"
#include "../Transformation/Transformations/SubtractXORAverageTransform.hpp"
#include "../EvolutionaryFileCompressor/EvolutionaryFileCompressor.hpp"
#include "../Evolver/Individual/TCodes.hpp"

namespace GC {

    Block applyAndUndoTransform(const TCode tcode, const Block& input) {
        Block toBeModified = input;
        EvolutionaryFileCompressor::applyTransformCode(tcode, toBeModified);
        EvolutionaryFileCompressor::undoTransformCode(tcode, toBeModified);
        return toBeModified;
    }

    bool isInvertedCorrectly(const TCode tcode, const Block& input) {
        return input == applyAndUndoTransform(tcode, input);
    }




    TEST_CASE("Transformations", "[Transforms]") {


        SECTION("Transformations are reversible") {

#define TEST_ALL_TRANSFORMS(input) \
    THEN("The BurrowsWheelerTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_BurrowsWheelerTransform, input)); \
    } \
    THEN("The DeltaTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_DeltaTransform, input)); \
    } \
    THEN("The DeltaXORTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_DeltaXORTransform, input)); \
    } \
    THEN("The IdentityTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_IdentityTransform, input)); \
    } \
    THEN("The LempelZivWelchTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_LempelZivWelchTransform, input)); \
    } \
    THEN("The RunLengthTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_RunLengthTransform, input)); \
    } \
    THEN("The SplitTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_SplitTransform, input)); \
    } \
    THEN("The StackTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_StackTransform, input)); \
    } \
    THEN("The StrideTransform is inverted correctly for arguments 2, 3, 4") { \
        CHECK(isInvertedCorrectly(T_StrideTransform_2, input)); \
        CHECK(isInvertedCorrectly(T_StrideTransform_3, input)); \
        CHECK(isInvertedCorrectly(T_StrideTransform_4, input)); \
    } \
    THEN("The SubtractAverageTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_SubtractAverageTransform, input)); \
    } \
    THEN("The SubtractXORAverageTransform is inverted correctly") { \
        CHECK(isInvertedCorrectly(T_SubtractXORAverageTransform, input)); \
    }

            WHEN("The input is a block of 2 bytes") {
                const Unit firstValue = GENERATE(0, 1, 6, 128, 255);
                const Unit secondValue = GENERATE(0, 1, 2, 200, 255);
                Block blockOf2 = {firstValue, secondValue};
                TEST_ALL_TRANSFORMS(blockOf2);
            }
            WHEN("The input is all zeros") {
                const size_t amountOfZeros = GENERATE(3, 4, 20, 100, 256, 500);
                Block allZeros(amountOfZeros, 0);
                TEST_ALL_TRANSFORMS(allZeros);
            }
            WHEN("The input is all 255") {
                const size_t amountOfFF = GENERATE(3, 4, 20, 100, 256, 500);
                Block allFF(amountOfFF, 255);
                TEST_ALL_TRANSFORMS(allFF);
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

                TEST_ALL_TRANSFORMS(increasingValues);
                TEST_ALL_TRANSFORMS(manyRepetitions);
                TEST_ALL_TRANSFORMS(almostRandomBlock);
        }
    }
}


}