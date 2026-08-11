find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(tst_crc16
    tst_crc16.cpp
    ../core/Crc16.cpp
    ../core/Crc16.h
)
target_include_directories(tst_crc16 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../core)
target_link_libraries(tst_crc16 PRIVATE Qt::Test Qt::Core)
add_test(NAME tst_crc16 COMMAND tst_crc16)

qt_add_executable(tst_protocolparser
    tst_protocolparser.cpp
    ../core/Crc16.cpp
    ../core/Crc16.h
    ../core/ProtocolParser.cpp
    ../core/ProtocolParser.h
    ../core/ParsedFrame.h
)
target_include_directories(tst_protocolparser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../core)
target_link_libraries(tst_protocolparser PRIVATE Qt::Test Qt::Core)
add_test(NAME tst_protocolparser COMMAND tst_protocolparser)
