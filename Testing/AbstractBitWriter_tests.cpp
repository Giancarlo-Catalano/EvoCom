#include <catch2/catch.hpp>
#include "../AbstractBit/VectorBitWriter/VectorBitWriter.hpp"
#include "../Evolver/Evaluator/BitCounter/BitCounter.hpp"


namespace GC {

    TEST_CASE("VectorBitWriter", "[AbstractBitWriter][VectorBitWriter]") {

        SECTION("pushBit Implementation") {
            GIVEN("Starting from an empty vector") {
                VectorBitWriter writer;
                THEN("It should be initially empty") {
                    CHECK(writer.getVectorOfBits().empty());
                }


                WHEN("calling .push(0)") {
                    writer.pushBit(false);
                    std::vector<bool> expected{false};
                    THEN("we expect the result to be just {0}") {
                        CHECK(writer.getVectorOfBits() == expected);
                    }
                }

                WHEN("calling .push(1)") {
                    writer.pushBit(true);
                    std::vector<bool> expected{true};
                    THEN("we expect the result to be just {1}") {
                        CHECK(writer.getVectorOfBits() == expected);
                    }
                }
            }

            GIVEN("writer is already filled") {
                VectorBitWriter writer;
                writer.pushBit(false);
                writer.pushBit(false);

                WHEN("pushing a value") {
                    THEN("It should be appended to the already written vector") {
                        writer.pushBit(true);
                        std::vector<bool> expected{false, false, true};
                        CHECK(writer.getVectorOfBits() == expected);
                    }
                }
            }
        };

        SECTION("Checking the implementation of the encodings") {
            GIVEN("An initially empty writer") {
                VectorBitWriter writer;

                WHEN("Calling writeAmountOfBits") {
                    THEN("the bits are written in the right order") {
                        writer.writeAmountOfBits(0b1010010001, 10);
                        std::vector<bool> expected{1, 0, 1, 0, 0, 1, 0, 0, 0, 1};
                        CHECK(writer.getVectorOfBits() == expected);
                    }

                    THEN("We can ask for as much as 64 bits to be written") {
                        writer.writeAmountOfBits(0, 64);
                        CHECK(writer.getVectorOfBits().size() == 64);
                    }
                }

                WHEN("Calling writeRiceEncoded") {
                    THEN("Writing 0 works as expected") {
                        writer.writeRiceEncoded(0);
                        std::vector<bool> expected{1, 0, 0};
                        CHECK(writer.getVectorOfBits() == expected);
                    }
                    THEN("Writing 1 works as expected") {
                        writer.writeRiceEncoded(1);
                        std::vector<bool> expected{1, 0, 1};
                        CHECK(writer.getVectorOfBits() == expected);
                    }

                    THEN("Writing 4 works as expected") {
                        writer.writeRiceEncoded(4);
                        std::vector<bool> expected{0, 1, 0, 0, 0, 0};
                        CHECK(writer.getVectorOfBits() == expected);
                    }
                    THEN("Writing 12 works as expected") {
                        writer.writeRiceEncoded(20);
                        std::vector<bool> expected{0, 0, 1, 0, 0, 0, 0, 0, 0};
                        CHECK(writer.getVectorOfBits() == expected);
                    }
                }
            }
        }



    }


    TEST_CASE("BitCounter", "[AbstractBitWriter][BitCounter]") {
        GIVEN("Starting from an empty counter") {
            BitCounter writer;

            THEN("It should be initially 0") {
                CHECK(writer.getAmountOfBits() == 0);
            }


            WHEN("call .push(0)") {
                writer.pushBit(false);
                THEN("we expect the result to be just 1") {
                    CHECK(writer.getAmountOfBits() == 1);
                }
            }

            WHEN("call pushing 3 items") {
                writer.pushBit(true);
                writer.pushBit(false);
                writer.pushBit(false);
                THEN("we expect the result to be just 3") {
                    CHECK(writer.getAmountOfBits() == 3);
                }
            }

            WHEN("pushing more than 8 items") {
                repeat(9, [&](){writer.pushBit(false);});
                THEN("we expect the counter to be more than 8 as well") {
                    CHECK(writer.getAmountOfBits() == 9);
                }
            }
        }
    }

    TEST_CASE("BitCounter remains consistent with VectorBitWriter") {
        GIVEN("Writers is initially empty") {
            BitCounter bitCounter;
            VectorBitWriter vectorWriter;

#define TO_BOTH_WRITERS(funcCall) bitCounter . funcCall; vectorWriter . funcCall

            auto stillConsistent = [&bitCounter, &vectorWriter]()->bool {
                return bitCounter.getAmountOfBits() == vectorWriter.getVectorOfBits().size();
            };

            WHEN("Pushing a single value") {
                TO_BOTH_WRITERS(pushBit(0));

                THEN("They remain consistent") {
                    CHECK(stillConsistent());
                }
            }

            WHEN("Pushing multiple values") {
                size_t amount = 108;
                repeat(108, [&]() {
                    TO_BOTH_WRITERS(pushBit(0));
                });

                THEN("They remain consistent") {
                    CHECK(stillConsistent());
                }
            }

            WHEN("Calling .writeAmountOfBits") {
                size_t amountOfBits = GENERATE(1, 12);
                THEN("They remain consistent") {
                    TO_BOTH_WRITERS(writeAmountOfBits(6, amountOfBits));
                    CHECK(stillConsistent());
                }
            }

            WHEN("Calling writeUnary") {
                size_t amountToEncode = GENERATE(1, 6, 36, 108);
                THEN("Remain consistent") {
                    TO_BOTH_WRITERS(writeUnary(amountToEncode));
                    CHECK(stillConsistent());
                }
            }

            WHEN("Calling writeSmallAmount (same as RiceEncoded)") {
                size_t amountToEncode = GENERATE(1, 6, 36, 108);
                THEN("Remain consistent") {
                    TO_BOTH_WRITERS(writeSmallAmount(amountToEncode));
                    CHECK(stillConsistent());
                }
            }

            WHEN("Writing a vector") {
                size_t lengthOfVector = GENERATE(0, 1, 6);
                std::vector<bool> toWrite(true, lengthOfVector);

                THEN("Remain consistent") {
                    TO_BOTH_WRITERS(writeVector(toWrite));
                    CHECK(stillConsistent());
                }
            }
        }
    }
}

