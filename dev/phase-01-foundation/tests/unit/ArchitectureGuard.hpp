#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace test_support {

inline const std::vector<std::string> &forbiddenArchitectureFolderNames() {
    static const std::vector<std::string> names = {
        "domain", "application", "infrastructure", "usecases", "entities",
    };
    return names;
}

inline std::vector<std::filesystem::path>
findForbiddenArchitectureFolders(const std::filesystem::path &sourceRoot) {
    std::vector<std::filesystem::path> violations;
    const auto srcRoot = sourceRoot / "src";

    if (!std::filesystem::exists(srcRoot)) {
        return violations;
    }

    for (const auto &entry : std::filesystem::recursive_directory_iterator(
             srcRoot, std::filesystem::directory_options::skip_permission_denied)) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto folderName = entry.path().filename().string();
        for (const auto &forbidden : forbiddenArchitectureFolderNames()) {
            if (folderName == forbidden) {
                violations.push_back(entry.path());
            }
        }
    }

    return violations;
}

} // namespace test_support
