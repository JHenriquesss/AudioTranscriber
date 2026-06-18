#include "platform/ProcessRunner.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

TEST_CASE("ProcessRunner escapes arguments for Windows command lines", "[platform][process]") {
    REQUIRE(platform::escapeWindowsArgument("plain") == "plain");
    REQUIRE(platform::escapeWindowsArgument("has space") == "\"has space\"");
    REQUIRE(platform::escapeWindowsArgument("quote\"inside") == "\"quote\\\"inside\"");
}

TEST_CASE("ProcessRunner builds deterministic command lines without shell injection",
          "[platform][process]") {
    const auto commandLine = platform::buildWindowsCommandLine(
        std::filesystem::path("C:/tools/ffmpeg.exe"),
        {"-i", R"(C:/unsafe"; malicious.exe)", "-ac", "1"});

    REQUIRE(commandLine.find("\"C:/unsafe\\\"; malicious.exe\"") != std::string::npos);
    REQUIRE(commandLine.find(" -ac ") != std::string::npos);
}

TEST_CASE("ProcessRunner captures exit code and output", "[platform][process]") {
    platform::ProcessRunner runner{};
    platform::ProcessSpec spec{};
    spec.executable = std::filesystem::path("cmd.exe");
    spec.arguments = {"/c", "echo captured-output"};
    spec.timeout = std::chrono::seconds{10};

    const auto result = runner.run(spec);
    REQUIRE(result.ok);
    REQUIRE(result.value.exitCode == 0);
    REQUIRE(result.value.stdoutText.find("captured-output") != std::string::npos);
}

TEST_CASE("ProcessRunner returns typed failure for missing executable", "[platform][process]") {
    platform::ProcessRunner runner{};
    platform::ProcessSpec spec{};
    spec.executable = std::filesystem::path("C:/missing/ffmpeg-does-not-exist.exe");
    spec.arguments = {"-version"};
    spec.timeout = std::chrono::seconds{5};

    const auto result = runner.run(spec);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.code == shared::ErrorCode::FileNotFound);
}

TEST_CASE("ProcessRunner maps non-zero exit to failure with technical details",
          "[platform][process]") {
    platform::ProcessRunner runner{};
    platform::ProcessSpec spec{};
    spec.executable = std::filesystem::path("cmd.exe");
    spec.arguments = {"/c", "exit 7"};
    spec.timeout = std::chrono::seconds{10};

    const auto result = runner.run(spec);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error.technicalDetails.find("exit_code=7") != std::string::npos);
}
