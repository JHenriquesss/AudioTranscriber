#pragma once

#include <cstdint>
#include <string>

namespace export_format {

// SRT uses comma as the millisecond separator: 00:00:00,000
std::string formatSrtTimestamp(int64_t totalMs);

// VTT uses dot as the millisecond separator: 00:00:00.000
std::string formatVttTimestamp(int64_t totalMs);

} // namespace export_format
