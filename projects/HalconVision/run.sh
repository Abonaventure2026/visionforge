#!/bin/bash
# HalconVision 运行脚本
# 支持 Qt UI、MFC UI（Windows）、控制台三种模式

export QT_QPA_PLATFORM=xcb
export DISPLAY=$DISPLAY

# WSL环境适配
if [ -d /mnt/wslg/runtime-dir ]; then
    chmod 0700 /mnt/wslg/runtime-dir 2>/dev/null || true
fi

# Halcon依赖路径
export HALCONROOT=$(pwd)/third_party/halcon_linux
export HALCONARCH=x64-linux
export LD_LIBRARY_PATH=$HALCONROOT/lib/$HALCONARCH:$LD_LIBRARY_PATH

# 检测操作系统
if [ "$(uname)" = "Linux" ]; then
    QT_EXEC="./build/HalconQtUI"
    CONSOLE_EXEC="./build/HalconConsole"
elif [ "$(uname)" = "Darwin" ]; then
    QT_EXEC="./build/HalconQtUI"
    CONSOLE_EXEC="./build/HalconConsole"
else
    # Windows
    QT_EXEC="./build/Release/HalconQtUI.exe"
    MFC_EXEC="./build/Release/HalconMFC.exe"
    CONSOLE_EXEC="./build/Release/HalconConsole.exe"
fi

# 选择运行模式
case "$1" in
    --console)
        echo "运行控制台模式..."
        exec $CONSOLE_EXEC "${@:2}"
        ;;
    --mfc)
        if [ -f "$MFC_EXEC" ]; then
            echo "运行MFC界面模式（Windows兼容）..."
            exec $MFC_EXEC "${@:2}"
        else
            echo "错误：MFC界面未编译，请使用 --qt 或 --console"
            echo "提示：编译时添加 -DBUILD_MFC=ON 启用MFC界面"
            exit 1
        fi
        ;;
    --qt|--ui|"")
        if [ -f "$QT_EXEC" ]; then
            echo "运行Qt界面模式..."
            exec $QT_EXEC "$@"
        else
            echo "错误：Qt界面未编译，请先编译"
            echo "提示：编译时添加 -DBUILD_UI=ON 启用Qt界面"
            exit 1
        fi
        ;;
    *)
        echo "用法: ./run.sh [选项]"
        echo ""
        echo "选项:"
        echo "  --qt, --ui    运行Qt界面（默认，跨平台）"
        echo "  --mfc         运行MFC界面（仅Windows）"
        echo "  --console     运行控制台模式"
        echo "  --help        显示帮助"
        echo ""
        echo "示例:"
        echo "  ./run.sh                           # 运行Qt界面"
        echo "  ./run.sh --console --ocr ./assets/test.png"
        echo "  ./run.sh --console --mock          # 打桩模式测试"
        echo "  ./run.sh --mfc                     # Windows下运行MFC界面"
        exit 0
        ;;
esac
