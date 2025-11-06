cmake_minimum_required(VERSION 3.16)

project(benchmark LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(benchmark tst_benchmark.cpp)
add_test(NAME benchmark COMMAND json)

qt_add_resources(benchmark benchmark_resource_files
    FILES
        benchmark.json
)

target_link_libraries(benchmark PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(benchmark PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(benchmark PRIVATE QJsonVariant)

install(TARGETS benchmark
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
