#pragma once

#include "shared/Result.hpp"

#include <filesystem>
#include <string>

namespace whisper_engine {

class WhisperModel {
  public:
    explicit WhisperModel(std::filesystem::path modelsDirectory);

    [[nodiscard]] shared::Result<std::filesystem::path>
    resolveModelPath(const std::string &modelId) const;

  private:
    std::filesystem::path modelsDirectory_;
};

} // namespace whisper_engine
