cmake_minimum_required(VERSION 3.24)

project(OfflineTranscriber VERSION 0.7.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(OFFLINE_TRANSCRIBER_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
set(OFFLINE_TRANSCRIBER_SQLITE_DIR "${OFFLINE_TRANSCRIBER_THIRD_PARTY_DIR}/sqlite")
set(OFFLINE_TRANSCRIBER_CATCH2_DIR "${OFFLINE_TRANSCRIBER_THIRD_PARTY_DIR}/catch2")
set(OFFLINE_TRANSCRIBER_WHISPER_DIR "${OFFLINE_TRANSCRIBER_THIRD_PARTY_DIR}/whisper.cpp")

if(NOT EXISTS "${OFFLINE_TRANSCRIBER_SQLITE_DIR}/sqlite3.c")
    message(FATAL_ERROR
        "Missing vendored SQLite sources at ${OFFLINE_TRANSCRIBER_SQLITE_DIR}. "
        "Run tools/bootstrap-third-party.ps1")
endif()
if(NOT EXISTS "${OFFLINE_TRANSCRIBER_CATCH2_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Missing vendored Catch2 sources at ${OFFLINE_TRANSCRIBER_CATCH2_DIR}. "
        "Run tools/bootstrap-third-party.ps1")
endif()
if(NOT EXISTS "${OFFLINE_TRANSCRIBER_WHISPER_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Missing vendored whisper.cpp sources at ${OFFLINE_TRANSCRIBER_WHISPER_DIR}. "
        "Run tools/bootstrap-third-party.ps1")
endif()

add_library(offline_transcriber_sqlite STATIC
    ${OFFLINE_TRANSCRIBER_SQLITE_DIR}/sqlite3.c
)
target_include_directories(offline_transcriber_sqlite PUBLIC ${OFFLINE_TRANSCRIBER_SQLITE_DIR})
if(MSVC)
    target_compile_options(offline_transcriber_sqlite PRIVATE /wd4018 /wd4244 /wd4267)
endif()

set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_SERVER OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(${OFFLINE_TRANSCRIBER_WHISPER_DIR} ${CMAKE_BINARY_DIR}/third_party/whisper.cpp EXCLUDE_FROM_ALL)

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

add_library(offline_transcriber_whisper_engine STATIC
    src/whisper_engine/WhisperModel.cpp
    src/whisper_engine/WhisperOptionsMapper.cpp
    src/whisper_engine/WhisperEngine.cpp
    src/whisper_engine/WhisperEngineCpp.cpp
    src/whisper_engine/WhisperWavReader.cpp
)
target_link_libraries(offline_transcriber_whisper_engine PUBLIC offline_transcriber_shared whisper)
target_include_directories(offline_transcriber_whisper_engine PRIVATE
    ${OFFLINE_TRANSCRIBER_WHISPER_DIR}/include)

add_library(offline_transcriber_transcription STATIC
    src/transcription/TranscriptValidation.cpp
    src/transcription/TranscriptPostProcessor.cpp
    src/transcription/TranscriptionPipeline.cpp
)
target_link_libraries(offline_transcriber_transcription PUBLIC offline_transcriber_whisper_engine)

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

add_library(offline_transcriber_storage STATIC
    src/storage/Database.cpp
    src/storage/JobRepository.cpp
    src/storage/TranscriptRepository.cpp
    src/storage/ModelRepository.cpp
)
target_link_libraries(offline_transcriber_storage PUBLIC offline_transcriber_shared offline_transcriber_sqlite)

add_library(offline_transcriber_app STATIC
    src/app/AppPaths.cpp
    src/app/AppSettings.cpp
    src/app/JobStatus.cpp
    src/app/TranscriptionJob.cpp
    src/app/TranscriptionRequestValidation.cpp
    src/app/JobManager.cpp
    src/app/JobHistoryCoordinator.cpp
    src/app/ModelCatalog.cpp
)
target_link_libraries(offline_transcriber_app PUBLIC offline_transcriber_export offline_transcriber_logging
    offline_transcriber_storage offline_transcriber_audio)

add_subdirectory(${OFFLINE_TRANSCRIBER_CATCH2_DIR} ${CMAKE_BINARY_DIR}/third_party/catch2 EXCLUDE_FROM_ALL)

find_package(Qt6 QUIET COMPONENTS Widgets Test Concurrent)

option(OFFLINE_TRANSCRIBER_ENABLE_DESKTOP "Build the Qt desktop application" ON)

if(OFFLINE_TRANSCRIBER_ENABLE_DESKTOP AND Qt6_FOUND)
    add_subdirectory(apps/desktop)
endif()

enable_testing()

add_subdirectory(tests/unit)

if(Qt6_FOUND)
    add_subdirectory(tests/ui)
endif()

add_test(
    NAME phase07_verify_release_fails_on_missing
    COMMAND powershell -ExecutionPolicy Bypass -File
        ${CMAKE_SOURCE_DIR}/tools/verify-release.ps1
        -PackageRoot ${CMAKE_BINARY_DIR}/nonexistent-release-package
)
set_tests_properties(phase07_verify_release_fails_on_missing PROPERTIES WILL_FAIL TRUE)
