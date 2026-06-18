#pragma once

#include "audio/AudioTypes.hpp"
#include "platform/ProcessRunner.hpp"
#include "shared/Result.hpp"

#include <filesystem>

namespace audio {

class AudioProbe {
public:
    explicit AudioProbe(AudioToolPaths toolPaths, platform::ProcessRunner processRunner = {});

    shared::Result<MediaProbeResult> probe(const std::filesystem::path &inputPath) const;

private:
    AudioToolPaths toolPaths_;
    platform::ProcessRunner processRunner_;
};

MediaProbeResult parseProbeOutput(const std::filesystem::path &inputPath,
                                  const std::string &ffmpegStderr);

} // namespace audio
