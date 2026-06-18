#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>

namespace test_support {

inline std::filesystem::path uniqueTempRoot(const std::string &prefix) {
    static std::atomic<int> counter{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() /
                      (prefix + "-" + std::to_string(stamp) + "-" +
                       std::to_string(counter.fetch_add(1)));
    std::filesystem::create_directories(root);
    return root;
}

inline void cleanupTempRoot(const std::filesystem::path &root) {
    std::error_code errorCode;
    std::filesystem::remove_all(root, errorCode);
}

} // namespace test_support
