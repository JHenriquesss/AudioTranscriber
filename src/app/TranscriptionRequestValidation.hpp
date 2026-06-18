#pragma once

#include "TranscriptionJob.hpp"

#include "shared/Result.hpp"

namespace app {

[[nodiscard]] shared::Result<void> validateTranscriptionJobRequest(
    const TranscriptionJobRequest &request);

} // namespace app
