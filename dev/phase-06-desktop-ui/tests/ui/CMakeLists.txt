cmake_minimum_required(VERSION 3.24)

set(CMAKE_AUTOMOC ON)

set(DESKTOP_UI_SOURCES
    ${CMAKE_SOURCE_DIR}/apps/desktop/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/DesktopJobController.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/widgets/TranscriptionPanel.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/widgets/TranscriptEditor.cpp
)

add_executable(phase06_ui_tests
    test_main_window_smoke.cpp
    ${DESKTOP_UI_SOURCES}
)

target_include_directories(phase06_ui_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/apps/desktop
)

target_link_libraries(phase06_ui_tests
    PRIVATE
        offline_transcriber_app
        Qt6::Widgets
        Qt6::Test
        Qt6::Concurrent
)

target_compile_definitions(phase06_ui_tests
    PRIVATE
        PROJECT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME phase06_main_window_smoke COMMAND phase06_ui_tests)
