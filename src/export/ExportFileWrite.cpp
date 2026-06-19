#include "export/ExportFileWrite.hpp"

#include "shared/Error.hpp"

#include <fstream>
#include <system_error>

namespace export_format {

shared::Result<void> writeTextFile(const std::filesystem::path &outputFile,
                                   const std::string &content) {
    std::error_code errorCode;
    std::filesystem::create_directories(outputFile.parent_path(), errorCode);
    if (errorCode) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Could not create export folder.",
            errorCode.message(),
        });
    }

    std::ofstream stream(outputFile, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Could not write export file.",
            "Failed to open output path for writing: " + outputFile.generic_string(),
        });
    }

    stream << content;
    if (!stream.good()) {
        return shared::Result<void>::failure(shared::AppError{
            shared::ErrorCode::ExportFailed,
            "Could not write export file.",
            "Write failed for output path: " + outputFile.generic_string(),
        });
    }

    return shared::Result<void>::success();
}

} // namespace export_format
