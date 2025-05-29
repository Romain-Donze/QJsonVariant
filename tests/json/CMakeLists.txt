cmake_minimum_required(VERSION 3.16)

project(json LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(json tst_json.cpp)
add_test(NAME json COMMAND json)

qt_add_resources(json json_resource_files
    FILES
        test.json
        test2.json
        test3.json
        test4.json
)

target_link_libraries(json PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(json PRIVATE QJsonVariant)
