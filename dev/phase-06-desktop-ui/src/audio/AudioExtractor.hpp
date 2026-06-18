#pragma once

#include "audio/AudioTypes.hpp"
#include "platform/ProcessRunner.hpp"
#include "shared/Result.hpp"

#include <filesystem>
#include <string>

namespace audio {

class AudioExtractor {
public:
    explicit AudioExtractor(AudioToolPaths toolPaths, AudioWorkspaceOptions workspaceOptions,
                            platform::ProcessRunner processRunner = {});

    shared::Result<std::filesystem::path>
    extract(const std::filesystem::path &inputPath, const std::string &jobId) const;

private:
    AudioToolPaths toolPaths_;
    AudioWorkspaceOptions workspaceOptions_;
    platform::ProcessRunner processRunner_;

    std::filesystem::path buildOutputPath(const std::string &jobId) const;
};

} // namespace audio
