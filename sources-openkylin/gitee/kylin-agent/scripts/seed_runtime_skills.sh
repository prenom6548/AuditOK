#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $(basename "$0") SOURCE_SKILLS_DIR TARGET_SKILLS_DIR" >&2
    exit 2
fi

SOURCE_DIR="$1"
TARGET_DIR="$2"

if [[ -z "${SOURCE_DIR}" || -z "${TARGET_DIR}" || "${TARGET_DIR}" == "/" ]]; then
    echo "Invalid skills path" >&2
    exit 2
fi

if [[ ! -d "${SOURCE_DIR}" ]]; then
    echo "Source skills directory not found" >&2
    exit 1
fi

if [[ -e "${TARGET_DIR}" ]]; then
    if [[ -d "${TARGET_DIR}" ]]; then
        exit 0
    fi
    echo "Target skills path exists but is not a directory" >&2
    exit 1
fi

TARGET_PARENT="$(dirname "${TARGET_DIR}")"
mkdir -p "${TARGET_PARENT}"

OWNER_UID="${PKEXEC_UID:-${SUDO_UID:-}}"
OWNER_GID=""
if [[ -n "${OWNER_UID}" ]]; then
    OWNER_GID="$(id -g "${OWNER_UID}" 2>/dev/null || true)"
fi

if [[ -z "${OWNER_UID}" || -z "${OWNER_GID}" ]]; then
    OWNER_UID="$(stat -c '%u' "${TARGET_PARENT}")"
    OWNER_GID="$(stat -c '%g' "${TARGET_PARENT}")"
fi

TMP_DIR="${TARGET_DIR}.tmp.$$"
cleanup() {
    rm -rf "${TMP_DIR}"
}
trap cleanup EXIT

rm -rf "${TMP_DIR}"
cp -a "${SOURCE_DIR}" "${TMP_DIR}"
chown -R "${OWNER_UID}:${OWNER_GID}" "${TMP_DIR}"
mv "${TMP_DIR}" "${TARGET_DIR}"
trap - EXIT
