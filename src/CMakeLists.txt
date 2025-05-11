
cmake_minimum_required(VERSION 3.16)

project(QJsonVariant
    VERSION 1.0.0
    LANGUAGES CXX
)

if(NOT DEFINED QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

set(ALL_SRC_FILES
    qcborparser.h qcborparser.cpp
    qcborwriter.h qcborwriter.cpp
    qjsonparser.h qjsonparser.cpp
    qjsonwriter.h qjsonwriter.cpp
)

qt_add_library(${PROJECT_NAME}
    STATIC
    ${ALL_SRC_FILES}
    QJsonVariant
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_MESSAGELOGCONTEXT
    QT_DISABLE_DEPRECATED_UP_TO=0x060500
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

