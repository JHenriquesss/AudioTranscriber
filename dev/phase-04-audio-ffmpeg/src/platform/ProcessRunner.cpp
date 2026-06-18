#include "platform/ProcessRunner.hpp"

#include "shared/Error.hpp"

#include <array>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace platform {

namespace {

shared::AppError processFailure(ProcessErrorCode code, std::string message,
                                std::string technicalDetails, int exitCode = 0,
                                std::string stdoutText = {}, std::string stderrText = {}) {
    if (!stdoutText.empty()) {
        technicalDetails += "\nstdout:\n" + stdoutText;
    }
    if (!stderrText.empty()) {
        technicalDetails += "\nstderr:\n" + stderrText;
    }
    if (exitCode != 0) {
        technicalDetails += "\nexit_code=" + std::to_string(exitCode);
    }

    shared::ErrorCode mappedCode = shared::ErrorCode::Unknown;
    switch (code) {
    case ProcessErrorCode::ExecutableNotFound:
        mappedCode = shared::ErrorCode::FileNotFound;
        break;
    case ProcessErrorCode::StartFailed:
    case ProcessErrorCode::Timeout:
    case ProcessErrorCode::NonZeroExit:
        mappedCode = shared::ErrorCode::Unknown;
        break;
    }

    return shared::AppError{mappedCode, std::move(message), std::move(technicalDetails)};
}

#ifdef _WIN32

bool needsQuoting(const std::string &argument) {
    if (argument.empty()) {
        return true;
    }
    for (const char character : argument) {
        if (character == ' ' || character == '\t' || character == '"') {
            return true;
        }
    }
    return false;
}

HANDLE createReadablePipe(HANDLE &readHandle, HANDLE &writeHandle) {
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;

    if (!CreatePipe(&readHandle, &writeHandle, &attributes, 0)) {
        return INVALID_HANDLE_VALUE;
    }

    if (!SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(readHandle);
        CloseHandle(writeHandle);
        return INVALID_HANDLE_VALUE;
    }

    return readHandle;
}

std::string readPipeToEnd(HANDLE pipeHandle) {
    std::string output;
    std::array<char, 4096> buffer{};
    DWORD bytesRead = 0;

    while (ReadFile(pipeHandle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead,
                    nullptr) &&
           bytesRead > 0) {
        output.append(buffer.data(), bytesRead);
    }

    return output;
}

bool isBareExecutableName(const std::filesystem::path &executable) {
    return executable.parent_path().empty() || executable.parent_path() == ".";
}

HANDLE openNullInputHandle() {
    return CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
}

#endif

} // namespace

std::string escapeWindowsArgument(const std::string &argument) {
    if (!needsQuoting(argument)) {
        return argument;
    }

    std::string escaped = "\"";
    for (std::size_t index = 0; index < argument.size(); ++index) {
        std::size_t backslashCount = 0;
        while (index < argument.size() && argument[index] == '\\') {
            ++backslashCount;
            ++index;
        }

        if (index == argument.size()) {
            escaped.append(backslashCount * 2, '\\');
            break;
        }

        if (argument[index] == '"') {
            escaped.append(backslashCount * 2 + 1, '\\');
            escaped.push_back('"');
        } else {
            escaped.append(backslashCount, '\\');
            escaped.push_back(argument[index]);
        }
    }
    escaped.push_back('"');
    return escaped;
}

std::string buildWindowsCommandLine(const std::filesystem::path &executable,
                                    const std::vector<std::string> &arguments) {
    std::ostringstream commandLine;
    commandLine << escapeWindowsArgument(executable.string());
    for (const std::string &argument : arguments) {
        commandLine << ' ' << escapeWindowsArgument(argument);
    }
    return commandLine.str();
}

shared::Result<ProcessOutput> ProcessRunner::run(const ProcessSpec &spec) const {
#ifndef _WIN32
    return shared::Result<ProcessOutput>::failure(shared::AppError{
        shared::ErrorCode::Unknown,
        "Process execution is only implemented on Windows in this phase.",
        spec.executable.generic_string(),
    });
#else
    if (!isBareExecutableName(spec.executable) && !std::filesystem::exists(spec.executable)) {
        return shared::Result<ProcessOutput>::failure(processFailure(
            ProcessErrorCode::ExecutableNotFound, "The external tool was not found.",
            "Executable path does not exist: " + spec.executable.generic_string()));
    }

    HANDLE stdoutRead = INVALID_HANDLE_VALUE;
    HANDLE stdoutWrite = INVALID_HANDLE_VALUE;
    HANDLE stderrRead = INVALID_HANDLE_VALUE;
    HANDLE stderrWrite = INVALID_HANDLE_VALUE;

    if (createReadablePipe(stdoutRead, stdoutWrite) == INVALID_HANDLE_VALUE ||
        createReadablePipe(stderrRead, stderrWrite) == INVALID_HANDLE_VALUE) {
        if (stdoutRead != INVALID_HANDLE_VALUE) {
            CloseHandle(stdoutRead);
        }
        if (stdoutWrite != INVALID_HANDLE_VALUE) {
            CloseHandle(stdoutWrite);
        }
        if (stderrRead != INVALID_HANDLE_VALUE) {
            CloseHandle(stderrRead);
        }
        if (stderrWrite != INVALID_HANDLE_VALUE) {
            CloseHandle(stderrWrite);
        }
        return shared::Result<ProcessOutput>::failure(
            processFailure(ProcessErrorCode::StartFailed, "The external tool could not be started.",
                           "Failed to create output pipes."));
    }

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    const HANDLE nullInput = openNullInputHandle();
    startupInfo.hStdInput = nullInput != INVALID_HANDLE_VALUE ? nullInput : GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = stdoutWrite;
    startupInfo.hStdError = stderrWrite;

    PROCESS_INFORMATION processInfo{};
    const std::string commandLine = buildWindowsCommandLine(spec.executable, spec.arguments);
    std::vector<char> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back('\0');

    const BOOL started = CreateProcessA(
        nullptr, mutableCommandLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
        spec.workingDirectory.has_value() ? spec.workingDirectory->string().c_str() : nullptr,
        &startupInfo, &processInfo);

    if (nullInput != INVALID_HANDLE_VALUE) {
        CloseHandle(nullInput);
    }

    CloseHandle(stdoutWrite);
    CloseHandle(stderrWrite);

    if (!started) {
        CloseHandle(stdoutRead);
        CloseHandle(stderrRead);
        return shared::Result<ProcessOutput>::failure(processFailure(
            ProcessErrorCode::StartFailed, "The external tool could not be started.",
            "CreateProcess failed for: " + spec.executable.generic_string()));
    }

    const DWORD timeoutMs =
        spec.timeout.count() < 0
            ? INFINITE
            : static_cast<DWORD>(std::min<std::int64_t>(spec.timeout.count(), INT32_MAX));

    const DWORD waitResult = WaitForSingleObject(processInfo.hProcess, timeoutMs);
    ProcessOutput output{};

    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(processInfo.hProcess, 1);
        output.exitCode = 1;
        output.stdoutText = readPipeToEnd(stdoutRead);
        output.stderrText = readPipeToEnd(stderrRead);
        CloseHandle(stdoutRead);
        CloseHandle(stderrRead);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return shared::Result<ProcessOutput>::failure(processFailure(
            ProcessErrorCode::Timeout, "The external tool timed out.",
            "Process exceeded timeout of " + std::to_string(spec.timeout.count()) + " ms.",
            output.exitCode, output.stdoutText, output.stderrText));
    }

    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    output.exitCode = static_cast<int>(exitCode);
    output.stdoutText = readPipeToEnd(stdoutRead);
    output.stderrText = readPipeToEnd(stderrRead);

    CloseHandle(stdoutRead);
    CloseHandle(stderrRead);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    if (output.exitCode != 0) {
        return shared::Result<ProcessOutput>::failure(processFailure(
            ProcessErrorCode::NonZeroExit, "The external tool reported a failure.",
            "Process exited with non-zero status.", output.exitCode, output.stdoutText,
            output.stderrText));
    }

    return shared::Result<ProcessOutput>::success(std::move(output));
#endif
}

} // namespace platform
