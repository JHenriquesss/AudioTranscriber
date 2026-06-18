#include "transcription/TranscriptionPipeline.hpp"

#include "shared/Time.hpp"
#include "transcription/TranscriptPostProcessor.hpp"
#include "transcription/TranscriptValidation.hpp"
#include "whisper_engine/WhisperModel.hpp"
#include "whisper_engine/WhisperOptionsMapper.hpp"

#include <algorithm>
#include <chrono>

namespace transcription {

namespace {

shared::AppError cancelledError(const char *stage) {
    return shared::AppError{
        shared::ErrorCode::Cancelled,
        "Transcription was cancelled.",
        std::string("stage=") + stage,
    };
}

shared::Result<void> ensureNotCancelled(shared::CancellationToken &token, const char *stage) {
    if (token.isCancelled()) {
        return shared::Result<void>::failure(cancelledError(stage));
    }
    return shared::Result<void>::success();
}

int64_t computeDurationMs(const std::vector<whisper_engine::WhisperTranscriptSegment> &segments) {
    int64_t durationMs = 0;
    for (const auto &segment : segments) {
        durationMs = std::max(durationMs, segment.endMs);
    }
    return durationMs;
}

TranscriptDocument buildTranscriptDocument(const TranscriptionOptions &options,
                                           const std::string &detectedLanguage,
                                           const std::vector<whisper_engine::WhisperTranscriptSegment> &segments) {
    TranscriptDocument document;
    document.id = options.jobId;
    document.sourceFile = options.sourceFile;
    document.requestedLanguage = options.language;
    document.detectedLanguage = detectedLanguage;
    document.modelId = options.modelId;
    document.durationMs = computeDurationMs(segments);
    document.createdAtIso =
        shared::formatTimestampUtc(std::chrono::system_clock::now());

    document.segments.reserve(segments.size());
    for (std::size_t index = 0; index < segments.size(); ++index) {
        const auto &sourceSegment = segments[index];
        TranscriptSegment segment;
        segment.index = static_cast<int>(index);
        segment.startMs = sourceSegment.startMs;
        segment.endMs = sourceSegment.endMs;
        segment.text = sourceSegment.text;
        document.segments.push_back(std::move(segment));
    }

    return document;
}

} // namespace

TranscriptionPipeline::TranscriptionPipeline(whisper_engine::WhisperEngine engine)
    : engine_(std::move(engine)) {}

shared::Result<TranscriptionResult>
TranscriptionPipeline::run(const TranscriptionOptions &options,
                           shared::CancellationToken &token) {
    if (auto cancelled = ensureNotCancelled(token, "pipeline_start"); !cancelled.ok) {
        return shared::Result<TranscriptionResult>::failure(cancelled.error);
    }

    if (!std::filesystem::is_regular_file(options.normalizedAudioPath)) {
        return shared::Result<TranscriptionResult>::failure(shared::AppError{
            shared::ErrorCode::FileNotFound,
            "Normalized audio file was not found.",
            "normalizedAudioPath=" + options.normalizedAudioPath.string(),
        });
    }

    whisper_engine::WhisperModel modelResolver(options.modelsDirectory);
    const auto modelPathResult = modelResolver.resolveModelPath(options.modelId);
    if (!modelPathResult.ok) {
        return shared::Result<TranscriptionResult>::failure(modelPathResult.error);
    }

    if (auto cancelled = ensureNotCancelled(token, "before_model_load"); !cancelled.ok) {
        return shared::Result<TranscriptionResult>::failure(cancelled.error);
    }

    const auto loadResult = engine_.loadModel(modelPathResult.value, token);
    if (!loadResult.ok) {
        return shared::Result<TranscriptionResult>::failure(loadResult.error);
    }

    if (auto cancelled = ensureNotCancelled(token, "before_transcribe"); !cancelled.ok) {
        return shared::Result<TranscriptionResult>::failure(cancelled.error);
    }

    whisper_engine::WhisperRuntimeOptions runtimeOptions;
    runtimeOptions.languageCode = whisper_engine::mapLanguageToWhisperCode(options.language);
    runtimeOptions.enableWordTimestamps = options.enableWordTimestamps;

    const auto transcribeResult =
        engine_.transcribe(options.normalizedAudioPath, runtimeOptions, token);
    if (!transcribeResult.ok) {
        return shared::Result<TranscriptionResult>::failure(transcribeResult.error);
    }

    if (auto cancelled = ensureNotCancelled(token, "after_transcribe"); !cancelled.ok) {
        return shared::Result<TranscriptionResult>::failure(cancelled.error);
    }

    const auto &segments = transcribeResult.value;
    if (segments.empty()) {
        return shared::Result<TranscriptionResult>::failure(shared::AppError{
            shared::ErrorCode::TranscriptionFailed,
            "Transcription produced no segments.",
            "jobId=" + options.jobId,
        });
    }

    const std::string detectedLanguage =
        segments.front().detectedLanguage.empty() ? options.language
                                                  : segments.front().detectedLanguage;

    auto document = buildTranscriptDocument(options, detectedLanguage, segments);
    document = postProcessTranscript(document);

    const auto validationResult = validateTranscript(document);
    if (!validationResult.ok) {
        return shared::Result<TranscriptionResult>::failure(validationResult.error);
    }

    if (auto cancelled = ensureNotCancelled(token, "before_export_handoff"); !cancelled.ok) {
        return shared::Result<TranscriptionResult>::failure(cancelled.error);
    }

    TranscriptionResult result;
    result.transcript = std::move(document);
    return shared::Result<TranscriptionResult>::success(std::move(result));
}

} // namespace transcription
