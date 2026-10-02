#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

PACKAGE_NAME="${PACKAGE_NAME:-kylin-agent-runtime-cache}"
PACKAGE_VERSION="${PACKAGE_VERSION:-1.0.0}"
MAINTAINER="${MAINTAINER:-Kylin Agent Team <lihanhua@yhkylin.com>}"
DESCRIPTION="${DESCRIPTION:-Kylin Agent runtime cache package}"
SOURCE_HOME="${SOURCE_HOME:-${HOME}}"
RUNTIME_HOME_BASENAME="${RUNTIME_HOME_BASENAME:-.kylin-agent-runtime}"
RUNTIME_SHARE_DIR="${RUNTIME_SHARE_DIR:-/usr/local/share/kylin-agent-runtime}"
BUILD_ROOT="${BUILD_ROOT:-${ROOT_DIR}/build-runtime-cache-package}"
DIST_DIR="${DIST_DIR:-${ROOT_DIR}/dist}"
ARCH="${ARCH:-$(dpkg --print-architecture)}"
PYTHON_VERSION="${PYTHON_VERSION:-3.11}"
PREWARM_RUNTIME_CACHE="${PREWARM_RUNTIME_CACHE:-1}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [SOURCE_HOME]

Build a Debian package that installs selected Kylin Agent runtime data into:
  ${RUNTIME_SHARE_DIR}/local
  ${RUNTIME_SHARE_DIR}/cache

Defaults:
  SOURCE_HOME=${HOME}
  output=${DIST_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb

Environment overrides:
  SOURCE_HOME         Source user's home directory
  RUNTIME_HOME_BASENAME Runtime home directory name under SOURCE_HOME
  RUNTIME_SHARE_DIR  Runtime cache package install directory
  PACKAGE_NAME       Debian package name
  PACKAGE_VERSION    Debian package version
  MAINTAINER         Debian package maintainer
  DESCRIPTION        Debian package description
  BUILD_ROOT         Temporary package root
  DIST_DIR           Output directory
  ARCH               Debian architecture
  PYTHON_VERSION     Python version used to validate uv cache (default: ${PYTHON_VERSION})
  PREWARM_RUNTIME_CACHE
                     Set to 0 to skip dependency cache prewarm/validation
EOF
}

log_info() {
    printf '[INFO] %s\n' "$1"
}

log_warn() {
    printf '[WARN] %s\n' "$1" >&2
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

find_uv() {
    local candidate
    for candidate in \
        "${SOURCE_HOME}/${RUNTIME_HOME_BASENAME}/bin/uv" \
        "${SOURCE_HOME}/.local/bin/uv"; do
        if [[ -x "${candidate}" ]]; then
            printf '%s\n' "${candidate}"
            return 0
        fi
    done

    if command -v uv >/dev/null 2>&1; then
        command -v uv
        return 0
    fi

    return 1
}

prewarm_runtime_cache() {
    local runtime_dir="${SOURCE_HOME}/${RUNTIME_HOME_BASENAME}/agent-runtime"
    local uv_cmd
    local prewarm_venv="${BUILD_ROOT}/cache-prewarm-venv"

    if [[ "${PREWARM_RUNTIME_CACHE}" != "1" ]]; then
        log_warn "Skipping runtime dependency cache prewarm (PREWARM_RUNTIME_CACHE=${PREWARM_RUNTIME_CACHE})."
        return 0
    fi

    if [[ ! -d "${runtime_dir}" ]]; then
        log_warn "Runtime checkout not found; cannot prewarm dependency cache: ${runtime_dir}"
        return 0
    fi

    if ! uv_cmd="$(find_uv)"; then
        log_error "uv not found; cannot prewarm dependency cache."
        log_error "Install runtime once, or set PREWARM_RUNTIME_CACHE=0 to build without validation."
        exit 1
    fi

    log_info "Prewarming uv dependency cache for ${runtime_dir}"
    mkdir -p "${SOURCE_HOME}/.cache/uv"
    rm -rf "${prewarm_venv}"

    (
        cd "${runtime_dir}"
        export UV_NO_CONFIG=1
        export UV_CACHE_DIR="${SOURCE_HOME}/.cache/uv"
        "${uv_cmd}" venv "${prewarm_venv}" --python "${PYTHON_VERSION}"

        if [[ -f "uv.lock" ]]; then
            if UV_PROJECT_ENVIRONMENT="${prewarm_venv}" "${uv_cmd}" sync --extra all --locked; then
                exit 0
            fi
            log_warn "uv.lock sync failed during cache prewarm; falling back to uv pip install -e '.[all]'."
        fi

        VIRTUAL_ENV="${prewarm_venv}" UV_PYTHON="${prewarm_venv}/bin/python" \
            "${uv_cmd}" pip install -e '.[all]'
    ) || {
        log_error "Failed to prewarm runtime dependency cache."
        log_error "Run this package build on a machine with network access, then rebuild the cache deb."
        exit 1
    }

    rm -rf "${prewarm_venv}"
    log_info "uv dependency cache prewarm complete."
}

copy_dir() {
    local source="$1"
    local target="$2"
    shift 2

    if [[ ! -d "${source}" ]]; then
        log_warn "Skip missing directory: ${source}"
        return 0
    fi

    log_info "Copying ${source} -> ${target}"
    mkdir -p "${target}"
    rsync -a --delete "$@" "${source}/" "${target}/"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

if [[ $# -ge 1 ]]; then
    SOURCE_HOME="$1"
fi
if [[ $# -gt 1 ]]; then
    usage >&2
    exit 1
fi

if [[ ! -d "${SOURCE_HOME}" ]]; then
    log_error "Source home directory not found: ${SOURCE_HOME}"
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
PAYLOAD_DIR="${PKG_DIR}${RUNTIME_SHARE_DIR}"
DEB_FILE="${DIST_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb"

log_info "Cleaning package workspace..."
rm -rf "${PKG_DIR}"
mkdir -p "${PAYLOAD_DIR}/local" "${PAYLOAD_DIR}/cache" "${PKG_DIR}/DEBIAN" "${DIST_DIR}"

RUNTIME_HOME_SOURCE="${SOURCE_HOME}/${RUNTIME_HOME_BASENAME}"

prewarm_runtime_cache

copy_dir "${RUNTIME_HOME_SOURCE}" "${PAYLOAD_DIR}/local/${RUNTIME_HOME_BASENAME}" \
    --exclude='/hermes-agent/' \
    --exclude='/agent-runtime/venv/' \
    --exclude='/cron/' \
    --exclude='/sessions/' \
    --exclude='/logs/' \
    --exclude='/pairing/' \
    --exclude='/hooks/' \
    --exclude='/image_cache/' \
    --exclude='/audio_cache/' \
    --exclude='/memories/' \
    --exclude='/kylin-agent/message-details/' \
    --exclude='/state.db' \
    --exclude='/response_store.db' \
    --exclude='/kanban.db' \
    --exclude='/desktop/sessions.json' \
    --exclude='/channel_directory.json' \
    --exclude='/cache-installed' \
    --exclude='/.tirith-install-failed' \
    --exclude='/config.yaml.bak-*' \
    --exclude='/.env' \
    --exclude='/config.yaml' \
    --exclude='/models.json' \
    --exclude='/SOUL.md' \
    --exclude='/gateway.pid' \
    --exclude='/gateway.lock' \
    --exclude='/gateway_state.json' \
    --exclude='/cron/.tick.lock' \
    --exclude='*.lock' \
    --exclude='*-shm' \
    --exclude='*-wal'

copy_dir "${SOURCE_HOME}/.local/share/uv" "${PAYLOAD_DIR}/local/share/uv"
copy_dir "${SOURCE_HOME}/.local/lib/python3.12" "${PAYLOAD_DIR}/local/lib/python3.12"
copy_dir "${SOURCE_HOME}/.local/bin" "${PAYLOAD_DIR}/local/bin" \
    --exclude='/hermes' \
    --exclude='/node' \
    --exclude='/npm' \
    --exclude='/npx' \
    --exclude='/python3.11'
copy_dir "${SOURCE_HOME}/.local/share/kylin-cua" "${PAYLOAD_DIR}/local/share/kylin-cua"

copy_dir "${SOURCE_HOME}/.cache/uv" "${PAYLOAD_DIR}/cache/uv"
copy_dir "${SOURCE_HOME}/.cache/pip" "${PAYLOAD_DIR}/cache/pip"
copy_dir "${SOURCE_HOME}/.cache/ms-playwright" "${PAYLOAD_DIR}/cache/ms-playwright"

install -m 0755 "${ROOT_DIR}/scripts/install_runtime_cache.sh" "${PAYLOAD_DIR}/install_runtime_cache.sh"

cat > "${PAYLOAD_DIR}/manifest" <<EOF
SOURCE_HOME=${SOURCE_HOME}
PACKAGE_NAME=${PACKAGE_NAME}
PACKAGE_VERSION=${PACKAGE_VERSION}
RUNTIME_SHARE_DIR=${RUNTIME_SHARE_DIR}
EOF

log_info "Creating Debian metadata..."
cat > "${PKG_DIR}/DEBIAN/control" <<EOF
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Depends: rsync
Description: ${DESCRIPTION}
 This package installs Kylin Agent runtime cache data under
 ${RUNTIME_SHARE_DIR}. Run ${RUNTIME_SHARE_DIR}/install_runtime_cache.sh
 as the target user to copy the cache into that user's home directory.
EOF

cat > "${PKG_DIR}/DEBIAN/postinst" <<EOF
#!/usr/bin/env bash
set -e
chmod -R a+rX "${RUNTIME_SHARE_DIR}" 2>/dev/null || true
chmod 0755 "${RUNTIME_SHARE_DIR}/install_runtime_cache.sh" 2>/dev/null || true
exit 0
EOF

cat > "${PKG_DIR}/DEBIAN/postrm" <<EOF
#!/usr/bin/env bash
set -e
if [ "\${1:-}" = "purge" ]; then
    rm -rf "${RUNTIME_SHARE_DIR}/local" "${RUNTIME_SHARE_DIR}/cache" "${RUNTIME_SHARE_DIR}/manifest" "${RUNTIME_SHARE_DIR}/install_runtime_cache.sh"
fi
exit 0
EOF

chmod 0644 "${PKG_DIR}/DEBIAN/control"
chmod 0755 "${PKG_DIR}/DEBIAN/postinst" "${PKG_DIR}/DEBIAN/postrm"
chmod 0755 "${PAYLOAD_DIR}/install_runtime_cache.sh"

log_info "Building Debian package..."
rm -f "${DEB_FILE}"
dpkg-deb --root-owner-group --build "${PKG_DIR}" "${DEB_FILE}"

log_info "Package created: ${DEB_FILE}"
log_info "Install with: sudo dpkg -i \"${DEB_FILE}\""
log_info "Seed current user with: ${RUNTIME_SHARE_DIR}/install_runtime_cache.sh"
