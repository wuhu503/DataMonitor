cmake_minimum_required(VERSION 3.19)
project(DataMonitor LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(DataMonitor
    WIN32 MACOSX_BUNDLE
    app/main.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/mainwindow.ui
)

target_link_libraries(DataMonitor
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS DataMonitor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DataMonitor
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
