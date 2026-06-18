#pragma once

#include "app/AppPaths.hpp"
#include "storage/ModelRepository.hpp"

#include "shared/Result.hpp"

#include <memory>
#include <vector>

namespace app {

struct ModelDisplayEntry {
    std::string id;
    std::string name;
};

class ModelCatalog {
  public:
    [[nodiscard]] static shared::Result<std::unique_ptr<ModelCatalog>> load(const AppPaths &paths);

    [[nodiscard]] const std::vector<ModelDisplayEntry> &entries() const;

  private:
    ModelCatalog() = default;
    explicit ModelCatalog(std::vector<ModelDisplayEntry> entries);

    std::vector<ModelDisplayEntry> entries_;
};

} // namespace app
