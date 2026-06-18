#pragma once

#include "AppPaths.hpp"

#include "shared/Result.hpp"

#include <filesystem>
#include <string>

namespace app {

struct AppSettings {
    std::string defaultLanguage = "auto";
    std::string defaultModel = "small";
    std::filesystem::path outputDirectory = "exports";
    bool keepTempFiles = false;
    int maxParallelJobs = 1;
    bool enableWordTimestamps = true;
    std::string theme = "system";
};

class AppSettingsStore {
  public:
    explicit AppSettingsStore(AppPaths paths);

    static AppSettings defaults();

    [[nodiscard]] shared::Result<AppSettings> load() const;
    [[nodiscard]] shared::Result<void> save(const AppSettings &settings) const;
    [[nodiscard]] shared::Result<AppSettings> loadOrCreateDefaults();

  private:
    AppPaths paths_;
};

shared::Result<AppSettings> validateSettings(const AppSettings &settings);

} // namespace app
