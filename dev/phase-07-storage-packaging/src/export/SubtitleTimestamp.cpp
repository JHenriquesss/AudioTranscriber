#include "export/SubtitleTimestamp.hpp"

#include <iomanip>
#include <sstream>

namespace export_format {

namespace {

std::string formatSubtitleTimestamp(int64_t totalMs, char millisecondSeparator) {
    if (totalMs < 0) {
        totalMs = 0;
    }

    const auto hours = totalMs / 3'600'000;
    const auto minutes = (totalMs % 3'600'000) / 60'000;
    const auto seconds = (totalMs % 60'000) / 1'000;
    const auto millis = totalMs % 1'000;

    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(2) << hours << ':' << std::setw(2) << minutes << ':'
           << std::setw(2) << seconds << millisecondSeparator << std::setw(3) << millis;
    return stream.str();
}

} // namespace

std::string formatSrtTimestamp(int64_t totalMs) {
    return formatSubtitleTimestamp(totalMs, ',');
}

std::string formatVttTimestamp(int64_t totalMs) {
    return formatSubtitleTimestamp(totalMs, '.');
}

} // namespace export_format
