#include "Logger.hpp"

#include <sstream>

namespace shared {

namespace {

std::string escapeJsonString(std::string_view value) {
    std::ostringstream stream;
    stream << '"';
    for (const char ch : value) {
        switch (ch) {
        case '"':
            stream << "\\\"";
            break;
        case '\\':
            stream << "\\\\";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            stream << ch;
            break;
        }
    }
    stream << '"';
    return stream.str();
}

bool looksLikeTranscriptPayload(std::string_view text) {
    return text.find("\"segments\"") != std::string_view::npos ||
           text.find("\"transcript\"") != std::string_view::npos ||
           text.find("\"text\":") != std::string_view::npos;
}

} // namespace

std::string redactSensitiveLogText(std::string_view text) {
    if (looksLikeTranscriptPayload(text)) {
        return "[REDACTED]";
    }

    constexpr std::string_view redactedMarker = "[REDACTED]";
    std::string result(text);
    const std::string segmentMarker = "\"text\":";
    std::size_t position = 0;
    while ((position = result.find(segmentMarker, position)) != std::string::npos) {
        const auto valueStart = result.find('"', position + segmentMarker.size());
        if (valueStart == std::string::npos) {
            break;
        }

        const auto valueEnd = result.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) {
            break;
        }

        result.replace(valueStart + 1, valueEnd - valueStart - 1, redactedMarker);
        position = valueStart + 1 + redactedMarker.size();
    }

    return result;
}

std::string formatStructuredLogEntry(const StructuredLogEntry &entry) {
    std::ostringstream stream;
    stream << '{';
    stream << "\"timestamp\":" << escapeJsonString(entry.timestamp) << ',';
    stream << "\"level\":" << escapeJsonString(to_string(entry.level)) << ',';
    stream << "\"event\":" << escapeJsonString(entry.event);

    for (const auto &[key, value] : entry.fields) {
        stream << ',' << escapeJsonString(key) << ':' << escapeJsonString(redactSensitiveLogText(value));
    }

    stream << '}';
    return stream.str();
}

StructuredLogEntry makeLogEntry(LogLevel level, std::string_view event,
                                std::map<std::string, std::string> fields) {
    StructuredLogEntry entry;
    entry.timestamp = formatTimestampUtc(std::chrono::system_clock::now());
    entry.level = level;
    entry.event = std::string(event);
    entry.fields = std::move(fields);
    return entry;
}

} // namespace shared
