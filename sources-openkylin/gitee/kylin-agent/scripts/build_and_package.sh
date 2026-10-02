#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

if [[ ! -f "${ROOT_DIR}/CMakeLists.txt" ]]; then
    echo "[ERROR] Could not find project root from ${BASH_SOURCE[0]}" >&2
    exit 1
fi

APP_NAME="${APP_NAME:-kylin-agent}"
DISPLAY_NAME="${DISPLAY_NAME:-KylinAgent}"
MAINTAINER="${MAINTAINER:-Kylin Agent Team <lihanhua@yhkylin.com>}"
DESCRIPTION="${DESCRIPTION:-Kylin AI agent desktop application}"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build-package}"
PKG_DIR="${PKG_DIR:-${ROOT_DIR}/package}"
DIST_DIR="${DIST_DIR:-${ROOT_DIR}/dist}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
ARCH="${ARCH:-$(dpkg --print-architecture)}"
APP_SHARE_DIR="/usr/local/share/${APP_NAME}"

case "${BUILD_DIR}" in
    /*) ;;
    *) BUILD_DIR="${ROOT_DIR}/${BUILD_DIR}" ;;
esac
case "${PKG_DIR}" in
    /*) ;;
    *) PKG_DIR="${ROOT_DIR}/${PKG_DIR}" ;;
esac
case "${DIST_DIR}" in
    /*) ;;
    *) DIST_DIR="${ROOT_DIR}/${DIST_DIR}" ;;
esac

if [[ -z "${APP_VERSION:-}" ]]; then
    APP_VERSION="$(
        sed -nE 's/^[[:space:]]*project\([^)]*VERSION[[:space:]]+([0-9][^[:space:]\)]*).*/\1/p' "${ROOT_DIR}/CMakeLists.txt" | head -1
    )"
fi
APP_VERSION="${APP_VERSION:-1.0.0}"

DEB_FILE="${DIST_DIR}/${APP_NAME}_${APP_VERSION}_${ARCH}.deb"
RUNTIME_INSTALLER_PATH="${APP_SHARE_DIR}/scripts/agent_runtime_install.sh"
# YDOTOOL_INSTALLER_PATH="${APP_SHARE_DIR}/scripts/ydotool_install.sh"
RUNTIME_SKILLS_SEEDER_PATH="${APP_SHARE_DIR}/scripts/seed_runtime_skills.sh"
HERMES_HOME_REPAIR_PATH="${APP_SHARE_DIR}/scripts/repair_hermes_home.sh"
RUNTIME_CACHE_INSTALLER_PATH="${APP_SHARE_DIR}/scripts/install_runtime_cache.sh"

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "[ERROR] Missing required command: $1" >&2
        exit 1
    fi
}

echo "[INFO] Checking build tools..."
require_command cmake
require_command dpkg-deb
require_command dpkg
require_command tar
require_command rsync

echo "[INFO] Cleaning package workspace..."
rm -rf "${BUILD_DIR}" "${PKG_DIR}"
mkdir -p "${BUILD_DIR}" "${PKG_DIR}" "${DIST_DIR}"

echo "[INFO] Configuring ${APP_NAME} ${APP_VERSION} (${BUILD_TYPE})..."
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_INSTALL_PREFIX=/usr

echo "[INFO] Building..."
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

echo "[INFO] Installing into package root..."
DESTDIR="${PKG_DIR}" cmake --install "${BUILD_DIR}"

echo "[INFO] Installing bundled executables..."
mkdir -p "${PKG_DIR}/usr/bin"
install -m 0755 "${ROOT_DIR}/bin/intent-act" "${PKG_DIR}/usr/bin/intent-act"
install -m 0755 "${ROOT_DIR}/bin/kylin-actuator" "${PKG_DIR}/usr/bin/kylin-actuator"

echo "[INFO] Installing documentation..."
mkdir -p "${PKG_DIR}${APP_SHARE_DIR}/docs"
install -m 0644 "${ROOT_DIR}/docs/openKylin系统智能体用户手册.pdf" "${PKG_DIR}${APP_SHARE_DIR}/docs/openKylin系统智能体用户手册.pdf"
install -m 0644 "${ROOT_DIR}/docs/openKylin系统智能体开发者手册.pdf" "${PKG_DIR}${APP_SHARE_DIR}/docs/openKylin系统智能体开发者手册.pdf"

echo "[INFO] Installing runtime installer..."
mkdir -p "${PKG_DIR}$(dirname "${RUNTIME_INSTALLER_PATH}")"
install -m 0755 "${ROOT_DIR}/scripts/agent_runtime_install.sh" "${PKG_DIR}${RUNTIME_INSTALLER_PATH}"
# install -m 0755 "${ROOT_DIR}/scripts/ydotool_install.sh" "${PKG_DIR}${YDOTOOL_INSTALLER_PATH}"
install -m 0755 "${ROOT_DIR}/scripts/seed_runtime_skills.sh" "${PKG_DIR}${RUNTIME_SKILLS_SEEDER_PATH}"
install -m 0755 "${ROOT_DIR}/scripts/repair_hermes_home.sh" "${PKG_DIR}${HERMES_HOME_REPAIR_PATH}"
install -m 0755 "${ROOT_DIR}/scripts/install_runtime_cache.sh" "${PKG_DIR}${RUNTIME_CACHE_INSTALLER_PATH}"

echo "[INFO] Creating Debian metadata..."
mkdir -p "${PKG_DIR}/DEBIAN"

DEPENDS=""
if command -v dpkg-shlibdeps >/dev/null 2>&1; then
    set +e
    DEPENDS="$(dpkg-shlibdeps -O -e"${PKG_DIR}/usr/bin/${APP_NAME}" 2>/dev/null | sed -n 's/^shlibs:Depends=//p')"
    set -e
fi

if [[ -z "${DEPENDS}" ]]; then
    DEPENDS="libc6, libgcc-s1, libstdc++6, zlib1g, libkysdk-qtwidgets, build-essential, curl, ydotool, git, grim, wl-clipboard, kylin-agent-runtime-cache, portaudio19-dev, ripgrep, wget, rsync, aria2, ffmpeg, python3-dev, python3-pip, clang, make, pkg-config, openssl, ca-certificates, libffi-dev, libqt5core5a, libqt5gui5, libqt5widgets5, libqt5sql5, libsqlite3-dev, libqt5network5, libqt5xml5, libqt5svg5, libqt5websockets5, libqt5multimedia5, libnotify-bin, xclip, xdotool, wmctrl, x11-utils, x11-xserver-utils, xdg-utils, libglib2.0-bin, libgtk-3-bin, scrot, python3-tk"
fi

append_dependency() {
    local dep="$1"
    if [[ ", ${DEPENDS}, " != *", ${dep},"* ]]; then
        DEPENDS="${DEPENDS}, ${dep}"
    fi
}

append_dependency "curl"
append_dependency "git"
append_dependency "rsync"
append_dependency "python3"
append_dependency "libnotify-bin"
append_dependency "libqt5websockets5"
append_dependency "libqt5multimedia5"

cat > "${PKG_DIR}/DEBIAN/control" <<EOF
Package: ${APP_NAME}
Version: ${APP_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Depends: ${DEPENDS}
Description: ${DESCRIPTION}
 ${DISPLAY_NAME} provides a Qt desktop interface for chat sessions,
 model settings, skills, and agent/tool execution details.
EOF

cat > "${PKG_DIR}/DEBIAN/postinst" <<'EOF'
#!/usr/bin/env bash
set -e
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/share/applications >/dev/null 2>&1 || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -q /usr/share/icons/hicolor >/dev/null 2>&1 || true
fi
exit 0
EOF

chmod 0755 "${PKG_DIR}/DEBIAN/postinst"
find "${PKG_DIR}" -type d -exec chmod 0755 {} +
find "${PKG_DIR}" -type f ! -path "${PKG_DIR}/DEBIAN/postinst" -exec chmod u+rw,go+r,go-w {} +
chmod 0755 "${PKG_DIR}/usr/bin/${APP_NAME}"
chmod 0755 "${PKG_DIR}${RUNTIME_INSTALLER_PATH}"
chmod 0755 "${PKG_DIR}${RUNTIME_SKILLS_SEEDER_PATH}"
chmod 0755 "${PKG_DIR}${HERMES_HOME_REPAIR_PATH}"
chmod 0755 "${PKG_DIR}${RUNTIME_CACHE_INSTALLER_PATH}"

echo "[INFO] Building deb package..."
rm -f "${DEB_FILE}"
dpkg-deb --root-owner-group --build "${PKG_DIR}" "${DEB_FILE}"

echo "[INFO] Package created: ${DEB_FILE}"
dpkg-deb --info "${DEB_FILE}"
