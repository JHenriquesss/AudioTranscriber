#include "export/SubtitleTimestamp.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("formatSrtTimestamp renders zero sub-second minute and hour boundaries",
          "[export][timestamp]") {
    REQUIRE(export_format::formatSrtTimestamp(0) == "00:00:00,000");
    REQUIRE(export_format::formatSrtTimestamp(500) == "00:00:00,500");
    REQUIRE(export_format::formatSrtTimestamp(65'000) == "00:01:05,000");
    REQUIRE(export_format::formatSrtTimestamp(3'661'500) == "01:01:01,500");
}

TEST_CASE("formatVttTimestamp renders zero sub-second minute and hour boundaries",
          "[export][timestamp]") {
    REQUIRE(export_format::formatVttTimestamp(0) == "00:00:00.000");
    REQUIRE(export_format::formatVttTimestamp(500) == "00:00:00.500");
    REQUIRE(export_format::formatVttTimestamp(65'000) == "00:01:05.000");
    REQUIRE(export_format::formatVttTimestamp(3'661'500) == "01:01:01.500");
}
