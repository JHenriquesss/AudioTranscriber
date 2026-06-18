#pragma once

#include "Time.hpp"

#include <chrono>
#include <map>
#include <string>
#include <string_view>

namespace shared {

enum class LogLevel { Info, Warning, Error };

namespace log_events {

constexpr const char *appStarted = "app_started";
constexpr const char *appShutdown = "app_shutdown";
constexpr const char *settingsLoaded = "settings_loaded";
constexpr const char *jobCreated = "job_created";
constexpr const char *jobCompleted = "job_completed";
constexpr const char *jobFailed = "job_failed";
constexpr const char *jobCancelled = "job_cancelled";
constexpr const char *audioProbeStarted = "audio_probe_started";
constexpr const char *audioProbeCompleted = "audio_probe_completed";
constexpr const char *audioExtractionStarted = "audio_extraction_started";
constexpr const char *audioExtractionFailed = "audio_extraction_failed";
constexpr const char *audioNormalizationStarted = "audio_normalization_started";
constexpr const char *audioNormalizationCompleted = "audio_normalization_completed";
constexpr const char *transcriptionStarted = "transcription_started";
constexpr const char *transcriptionCompleted = "transcription_completed";
constexpr const char *exportStarted = "export_started";
constexpr const char *exportCompleted = "export_completed";

} // namespace log_events

inline std::string to_string(LogLevel level) {
    switch (level) {
    case LogLevel::Info:
        return "info";
    case LogLevel::Warning:
        return "warning";
    case LogLevel::Error:
        return "error";
    }
    return "info";
}

struct StructuredLogEntry {
    std::string timestamp;
    LogLevel level = LogLevel::Info;
    std::string event;
    std::map<std::string, std::string> fields;
};

std::string redactSensitiveLogText(std::string_view text);
std::string formatStructuredLogEntry(const StructuredLogEntry &entry);

StructuredLogEntry makeLogEntry(LogLevel level, std::string_view event,
                                std::map<std::string, std::string> fields = {});

} // namespace shared
