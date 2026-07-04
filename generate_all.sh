#!/bin/bash
# ============================================================================
# generate_all.sh - 一键生成所有工业视觉检测工程
#
# 位置：项目根目录 /machinevision_vs/
# 版本: 2.4
# 日期: 2026-07-04
#
# 用法：
#   ./generate_all.sh                               # 生成到 ./projects/
#   ./generate_all.sh --output-dir ./my_projects    # 生成到指定目录
#   ./generate_all.sh --project halcon              # 仅生成指定工程
#   ./generate_all.sh --push                        # 生成并推送
#   ./generate_all.sh --skip-tests                  # 跳过测试图生成
#   ./generate_all.sh --no-download-stb             # 跳过STB文件下载
#   ./generate_all.sh --help                        # 显示帮助
#
# 可用工程: halcon, 3d, shape, measure, vision
# ============================================================================

set -e

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$BASE_DIR"

# ============================================================================
# 颜色定义
# ============================================================================
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
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
PUSH=false
SKIP_TESTS=false
NO_DOWNLOAD_STB=false
SPECIFIC_PROJECT=""
OUTPUT_DIR="./projects"
FORCE_STB=false

# ============================================================================
# 解析参数
# ============================================================================
while [[ $# -gt 0 ]]; do
    case $1 in
        --push) PUSH=true; shift ;;
        --skip-tests) SKIP_TESTS=true; shift ;;
        --no-download-stb) NO_DOWNLOAD_STB=true; shift ;;
        --force-download-stb) FORCE_STB=true; shift ;;
        --output-dir|-o) OUTPUT_DIR="$2"; shift 2 ;;
        --project|-p) SPECIFIC_PROJECT="$2"; shift 2 ;;
        --help|-h)
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  --output-dir, -o DIR   输出目录（默认 ./projects/）"
            echo "  --push                 生成后自动推送到远程仓库"
            echo "  --skip-tests           跳过测试图生成"
            echo "  --no-download-stb      跳过STB头文件下载（使用已有文件）"
            echo "  --force-download-stb   强制重新下载STB头文件"
            echo "  --project, -p NAME     仅生成指定工程 (halcon|3d|shape|measure|vision)"
            echo "  --help, -h             显示此帮助信息"
            echo ""
            echo "可用工程: halcon, 3d, shape, measure, vision"
            echo ""
            echo "示例:"
            echo "  ./generate_all.sh                               # 生成到 ./projects/"
            echo "  ./generate_all.sh --output-dir ./my_projects    # 生成到 ./my_projects/"
            echo "  ./generate_all.sh --project halcon              # 仅生成HalconVision"
            echo "  ./generate_all.sh --push --output-dir ./release # 生成到 release/ 并推送"
            exit 0
            ;;
        *) print_error "未知参数: $1"; exit 1 ;;
    esac
done

# ============================================================================
# 安全创建目录函数
# ============================================================================
safe_mkdir() {
    local dir="$1"
    if [ -f "$dir" ]; then
        print_warn "$dir 是文件，正在删除..."
        rm -f "$dir"
    fi
    mkdir -p "$dir"
}

# ============================================================================
# 整理工程到输出目录
# ============================================================================
move_to_output() {
    local proj_name="$1"
    local src_dir="$BASE_DIR/$proj_name"
    local dst_dir="$BASE_DIR/$OUTPUT_DIR/$proj_name"

    if [ -d "$src_dir" ] && [ "$src_dir" != "$dst_dir" ]; then
        print_step "移动: $proj_name -> $OUTPUT_DIR/"
        safe_mkdir "$BASE_DIR/$OUTPUT_DIR"
        if [ -d "$dst_dir" ]; then
            rm -rf "$dst_dir"
        fi
        mv "$src_dir" "$dst_dir"
        print_info "  ✅ 已移动到: $OUTPUT_DIR/$proj_name"
        return 0
    elif [ -d "$dst_dir" ]; then
        print_info "  ✅ $proj_name 已在目标位置: $OUTPUT_DIR/$proj_name"
        return 0
    else
        print_warn "  ⚠️ $proj_name 未找到，跳过移动"
        return 1
    fi
}

# ============================================================================
# 主流程
# ============================================================================
print_section "一键生成工业视觉检测工程"
print_info "工作目录: $BASE_DIR"
print_info "输出目录: $OUTPUT_DIR"
print_info "推送模式: $([ "$PUSH" = true ] && echo "是" || echo "否")"
print_info "跳过测试: $([ "$SKIP_TESTS" = true ] && echo "是" || echo "否")"
print_info "跳过STB下载: $([ "$NO_DOWNLOAD_STB" = true ] && echo "是" || echo "否")"

# 安全创建必要目录
safe_mkdir "$OUTPUT_DIR"
safe_mkdir testdata
safe_mkdir logs

# 确定要生成的工程
if [ -n "$SPECIFIC_PROJECT" ]; then
    print_info "仅生成工程: $SPECIFIC_PROJECT"
    PROJECTS=("$SPECIFIC_PROJECT")
else
    print_info "生成所有工程"
    PROJECTS=("halcon" "3d" "shape" "measure" "vision")
fi

FAILED=()
SUCCESS_COUNT=0
TOTAL_COUNT=${#PROJECTS[@]}

for proj in "${PROJECTS[@]}"; do
    case $proj in
        halcon)
            print_section "生成 HalconVision 跨平台工程"
            print_step "执行: scripts/halcon_build.py"

            HALCON_ARGS="--workspace . --overwrite"
            if [ "$NO_DOWNLOAD_STB" = true ]; then
                HALCON_ARGS="$HALCON_ARGS --no-download-stb"
            fi
            # 默认启用深度学习支持
            HALCON_ARGS="$HALCON_ARGS --with-dl"

            python3 scripts/halcon_build.py $HALCON_ARGS || {
                print_error "HalconVision工程生成失败"
                FAILED+=("halcon")
                continue
            }

            # 移动到输出目录
            move_to_output "HalconVision"

            print_step "执行: scripts/halcon_readme_license.py"
            python3 scripts/halcon_readme_license.py --workspace "$OUTPUT_DIR" || true

            if [ "$SKIP_TESTS" = false ]; then
                print_step "执行: scripts/gen_halcon_test.py"
                python3 scripts/gen_halcon_test.py --type all --count 3 || print_warn "Halcon测试图生成失败"
            fi
            print_success "HalconVision工程完成"
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            ;;
        3d)
            print_section "生成 3D结构光检测工程"
            print_step "执行: scripts/slight3d_build.py"
            if [ "$NO_DOWNLOAD_STB" = true ]; then
                python3 scripts/slight3d_build.py --workspace . --overwrite --no-download-stb || {
                    print_error "3D结构光工程生成失败"
                    FAILED+=("3d")
                    continue
                }
            else
                python3 scripts/slight3d_build.py --workspace . --overwrite || {
                    print_error "3D结构光工程生成失败"
                    FAILED+=("3d")
                    continue
                }
            fi

            # 移动到输出目录
            move_to_output "Struct3DInspect"

            print_step "执行: scripts/slight3d_readme_license.py"
            python3 scripts/slight3d_readme_license.py --workspace "$OUTPUT_DIR" || true

            if [ "$SKIP_TESTS" = false ]; then
                print_step "执行: scripts/gen_slight3d_test.py"
                python3 scripts/gen_slight3d_test.py || print_warn "3D测试图生成失败"
            fi
            print_success "3D结构光检测工程完成"
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            ;;
        shape)
            print_section "生成 形状匹配工程"
            print_step "执行: scripts/shape_build.py"
            if [ "$NO_DOWNLOAD_STB" = true ]; then
                python3 scripts/shape_build.py --workspace . --overwrite --no-download-stb || {
                    print_error "形状匹配工程生成失败"
                    FAILED+=("shape")
                    continue
                }
            else
                python3 scripts/shape_build.py --workspace . --overwrite || {
                    print_error "形状匹配工程生成失败"
                    FAILED+=("shape")
                    continue
                }
            fi

            # 移动到输出目录
            move_to_output "ShapeMatch"

            print_step "执行: scripts/shape_readme_license.py"
            python3 scripts/shape_readme_license.py --workspace "$OUTPUT_DIR" || true

            if [ "$SKIP_TESTS" = false ]; then
                print_step "执行: scripts/gen_shape_test.py"
                python3 scripts/gen_shape_test.py || print_warn "形状匹配测试图生成失败"
            fi
            print_success "形状匹配工程完成"
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            ;;
        measure)
            print_section "生成 结构件测量工程"
            print_step "执行: scripts/measure_build.py"
            python3 scripts/measure_build.py --workspace . --overwrite || {
                print_error "结构件测量工程生成失败"
                FAILED+=("measure")
                continue
            }

            # 移动到输出目录
            move_to_output "StructMeasure"

            print_step "执行: scripts/measure_readme_license.py"
            python3 scripts/measure_readme_license.py --workspace "$OUTPUT_DIR" || true

            if [ "$SKIP_TESTS" = false ]; then
                print_step "执行: scripts/gen_measure_test.py"
                python3 scripts/gen_measure_test.py || print_warn "结构测量测试图生成失败"
            fi
            print_success "结构件测量工程完成"
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            ;;
        vision)
            print_section "生成 工业视觉全场景工程"
            print_step "执行: scripts/vision_build.py"
            VISION_ARGS="--workspace . --overwrite"
            if [ "$NO_DOWNLOAD_STB" = true ]; then
                VISION_ARGS="$VISION_ARGS --no-download-stb"
            fi
            # 默认启用深度学习
            VISION_ARGS="$VISION_ARGS --enable-ai"

            python3 scripts/vision_build.py $VISION_ARGS || {
                print_error "工业视觉全场景工程生成失败"
                FAILED+=("vision")
                continue
            }

            # 移动到输出目录
            move_to_output "VisionInspect"

            print_step "执行: scripts/vision_readme_license.py"
            python3 scripts/vision_readme_license.py --workspace "$OUTPUT_DIR" || true

            if [ "$SKIP_TESTS" = false ]; then
                print_step "执行: scripts/gen_vision_test.py"
                python3 scripts/gen_vision_test.py || print_warn "工业视觉测试图生成失败"
            fi
            print_success "工业视觉全场景工程完成"
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            ;;
        *)
            print_error "未知工程: $proj"
            FAILED+=("$proj")
            ;;
    esac
done

# ============================================================================
# 生成根目录文档（所有工程生成完成后）
# ============================================================================
print_section "生成根目录文档"
print_step "执行: scripts/generate_root_readme_license.py"
python3 scripts/generate_root_readme_license.py || print_warn "根目录文档生成失败"

# ============================================================================
# 推送
# ============================================================================
if [ "$PUSH" = true ] && [ ${#FAILED[@]} -eq 0 ]; then
    print_section "推送到远程仓库"
    print_step "执行: scripts/git_push_all.py"
    cd "$OUTPUT_DIR"
    python3 "$BASE_DIR/scripts/git_push_all.py" || print_warn "推送失败，请检查远程仓库配置"
    cd "$BASE_DIR"
fi

# ============================================================================
# 输出结果
# ============================================================================
print_section "✅ 生成完成"
print_info "成功: $SUCCESS_COUNT/$TOTAL_COUNT 个工程"
print_info "输出目录: $BASE_DIR/$OUTPUT_DIR/"

if [ ${#FAILED[@]} -eq 0 ]; then
    print_success "🎉 所有工程生成成功！"
    echo ""
    print_info "📋 编译命令:"
    echo "  ./build_all.sh"
    echo "  ./build_all.sh --project HalconVision   # 仅编译HalconVision"
    echo ""
    print_info "📤 运行命令示例:"
    echo "  cd $OUTPUT_DIR/HalconVision && ./run.sh"
    echo "  cd $OUTPUT_DIR/HalconVision && ./run.sh --console --ocr <image>"
    echo "  cd $OUTPUT_DIR/Struct3DInspect && ./run.sh"
    echo "  cd $OUTPUT_DIR/ShapeMatch && ./run.sh"
    echo "  cd $OUTPUT_DIR/StructMeasure && ./run.sh"
    echo "  cd $OUTPUT_DIR/VisionInspect && ./run.sh"
    echo ""
    print_info "📤 推送命令:"
    echo "  ./push_all.sh"
else
    print_error "❌ 以下工程生成失败: ${FAILED[*]}"
    exit 1
fi