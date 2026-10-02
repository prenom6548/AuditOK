#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

PACKAGE_NAME="${PACKAGE_NAME:-kylin-agent-runtime}"
PACKAGE_VERSION="${PACKAGE_VERSION:-1.0.0}"
MAINTAINER="${MAINTAINER:-Kylin Agent Team <lihanhua@yhkylin.com>}"
DESCRIPTION="${DESCRIPTION:-Kylin Agent runtime skills package}"
SOURCE_SKILLS_DIR="${SOURCE_SKILLS_DIR:-${HOME}/.kylin-agent-runtime/skills}"
RUNTIME_SHARE_DIR="${RUNTIME_SHARE_DIR:-/usr/local/share/kylin-agent-runtime}"
BUILD_ROOT="${BUILD_ROOT:-${ROOT_DIR}/build-runtime-package}"
DIST_DIR="${DIST_DIR:-${ROOT_DIR}/dist}"
ARCH="${ARCH:-$(dpkg --print-architecture)}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [SOURCE_SKILLS_DIR]

Build a Debian package that installs skills into:
  ${RUNTIME_SHARE_DIR}/skills

Defaults:
  SOURCE_SKILLS_DIR=${HOME}/./skills
  output=${DIST_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb

Environment overrides:
  SOURCE_SKILLS_DIR   Source skills directory
  RUNTIME_SHARE_DIR   Runtime share directory
  PACKAGE_NAME        Debian package name
  PACKAGE_VERSION     Debian package version
  MAINTAINER          Debian package maintainer
  DESCRIPTION         Debian package description
  BUILD_ROOT          Temporary package root
  DIST_DIR            Output directory
  ARCH                Debian architecture
EOF
}

log_info() {
    printf '[INFO] %s\n' "$1"
}

log_error() {
    printf '[ERROR] %s\n' "$1" >&2
}

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        log_error "Missing required command: $1"
        exit 1
    fi
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

if [[ $# -ge 1 ]]; then
    SOURCE_SKILLS_DIR="$1"
fi
if [[ $# -gt 1 ]]; then
    usage >&2
    exit 1
fi

if [[ ! -d "${SOURCE_SKILLS_DIR}" ]]; then
    log_error "Source skills directory not found: ${SOURCE_SKILLS_DIR}"
    exit 1
fi

require_command dpkg
require_command dpkg-deb
require_command rsync

case "${BUILD_ROOT}" in
    /*) ;;
    *) BUILD_ROOT="${ROOT_DIR}/${BUILD_ROOT}" ;;
esac
case "${DIST_DIR}" in
    /*) ;;
    *) DIST_DIR="${ROOT_DIR}/${DIST_DIR}" ;;
esac

PKG_DIR="${BUILD_ROOT}/${PACKAGE_NAME}"
DEST_SKILLS_DIR="${PKG_DIR}${RUNTIME_SHARE_DIR}/skills"
DEB_FILE="${DIST_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb"

log_info "Cleaning package workspace..."
rm -rf "${PKG_DIR}"
mkdir -p "${DEST_SKILLS_DIR}" "${PKG_DIR}/DEBIAN" "${DIST_DIR}"

log_info "Copying skills from ${SOURCE_SKILLS_DIR}"
rsync -a --delete "${SOURCE_SKILLS_DIR}/" "${DEST_SKILLS_DIR}/"

log_info "Creating Debian metadata..."
cat > "${PKG_DIR}/DEBIAN/control" <<EOF
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Description: ${DESCRIPTION}
 This package installs Kylin Agent runtime skills under
 ${RUNTIME_SHARE_DIR}/skills.
EOF

cat > "${PKG_DIR}/DEBIAN/postinst" <<EOF
#!/usr/bin/env bash
set -e
chmod -R a+rX "${RUNTIME_SHARE_DIR}" 2>/dev/null || true
exit 0
EOF

cat > "${PKG_DIR}/DEBIAN/postrm" <<EOF
#!/usr/bin/env bash
set -e
if [ "\${1:-}" = "purge" ]; then
    rm -rf "${RUNTIME_SHARE_DIR}/skills"
fi
exit 0
EOF

chmod 0755 "${PKG_DIR}/DEBIAN/postinst" "${PKG_DIR}/DEBIAN/postrm"
find "${PKG_DIR}" -type d -exec chmod 0755 {} +
find "${PKG_DIR}" -type f \
    ! -path "${PKG_DIR}/DEBIAN/postinst" \
    ! -path "${PKG_DIR}/DEBIAN/postrm" \
    -exec chmod u+rw,go+r,go-w {} +

log_info "Building Debian package..."
rm -f "${DEB_FILE}"
dpkg-deb --root-owner-group --build "${PKG_DIR}" "${DEB_FILE}"

log_info "Package created: ${DEB_FILE}"
log_info "Install with: sudo dpkg -i \"${DEB_FILE}\""
