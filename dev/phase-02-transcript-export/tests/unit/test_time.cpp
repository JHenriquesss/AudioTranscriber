#include "shared/Time.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>

TEST_CASE("formatDurationMs renders minutes seconds and milliseconds", "[time]") {
    const auto duration = std::chrono::milliseconds(65'500);

    REQUIRE(shared::formatDurationMs(duration) == "1:05.500");
}

TEST_CASE("formatDurationMs renders hours when needed", "[time]") {
    const auto duration = std::chrono::milliseconds(3'661'500);

    REQUIRE(shared::formatDurationMs(duration) == "1:01:01.500");
}

TEST_CASE("formatTimestampUtc renders ISO-8601 UTC", "[time]") {
    const auto timePoint = std::chrono::system_clock::from_time_t(1'717'632'000);

    REQUIRE(shared::formatTimestampUtc(timePoint) == "2024-06-06T00:00:00Z");
}

TEST_CASE("durationFromSeconds converts to milliseconds", "[time]") {
    const auto duration = shared::durationFromSeconds(1.25);

    REQUIRE(duration.count() == 1250);
}
