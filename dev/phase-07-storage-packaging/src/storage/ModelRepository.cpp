#include "storage/ModelRepository.hpp"

#include <fstream>
#include <sstream>
#include <string_view>

namespace storage {

namespace {

shared::AppError configurationError(std::string message, std::string details = {}) {
    return shared::AppError{
        shared::ErrorCode::ConfigurationError,
        std::move(message),
        std::move(details),
    };
}

shared::AppError modelNotFoundError(std::string modelId) {
    return shared::AppError{
        shared::ErrorCode::ModelNotFound,
        "Model is not listed in the registry.",
        std::move(modelId),
    };
}

std::string_view beginningOfArray(std::string_view json, std::string_view fieldName) {
    const std::string key = "\"" + std::string(fieldName) + "\"";
    const auto keyPos = json.find(key);
    if (keyPos == std::string_view::npos) {
        return std::string_view{};
    }

    const auto bracketPos = json.find('[', keyPos);
    if (bracketPos == std::string_view::npos) {
        return std::string_view{};
    }

    return json.substr(bracketPos);
}

std::optional<std::string> readQuotedField(std::string_view objectJson, std::string_view fieldName) {
    const std::string key = "\"" + std::string(fieldName) + "\"";
    const auto keyPos = objectJson.find(key);
    if (keyPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto colonPos = objectJson.find(':', keyPos + key.size());
    if (colonPos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto quoteStart = objectJson.find('"', colonPos + 1);
    if (quoteStart == std::string_view::npos) {
        return std::nullopt;
    }

    const auto quoteEnd = objectJson.find('"', quoteStart + 1);
    if (quoteEnd == std::string_view::npos) {
        return std::nullopt;
    }

    return std::string(objectJson.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
}

std::vector<std::string> readStringArray(std::string_view objectJson, std::string_view fieldName) {
    const std::string key = "\"" + std::string(fieldName) + "\"";
    const auto keyPos = objectJson.find(key);
    if (keyPos == std::string_view::npos) {
        return {};
    }

    const auto bracketStart = objectJson.find('[', keyPos);
    const auto bracketEnd = objectJson.find(']', bracketStart);
    if (bracketStart == std::string_view::npos || bracketEnd == std::string_view::npos) {
        return {};
    }

    const std::string_view arrayBody = objectJson.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    std::vector<std::string> values;
    std::size_t cursor = 0;
    while (cursor < arrayBody.size()) {
        const auto quoteStart = arrayBody.find('"', cursor);
        if (quoteStart == std::string_view::npos) {
            break;
        }
        const auto quoteEnd = arrayBody.find('"', quoteStart + 1);
        if (quoteEnd == std::string_view::npos) {
            break;
        }
        values.emplace_back(arrayBody.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
        cursor = quoteEnd + 1;
    }
    return values;
}

std::vector<std::string_view> splitTopLevelObjects(std::string_view modelsArrayJson) {
    std::vector<std::string_view> objects;
    std::size_t depth = 0;
    std::size_t objectStart = std::string_view::npos;

    for (std::size_t index = 0; index < modelsArrayJson.size(); ++index) {
        const char character = modelsArrayJson[index];
        if (character == '{') {
            if (depth == 0) {
                objectStart = index;
            }
            ++depth;
        } else if (character == '}') {
            --depth;
            if (depth == 0 && objectStart != std::string_view::npos) {
                objects.push_back(modelsArrayJson.substr(objectStart, index - objectStart + 1));
                objectStart = std::string_view::npos;
            }
        }
    }

    return objects;
}

std::optional<ModelRegistryEntry> parseModelObject(std::string_view objectJson) {
    const auto id = readQuotedField(objectJson, "id");
    const auto name = readQuotedField(objectJson, "name");
    const auto path = readQuotedField(objectJson, "path");
    if (!id.has_value() || !name.has_value() || !path.has_value()) {
        return std::nullopt;
    }

    ModelRegistryEntry entry;
    entry.id = *id;
    entry.name = *name;
    entry.relativePath = *path;
    entry.recommendedFor = readStringArray(objectJson, "recommended_for");
    entry.languages = readStringArray(objectJson, "languages");
    return entry;
}

} // namespace

shared::Result<std::vector<ModelRegistryEntry>>
ModelRepository::loadIndexFile(const std::filesystem::path &indexPath) {
    if (!std::filesystem::exists(indexPath)) {
        return shared::Result<std::vector<ModelRegistryEntry>>::failure(
            configurationError("Model index file was not found.", indexPath.string()));
    }

    std::ifstream stream(indexPath, std::ios::binary);
    if (!stream.is_open()) {
        return shared::Result<std::vector<ModelRegistryEntry>>::failure(
            configurationError("Model index file could not be opened.", indexPath.string()));
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    const std::string json = buffer.str();
    if (json.empty()) {
        return shared::Result<std::vector<ModelRegistryEntry>>::failure(
            configurationError("Model index file is empty.", indexPath.string()));
    }

    const std::string_view modelsArray = beginningOfArray(json, "models");
    if (modelsArray.empty()) {
        return shared::Result<std::vector<ModelRegistryEntry>>::failure(
            configurationError("Model index is missing a models array.", indexPath.string()));
    }

    std::vector<ModelRegistryEntry> entries;
    for (const std::string_view objectJson : splitTopLevelObjects(modelsArray)) {
        const auto entry = parseModelObject(objectJson);
        if (!entry.has_value()) {
            return shared::Result<std::vector<ModelRegistryEntry>>::failure(
                configurationError("Model index contains an invalid model entry.", indexPath.string()));
        }
        entries.push_back(*entry);
    }

    if (entries.empty()) {
        return shared::Result<std::vector<ModelRegistryEntry>>::failure(
            configurationError("Model index does not list any models.", indexPath.string()));
    }

    return shared::Result<std::vector<ModelRegistryEntry>>::success(std::move(entries));
}

shared::Result<ModelRegistryEntry>
ModelRepository::findById(const std::vector<ModelRegistryEntry> &entries, const std::string &modelId) {
    for (const ModelRegistryEntry &entry : entries) {
        if (entry.id == modelId) {
            return shared::Result<ModelRegistryEntry>::success(entry);
        }
    }
    return shared::Result<ModelRegistryEntry>::failure(modelNotFoundError(modelId));
}

} // namespace storage
