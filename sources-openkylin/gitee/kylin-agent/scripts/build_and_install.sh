#!/usr/bin/env bash
# Keep credentials out of shell tracing, including when invoked with bash -x.
set +x
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

sudo_password="${KYLIN_AGENT_SUDO_PASSWORD-}"
unset KYLIN_AGENT_SUDO_PASSWORD
password_file="${KYLIN_AGENT_SUDO_PASSWORD_FILE:-${ROOT_DIR}/.build-install-password}"
if [[ ${EUID} -ne 0 ]]; then
    command -v sudo >/dev/null || { echo '[ERROR] sudo not found' >&2; exit 1; }
    if [[ -z "${sudo_password}" && -r "${password_file}" ]]; then
        sudo_password="$(<"${password_file}")"
    fi
    if [[ -z "${sudo_password}" ]]; then
        echo '[ERROR] Set KYLIN_AGENT_SUDO_PASSWORD or provide .build-install-password (mode 600).' >&2
        exit 1
    fi
fi

# Pass the same package identity to the builder and installer. Never select an
# arbitrary/older deb using a wildcard or directory modification time.
export APP_NAME="${APP_NAME:-kylin-agent}"
export ARCH="${ARCH:-$(dpkg --print-architecture)}"
export DIST_DIR="${DIST_DIR:-${ROOT_DIR}/dist}"
if [[ "${DIST_DIR}" != /* ]]; then
    DIST_DIR="${ROOT_DIR}/${DIST_DIR}"
fi
if [[ -z "${APP_VERSION:-}" ]]; then
    APP_VERSION="$(sed -nE 's/^[[:space:]]*project\([^)]*VERSION[[:space:]]+([0-9][^[:space:]\)]*).*/\1/p' CMakeLists.txt | head -1)"
fi
export APP_VERSION="${APP_VERSION:-1.0.0}"
deb_file="${DIST_DIR}/${APP_NAME}_${APP_VERSION}_${ARCH}.deb"

bash "${SCRIPT_DIR}/build_and_package.sh"
if [[ ! -f "${deb_file}" ]]; then
    echo "[ERROR] Build completed but package was not found: ${deb_file}" >&2
    exit 1
fi

echo "[INFO] Installing: ${deb_file}"
if [[ ${EUID} -eq 0 ]]; then
    dpkg --force-confdef --force-confold -i "${deb_file}" </dev/null
else
    # Authenticate via stdin, then prevent sudo/dpkg from waiting for input.
    # Password stdin is never passed to the package's maintainer scripts.
    sudo -S -p '' -v <<<"${sudo_password}"
    unset sudo_password
    sudo -n -- dpkg --force-confdef --force-confold -i "${deb_file}" </dev/null
fi
echo "[INFO] Installed: ${deb_file}"
