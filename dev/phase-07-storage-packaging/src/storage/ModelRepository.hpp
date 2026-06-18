#pragma once

#include "shared/Result.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace storage {

struct ModelRegistryEntry {
    std::string id;
    std::string name;
    std::filesystem::path relativePath;
    std::vector<std::string> recommendedFor;
    std::vector<std::string> languages;
};

class ModelRepository {
  public:
    [[nodiscard]] static shared::Result<std::vector<ModelRegistryEntry>>
    loadIndexFile(const std::filesystem::path &indexPath);

    [[nodiscard]] static shared::Result<ModelRegistryEntry>
    findById(const std::vector<ModelRegistryEntry> &entries, const std::string &modelId);
};

} // namespace storage
