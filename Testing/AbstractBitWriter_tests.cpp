#include <catch2/catch.hpp>
#include "../AbstractBit/VectorBitWriter/VectorBitWriter.hpp"
#include "../Evolver/Evaluator/BitCounter/BitCounter.hpp"


namespace GC {

    TEST_CASE("VectorBitWriter", "[AbstractBitWriter][VectorBitWriter]") {
        GIVEN("Starting from an empty vector") {
            VectorBitWriter writer;

            THEN("It should be initially empty") {
                CHECK(writer.getVector().empty());
            }


            WHEN("call .push(0)") {
                writer.pushBit(false);
                std::vector<bool> expected{false};
                THEN("we expect the result to be just {0}") {
                    CHECK(writer.getVector() == expected);
                }
            }

            WHEN("call .push(1)") {
                writer.pushBit(true);
                std::vector<bool> expected{true};
                THEN("we expect the result to be just {1}") {
                    CHECK(writer.getVector() == expected);
                }
            }
        }

    }


    TEST_CASE("BitCounter", "[AbstractBitWriter][BitCounter]") {
        GIVEN("Starting from an empty counter") {
            BitCounter writer;

            THEN("It should be initially 0") {
                CHECK(writer.getCounterValue() == 0);
            }


            WHEN("call .push(0)") {
                writer.pushBit(false);
                THEN("we expect the result to be just 1") {
                    CHECK(writer.getCounterValue() == 1);
                }
            }

            WHEN("call pushing 3 items") {
                writer.pushBit(true);
                writer.pushBit(false);
                writer.pushBit(false);
                THEN("we expect the result to be just 3") {
                    CHECK(writer.getCounterValue() == 3);
                }
            }

            WHEN("pushing more than 8 items") {
                repeat(9, [&](){writer.pushBit(false);});
                THEN("we expect the counter to be more than 8 as well") {
                    CHECK(writer.getCounterValue() == 9);
                }
            }
        }

    }
}

