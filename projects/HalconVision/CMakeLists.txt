cmake_minimum_required(VERSION 3.20)
project(HalconVision LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ===== 检测操作系统 =====
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
endif()

# ===== 编译开关 =====
option(USE_HALCON "启用Halcon算法模块（ON=真实Halcon，OFF=打桩测试模式）" ON)
option(BUILD_UI "编译Qt可视化界面（需要src/app/QtUI/下有源文件）" ON)
option(BUILD_MFC "Windows编译MFC备用界面" OFF)
option(USE_OPENMP "多线程加速" ON)
option(USE_DL "启用Halcon深度学习" OFF)
option(ENABLE_TENSORRT "启用TensorRT加速" OFF)
option(ENABLE_CUDA "启用CUDA加速" ON)

# ===== 平台兼容性提示（用户完全控制） =====
if(PLATFORM_LINUX AND BUILD_UI)
    message(STATUS "Linux平台：Qt UI已启用 (BUILD_UI=ON)")
elseif(PLATFORM_LINUX AND NOT BUILD_UI)
    message(STATUS "Linux平台：Qt UI已禁用 (BUILD_UI=OFF)")
endif()

if(WIN32 AND NOT BUILD_UI AND NOT BUILD_MFC)
    message(STATUS "Windows平台：仅编译控制台程序 HalconConsole")
endif()

# ===== 开启优化 =====
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -fomit-frame-pointer")

if(ENABLE_CUDA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCUDA_ACCELERATE")
endif()

# ===== USE_HALCON 宏定义 =====
if(USE_HALCON)
    add_definitions(-DUSE_HALCON)
    message(STATUS "✅ Halcon 真实模式已启用 (USE_HALCON=ON)")
else()
    message(STATUS "🔧 Halcon 打桩测试模式已启用 (USE_HALCON=OFF)")
endif()

# ===== 查找Halcon =====
if(USE_HALCON)
    if(WIN32)
        set(HALCON_ROOT ${CMAKE_SOURCE_DIR}/third_party/halcon_win)
        set(HALCON_LIB_DIR ${HALCON_ROOT}/lib/x64-win64)
        link_directories(${HALCON_LIB_DIR})
    elseif(UNIX)
        set(HALCON_ROOT ${CMAKE_SOURCE_DIR}/third_party/halcon_linux)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(HALCON_ARCH "arm64-linux")
        else()
            set(HALCON_ARCH "x64-linux")
        endif()
        set(HALCON_LIB_DIR ${HALCON_ROOT}/lib/${HALCON_ARCH})
        link_directories(${HALCON_LIB_DIR})
    endif()
    include_directories(${HALCON_ROOT}/include)
    message(STATUS "Halcon库路径: ${HALCON_LIB_DIR}")
else()
    include_directories(${CMAKE_SOURCE_DIR}/third_party/mock_halcon/include)
    message(STATUS "🔧 使用Mock Halcon头文件（打桩模式）")
endif()

# ===== 查找Qt =====
if(BUILD_UI)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    message(STATUS "Qt6 已找到，将编译 Qt UI 界面")
endif()

# ===== OpenMP并行加速 =====
if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# ===== CUDA加速 =====
if(ENABLE_CUDA)
    find_package(CUDA REQUIRED)
    include_directories(${CUDA_INCLUDE_DIRS})
    link_directories(${CUDA_LIBRARY_DIRS})
endif()

# ===== 深度学习依赖 =====
if(USE_DL)
    find_package(onnxruntime REQUIRED)
    include_directories(${ONNXRUNTIME_INCLUDE_DIRS})
    add_definitions(-DENABLE_DEEP_LEARNING)

    if(ENABLE_TENSORRT)
        find_package(TensorRT REQUIRED)
        add_definitions(-DENABLE_TENSORRT)
    endif()
endif()

# ===== 头文件目录 =====
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/algorithm
    ${PROJECT_SOURCE_DIR}/src/algorithm/Preprocess
    ${PROJECT_SOURCE_DIR}/src/algorithm/TemplateMatch
    ${PROJECT_SOURCE_DIR}/src/algorithm/OcrModule
    ${PROJECT_SOURCE_DIR}/src/algorithm/Struct3D
    ${PROJECT_SOURCE_DIR}/src/algorithm/Measure2D
    ${PROJECT_SOURCE_DIR}/src/algorithm/DefectDetect
    ${PROJECT_SOURCE_DIR}/src/algorithm/deep_learning
    ${PROJECT_SOURCE_DIR}/src/capture
    ${PROJECT_SOURCE_DIR}/src/io
    ${PROJECT_SOURCE_DIR}/src/app
    ${PROJECT_SOURCE_DIR}/third_party
)

# ===== 核心源文件 =====
file(GLOB_RECURSE CORE_SRCS src/core/*.cpp)
file(GLOB_RECURSE ALGO_SRCS src/algorithm/*.cpp)
file(GLOB_RECURSE CAPTURE_SRCS src/capture/*.cpp)
file(GLOB_RECURSE IO_SRCS src/io/*.cpp)

set(COMMON_SRCS ${CORE_SRCS} ${ALGO_SRCS} ${CAPTURE_SRCS} ${IO_SRCS})

# ===== Halcon通用库链接 =====
if(USE_HALCON)
    set(HALCON_LIB halconcpp)
else()
    set(HALCON_LIB "")
endif()

# ===== 模式一：Qt跨平台UI =====
if(BUILD_UI)
    file(GLOB_RECURSE UI_SRCS src/app/QtUI/*.cpp)
    file(GLOB_RECURSE UI_HDRS src/app/QtUI/*.h)
    
    if(UI_SRCS)
        add_executable(HalconQtUI ${UI_SRCS} ${UI_HDRS} ${COMMON_SRCS})
        target_link_libraries(HalconQtUI Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL ${HALCON_LIB})

        if(ENABLE_CUDA)
            target_link_libraries(HalconQtUI ${CUDA_LIBRARIES})
        endif()

        if(USE_DL)
            target_link_libraries(HalconQtUI onnxruntime)
        endif()

        if(WIN32)
            qt_generate_deploy_app_script(TARGET HalconQtUI OUTPUT_SCRIPT deploy_script_qt)
        endif()

        message(STATUS "✅ HalconQtUI 将编译（Qt界面）")
        set(HALCON_QT_AVAILABLE TRUE)
    else()
        message(STATUS "⚠️ BUILD_UI=ON 但 src/app/QtUI/ 下无源文件，跳过 HalconQtUI 编译")
        message(STATUS "   提示：创建 src/app/QtUI/main.cpp 以启用Qt界面")
        set(HALCON_QT_AVAILABLE FALSE)
    endif()
else()
    set(HALCON_QT_AVAILABLE FALSE)
endif()

# ===== 模式二：MFC备用UI（仅Windows） =====
if(WIN32 AND BUILD_MFC)
    file(GLOB_RECURSE MFC_SRCS src/app/MfcUI/*.cpp)
    file(GLOB_RECURSE MFC_HDRS src/app/MfcUI/*.h)

    if(MFC_SRCS)
        find_package(MFC REQUIRED)

        add_executable(HalconMFC WIN32 ${MFC_SRCS} ${MFC_HDRS} ${COMMON_SRCS})
        target_link_libraries(HalconMFC ${HALCON_LIB})

        if(ENABLE_CUDA)
            target_link_libraries(HalconMFC ${CUDA_LIBRARIES})
        endif()

        if(USE_DL)
            target_link_libraries(HalconMFC onnxruntime)
        endif()

        message(STATUS "✅ HalconMFC 将编译（MFC界面，仅Windows）")
    else()
        message(STATUS "⚠️ BUILD_MFC=ON 但 src/app/MfcUI/ 下无源文件，跳过 HalconMFC 编译")
    endif()
endif()

# ===== 模式三：纯控制台 =====
add_executable(HalconConsole src/app/ConsoleMain.cpp ${COMMON_SRCS})
target_link_libraries(HalconConsole ${HALCON_LIB})

if(ENABLE_CUDA)
    target_link_libraries(HalconConsole ${CUDA_LIBRARIES})
endif()

if(USE_DL)
    target_link_libraries(HalconConsole onnxruntime)
endif()

message(STATUS "✅ HalconConsole 将编译（控制台模式）")

# ===== 安装与部署（修复：条件判断） =====
if(BUILD_UI AND HALCON_QT_AVAILABLE)
    install(TARGETS HalconQtUI HalconConsole
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib)
else()
    install(TARGETS HalconConsole
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib)
endif()

if(WIN32 AND BUILD_MFC AND TARGET HalconMFC)
    install(TARGETS HalconMFC RUNTIME DESTINATION bin)
endif()

# ===== 拷贝依赖 =====
if(WIN32 AND USE_HALCON)
    file(GLOB HALCON_DLL ${HALCON_ROOT}/lib/x64-win64/*.dll)
    install(FILES ${HALCON_DLL} DESTINATION bin)
endif()

# ===== 配置文件拷贝 =====
file(COPY config DESTINATION ${CMAKE_BINARY_DIR})
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# ===== 编译状态汇总 =====
message(STATUS "")
message(STATUS "========== HalconVision 编译配置汇总 ==========")
message(STATUS "平台: ${CMAKE_SYSTEM_NAME}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "Halcon模式 (USE_HALCON): ${USE_HALCON}")
message(STATUS "  -> ${USE_HALCON} 表示真实Halcon模式")
message(STATUS "  -> OFF 表示打桩测试模式（无需Halcon库）")
if(BUILD_UI AND HALCON_QT_AVAILABLE)
    message(STATUS "Qt界面 (BUILD_UI): ${BUILD_UI} (已编译)")
elseif(BUILD_UI AND NOT HALCON_QT_AVAILABLE)
    message(STATUS "Qt界面 (BUILD_UI): ${BUILD_UI} (跳过：无源文件)")
else()
    message(STATUS "Qt界面 (BUILD_UI): ${BUILD_UI}")
endif()
if(WIN32)
    message(STATUS "MFC界面 (BUILD_MFC): ${BUILD_MFC}")
endif()
message(STATUS "深度学习 (USE_DL): ${USE_DL}")
message(STATUS "TensorRT加速 (ENABLE_TENSORRT): ${ENABLE_TENSORRT}")
message(STATUS "CUDA加速 (ENABLE_CUDA): ${ENABLE_CUDA}")
message(STATUS "================================================")
message(STATUS "")
message(STATUS "📌 编译提示:")
if(USE_HALCON)
    message(STATUS "   - 真实Halcon模式：需要安装Halcon库和许可证")
    message(STATUS "   - 编译命令: cmake .. -DUSE_HALCON=ON")
else()
    message(STATUS "   - 打桩测试模式：无需Halcon库，使用Mock API")
    message(STATUS "   - 编译命令: cmake .. -DUSE_HALCON=OFF")
endif()
message(STATUS "   - 切换模式: cmake .. -DUSE_HALCON=ON/OFF")
message(STATUS "   - 禁用Qt UI: cmake .. -DBUILD_UI=OFF")
if(BUILD_UI AND NOT HALCON_QT_AVAILABLE)
    message(STATUS "   - 创建 src/app/QtUI/main.cpp 以启用Qt界面")
endif()
message(STATUS "================================================")
message(STATUS "")
