#pragma once

#include "shared/Result.hpp"
#include "transcription/Transcript.hpp"

namespace transcription {

// Empty transcripts are valid. Exporters produce deterministic empty outputs documented
// in each exporter (TXT/SRT: empty file; VTT: WEBVTT header only; JSON: metadata with
// empty segments array).
shared::Result<void> validateTranscript(const TranscriptDocument &document);

} // namespace transcription
