cmake_minimum_required(VERSION 3.24)

project(OfflineTranscriberPhase04 VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(offline_transcriber_shared INTERFACE)
target_include_directories(offline_transcriber_shared INTERFACE "${CMAKE_SOURCE_DIR}/src")

add_library(offline_transcriber_logging STATIC
    src/shared/Logger.cpp
)
target_link_libraries(offline_transcriber_logging PUBLIC offline_transcriber_shared)

add_library(offline_transcriber_platform STATIC
    src/platform/ProcessRunner.cpp
)
target_link_libraries(offline_transcriber_platform PUBLIC offline_transcriber_shared)

add_library(offline_transcriber_audio STATIC
    src/audio/FfmpegArgs.cpp
    src/audio/AudioProbe.cpp
    src/audio/AudioExtractor.cpp
    src/audio/AudioNormalizer.cpp
)
target_link_libraries(offline_transcriber_audio PUBLIC offline_transcriber_platform)

add_library(offline_transcriber_transcription STATIC
    src/transcription/TranscriptValidation.cpp
)
target_link_libraries(offline_transcriber_transcription PUBLIC offline_transcriber_shared)

add_library(offline_transcriber_export STATIC
    src/export/SubtitleTimestamp.cpp
    src/export/ExportFileWrite.cpp
    src/export/TxtExporter.cpp
    src/export/SrtExporter.cpp
    src/export/VttExporter.cpp
    src/export/JsonExporter.cpp
    src/export/ExportService.cpp
)
target_link_libraries(offline_transcriber_export PUBLIC offline_transcriber_transcription)

add_library(offline_transcriber_app STATIC
    src/app/AppPaths.cpp
    src/app/AppSettings.cpp
    src/app/JobStatus.cpp
    src/app/TranscriptionJob.cpp
    src/app/JobManager.cpp
)
target_link_libraries(offline_transcriber_app PUBLIC offline_transcriber_logging)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_subdirectory(tests/unit)
