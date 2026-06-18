#pragma once

#include <filesystem>

namespace app {

struct AppPaths {
    std::filesystem::path applicationRoot;

    static AppPaths resolve(const std::filesystem::path &executableDirectory);
    static AppPaths fromUserDataRoot(const std::filesystem::path &userDataRoot, bool portableMode);

    [[nodiscard]] bool isPortableMode() const;

    [[nodiscard]] std::filesystem::path dataDirectory() const;
    [[nodiscard]] std::filesystem::path settingsFile() const;
    [[nodiscard]] std::filesystem::path logsDirectory() const;
    [[nodiscard]] std::filesystem::path exportsDirectory() const;
    [[nodiscard]] std::filesystem::path tempDirectory() const;

  private:
    std::filesystem::path userDataRoot_;
    bool portableMode_ = true;
};

} // namespace app
