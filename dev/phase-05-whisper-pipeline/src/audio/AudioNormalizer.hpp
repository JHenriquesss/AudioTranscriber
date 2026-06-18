#pragma once

#include "audio/AudioTypes.hpp"
#include "platform/ProcessRunner.hpp"
#include "shared/Result.hpp"

#include <filesystem>
#include <string>

namespace audio {

class AudioNormalizer {
public:
    explicit AudioNormalizer(AudioToolPaths toolPaths, AudioWorkspaceOptions workspaceOptions,
                             platform::ProcessRunner processRunner = {});

    shared::Result<NormalizedAudioResult>
    normalize(const std::filesystem::path &inputPath, const std::string &jobId) const;

    std::filesystem::path buildOutputPath(const std::string &jobId) const;

private:
    AudioToolPaths toolPaths_;
    AudioWorkspaceOptions workspaceOptions_;
    platform::ProcessRunner processRunner_;
};

} // namespace audio
