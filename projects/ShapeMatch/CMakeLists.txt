cmake_minimum_required(VERSION 3.20)
project(ShapeMatch LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_OPENCV "Enable OpenCV for algorithm comparison" ON)
find_package(OpenMP REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
if(USE_OPENCV)
    add_definitions(-DUSE_OPENCV)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/utils
    ${PROJECT_SOURCE_DIR}/src/base
    ${PROJECT_SOURCE_DIR}/src/algorithm
    ${PROJECT_SOURCE_DIR}/src/ui
    ${PROJECT_SOURCE_DIR}/third_party
)

set(ALG_SRCS
    src/utils/cross_util.cpp
    src/base/image_base.cpp
    src/algorithm/ShapeMatch.cpp
)
set(UI_SRCS
    src/ui/main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)
set(ALL_SRCS ${ALG_SRCS} ${UI_SRCS})

if(USE_OPENCV)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

qt_add_executable(ShapeMatch ${ALL_SRCS})

target_link_libraries(ShapeMatch PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets OpenMP::OpenMP_CXX
)
if(USE_OPENCV)
    target_link_libraries(ShapeMatch PRIVATE ${OpenCV_LIBS})
endif()

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

if(WIN32)
    qt_generate_deploy_app_script(TARGET ShapeMatch OUTPUT_SCRIPT deploy_script)
endif()
