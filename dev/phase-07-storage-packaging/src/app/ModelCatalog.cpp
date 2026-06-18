#include "app/ModelCatalog.hpp"

namespace app {

ModelCatalog::ModelCatalog(std::vector<ModelDisplayEntry> entries) : entries_(std::move(entries)) {}

shared::Result<std::unique_ptr<ModelCatalog>> ModelCatalog::load(const AppPaths &paths) {
    const auto registryResult = storage::ModelRepository::loadIndexFile(paths.modelIndexFile());
    if (!registryResult.ok) {
        return shared::Result<std::unique_ptr<ModelCatalog>>::failure(registryResult.error);
    }

    std::vector<ModelDisplayEntry> entries;
    entries.reserve(registryResult.value.size());
    for (const storage::ModelRegistryEntry &entry : registryResult.value) {
        entries.push_back(ModelDisplayEntry{entry.id, entry.name});
    }

    return shared::Result<std::unique_ptr<ModelCatalog>>::success(
        std::unique_ptr<ModelCatalog>(new ModelCatalog(std::move(entries))));
}

const std::vector<ModelDisplayEntry> &ModelCatalog::entries() const {
    return entries_;
}

} // namespace app
