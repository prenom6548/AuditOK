#!/usr/bin/env bash
# 可选：在可访问 crates.io 的环境中重建 rust/vendor/ 并做离线自检。
# 若已改用 debian/cargo-mirror.toml 走平台镜像，一般不再需要本脚本。
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUST_DIR="${ROOT_DIR}/rust"
CFG="${RUST_DIR}/.cargo/config.toml"

if [[ -f "${CFG}" ]]; then
  BAK="$(mktemp)"
  cp "${CFG}" "${BAK}"
else
  BAK=""
fi

rm -rf "${RUST_DIR}/vendor"
mkdir -p "${RUST_DIR}/.cargo"
(
  cd "${RUST_DIR}"
  cargo vendor --locked vendor
)

if [[ -n "${BAK}" ]]; then
  cp "${BAK}" "${CFG}"
  rm -f "${BAK}"
fi

(
  cd "${RUST_DIR}"
  cargo build --release --locked --offline
)

echo "[refresh_vendor] OK."
