#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace test_support {

inline std::vector<std::filesystem::path>
findWhisperHeaderLeaks(const std::filesystem::path &sourceRoot) {
    std::vector<std::filesystem::path> violations;
    const auto srcRoot = sourceRoot / "src";

    if (!std::filesystem::exists(srcRoot)) {
        return violations;
    }

    for (const auto &entry : std::filesystem::recursive_directory_iterator(
             srcRoot, std::filesystem::directory_options::skip_permission_denied)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto extension = entry.path().extension().string();
        if (extension != ".hpp" && extension != ".cpp" && extension != ".h") {
            continue;
        }

        const auto relativePath = std::filesystem::relative(entry.path(), srcRoot);
        if (relativePath.string().rfind("whisper_engine", 0) == 0) {
            continue;
        }

        std::ifstream stream(entry.path());
        std::string line;
        while (std::getline(stream, line)) {
            if (line.find("whisper.h") != std::string::npos ||
                line.find("whisper.cpp") != std::string::npos) {
                violations.push_back(entry.path());
                break;
            }
        }
    }

    return violations;
}

} // namespace test_support
