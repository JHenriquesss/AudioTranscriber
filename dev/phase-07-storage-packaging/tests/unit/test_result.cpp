#include "shared/Result.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Result returns success value without exceptions", "[result]") {
    const auto result = shared::Result<int>::success(42);

    REQUIRE(result.ok);
    REQUIRE(result.value == 42);
    REQUIRE(result.error.message.empty());
}

TEST_CASE("Result returns failure without exceptions", "[result]") {
    shared::AppError error;
    error.code = shared::ErrorCode::FileNotFound;
    error.message = "Missing file";
    error.technicalDetails = "path does not exist";

    const auto result = shared::Result<std::string>::failure(std::move(error));

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.value.empty());
    REQUIRE(result.error.code == shared::ErrorCode::FileNotFound);
    REQUIRE(result.error.message == "Missing file");
    REQUIRE(result.error.technicalDetails == "path does not exist");
}

TEST_CASE("Result void specialization reports success", "[result]") {
    const auto result = shared::Result<void>::success();

    REQUIRE(result.ok);
    REQUIRE(result.error.message.empty());
}

TEST_CASE("Result void specialization preserves failure details", "[result]") {
    shared::AppError error;
    error.code = shared::ErrorCode::Cancelled;
    error.message = "Operation cancelled";

    const auto result = shared::Result<void>::failure(std::move(error));

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::Cancelled);
    REQUIRE(result.error.message == "Operation cancelled");
}
