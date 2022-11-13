#include <catch2/catch.hpp>
#include "../names.hpp"
#include "../BlockReport/BlockReport.hpp"

namespace GC {
    TEST_CASE("BlockReport", "[BlockReport]") {
        GIVEN("A BlockReport has to be produced") {
            //block size is always at least 2
            WHEN("A block of size 2 is provided") {
                THEN("The block report should be valid") {
                    Block block = {6, 6};
                    BlockReport br(block);
                    CHECK(br.unitFeatures.average == 6);
                    CHECK(br.unitFeatures.mode == 6);
                    CHECK(br.unitFeatures.minimum == 6);
                    CHECK(br.unitFeatures.maximum == 6);
                }
            }
        }

        GIVEN("2 block reports, they should satisfy metric requirements") {
            WHEN("Two identical reports are compared") {
                THEN("Their distance should be zero") {
                    Block block = {1, 2, 3};
                    BlockReport br1(block);
                    BlockReport br2 = br1;
                    double distance = br1.distanceFrom(br2);
                    CHECK(distance == 0);
                }
            }

            WHEN("comparing any 2 reports") {
                THEN("distance (a, b) = distance (b, a)") {
                    Block blockA = {1, 2, 3};
                    Block blockB = {99, 99, 99, 99};
                    BlockReport brA(blockA);
                    BlockReport brB(blockB);

                    double distanceAB = brA.distanceFrom(brB);
                    double distanceBA = brB.distanceFrom(brA);

                    CHECK(distanceAB == distanceBA);
                }
            }

            WHEN("Comparing 2 reports") {
                THEN("the result is the minimum of the distances between features") {
                    Block blockA = {0, 100};
                    Block blockB = {50, 50, 50, 50, 50, 50, 50, 50, 50};
                    BlockReport brA(blockA);
                    BlockReport brB(blockB);

                    //they should have the same average
                    REQUIRE(brA.unitFeatures.average == 50);
                    REQUIRE(brB.unitFeatures.average == 50);

                    double distance = brA.distanceFrom(brB);

                    CHECK(distance == 0.0);
                }
            }
        }
    }

}

//
// Created by gian on 13/11/22.
//
