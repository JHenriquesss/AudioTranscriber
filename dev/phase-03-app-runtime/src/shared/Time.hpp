#pragma once

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace shared {

inline std::string formatDurationMs(std::chrono::milliseconds duration) {
    const auto totalMs = duration.count();
    const auto hours = totalMs / 3'600'000;
    const auto minutes = (totalMs % 3'600'000) / 60'000;
    const auto seconds = (totalMs % 60'000) / 1'000;
    const auto millis = totalMs % 1'000;

    std::ostringstream stream;
    stream << std::setfill('0');
    if (hours > 0) {
        stream << hours << ':' << std::setw(2) << minutes << ':' << std::setw(2) << seconds << '.'
               << std::setw(3) << millis;
    } else {
        stream << minutes << ':' << std::setw(2) << seconds << '.' << std::setw(3) << millis;
    }
    return stream.str();
}

inline std::string formatTimestampUtc(std::chrono::system_clock::time_point timePoint) {
    const auto timeT = std::chrono::system_clock::to_time_t(timePoint);
    std::tm utcTime{};
#if defined(_WIN32)
    gmtime_s(&utcTime, &timeT);
#else
    gmtime_r(&timeT, &utcTime);
#endif

    std::ostringstream stream;
    stream << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

inline std::chrono::milliseconds durationFromSeconds(double seconds) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::duration<double>(seconds));
}

} // namespace shared
