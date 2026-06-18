#pragma once

#include "shared/Result.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace platform {

enum class ProcessErrorCode {
    ExecutableNotFound,
    StartFailed,
    Timeout,
    NonZeroExit,
};

struct ProcessOutput {
    int exitCode = 0;
    std::string stdoutText;
    std::string stderrText;
};

struct ProcessSpec {
    std::filesystem::path executable;
    std::vector<std::string> arguments;
    std::chrono::milliseconds timeout{std::chrono::minutes{5}};
    std::optional<std::filesystem::path> workingDirectory;
};

std::string escapeWindowsArgument(const std::string &argument);

std::string buildWindowsCommandLine(const std::filesystem::path &executable,
                                    const std::vector<std::string> &arguments);

class ProcessRunner {
public:
    shared::Result<ProcessOutput> run(const ProcessSpec &spec) const;
};

} // namespace platform
