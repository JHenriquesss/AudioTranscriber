#pragma once

#include "shared/Result.hpp"

#include <filesystem>
#include <string>

namespace export_format {

shared::Result<void> writeTextFile(const std::filesystem::path &outputFile,
                                   const std::string &content);

} // namespace export_format
