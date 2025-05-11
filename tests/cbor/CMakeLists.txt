cmake_minimum_required(VERSION 3.16)

project(cbor LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cbor tst_cbor.cpp)
add_test(NAME cbor COMMAND cbor)

target_link_libraries(cbor PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(cbor PRIVATE QJsonVariant)
