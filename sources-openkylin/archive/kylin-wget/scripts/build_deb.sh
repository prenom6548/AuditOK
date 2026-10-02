#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

DIST_DIR="${ROOT_DIR}/dist"
mkdir -p "${DIST_DIR}"

if [[ ! -f "${ROOT_DIR}/debian/control" ]]; then
  echo "[build_deb] ERROR: missing debian/control (template build requires debian/ directory)" >&2
  exit 2
fi

# Ensure debian/rules executable.
chmod +x "${ROOT_DIR}/debian/rules" >/dev/null 2>&1 || true

cd "${ROOT_DIR}"

echo "[build_deb] dpkg-buildpackage (template debian/ rules) ..."

# -b: binary-only, avoid .orig tar dependency
# -d: skip build-dep check for local developer convenience
# -rfakeroot: allow building without real root
dpkg-buildpackage -us -uc -b -d -rfakeroot >/tmp/wget-rust-build_deb.log 2>&1 || {
  echo "[build_deb] ERROR: dpkg-buildpackage failed. tail log:"
  tail -n 80 /tmp/wget-rust-build_deb.log || true
  exit 1
}

# dpkg-buildpackage puts .deb one directory up by default.
OUT_DIR_PARENT="$(cd "${ROOT_DIR}" && pwd)/.."

PKG_NAME="$(awk '/^Package:/{print $2; exit}' "${ROOT_DIR}/debian/control")"
DEB_GLOB="${OUT_DIR_PARENT}/${PKG_NAME}_"*.deb

DEB_PATH="$(ls -1t ${DEB_GLOB} 2>/dev/null | head -n 1 || true)"
if [[ -z "${DEB_PATH}" ]]; then
  echo "[build_deb] ERROR: built .deb not found under: ${DEB_GLOB}" >&2
  exit 2
fi

DEB_BASENAME="$(basename "${DEB_PATH}")"
cp "${DEB_PATH}" "${DIST_DIR}/${DEB_BASENAME}"

echo "[build_deb] done: ${DIST_DIR}/${DEB_BASENAME}"

if command -v dpkg-deb >/dev/null 2>&1; then
  echo "[build_deb] package info:"
  dpkg-deb -I "${DIST_DIR}/${DEB_BASENAME}" | sed -n '1,15p' || true
fi

