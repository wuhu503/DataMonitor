cmake_minimum_required(VERSION 3.19)
project(DataMonitor LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort Charts Sql Network Test)

qt_standard_project_setup()

enable_testing()
add_subdirectory(tests)

qt_add_executable(DataMonitor
    WIN32 MACOSX_BUNDLE
    app/main.cpp
    core/Crc16.cpp
    core/Crc16.h
    core/FileLogger.h core/FileLogger.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/mainwindow.ui
    core/ProtocolParser.h core/ProtocolParser.cpp
    core/ParsedFrame.h
    core/CommunicationManager.h core/CommunicationManager.cpp
    core/TcpCommunicationManager.h core/TcpCommunicationManager.cpp
    model/ChannelConfig.h
    thread/CommWorker.h thread/CommWorker.cpp
    resources/resources.qrc
    ui/SettingsDialog.ui
    ui/SettingsDialog.h ui/SettingsDialog.cpp
    db/DatabaseManager.h db/DatabaseManager.cpp
    thread/DatabaseWriter.h thread/DatabaseWriter.cpp
)

target_link_libraries(DataMonitor
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
        Qt::Charts
        Qt::Sql
        Qt::Network
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
