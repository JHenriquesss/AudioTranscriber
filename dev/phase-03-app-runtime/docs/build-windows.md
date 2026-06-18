# Build on Windows

Phase 01 builds with C++20, CMake Presets, MSVC, and Ninja.

## Prerequisites

- Visual Studio 2022 with the **Desktop development with C++** workload
- CMake 3.24 or newer (included with VS or installed separately)
- Git (FetchContent downloads Catch2 during configure)

Open a **x64 Native Tools Command Prompt for VS 2022**, or run `vcvars64.bat` before commands below.

## Configure

From the phase root (`dev/phase-01-foundation`):

```powershell
cmake --preset windows-msvc-debug
```

This generates the build tree at `build/windows-msvc-debug/`.

## Build

```powershell
cmake --build --preset debug
```

## Run tests

```powershell
ctest --test-dir build/windows-msvc-debug --output-on-failure
```

Or run the test executable directly:

```powershell
build\windows-msvc-debug\tests\unit\phase01_unit_tests.exe
```

## Release build (optional)

```powershell
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir build/windows-msvc-release --output-on-failure
```

## Formatting check

When `clang-format` is available:

```powershell
clang-format --dry-run --Werror src/shared/*.hpp tests/unit/*.cpp tests/unit/*.hpp
```

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `cl` or `ninja` not found | Use the VS Native Tools prompt or run `vcvars64.bat` |
| FetchContent fails | Check network access for the first configure; Catch2 is fetched from GitHub |
| Preset not found | Run commands from the phase folder that contains `CMakePresets.json` |

## Out of scope for phase 01

Qt, whisper.cpp, SQLite, and FFmpeg are not required to build or test phase 01.
