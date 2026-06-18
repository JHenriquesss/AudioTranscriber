cmake_minimum_required(VERSION 3.24)

add_executable(phase02_unit_tests
    test_result.cpp
    test_error.cpp
    test_cancellation_token.cpp
    test_time.cpp
    test_architecture_guard.cpp
    test_subtitle_timestamp.cpp
    test_transcript_validation.cpp
    test_export_golden.cpp
    test_export_service.cpp
)

target_link_libraries(phase02_unit_tests
    PRIVATE
        offline_transcriber_export
        Catch2::Catch2WithMain
)

target_compile_definitions(phase02_unit_tests
    PRIVATE
        PROJECT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

include(Catch)
catch_discover_tests(phase02_unit_tests)
