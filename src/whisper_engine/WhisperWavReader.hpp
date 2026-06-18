#pragma once

#include "shared/Result.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace whisper_engine {

[[nodiscard]] shared::Result<std::vector<float>>
readMonoPcm16Wav(const std::filesystem::path &wavPath, int expectedSampleRateHz = 16000);

} // namespace whisper_engine
