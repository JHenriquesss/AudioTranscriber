#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace test_support {

inline std::filesystem::path fixturesRoot() {
#ifdef PROJECT_SOURCE_ROOT
    return std::filesystem::path(PROJECT_SOURCE_ROOT) / "tests" / "fixtures";
#else
    return std::filesystem::path("tests") / "fixtures";
#endif
}

inline std::string readTextFile(const std::filesystem::path &path) {
    std::ifstream stream(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    std::string content = buffer.str();

    std::string normalized;
    normalized.reserve(content.size());
    for (std::size_t index = 0; index < content.size(); ++index) {
        if (content[index] == '\r' && index + 1 < content.size() && content[index + 1] == '\n') {
            normalized.push_back('\n');
            ++index;
            continue;
        }
        normalized.push_back(content[index]);
    }

    return normalized;
}

} // namespace test_support
