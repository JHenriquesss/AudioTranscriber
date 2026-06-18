cmake_minimum_required(VERSION 3.24)

set(CMAKE_AUTOMOC ON)

set(DESKTOP_UI_SOURCES
    ${CMAKE_SOURCE_DIR}/apps/desktop/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/DesktopJobController.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/widgets/TranscriptionPanel.cpp
    ${CMAKE_SOURCE_DIR}/apps/desktop/widgets/TranscriptEditor.cpp
)

add_executable(phase07_ui_tests
    test_main_window_smoke.cpp
    ${DESKTOP_UI_SOURCES}
)

target_include_directories(phase07_ui_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/apps/desktop
        ${CMAKE_SOURCE_DIR}/tests/unit
)

target_link_libraries(phase07_ui_tests
    PRIVATE
        offline_transcriber_app
        Qt6::Widgets
        Qt6::Test
        Qt6::Concurrent
)

target_compile_definitions(phase07_ui_tests
    PRIVATE
        PROJECT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

add_custom_command(TARGET phase07_ui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:phase07_ui_tests>/models"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/models/index.json"
        "$<TARGET_FILE_DIR:phase07_ui_tests>/models/index.json"
)

add_test(NAME phase07_main_window_smoke COMMAND phase07_ui_tests)
set_tests_properties(phase07_main_window_smoke PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:C:/Qt/6.8.2/msvc2022_64/bin"
)
