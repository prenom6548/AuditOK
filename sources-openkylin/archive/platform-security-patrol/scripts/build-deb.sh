#!/bin/bash
# Build DEB package for platform-security-patrol using standard debian directory

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEB_OUTPUT_DIR="$PROJECT_DIR/packages"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    log_info "Checking dependencies..."
    
    if ! command -v dpkg-buildpackage &> /dev/null; then
        log_error "dpkg-buildpackage is not installed. Please install dpkg-dev."
        exit 1
    fi
    
    if ! command -v dh &> /dev/null; then
        log_error "debhelper is not installed. Please install debhelper."
        exit 1
    fi
    
    log_info "All dependencies are satisfied."
}

# 清理旧构建
cleanup() {
    log_info "Cleaning up old build artifacts..."
    rm -rf "$PROJECT_DIR/build"
    rm -rf "$PROJECT_DIR/obj-*"
    mkdir -p "$DEB_OUTPUT_DIR"

    # 确保 debian/ 数据文件无可执行权限，避免 dh 将其当作配置脚本执行
    chmod -x "$PROJECT_DIR"/debian/changelog \
             "$PROJECT_DIR"/debian/compat \
             "$PROJECT_DIR"/debian/control \
             "$PROJECT_DIR"/debian/copyright \
             "$PROJECT_DIR"/debian/docs \
             "$PROJECT_DIR"/debian/*.lintian-overrides 2>/dev/null || true
}

# 构建 DEB 包
build_deb_package() {
    log_info "Building DEB package using standard debian directory..."
    
    cd "$PROJECT_DIR"
    
    # 使用 dpkg-buildpackage 标准构建
    # -us -uc: 不签名
    # -b: 仅构建二进制包
    # -d: 忽略构建依赖检查 (如果需要严格检查可去除此选项)
    dpkg-buildpackage \
        -us -uc \
        -b \
        --build=binary \
        -d
    
    if [ $? -eq 0 ]; then
        # 移动生成的包到 packages 目录
        # dpkg-buildpackage 默认在父目录生成包，即 PROJECT_DIR 的父目录或者当前目录取决于具体配置
        # 通常在源码根目录执行时，包会生成在上级目录或当前目录。
        # 这里假设包生成在 PROJECT_DIR 的上级目录或者当前目录，尝试移动所有 deb 文件
        shopt -s nullglob
        local debs=("$PROJECT_DIR"/*.deb)
        
        # 如果当前目录没有，可能在上级目录 (dpkg-buildpackage 默认行为)
        if [ ${#debs[@]} -eq 0 ]; then
             debs=("$PROJECT_DIR"/../*.deb)
        fi

        for deb in "${debs[@]}"; do
            if [ -f "$deb" ]; then
                mv "$deb" "$DEB_OUTPUT_DIR/"
                log_info "Moved $(basename "$deb") to $DEB_OUTPUT_DIR/"
            fi
        done
        
        log_info "DEB package built successfully!"
        
        # 显示包信息
        local deb_file=$(ls "$DEB_OUTPUT_DIR"/*.deb 2>/dev/null | head -1)
        if [ -n "$deb_file" ]; then
            echo ""
            log_info "Package information:"
            dpkg-deb --info "$deb_file"
            echo ""
            log_info "Package location: $deb_file"
        fi
    else
        log_error "Failed to build DEB package."
        exit 1
    fi
}

# 主流程
main() {
    echo "=========================================="
    echo "Platform Security Patrol - DEB Builder"
    echo "Using standard debian/ directory structure"
    echo "=========================================="
    echo ""
    
    check_dependencies
    cleanup
    build_deb_package
    
    echo ""
    echo "=========================================="
    log_info "Build completed successfully!"
    echo "=========================================="
}

main "$@"
