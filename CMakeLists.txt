cmake_minimum_required(VERSION 3.21)

project(workpiece-inspector
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

qt_add_executable(workpiece-inspector
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
)

target_link_libraries(workpiece-inspector
    PRIVATE
        Qt6::Widgets
)

target_include_directories(workpiece-inspector
    PRIVATE
        src
)

enable_testing()
add_subdirectory(tests)
