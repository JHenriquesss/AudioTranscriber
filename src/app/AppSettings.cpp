#include "AppSettings.hpp"

#include "JobRequestPaths.hpp"

#include <fstream>
#include <optional>
#include <sstream>

namespace app {

namespace {

shared::AppError configurationError(std::string message, std::string details = {}) {
    return shared::AppError{
        shared::ErrorCode::ConfigurationError,
        std::move(message),
        std::move(details),
    };
}

std::string trim(std::string_view value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(start, end - start + 1));
}

std::optional<std::string> readJsonStringField(std::string_view json, std::string_view fieldName) {
    const std::string key = "\"" + std::string(fieldName) + "\"";
    const auto keyPos = json.find(key);
    if (keyPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto colonPos = json.find(':', keyPos + key.size());
    if (colonPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto quoteStart = json.find('"', colonPos + 1);
    if (quoteStart == std::string_view::npos) {
        return std::nullopt;
    }

    const auto quoteEnd = json.find('"', quoteStart + 1);
    if (quoteEnd == std::string_view::npos) {
        return std::nullopt;
    }

    return std::string(json.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
}

std::optional<bool> readJsonBoolField(std::string_view json, std::string_view fieldName) {
    const std::string key = "\"" + std::string(fieldName) + "\"";
    const auto keyPos = json.find(key);
    if (keyPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto colonPos = json.find(':', keyPos + key.size());
    if (colonPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto valueStart = json.find_first_not_of(" \t\r\n", colonPos + 1);
    if (valueStart == std::string_view::npos) {
        return std::nullopt;
    }

    if (json.compare(valueStart, 4, "true") == 0) {
        return true;
    }
    if (json.compare(valueStart, 5, "false") == 0) {
        return false;
    }
    return std::nullopt;
}

std::optional<int> readJsonIntField(std::string_view json, std::string_view fieldName) {
    const std::string key = "\"" + std::string(fieldName) + "\"";
    const auto keyPos = json.find(key);
    if (keyPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto colonPos = json.find(':', keyPos + key.size());
    if (colonPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto valueStart = json.find_first_not_of(" \t\r\n", colonPos + 1);
    if (valueStart == std::string_view::npos) {
        return std::nullopt;
    }

    const auto valueEnd = json.find_first_of(",}\r\n", valueStart);
    const auto token = trim(json.substr(valueStart, valueEnd - valueStart));
    try {
        return std::stoi(token);
    } catch (...) {
        return std::nullopt;
    }
}

std::string escapeJsonString(std::string_view value) {
    std::ostringstream stream;
    stream << '"';
    for (const char ch : value) {
        switch (ch) {
        case '"':
            stream << "\\\"";
            break;
        case '\\':
            stream << "\\\\";
            break;
        default:
            stream << ch;
            break;
        }
    }
    stream << '"';
    return stream.str();
}

std::string serializeSettings(const AppSettings &settings) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"default_language\": " << escapeJsonString(settings.defaultLanguage) << ",\n";
    stream << "  \"default_model\": " << escapeJsonString(settings.defaultModel) << ",\n";
    stream << "  \"output_directory\": " << escapeJsonString(settings.outputDirectory.string())
           << ",\n";
    stream << "  \"keep_temp_files\": " << (settings.keepTempFiles ? "true" : "false") << ",\n";
    stream << "  \"max_parallel_jobs\": " << settings.maxParallelJobs << ",\n";
    stream << "  \"enable_word_timestamps\": "
           << (settings.enableWordTimestamps ? "true" : "false") << ",\n";
    stream << "  \"theme\": " << escapeJsonString(settings.theme) << "\n";
    stream << "}\n";
    return stream.str();
}

bool isAllowedLanguage(const std::string &language) {
    return language == "auto" || language == "pt" || language == "en";
}

bool isAllowedTheme(const std::string &theme) {
    return theme == "system" || theme == "light" || theme == "dark";
}

} // namespace

AppSettingsStore::AppSettingsStore(AppPaths paths) : paths_(std::move(paths)) {}

AppSettings AppSettingsStore::defaults() {
    return AppSettings{};
}

shared::Result<AppSettings> validateSettings(const AppSettings &settings) {
    if (!isAllowedLanguage(settings.defaultLanguage)) {
        return shared::Result<AppSettings>::failure(
            configurationError("Invalid default language.", settings.defaultLanguage));
    }

    if (settings.defaultModel.empty()) {
        return shared::Result<AppSettings>::failure(
            configurationError("Default model must not be empty."));
    }

    if (settings.outputDirectory.empty()) {
        return shared::Result<AppSettings>::failure(
            configurationError("Output directory must not be empty."));
    }

    if (settings.maxParallelJobs < 1) {
        return shared::Result<AppSettings>::failure(
            configurationError("max_parallel_jobs must be at least 1."));
    }

    if (!isAllowedTheme(settings.theme)) {
        return shared::Result<AppSettings>::failure(
            configurationError("Invalid theme.", settings.theme));
    }

    return shared::Result<AppSettings>::success(settings);
}

shared::Result<AppSettings> AppSettingsStore::load() const {
    const auto settingsPath = paths_.settingsFile();
    if (!std::filesystem::exists(settingsPath)) {
        return shared::Result<AppSettings>::failure(
            configurationError("Settings file does not exist.", settingsPath.string()));
    }

    std::ifstream input(settingsPath);
    if (!input) {
        return shared::Result<AppSettings>::failure(
            configurationError("Could not open settings file.", settingsPath.string()));
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const auto json = buffer.str();
    if (trim(json).empty()) {
        return shared::Result<AppSettings>::failure(
            configurationError("Settings file is empty.", settingsPath.string()));
    }

    AppSettings settings = defaults();

    if (const auto language = readJsonStringField(json, "default_language")) {
        settings.defaultLanguage = *language;
    }
    if (const auto model = readJsonStringField(json, "default_model")) {
        settings.defaultModel = *model;
    }
    if (const auto outputDirectory = readJsonStringField(json, "output_directory")) {
        settings.outputDirectory = *outputDirectory;
    }
    if (const auto keepTempFiles = readJsonBoolField(json, "keep_temp_files")) {
        settings.keepTempFiles = *keepTempFiles;
    }
    if (const auto maxParallelJobs = readJsonIntField(json, "max_parallel_jobs")) {
        settings.maxParallelJobs = *maxParallelJobs;
    }
    if (const auto enableWordTimestamps = readJsonBoolField(json, "enable_word_timestamps")) {
        settings.enableWordTimestamps = *enableWordTimestamps;
    }
    if (const auto theme = readJsonStringField(json, "theme")) {
        settings.theme = *theme;
    }

    settings.outputDirectory = resolveJobOutputDirectory(settings.outputDirectory, paths_);
    return validateSettings(settings);
}

shared::Result<void> AppSettingsStore::save(const AppSettings &settings) const {
    const auto validated = validateSettings(settings);
    if (!validated.ok) {
        return shared::Result<void>::failure(validated.error);
    }

    const auto settingsPath = paths_.settingsFile();
    std::error_code errorCode;
    std::filesystem::create_directories(settingsPath.parent_path(), errorCode);

    std::ofstream output(settingsPath, std::ios::trunc);
    if (!output) {
        return shared::Result<void>::failure(
            configurationError("Could not write settings file.", settingsPath.string()));
    }

    output << serializeSettings(validated.value);
    if (!output) {
        return shared::Result<void>::failure(
            configurationError("Failed while writing settings file.", settingsPath.string()));
    }

    return shared::Result<void>::success();
}

shared::Result<AppSettings> AppSettingsStore::loadOrCreateDefaults() {
    const auto settingsPath = paths_.settingsFile();
    if (!std::filesystem::exists(settingsPath)) {
        auto defaults = AppSettingsStore::defaults();
        defaults.outputDirectory = paths_.exportsDirectory();
        const auto saved = save(defaults);
        if (!saved.ok) {
            return shared::Result<AppSettings>::failure(saved.error);
        }
        return shared::Result<AppSettings>::success(defaults);
    }

    return load();
}

} // namespace app
