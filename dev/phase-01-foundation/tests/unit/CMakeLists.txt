cmake_minimum_required(VERSION 3.24)

add_executable(phase01_unit_tests
    test_result.cpp
    test_error.cpp
    test_cancellation_token.cpp
    test_time.cpp
    test_architecture_guard.cpp
)

target_link_libraries(phase01_unit_tests
    PRIVATE
        offline_transcriber_shared
        Catch2::Catch2WithMain
)

target_compile_definitions(phase01_unit_tests
    PRIVATE
        PROJECT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

include(Catch)
catch_discover_tests(phase01_unit_tests)
