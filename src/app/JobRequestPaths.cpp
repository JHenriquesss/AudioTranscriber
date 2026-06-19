#include "JobRequestPaths.hpp"

#include "AppPaths.hpp"

#include "shared/Error.hpp"

#include <system_error>

namespace app {

std::filesystem::path resolveJobOutputDirectory(const std::filesystem::path &outputDirectory,
                                                const AppPaths &paths) {
    if (outputDirectory.empty()) {
        return std::filesystem::absolute(paths.exportsDirectory());
    }

    if (outputDirectory.is_relative()) {
        return std::filesystem::absolute(paths.applicationRoot / outputDirectory);
    }

    return std::filesystem::absolute(outputDirectory);
}

std::string exportBaseNameForInputFile(const std::filesystem::path &inputFile) {
    const auto stem = inputFile.stem().u8string();
    return std::string(reinterpret_cast<const char *>(stem.data()), stem.size());
}

shared::Result<void> ensureDirectoryExists(const std::filesystem::path &directory) {
    if (directory.empty()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ConfigurationError,
            "Select an output folder before starting.",
            "output directory path is empty",
        });
    }

    std::error_code errorCode;
    std::filesystem::create_directories(directory, errorCode);
    if (errorCode) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Could not create the output folder.",
            errorCode.message(),
        });
    }

    return shared::Result<void>::success();
}

TranscriptionJobRequest normalizeJobRequest(const TranscriptionJobRequest &request,
                                              const AppPaths &paths) {
    TranscriptionJobRequest normalized = request;
    normalized.outputDirectory = resolveJobOutputDirectory(request.outputDirectory, paths);
    return normalized;
}

} // namespace app
