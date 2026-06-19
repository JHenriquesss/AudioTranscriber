cmake_minimum_required(VERSION 3.24)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(offline_transcriber_desktop
    main.cpp
    MainWindow.cpp
    DesktopJobController.cpp
    widgets/TranscriptionPanel.cpp
    widgets/TranscriptEditor.cpp
)

if(WIN32)
    target_sources(offline_transcriber_desktop PRIVATE
        ${CMAKE_SOURCE_DIR}/packaging/windows/app.manifest
    )
endif()

target_link_libraries(offline_transcriber_desktop
    PRIVATE
        offline_transcriber_app
        Qt6::Widgets
        Qt6::Concurrent
)

target_include_directories(offline_transcriber_desktop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(offline_transcriber_desktop PROPERTIES WIN32_EXECUTABLE TRUE)
