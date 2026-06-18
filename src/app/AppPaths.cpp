#include "AppPaths.hpp"

#include <cstdlib>

namespace app {

namespace {

constexpr const char *kPortableDataFolder = "data";
constexpr const char *kSettingsFileName = "settings.json";
constexpr const char *kLogsFolder = "logs";
constexpr const char *kExportsFolder = "exports";
constexpr const char *kTempFolder = "temp";
constexpr const char *kDatabaseFileName = "app.db";
constexpr const char *kTranscriptsFolder = "transcripts";
constexpr const char *kModelsFolder = "models";
constexpr const char *kModelIndexFileName = "index.json";
constexpr const char *kInstalledAppFolder = "OfflineTranscriber";

std::filesystem::path resolveInstalledUserDataRoot() {
#if defined(_WIN32)
    if (const char *appData = std::getenv("APPDATA")) {
        return std::filesystem::path(appData) / kInstalledAppFolder;
    }
#endif
    return std::filesystem::path(kInstalledAppFolder);
}

} // namespace

AppPaths AppPaths::resolve(const std::filesystem::path &executableDirectory) {
    AppPaths paths;
    paths.applicationRoot = executableDirectory;

    const auto portableDataDirectory = executableDirectory / kPortableDataFolder;
    if (std::filesystem::exists(portableDataDirectory)) {
        paths.portableMode_ = true;
        paths.userDataRoot_ = executableDirectory;
        return paths;
    }

    paths.portableMode_ = false;
    paths.userDataRoot_ = resolveInstalledUserDataRoot();
    return paths;
}

AppPaths AppPaths::fromUserDataRoot(const std::filesystem::path &userDataRoot, bool portableMode) {
    AppPaths paths;
    paths.applicationRoot = userDataRoot;
    paths.userDataRoot_ = userDataRoot;
    paths.portableMode_ = portableMode;
    return paths;
}

bool AppPaths::isPortableMode() const {
    return portableMode_;
}

std::filesystem::path AppPaths::dataDirectory() const {
    if (portableMode_) {
        return userDataRoot_ / kPortableDataFolder;
    }
    return userDataRoot_;
}

std::filesystem::path AppPaths::settingsFile() const {
    return dataDirectory() / kSettingsFileName;
}

std::filesystem::path AppPaths::logsDirectory() const {
    return dataDirectory() / kLogsFolder;
}

std::filesystem::path AppPaths::exportsDirectory() const {
    return dataDirectory() / kExportsFolder;
}

std::filesystem::path AppPaths::tempDirectory() const {
    return dataDirectory() / kTempFolder;
}

std::filesystem::path AppPaths::databaseFile() const {
    return dataDirectory() / kDatabaseFileName;
}

std::filesystem::path AppPaths::transcriptsDirectory() const {
    return dataDirectory() / kTranscriptsFolder;
}

std::filesystem::path AppPaths::modelsDirectory() const {
    return applicationRoot / kModelsFolder;
}

std::filesystem::path AppPaths::modelIndexFile() const {
    return modelsDirectory() / kModelIndexFileName;
}

} // namespace app
