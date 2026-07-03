cmake_minimum_required(VERSION 3.20)
project(Struct3DInspect LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ===== 选项 =====
option(USE_OPENCV "Enable OpenCV for comparison/visualization" OFF)
option(USE_OPENMP "Enable OpenMP acceleration" ON)
option(ENABLE_AI_ISP "Enable AI ISP placeholder" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# ===== 依赖 =====
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

if(USE_OPENCV)
    find_package(OpenCV REQUIRED)
    add_definitions(-DUSE_OPENCV)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

if(ENABLE_AI_ISP)
    add_definitions(-DENABLE_AI_ISP)
endif()

# ===== 头文件目录 =====
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/algorithm
    ${PROJECT_SOURCE_DIR}/src/ui
    ${PROJECT_SOURCE_DIR}/src/io
    ${PROJECT_SOURCE_DIR}/src/calib
    ${PROJECT_SOURCE_DIR}/third_party
)

# ===== 源文件 =====
set(CORE_SRCS
    src/core/ImageBase.cpp
    src/core/ImageProc.cpp
    src/core/Triangulate.cpp
    src/core/PointCloudFilter.cpp
)
set(ALG_SRCS
    src/algorithm/GrayCodeGen.cpp
    src/algorithm/PhaseShiftSolver.cpp
    src/algorithm/MultiFreqUnwrap.cpp
)
set(CALIB_SRCS)
set(IO_SRCS
    src/io/CSVWriter.cpp
    src/io/PointCloudIO.cpp
)
set(UI_SRCS
    src/ui/main.cpp
    src/ui/MainWindow.cpp
)
set(ALL_SRCS ${CORE_SRCS} ${ALG_SRCS} ${CALIB_SRCS} ${IO_SRCS} ${UI_SRCS})

qt_add_executable(Struct3DInspect ${ALL_SRCS})

target_link_libraries(Struct3DInspect PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets
)
if(USE_OPENMP)
    target_link_libraries(Struct3DInspect PRIVATE OpenMP::OpenMP_CXX)
endif()
if(USE_OPENCV)
    target_link_libraries(Struct3DInspect PRIVATE ${OpenCV_LIBS})
endif()

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

if(WIN32)
    qt_generate_deploy_app_script(TARGET Struct3DInspect OUTPUT_SCRIPT deploy_script)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
