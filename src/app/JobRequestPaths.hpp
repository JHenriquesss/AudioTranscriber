#pragma once

#include "AppPaths.hpp"
#include "TranscriptionJob.hpp"

#include "shared/Result.hpp"

namespace app {

[[nodiscard]] std::filesystem::path resolveJobOutputDirectory(
    const std::filesystem::path &outputDirectory, const AppPaths &paths);

[[nodiscard]] std::string exportBaseNameForInputFile(const std::filesystem::path &inputFile);

[[nodiscard]] shared::Result<void> ensureDirectoryExists(const std::filesystem::path &directory);

[[nodiscard]] TranscriptionJobRequest normalizeJobRequest(const TranscriptionJobRequest &request,
                                                          const AppPaths &paths);

} // namespace app
