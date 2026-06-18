#include "shared/Logger.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Logger redacts transcript-like payloads", "[shared][logger]") {
    const std::string payload = R"({"segments":[{"text":"private words"}]})";
    const auto redacted = shared::redactSensitiveLogText(payload);

    REQUIRE(redacted == "[REDACTED]");
}

TEST_CASE("Logger redacts text fields inside mixed payloads", "[shared][logger]") {
    const std::string payload = R"({"job_id":"job_1","text":"secret transcript"})";
    const auto redacted = shared::redactSensitiveLogText(payload);

    REQUIRE(redacted.find("secret transcript") == std::string::npos);
    REQUIRE(redacted.find("[REDACTED]") != std::string::npos);
}

TEST_CASE("Logger formats structured entries with stable event names", "[shared][logger]") {
    shared::StructuredLogEntry entry;
    entry.timestamp = "2026-06-17T14:22:31Z";
    entry.level = shared::LogLevel::Info;
    entry.event = shared::log_events::settingsLoaded;
    entry.fields["default_model"] = "small";

    const auto formatted = shared::formatStructuredLogEntry(entry);

    REQUIRE(formatted.find("\"event\":\"settings_loaded\"") != std::string::npos);
    REQUIRE(formatted.find("\"default_model\":\"small\"") != std::string::npos);
}

TEST_CASE("Logger exposes architecture-aligned event name constants", "[shared][logger]") {
    REQUIRE(std::string(shared::log_events::appStarted) == "app_started");
    REQUIRE(std::string(shared::log_events::appShutdown) == "app_shutdown");
    REQUIRE(std::string(shared::log_events::settingsLoaded) == "settings_loaded");
    REQUIRE(std::string(shared::log_events::jobFailed) == "job_failed");
    REQUIRE(std::string(shared::log_events::jobCancelled) == "job_cancelled");
    REQUIRE(std::string(shared::log_events::audioExtractionFailed) == "audio_extraction_failed");
    REQUIRE(std::string(shared::log_events::transcriptionStarted) == "transcription_started");
    REQUIRE(std::string(shared::log_events::exportCompleted) == "export_completed");
}
