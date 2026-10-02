#!/usr/bin/env bash
# =============================================================================
# rename-zeroclaw-to-kylinbot.sh
#
# 批量将项目中的 zeroclaw 全部改名为 kylinbot，保持大小写和分隔符。
#
# 映射规则：
#   zeroclaw       -> kylinbot       (全小写)
#   ZeroClaw       -> KylinBot       (PascalCase)
#   ZEROCLAW       -> KYLINBOT       (全大写)
#   zeroclawlabs   -> kylinbot       (去掉 labs)
#   zeroclaw-labs  -> kylinbot       (去掉 -labs)
#   zero<claw      -> kylin<bot      (中间任意非字母分隔符，保持原样)
#   ZeroClawAgent  -> KylinBotAgent  (PascalCase 拼接词)
#   crates/zeroclaw-xxx -> crates/kylinbot-xxx (替换前缀)
#   ... 以此类推
#
# 用法：
#   ./rename-zeroclaw-to-kylinbot.sh <模式> [选项]
#
# 模式（必选，二选一）：
#   --content    仅做文件内容替换（sed + banner + wizard 清理）
#   --rename     仅做文件/目录重命名（git mv + 清理空目录）
#   --all        以上全部执行
#
# 选项：
#   --dry-run    仅预览，不实际修改
#
# 示例：
#   ./rename-zeroclaw-to-kylinbot.sh --content --dry-run   # 预览内容替换
#   ./rename-zeroclaw-to-kylinbot.sh --content             # 执行内容替换
#   ./rename-zeroclaw-to-kylinbot.sh --rename --dry-run    # 预览重命名
#   ./rename-zeroclaw-to-kylinbot.sh --rename              # 执行重命名
#   ./rename-zeroclaw-to-kylinbot.sh --all --dry-run       # 预览全部
#   ./rename-zeroclaw-to-kylinbot.sh --all                 # 执行全部
#
# 注意：
#   - 需要在 git 仓库根目录执行
#   - 执行前建议先新建分支
#   - 建议先 --content 再 --rename，分两次提交以保持清晰的变更历史
#   - target/ 目录不在处理范围
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# 解析参数
# ---------------------------------------------------------------------------

DO_CONTENT=false
DO_RENAME=false
DO_ALL=false
DRY_RUN=false

show_usage() {
    sed -n '2,/^# =====/p' "$0" | grep '^#' | sed 's/^# \?//'
    exit 1
}

if [[ $# -eq 0 ]]; then
    show_usage
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        --content)  DO_CONTENT=true ;;
        --rename)   DO_RENAME=true ;;
        --all)      DO_ALL=true; DO_CONTENT=true; DO_RENAME=true ;;
        --dry-run)  DRY_RUN=true ;;
        -h|--help)  show_usage ;;
        *) echo "未知参数: $1"; show_usage ;;
    esac
    shift
done

if ! $DO_CONTENT && ! $DO_RENAME; then
    echo "错误: 请指定 --content, --rename 或 --all"
    show_usage
fi

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

echo "模式:$( $DO_CONTENT && echo ' 内容替换' )$( $DO_RENAME && echo ' 文件重命名' )$( $DRY_RUN && echo ' [DRY RUN]' )"
echo ""

# ---------------------------------------------------------------------------
# 辅助函数
# ---------------------------------------------------------------------------

log() { echo ":: $*"; }

# 构建 sed 替换表达式（按长度降序，长模式优先）
build_sed_expr() {
    cat <<'SED_EXPR'
# ===== 特殊复合词（最长模式优先） =====

# zeroclawlabs -> kylinbot  (去掉 labs)
s/zeroclawlabs/kylinbot/g
s/ZEROCLAWLABS/KYLINBOT/g

# zeroclaw-labs -> kylinbot  (去掉 -labs)
s/zeroclaw-labs/kylinbot/g
s/ZEROCLAW-LABS/KYLINBOT/g

# ===== PascalCase 复合类型 =====
s/ZeroClawAgent/KylinBotAgent/g
s/ZeroClawOperator/KylinBotOperator/g
s/ZeroClawMaintainer/KylinBotMaintainer/g
s/ZeroClawBuilder/KylinBotBuilder/g

# ===== SCREAMING_SNAKE (env vars) =====
s/ZEROCLAW_/KYLINBOT_/g

# ===== 独立的 SCREAMING =====
s/ZEROCLAW/KYLINBOT/g

# ===== PascalCase 基础 =====
s/ZeroClaw/KylinBot/g

# ===== 首字母大写变体（Zeroclaw） =====
s/Zeroclaw/Kylinbot/g

# ===== crates/ 下的目录路径 =====
# crates/zeroclaw-runtime -> crates/kylinbot-runtime, crates/zeroclaw-api -> crates/kylinbot-api ...
# 不需要特殊规则，通用 zeroclaw- -> kylinbot- 规则已覆盖

# ===== kebab-case crate 名 =====
s/zeroclaw-/kylinbot-/g

# ===== snake_case 模块/变量 =====
s/zeroclaw_/kylinbot_/g

# ===== zero + 任意非字母分隔符 + claw（通用规则） =====
# 匹配 zero<sep>claw 其中 <sep> 是一个或多个非字母字符
# 例如: zero_claw, zero-claw, zero.claw, zero<claw, zero>claw, zero claw ...
s/zero([^a-zA-Z]{1,})claw/kylin\1bot/g
s/ZERO([^a-zA-Z]{1,})CLAW/KYLIN\1BOT/g
s/Zero([^a-zA-Z]{1,})Claw/Kylin\1Bot/g

# ===== 独立小写 zeroclaw（最后处理） =====
s/zeroclaw/kylinbot/g
SED_EXPR
}

# 对路径组件做重命名
rename_path() {
    echo "$1" | sed -E \
        -e 's/zeroclawlabs/kylinbot/g' \
        -e 's/zeroclaw-labs/kylinbot/g' \
        -e 's/ZeroClawAgent/KylinBotAgent/g' \
        -e 's/ZeroClawOperator/KylinBotOperator/g' \
        -e 's/ZeroClawMaintainer/KylinBotMaintainer/g' \
        -e 's/ZEROCLAW_/KYLINBOT_/g' \
        -e 's/ZEROCLAW/KYLINBOT/g' \
        -e 's/ZeroClaw/KylinBot/g' \
        -e 's/Zeroclaw/Kylinbot/g' \
        -e 's/zeroclaw-/kylinbot-/g' \
        -e 's/zeroclaw_/kylinbot_/g' \
        -e 's/zero([^a-zA-Z]{1,})claw/kylin\1bot/g' \
        -e 's/ZERO([^a-zA-Z]{1,})CLAW/KYLIN\1BOT/g' \
        -e 's/zeroclaw/kylinbot/g' \
        -e 's/ZEROCLAW/KYLINBOT/g'
}

# ---------------------------------------------------------------------------
# 收集文件列表（两个模式共用）
# git ls-files 只返回 git 跟踪的文件，但 web/ 等目录在 .gitignore 中，
# 即使被 git add -f 强制跟踪，也可能被部分规则屏蔽。
# 因此额外用 find 扫描 web/ 目录（排除 node_modules 和 dist）。
# ---------------------------------------------------------------------------

log "获取文件列表..."

declare -A SEEN_FILES
GIT_TEXT_FILES=()

add_file() {
    local f="$1"
    [[ -z "$f" ]] && return
    [[ "$f" == target/* ]] && return
    [[ "$f" == */node_modules/* ]] && return
    [[ "$f" == */dist/* ]] && return
    case "${f##*/}" in
        *.png|*.jpg|*.jpeg|*.gif|*.ico|*.uf2|*.woff|*.woff2|*.ttf|*.eot|*.mp3|*.pdf|*.wasm) return ;;
    esac
    [[ -f "$f" ]] || return
    [[ -n "${SEEN_FILES[$f]:-}" ]] && return
    SEEN_FILES[$f]=1
    GIT_TEXT_FILES+=("$f")
}

# 1) git 跟踪的文件
while IFS= read -r f; do
    add_file "$f"
done < <(git ls-files)

# 2) web/ 目录下被 gitignore 排除的文件（如 web/src/）
if [[ -d web ]]; then
    while IFS= read -r f; do
        add_file "$f"
    done < <(find web -type f \( -name '*.ts' -o -name '*.tsx' -o -name '*.js' -o -name '*.jsx' -o -name '*.css' -o -name '*.scss' -o -name '*.html' -o -name '*.json' -o -name '*.svg' -o -name '*.md' \) 2>/dev/null)
fi

log "找到 ${#GIT_TEXT_FILES[@]} 个文本文件"

# ===========================================================================
# --content: 文件内容替换
# ===========================================================================

CHANGED_COUNT=0
BANNER_COUNT=0
COMMIT1=""

do_content() {
    echo ""
    echo "===== 内容替换 ====="

    # --- 2a: sed 文本替换 ---
    log "替换文本内容..."

    SED_SCRIPT=$(build_sed_expr)
    local count=0

    for f in "${GIT_TEXT_FILES[@]}"; do
        if grep -q -i -E 'zeroclaw|zero[^a-zA-Z]+claw' "$f" 2>/dev/null; then
            if $DRY_RUN; then
                NEW_CONTENT=$(sed -E "$SED_SCRIPT" "$f")
                if ! diff -q <(cat "$f") <(echo "$NEW_CONTENT") > /dev/null 2>&1; then
                    log "[DRY] 内容变更: $f"
                    count=$((count + 1))
                fi
            else
                if sed --version 2>&1 | grep -q GNU; then
                    sed -i -E "$SED_SCRIPT" "$f"
                else
                    sed -i '' -E "$SED_SCRIPT" "$f"
                fi
                count=$((count + 1))
            fi
        fi
    done
    CHANGED_COUNT=$count
    log "文本替换完成: $CHANGED_COUNT 个文件"

    # --- 2b: ASCII art banner 多行替换 ---

    OLD_BANNER_NOINDENT=' ███████╗███████╗██████╗  ██████╗  ██████╗██╗      █████╗ ██╗    ██╗
 ╚══███╔╝██╔════╝██╔══██╗██╔═══██╗██╔════╝██║     ██╔══██╗██║    ██║
   ███╔╝ █████╗  ██████╔╝██║   ██║██║     ██║     ███████║██║ █╗ ██║
  ███╔╝  ██╔══╝  ██╔══██╗██║   ██║██║     ██║     ██╔══██║██║███╗██║
 ███████╗███████╗██║  ██║╚██████╔╝╚██████╗███████╗██║  ██║╚███╔███╔╝
 ╚══════╝╚══════╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚═╝  ╚═╝ ╚══╝╚══╝'

    OLD_BANNER_INDENT='    ███████╗███████╗██████╗  ██████╗  ██████╗██╗      █████╗ ██╗    ██╗
    ╚══███╔╝██╔════╝██╔══██╗██╔═══██╗██╔════╝██║     ██╔══██╗██║    ██║
      ███╔╝ █████╗  ██████╔╝██║   ██║██║     ██║     ███████║██║ █╗ ██║
     ███╔╝  ██╔══╝  ██╔══██╗██║   ██║██║     ██║     ██╔══██║██║███╗██║
    ███████╗███████╗██║  ██║╚██████╔╝╚██████╗███████╗██║  ██║╚███╔███╔╝
    ╚══════╝╚══════╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚═╝  ╚═╝ ╚══╝╚══╝'

    NEW_BANNER_NOINDENT=' █████   ████ █████ █████ █████       █████ ██████   █████ ███████████     ███████    ███████████
░░███   ███░ ░░███ ░░███ ░░███       ░░███ ░░██████ ░░███ ░░███░░░░░███  ███░░░░░███ ░█░░░███░░░█
 ░███  ███    ░░███ ███   ░███        ░███  ░███░███ ░███  ░███    ░███ ███     ░░███░   ░███  ░
 ░███████      ░░█████    ░███        ░███  ░███░░███░███  ░██████████ ░███      ░███    ░███
 ░███░░███      ░░███     ░███        ░███  ░███ ░░██████  ░███░░░░░███░███      ░███    ░███
 ░███ ░░███      ░███     ░███      █ ░███  ░███  ░░█████  ░███    ░███░░███     ███     ░███
 █████ ░░████    █████    ███████████ █████ █████  ░░█████ ███████████  ░░░███████░      █████
░░░░░   ░░░░    ░░░░░    ░░░░░░░░░░░ ░░░░░ ░░░░░    ░░░░░ ░░░░░░░░░░░     ░░░░░░░       ░░░░░'

    NEW_BANNER_INDENT='     █████   ████ █████ █████ █████       █████ ██████   █████ ███████████     ███████    ███████████
    ░░███   ███░ ░░███ ░░███ ░░███       ░░███ ░░██████ ░░███ ░░███░░░░░███  ███░░░░░███ ░█░░░███░░░█
     ░███  ███    ░░███ ███   ░███        ░███  ░███░███ ░███  ░███    ░███ ███     ░░███░   ░███  ░
     ░███████      ░░█████    ░███        ░███  ░███░░███░███  ░██████████ ░███      ░███    ░███
     ░███░░███      ░░███     ░███        ░███  ░███ ░░██████  ░███░░░░░███░███      ░███    ░███
     ░███ ░░███      ░███     ░███      █ ░███  ░███  ░░█████  ░███    ░███░░███     ███     ░███
     █████ ░░████    █████    ███████████ █████ █████  ░░█████ ███████████  ░░░███████░      █████
    ░░░░░   ░░░░    ░░░░░    ░░░░░░░░░░░ ░░░░░ ░░░░░    ░░░░░ ░░░░░░░░░░░     ░░░░░░░       ░░░░░'

    log "替换 ASCII art banner..."
    BANNER_FILES=(
        crates/zeroclaw-tui/src/widgets.rs
        crates/zeroclaw-runtime/src/onboard/wizard.rs
        tests/manual/telegram/test_telegram_integration.sh
        docs/contributing/testing-telegram.md
        docs/i18n/zh-CN/contributing/testing-telegram.zh-CN.md
    )
    local bcount=0
    for bf in "${BANNER_FILES[@]}"; do
        [[ -f "$bf" ]] || continue
        if grep -qF '███████╗███████╗██████╗' "$bf" 2>/dev/null; then
            if $DRY_RUN; then
                log "[DRY] Banner 替换: $bf"
            else
                perl -i -0pe "
                    s|\Q$OLD_BANNER_NOINDENT\E|$NEW_BANNER_NOINDENT|g;
                    s|\Q$OLD_BANNER_INDENT\E|$NEW_BANNER_INDENT|g;
                " "$bf"
                log "Banner 替换: $bf"
            fi
            bcount=$((bcount + 1))
        fi
    done
    BANNER_COUNT=$bcount
    log "Banner 替换完成: $BANNER_COUNT 个文件"

    # --- 2c: wizard.rs 清理装饰行 ---

    WIZARD_FILE="crates/zeroclaw-runtime/src/onboard/wizard.rs"
    if [[ -f "$WIZARD_FILE" ]]; then
        log "清理 wizard.rs 中的装饰行..."
        if $DRY_RUN; then
            grep -n -E '^\s*⚡{5,}|^\s*Zero overhead\.' "$WIZARD_FILE" 2>/dev/null | while IFS= read -r line; do
                log "[DRY] 删除行: $line"
            done
        else
            perl -i -0pe '
                s/[ \t]*⚡⚡⚡⚡⚡+.*\n//g;
                s/\n?\n?[ \t]*Zero overhead\.[^\n]*\n(?:\n)?/\n/g;
            ' "$WIZARD_FILE"
            log "wizard.rs 装饰行清理完成"
        fi
    fi
}

# ===========================================================================
# --rename: 文件/目录重命名
# ===========================================================================

RENAMED_COUNT=0
COMMIT2=""

# 计算目录/文件的新名称
compute_new_name() {
    local base="$1"    # 目录/文件的 basename
    local parent="$2"  # 父目录路径
    # crates/ 下也使用通用 rename_path（zeroclaw- → kylinbot-）
    rename_path "$base"
}

do_rename() {
    echo ""
    echo "===== 文件重命名 ====="
    local count=0

    # ---- 阶段 1：重命名目录（最深优先，git mv 目录 = 原子搬移所有内容） ----

    log "[阶段1] 收集需要重命名的目录..."

    # 从 git ls-files 提取所有祖先目录（不仅是直接父目录）
    # 逐级展开路径：a/b/c/d.rs → a, a/b, a/b/c
    # 然后筛出 basename 含 zeroclaw 的目录
    local DIR_LIST=()
    while IFS= read -r dir; do
        [[ -z "$dir" ]] && continue
        [[ "$dir" == target/* ]] && continue
        base=$(basename "$dir")
        lower=$(echo "$base" | tr '[:upper:]' '[:lower:]')
        case "$lower" in
            *zeroclaw*|*zero_claw*|*zero-claw*) DIR_LIST+=("$dir") ;;
        esac
    done < <({
        git ls-files
        if [[ -d web ]]; then find web -type f 2>/dev/null; fi
    } | awk -F/ '{
        p = ""
        for (i = 1; i < NF; i++) {
            p = p ? p "/" $i : $i
            print p
        }
    }' | sort -u)

    # 按深度降序排列（最深的先处理，避免父目录先改名后找不到子目录）
    if [[ ${#DIR_LIST[@]} -gt 0 ]]; then
        IFS=$'\n' SORTED_DIRS=($(printf '%s\n' "${DIR_LIST[@]}" | awk -F/ '{print NF, $0}' | sort -rn | cut -d' ' -f2-))
        unset IFS

        log "找到 ${#SORTED_DIRS[@]} 个需要重命名的目录"

        for old_dir in "${SORTED_DIRS[@]}"; do
            [[ -d "$old_dir" ]] || continue  # 可能已被父目录搬移带走
            parent=$(dirname "$old_dir")
            base=$(basename "$old_dir")
            new_base=$(compute_new_name "$base" "$parent")
            new_dir="$parent/$new_base"

            if [[ "$old_dir" != "$new_dir" ]]; then
                if $DRY_RUN; then
                    log "[DRY] 目录重命名: $old_dir -> $new_dir"
                else
                    mkdir -p "$(dirname "$new_dir")"
                    git mv "$old_dir" "$new_dir"
                    log "目录重命名: $old_dir -> $new_dir"
                fi
                count=$((count + 1))
            fi
        done
    fi

    # ---- 阶段 2：重命名剩余的单个文件（文件名含 zeroclaw 但不在已改名目录中） ----

    log "[阶段2] 检查需要改名的单个文件..."

    local FILE_LIST=()
    while IFS= read -r f; do
        [[ "$f" == target/* ]] && continue
        base=$(basename "$f")
        lower=$(echo "$base" | tr '[:upper:]' '[:lower:]')
        case "$lower" in
            *zeroclaw*|*zero_claw*|*zero-claw*) FILE_LIST+=("$f") ;;
        esac
    done < <({
        git ls-files
        if [[ -d web ]]; then find web -type f 2>/dev/null; fi
    })

    if [[ ${#FILE_LIST[@]} -gt 0 ]]; then
        log "找到 ${#FILE_LIST[@]} 个需要改名的文件"

        for old_file in "${FILE_LIST[@]}"; do
            [[ -f "$old_file" ]] || continue  # 可能已在阶段1被目录搬移
            parent=$(dirname "$old_file")
            base=$(basename "$old_file")
            new_base=$(rename_path "$base")
            new_file="$parent/$new_base"

            if [[ "$old_file" != "$new_file" ]]; then
                if $DRY_RUN; then
                    log "[DRY] 文件重命名: $old_file -> $new_file"
                else
                    git mv "$old_file" "$new_file"
                    log "文件重命名: $old_file -> $new_file"
                fi
                count=$((count + 1))
            fi
        done
    fi

    RENAMED_COUNT=$count
    log "重命名完成: $RENAMED_COUNT 个目录+文件"
}

# ===========================================================================
# 执行
# ===========================================================================

if $DO_ALL && ! $DRY_RUN; then
    # --all 模式：分步执行并自动提交
    do_content
    git add -A -f
    COMMIT1=$(git commit -m "$(cat <<'EOF'
refactor: kylinbot in file contents
EOF
    )")
    COMMIT1=$(git rev-parse --short HEAD)
    log "内容替换已提交: $COMMIT1"

    do_rename
    git add -A -f
    COMMIT2=$(git commit -m "$(cat <<'EOF'
refactor: rename directories and files to kylinbot
EOF
    )")
    COMMIT2=$(git rev-parse --short HEAD)
    log "文件重命名已提交: $COMMIT2"
else
    # 单步模式（或 dry-run）
    if $DO_CONTENT; then do_content; fi
    if $DO_RENAME;  then do_rename; fi
fi

# ===========================================================================
# 总结
# ===========================================================================

echo ""
echo "=========================================="
echo "  执行总结"
echo "=========================================="
if $DO_CONTENT; then
echo "  文本内容替换: $CHANGED_COUNT 个文件"
echo "  ASCII art banner: $BANNER_COUNT 个文件"
fi
if $DO_RENAME; then
echo "  文件/目录重命名: $RENAMED_COUNT 个路径"
fi
if [[ -n "$COMMIT1" ]]; then
echo "  提交 1 (内容替换): $COMMIT1"
fi
if [[ -n "$COMMIT2" ]]; then
echo "  提交 2 (文件重命名): $COMMIT2"
fi
echo "=========================================="

if $DRY_RUN; then
    echo ""
    echo "这是 DRY RUN 预览。去掉 --dry-run 参数以实际执行。"
fi
