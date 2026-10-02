#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

BIN_NAME="wget"
DIST_DIR="${ROOT_DIR}/dist"

cd "${ROOT_DIR}/rust"

echo "[build_dist] cargo build --release ..."
cargo build --release

cd "${ROOT_DIR}"

mkdir -p "${DIST_DIR}"

TARGET_BIN="${ROOT_DIR}/rust/target/release/${BIN_NAME}"
if [[ ! -f "${TARGET_BIN}" ]]; then
  echo "[build_dist] ERROR: release binary not found at: ${TARGET_BIN}" >&2
  exit 2
fi

VERSION="$(git describe --tags --always --dirty 2>/dev/null || true)"
if [[ -z "${VERSION}" ]]; then
  VERSION="dev"
fi

TAR_NAME="wget-rust-${VERSION}.tar.gz"
TAR_PATH="${DIST_DIR}/${TAR_NAME}"

TMP_DIR="$(mktemp -d)"
cleanup() {
  rm -rf "${TMP_DIR}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

cp "${TARGET_BIN}" "${TMP_DIR}/${BIN_NAME}"
cp "${ROOT_DIR}/README.md" "${TMP_DIR}/README.md" || true

echo "[build_dist] packaging: ${TAR_PATH}"
tar -C "${TMP_DIR}" -czf "${TAR_PATH}" .

if command -v sha256sum >/dev/null 2>&1; then
  sha256sum "${TAR_PATH}" > "${TAR_PATH}.sha256"
fi

echo "[build_dist] done: ${TAR_PATH}"
if [[ -f "${TAR_PATH}.sha256" ]]; then
  echo "[build_dist] sha256: $(cat "${TAR_PATH}.sha256")"
fi

