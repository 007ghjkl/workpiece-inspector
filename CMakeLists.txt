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

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(OpenCV QUIET COMPONENTS core videoio imgproc)

add_library(workpiece-core STATIC
    src/core/DomainModels.cpp
    src/core/DomainModels.h
)

target_include_directories(workpiece-core
    PUBLIC
        src
)

add_library(workpiece-configuration STATIC
    src/configuration/AppConfig.cpp
    src/configuration/AppConfig.h
)

target_link_libraries(workpiece-configuration
    PUBLIC
        workpiece-core
        Qt6::Core
)

target_include_directories(workpiece-configuration
    PUBLIC
        src
)

add_library(workpiece-vision STATIC
    src/vision/ImageSource.h
    src/vision/NetworkCameraImageSource.cpp
    src/vision/NetworkCameraImageSource.h
    src/vision/SimulatedImageSource.cpp
    src/vision/SimulatedImageSource.h
)

target_link_libraries(workpiece-vision
    PUBLIC
        workpiece-core
        workpiece-configuration
        Qt6::Gui
)

target_include_directories(workpiece-vision
    PUBLIC
        src
)

if(OpenCV_FOUND)
    target_link_libraries(workpiece-vision
        PRIVATE
            ${OpenCV_LIBS}
    )
    target_include_directories(workpiece-vision
        PUBLIC
            ${OpenCV_INCLUDE_DIRS}
    )
    target_compile_definitions(workpiece-vision PUBLIC WORKPIECE_HAVE_OPENCV=1)
else()
    target_compile_definitions(workpiece-vision PUBLIC WORKPIECE_HAVE_OPENCV=0)
endif()

qt_add_executable(workpiece-inspector
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
)

target_link_libraries(workpiece-inspector
    PRIVATE
        workpiece-core
        workpiece-configuration
        workpiece-vision
        Qt6::Widgets
)

target_include_directories(workpiece-inspector
    PRIVATE
        src
)

enable_testing()
add_subdirectory(tests)
