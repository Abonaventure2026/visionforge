#!/bin/bash
# ============================================================================
# push_all.sh - 选择性推送指定文件和目录到远程仓库
#
# 位置：项目根目录 /visionforge/
# 版本: 2.4
# 日期: 2026-07-03
# 作者: Yongji Gao
# 邮箱: bonaventure@163.com
#
# 推送内容：
#   - projects/        # 所有子工程
#   - third_party/     # 第三方库
#   - build_all.sh     # 构建脚本
#   - LICENSE          # 许可证
#   - README.md        # 项目说明
#   - .gitignore       # Git忽略规则
#
# 用法：
#   ./push_all.sh                      # 推送到 Gitee
#   ./push_all.sh --remote github      # 推送到 GitHub
#   ./push_all.sh --force              # 强制推送
#   ./push_all.sh --dry-run            # 预览模式
#   ./push_all.sh --help               # 显示帮助
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
print_section() {
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN} $1${NC}"
    echo -e "${GREEN}========================================${NC}"
}

# ============================================================================
# 默认参数
# ============================================================================
REMOTE="gitee"
FORCE=false
DRY_RUN=false
COMMIT_MESSAGE=""

# 远程仓库 URL 映射
declare -A REMOTE_URLS
REMOTE_URLS["gitee"]="git@gitee.com:abonaventure/visionforge.git"
REMOTE_URLS["github"]="git@github.com:Abonaventure2026/visionforge.git"
REMOTE_URLS["origin"]="git@gitee.com:abonaventure/visionforge.git"

# ============================================================================
# 需要推送的文件和目录（白名单）
# ============================================================================
PUSH_ITEMS=(
    "projects/"
    "third_party/"
    "build_all.sh"
    "LICENSE"
    "README.md"
    ".gitignore"
)

# ============================================================================
# 显示帮助
# ============================================================================
show_help() {
    cat << EOF
用法: ./push_all.sh [选项]

推送内容:
  projects/      所有子工程
  third_party/   第三方库
  build_all.sh   构建脚本
  LICENSE        许可证
  README.md      项目说明
  .gitignore     Git忽略规则

选项:
  --remote, -r NAME    远程仓库名称 (gitee|github|origin，默认: gitee)
  --force, -f          强制推送（覆盖远程冲突）
  --dry-run, -d        预览模式，不执行实际推送
  --message, -m MSG    自定义提交信息
  --help, -h           显示帮助

示例:
  ./push_all.sh                         # 推送到 Gitee
  ./push_all.sh --remote github         # 推送到 GitHub
  ./push_all.sh --force                 # 强制推送
  ./push_all.sh --dry-run               # 预览模式

远程仓库:
  Gitee:  git@gitee.com:abonaventure/visionforge.git
  GitHub: git@github.com:Abonaventure2026/visionforge.git
EOF
}

# ============================================================================
# 解析参数
# ============================================================================
while [[ $# -gt 0 ]]; do
    case $1 in
        --remote|-r)
            REMOTE="$2"
            shift 2
            ;;
        --force|-f)
            FORCE=true
            shift
            ;;
        --dry-run|-d)
            DRY_RUN=true
            shift
            ;;
        --message|-m)
            COMMIT_MESSAGE="$2"
            shift 2
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            print_error "未知参数: $1"
            show_help
            exit 1
            ;;
    esac
done

# ============================================================================
# 验证远程仓库
# ============================================================================
if [[ -z "${REMOTE_URLS[$REMOTE]}" ]]; then
    print_warn "未知远程仓库: $REMOTE，使用默认值: gitee"
    REMOTE="gitee"
fi
REMOTE_URL="${REMOTE_URLS[$REMOTE]}"

# ============================================================================
# 生成根目录文档
# ============================================================================
generate_root_docs() {
    print_step "生成根目录 README.md, LICENSE 和 .gitignore..."

    if [[ ! -f "scripts/generate_root_readme_license.py" ]]; then
        print_error "脚本不存在: scripts/generate_root_readme_license.py"
        return 1
    fi

    if [[ "$DRY_RUN" = true ]]; then
        print_info "[DRY-RUN] 将执行: python3 scripts/generate_root_readme_license.py"
        return 0
    fi

    if python3 scripts/generate_root_readme_license.py; then
        print_success "根目录文档生成成功"
        return 0
    else
        print_error "根目录文档生成失败"
        return 1
    fi
}

# ============================================================================
# 选择性添加文件
# ============================================================================
selective_add() {
    print_step "选择性添加文件..."

    if [[ "$DRY_RUN" = true ]]; then
        print_info "[DRY-RUN] 将添加以下内容:"
        for item in "${PUSH_ITEMS[@]}"; do
            echo "  + $item"
        done
        return 0
    fi

    # 重置暂存区
    git reset HEAD 2>/dev/null || true

    # 添加白名单中的文件/目录
    for item in "${PUSH_ITEMS[@]}"; do
        if [[ -e "$item" ]]; then
            git add "$item"
            print_info "  ✅ 添加: $item"
        else
            print_warn "  ⚠️ 跳过: $item (不存在)"
        fi
    done

    print_success "选择性添加完成"
    return 0
}

# ============================================================================
# 检查是否有变更
# ============================================================================
has_changes() {
    if git diff --cached --quiet 2>/dev/null; then
        return 1
    else
        return 0
    fi
}

# ============================================================================
# 提交变更
# ============================================================================
commit_changes() {
    print_step "提交变更..."

    if [[ "$DRY_RUN" = true ]]; then
        print_info "[DRY-RUN] 将提交变更"
        return 0
    fi

    if ! has_changes; then
        print_info "没有需要提交的变更"
        return 0
    fi

    if [ -z "$COMMIT_MESSAGE" ]; then
        COMMIT_MSG="chore: update projects, third_party and root docs at $(date '+%Y-%m-%d %H:%M:%S')"
    else
        COMMIT_MSG="$COMMIT_MESSAGE"
    fi

    if git commit -m "$COMMIT_MSG"; then
        print_success "提交成功"
        return 0
    else
        print_error "提交失败"
        return 1
    fi
}

# ============================================================================
# 推送变更
# ============================================================================
push_changes() {
    print_step "推送到 $REMOTE..."

    if [[ "$DRY_RUN" = true ]]; then
        print_info "[DRY-RUN] 将推送到 $REMOTE"
        return 0
    fi

    BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "main")

    if [ "$FORCE" = false ]; then
        print_info "拉取远程变更..."
        git pull --rebase "$REMOTE" "$BRANCH" 2>/dev/null || print_warn "pull失败，继续推送"
    fi

    print_info "推送到 $REMOTE/$BRANCH ..."
    if [ "$FORCE" = true ]; then
        git push --force "$REMOTE" "$BRANCH"
    else
        git push -u "$REMOTE" "$BRANCH"
    fi

    print_success "✅ 推送成功: $REMOTE/$BRANCH"
    return 0
}

# ============================================================================
# 显示当前状态
# ============================================================================
show_status() {
    if [[ "$DRY_RUN" = true ]]; then
        print_info "[DRY-RUN] 当前暂存区状态:"
        git status --porcelain | head -20 || true
    fi
}

# ============================================================================
# 同步到第二个远程仓库
# ============================================================================
sync_second_remote() {
    local first_remote="$1"
    local branch="$2"

    local second_remote=""
    if [[ "$first_remote" == "gitee" ]]; then
        second_remote="github"
    elif [[ "$first_remote" == "github" ]]; then
        second_remote="gitee"
    else
        return 0
    fi

    print_step "同步到 $second_remote..."

    if [[ "$DRY_RUN" = true ]]; then
        print_info "[DRY-RUN] 将同步到 $second_remote"
        return 0
    fi

    if ! git remote get-url "$second_remote" &>/dev/null; then
        local second_url="${REMOTE_URLS[$second_remote]}"
        print_info "添加远程仓库: $second_remote -> $second_url"
        git remote add "$second_remote" "$second_url"
    fi

    print_info "推送到 $second_remote/$branch ..."
    if [ "$FORCE" = true ]; then
        git push --force "$second_remote" "$branch"
    else
        git push -u "$second_remote" "$branch"
    fi

    print_success "✅ 同步到 $second_remote 成功"
    return 0
}

# ============================================================================
# 主流程
# ============================================================================
print_section "VisionForge 选择性推送工具 v2.4"

print_info "远程仓库: $REMOTE ($REMOTE_URL)"
print_info "强制模式: $([ "$FORCE" = true ] && echo "是" || echo "否")"
print_info "预览模式: $([ "$DRY_RUN" = true ] && echo "是" || echo "否")"

echo ""
print_info "推送内容:"
for item in "${PUSH_ITEMS[@]}"; do
    echo "  ✅ $item"
done

# ============================================================================
# 1. 生成根目录文档
# ============================================================================
if ! generate_root_docs; then
    print_error "根目录文档生成失败，退出"
    exit 1
fi

# ============================================================================
# 2. 选择性添加文件
# ============================================================================
print_section "选择要推送的内容"
selective_add

# ============================================================================
# 3. 显示状态
# ============================================================================
show_status

# ============================================================================
# 4. 提交变更
# ============================================================================
print_section "提交变更"
if ! commit_changes; then
    print_warn "提交失败或无需提交，退出"
    exit 0
fi

# ============================================================================
# 5. 推送
# ============================================================================
print_section "推送到远程仓库"
if ! push_changes; then
    print_error "推送失败"
    exit 1
fi

# ============================================================================
# 6. 同步到第二个远程
# ============================================================================
if [[ "$REMOTE" == "gitee" || "$REMOTE" == "github" ]]; then
    BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "main")
    sync_second_remote "$REMOTE" "$BRANCH"
fi

# ============================================================================
# 7. 输出结果
# ============================================================================
print_section "推送完成"

if [[ "$DRY_RUN" = true ]]; then
    print_warn "*** DRY-RUN 模式，未执行实际更改 ***"
else
    print_success "🎉 推送完成！"
    echo ""
    print_info "Gitee:  https://gitee.com/abonaventure/visionforge"
    print_info "GitHub: https://github.com/Abonaventure2026/visionforge"
    echo ""
    print_info "查看状态: git status"
    print_info "查看日志: git log --oneline -5"
    print_info "查看远程: git remote -v"
    echo ""
    print_info "已推送内容:"
    for item in "${PUSH_ITEMS[@]}"; do
        if [[ -e "$item" ]]; then
            echo "  ✅ $item"
        fi
    done
fi