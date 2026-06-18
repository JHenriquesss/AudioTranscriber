#pragma once

#include <string>

namespace shared {

enum class ErrorCode {
    Unknown,
    FileNotFound,
    UnsupportedMediaFormat,
    AudioProbeFailed,
    AudioExtractionFailed,
    AudioNormalizationFailed,
    ModelNotFound,
    ModelLoadFailed,
    TranscriptionFailed,
    ExportFailed,
    DatabaseError,
    Cancelled
};

inline std::string to_string(ErrorCode code) {
    switch (code) {
    case ErrorCode::Unknown:
        return "Unknown";
    case ErrorCode::FileNotFound:
        return "FileNotFound";
    case ErrorCode::UnsupportedMediaFormat:
        return "UnsupportedMediaFormat";
    case ErrorCode::AudioProbeFailed:
        return "AudioProbeFailed";
    case ErrorCode::AudioExtractionFailed:
        return "AudioExtractionFailed";
    case ErrorCode::AudioNormalizationFailed:
        return "AudioNormalizationFailed";
    case ErrorCode::ModelNotFound:
        return "ModelNotFound";
    case ErrorCode::ModelLoadFailed:
        return "ModelLoadFailed";
    case ErrorCode::TranscriptionFailed:
        return "TranscriptionFailed";
    case ErrorCode::ExportFailed:
        return "ExportFailed";
    case ErrorCode::DatabaseError:
        return "DatabaseError";
    case ErrorCode::Cancelled:
        return "Cancelled";
    }
    return "Unknown";
}

struct AppError {
    ErrorCode code = ErrorCode::Unknown;
    std::string message;
    std::string technicalDetails;
};

} // namespace shared
