#!/bin/bash
# ISO Customizer 安装脚本
# 功能：编译Python脚本为二进制，打包为Debian软件包
#修复判断华为机器逻辑，无需自动勾选
# --- 配置参数 ---
APP_NAME="iso-customizer"
VERSION="2.8.0"
ARCH=$(uname -m)
MAINTAINER="wenyuanbo@kylinos.cn"
DESCRIPTION="ISO Customization Tool"

# 目录配置
PY_SCRIPT="iso_customizer.py"
ICON_SVG="iso-customizer.svg"
ICON_PNG="iso-customizer.png"
POLICY_FILE="com.iso-customizer.policy"

# 构建目录
DIST_DIR="dist"
BUILD_DIR="build"
DEB_DIR="iso-customizer-deb"

# --- 初始化检查 ---
# 检查root权限
if [ "$EUID" -ne 0 ]; then
    echo "错误：请使用sudo或以root权限运行此脚本"
    exit 1
fi

# 检查依赖
check_dependencies() {
    apt install -y python3-tk --reinstall

    if ! command -v dpkg-deb &> /dev/null || ! command -v convert &> /dev/null; then
        echo "正在安装必要依赖..."
        apt-get update && apt-get install -y \
            python3-pip \
            genisoimage \
            squashfs-tools \
            imagemagick
    fi

    if ! command -v pyinstaller &> /dev/null && ! python3 -m PyInstaller --version &> /dev/null; then
        echo "缺少依赖：pyinstaller，正在安装..."
        if ! pip3 install --default-timeout=120 pyinstaller; then
            echo "错误：安装 pyinstaller 失败（多为访问 PyPI 超时）"
            echo "请检查网络后重试，或手动执行：pip3 install --default-timeout=120 pyinstaller"
            exit 1
        fi
    fi

    if ! command -v pyinstaller &> /dev/null && ! python3 -m PyInstaller --version &> /dev/null; then
        echo "错误：pyinstaller 安装后仍不可用，请检查 pip3 安装路径是否在 PATH 中"
        exit 1
    fi
}

# --- 清理函数 ---
clean_build() {
    echo "清理旧构建文件..."
    rm -rf "$DIST_DIR" "$BUILD_DIR" "$DEB_DIR"
    find . -name "*.spec" -delete
}

# --- 编译Python脚本 ---
compile_app() {
    echo "使用PyInstaller编译应用程序..."
    
    # 生成48x48 PNG图标
    if [ ! -f "$ICON_PNG" ]; then
        convert -resize 48x48 "$ICON_SVG" "$ICON_PNG" 2>/dev/null || {
            echo "图标转换失败，请确保ImageMagick已安装"
            exit 1
        }
    fi

    if command -v pyinstaller &> /dev/null; then
        pyinstaller --onefile --windowed \
            --name "$APP_NAME" \
            --add-data "$ICON_PNG:." \
            --icon "$ICON_PNG" \
            "$PY_SCRIPT"
    else
        python3 -m PyInstaller --onefile --windowed \
            --name "$APP_NAME" \
            --add-data "$ICON_PNG:." \
            --icon "$ICON_PNG" \
            "$PY_SCRIPT"
    fi

    # 检查编译结果
    if [ ! -f "$DIST_DIR/$APP_NAME" ]; then
        echo "编译失败！"
        exit 1
    fi
}

# --- 准备Debian包结构 ---
prepare_deb() {
    echo "准备Debian包结构..."
    
    # 创建目录结构
    mkdir -p "$DEB_DIR/DEBIAN"
    mkdir -p "$DEB_DIR/usr/bin"
    mkdir -p "$DEB_DIR/usr/share/applications"
    mkdir -p "$DEB_DIR/usr/share/icons/hicolor/scalable/apps"


    # 复制文件
    cp "$DIST_DIR/$APP_NAME" "$DEB_DIR/usr/bin/"
    cp "$ICON_SVG" "$DEB_DIR/usr/share/icons/hicolor/scalable/apps/"
 
    # 创建.desktop文件
    cat > "$DEB_DIR/usr/share/applications/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Name=ISO Customizer
Comment=$DESCRIPTION
Exec=sh -c 'pkexec env DISPLAY=\$DISPLAY XAUTHORITY=\$XAUTHORITY /usr/bin/$APP_NAME'
Icon=$APP_NAME
Terminal=false
Type=Application
Categories=Utility;System;
EOF

    # 创建control文件
    cat > "$DEB_DIR/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: all
Depends: python3-tk,genisoimage,squashfs-tools,policykit-1,imagemagick,binfmt-support,qemu-user-static
Maintainer: wenyuanbo@kylinos.cn
Description: ISO Customization Tool
 A simple tool to customize ISO images.
EOF

    # 创建postinst脚本
    cat > "$DEB_DIR/DEBIAN/postinst" <<EOF
#!/bin/sh
set -e

case "$1" in
    configure)
        # 更新图标缓存
        if [ -d /usr/share/icons/hicolor ]; then
            find /usr/share/icons/hicolor -name icon-theme.cache -delete
            gtk-update-icon-cache -f /usr/share/icons/hicolor
        fi
        
        # 更新桌面数据库
        if [ -x "/usr/bin/update-desktop-database" ]; then
            update-desktop-database /usr/share/applications
        fi
    ;;
esac

exit 0
EOF
    chmod 755 "$DEB_DIR/DEBIAN/postinst"
}

# --- 构建Debian包 ---
build_deb() {
    echo "构建Debian包..."
    dpkg-deb --build "$DEB_DIR" "${APP_NAME}_${VERSION}_${ARCH}.deb"
    
    if [ $? -ne 0 ]; then
        echo "构建失败！"
        exit 1
    fi
    
    # 移动成品到当前目录
    if [ -f "$DEB_DIR.deb" ]; then
        mv "$DEB_DIR.deb" "${APP_NAME}_${VERSION}_${ARCH}.deb"
    fi
}

# --- 主流程 ---
main() {
    check_dependencies
    clean_build
    compile_app
    prepare_deb
    build_deb
    
    echo "构建成功！生成的安装包：${APP_NAME}_${VERSION}_${ARCH}.deb"
    
    # 安装提示
    read -p "是否立即安装？[y/N] " -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        dpkg -i "${APP_NAME}_${VERSION}_${ARCH}.deb" || apt-get install -f
    fi
}

# 执行主函数
main 
