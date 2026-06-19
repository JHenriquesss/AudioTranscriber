#pragma once

#include <QString>

#include <filesystem>

namespace desktop {

inline std::filesystem::path toFilesystemPath(const QString &path) {
#if defined(_WIN32)
    return std::filesystem::path(path.toStdWString());
#else
    return std::filesystem::path(path.toUtf8().constData());
#endif
}

inline QString fromFilesystemPath(const std::filesystem::path &path) {
#if defined(_WIN32)
    return QString::fromStdWString(path.wstring());
#else
    return QString::fromUtf8(reinterpret_cast<const char *>(path.u8string().c_str()));
#endif
}

} // namespace desktop
