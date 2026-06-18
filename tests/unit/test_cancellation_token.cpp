#include "shared/CancellationToken.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Cancellation token starts not cancelled", "[cancellation]") {
    shared::CancellationToken token;

    REQUIRE_FALSE(token.isCancelled());
}

TEST_CASE("Cancellation token changes to cancelled deterministically", "[cancellation]") {
    shared::CancellationToken token;

    token.cancel();

    REQUIRE(token.isCancelled());
}

TEST_CASE("Cancellation token remains cancelled after repeated cancel calls", "[cancellation]") {
    shared::CancellationToken token;

    token.cancel();
    token.cancel();

    REQUIRE(token.isCancelled());
}
