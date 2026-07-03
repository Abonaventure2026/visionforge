#!/bin/bash
# ============================================================================
# build_all.sh - 一键编译所有工业视觉检测工程
#
# 位置：项目根目录 /machinevision_vs/
# 版本: 2.4
# 日期: 2026-07-03
#
# 用法：
#   ./build_all.sh                      # 编译所有工程
#   ./build_all.sh --clean              # 清理后重新编译
#   ./build_all.sh --project NAME       # 仅编译指定工程
#   ./build_all.sh --verbose            # 显示详细输出
#   ./build_all.sh --help               # 显示帮助
# ============================================================================

set -e

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$BASE_DIR"

# ============================================================================
# 颜色定义
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
print_step() { echo -e "${CYAN}▶${NC} $1"; }
print_section() { echo -e "\n${GREEN}========================================${NC}"; echo -e "${GREEN} $1${NC}"; echo -e "${GREEN}========================================${NC}"; }

# ============================================================================
# 默认参数
# ============================================================================
CLEAN_MODE=false
VERBOSE=false
SPECIFIC_PROJECT=""
MAKE_JOBS=$(nproc 2>/dev/null || echo 4)

# ============================================================================
# 解析参数
# ============================================================================
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean|-c) CLEAN_MODE=true; shift ;;
        --verbose|-v) VERBOSE=true; shift ;;
        --project|-p) SPECIFIC_PROJECT="$2"; shift 2 ;;
        --help|-h)
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  --clean, -c        清理后重新编译"
            echo "  --verbose, -v      显示详细输出"
            echo "  --project, -p NAME 仅编译指定工程"
            echo "  --help, -h         显示帮助"
            echo ""
            echo "可用工程:"
            echo "  Struct3DInspect    - 3D结构光检测"
            echo "  ShapeMatch         - 形状匹配"
            echo "  StructMeasure      - 结构件测量"
            echo "  VisionInspect      - 工业视觉全场景"
            echo ""
            echo "示例:"
            echo "  ./build_all.sh                        # 编译所有工程"
            echo "  ./build_all.sh --clean                # 清理后重新编译"
            echo "  ./build_all.sh --project ShapeMatch   # 仅编译形状匹配"
            echo "  ./build_all.sh --verbose              # 显示详细输出"
            exit 0
            ;;
        *) print_error "未知参数: $1"; exit 1 ;;
    esac
done

# ============================================================================
# 检查依赖
# ============================================================================
check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "未安装: $1"
        return 1
    fi
    return 0
}

print_section "一键编译工业视觉检测工程"
print_info "工作目录: $BASE_DIR"
print_info "CPU核心数: $MAKE_JOBS"
print_info "清理模式: $([ "$CLEAN_MODE" = true ] && echo "是" || echo "否")"
print_info "详细输出: $([ "$VERBOSE" = true ] && echo "是" || echo "否")"

check_command cmake || exit 1
check_command make || exit 1

# ============================================================================
# 编译单个工程
# ============================================================================
build_project() {
    local proj_name="$1"
    local proj_dir="$BASE_DIR/projects/$proj_name"

    if [ ! -d "$proj_dir" ]; then
        print_warn "跳过 $proj_name: 工程目录不存在（请先运行 ./generate_all.sh）"
        return 1
    fi

    print_section "编译: $proj_name"
    cd "$proj_dir"

    if [ "$CLEAN_MODE" = true ] || [ -d "build" ]; then
        print_step "清理构建目录..."
        rm -rf build
    fi

    print_step "创建构建目录..."
    mkdir -p build && cd build

    print_step "配置CMake..."
    if [ "$VERBOSE" = true ]; then
        cmake .. || { print_error "CMake配置失败"; return 1; }
    else
        cmake .. > /dev/null 2>&1 || { print_error "CMake配置失败"; return 1; }
    fi

    print_step "编译中 (使用 $MAKE_JOBS 线程)..."
    if [ "$VERBOSE" = true ]; then
        make -j"$MAKE_JOBS" || { print_error "编译失败"; return 1; }
    else
        make -j"$MAKE_JOBS" > /dev/null 2>&1 || { print_error "编译失败"; return 1; }
    fi

    print_success "$proj_name 编译完成"
    print_info "运行: cd $proj_dir && ./run.sh"
    return 0
}

# ============================================================================
# 确定要编译的工程
# ============================================================================
if [ -n "$SPECIFIC_PROJECT" ]; then
    PROJECTS=("$SPECIFIC_PROJECT")
else
    PROJECTS=(
        "Struct3DInspect"
        "ShapeMatch"
        "StructMeasure"
        "VisionInspect"
    )
fi

FAILED=()
SUCCESS_COUNT=0
TOTAL_COUNT=${#PROJECTS[@]}

for proj in "${PROJECTS[@]}"; do
    if build_project "$proj"; then
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        FAILED+=("$proj")
    fi
done

# ============================================================================
# 输出结果
# ============================================================================
print_section "编译总结"
if [ ${#FAILED[@]} -eq 0 ]; then
    print_success "🎉 所有工程编译成功！ ($SUCCESS_COUNT/$TOTAL_COUNT)"
else
    print_error "❌ 以下工程编译失败: ${FAILED[*]}"
    print_info "成功: $SUCCESS_COUNT/$TOTAL_COUNT"
    exit 1
fi