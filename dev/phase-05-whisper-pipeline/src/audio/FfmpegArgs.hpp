#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace audio {

std::vector<std::string> buildProbeArguments(const std::filesystem::path &inputPath);

std::vector<std::string> buildExtractArguments(const std::filesystem::path &inputPath,
                                               const std::filesystem::path &outputPath);

std::vector<std::string> buildNormalizeArguments(const std::filesystem::path &inputPath,
                                                 const std::filesystem::path &outputPath);

} // namespace audio
