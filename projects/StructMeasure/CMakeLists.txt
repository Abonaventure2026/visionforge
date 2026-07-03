cmake_minimum_required(VERSION 3.20)
project(StructMeasure LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_OPENCV "Enable OpenCV for comparison" OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(USE_OPENCV)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    add_definitions(-DUSE_OPENCV)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/StructureMeasure.h
    src/StructureMeasure.cpp
)

qt_add_executable(StructMeasure ${SOURCES})
target_link_libraries(StructMeasure PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
if(USE_OPENCV)
    target_link_libraries(StructMeasure PRIVATE ${OpenCV_LIBS})
endif()

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
if(WIN32)
    qt_generate_deploy_app_script(TARGET StructMeasure OUTPUT_SCRIPT deploy_script)
endif()
