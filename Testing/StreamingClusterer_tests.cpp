#include <catch2/catch.hpp>


#include "../Utilities/StreamingClusterer/StreamingClusterer.hpp"


namespace GC {
    TEST_CASE("Streaming Clusterer ", "[StreamingClusterer]") {
        using T = size_t;
        using Field = size_t;
        using Cluster = std::vector<T>;
        using Accumulator = std::vector<Cluster>;
        using Clusterer = StreamingClusterer<T, Field>;
        using FakeStream = std::vector<T>;

        auto metric = [](const T &a, const T &b) -> Field {
            return (a < b ? (b - a) : (a - b));
        };

        auto addToAccumulator = [&](const Cluster &cluster, Accumulator &acc) {
            acc.push_back(cluster);
        };

        auto makeClustererProcessStream = [&](const FakeStream& items, Clusterer& c) {
            std::for_each(items.begin(), items.end(), [&](const T& item){c.pushItem(item);});
            c.finish();
        };

        const Accumulator emptyAcc{};

        SECTION("Works on small streams") {
            Accumulator acc;
            Clusterer c(metric, [&](auto newClust){addToAccumulator(newClust, acc);}, 10000, 1);

            GIVEN("A trivial clusterer") {
                WHEN("Accepting an empty stream") {
                    THEN("There should be no clusters") {
                        makeClustererProcessStream({}, c);
                        CHECK(acc.empty());
                    }
                }

                WHEN("Accepting a single element") {
                    FakeStream singleton{6};
                    Cluster expected{6};
                    THEN("There should be a single cluster with a single item") {
                        makeClustererProcessStream(singleton, c);
                        REQUIRE(acc.size() == 1);
                        CHECK(acc[0] == expected);
                    }
                }

                WHEN("Accepting many identical items") {
                    size_t amount = 100;
                    T item = 6;
                    FakeStream repetitions(amount, item);
                    THEN("There should be a single cluster, with all the items pushed into it") {
                        makeClustererProcessStream(repetitions, c);
                        REQUIRE(acc.size() == 1);
                        CHECK(acc[0].size() == amount);
                    }
                }
            }
        }
    }


}