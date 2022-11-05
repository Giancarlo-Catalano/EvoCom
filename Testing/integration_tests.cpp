#include <catch2/catch.hpp>
#include "../Transformation/Transformations/DeltaTransform.hpp"


TEST_CASE("test it works") {
    int one = 1;
    int two = 2;
    CHECK(one+two ==  3);
}

TEST_CASE("bogus test shouldn't work") {
    int three = 3;
    int four = 4;
    CHECK(three + four == 2);
}