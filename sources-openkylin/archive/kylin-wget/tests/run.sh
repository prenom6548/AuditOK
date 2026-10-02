#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

ORIG_WGET="${WGET_ORIG:-}"
if [[ -z "${ORIG_WGET}" ]]; then
  if command -v wget >/dev/null 2>&1; then
    ORIG_WGET="$(command -v wget)"
  fi
fi

if [[ -z "${ORIG_WGET}" ]]; then
  echo "ERROR: cannot find original wget. Set WGET_ORIG=/path/to/wget" >&2
  exit 2
fi

echo "Using orig wget: ${ORIG_WGET}"

echo "Building wget (debug)..."
(cd "${ROOT_DIR}/rust" && cargo build)

RUST_WGET="${WGET_RUST:-${ROOT_DIR}/rust/target/debug/wget}"
if [[ ! -x "${RUST_WGET}" ]]; then
  echo "ERROR: wget (Rust build) not found at ${RUST_WGET}" >&2
  exit 2
fi
echo "Using rust wget: ${RUST_WGET}"

echo "Starting local test server..."
SERVER_OUT="$(mktemp)"
HTTPS_OUT="$(mktemp)"
SERVER2_OUT="$(mktemp)"
FTP_OUT="$(mktemp)"
STATE_DIR="$(mktemp -d)"
python3 "${ROOT_DIR}/tests/server.py" --quiet --https --state-dir "${STATE_DIR}" >"${HTTPS_OUT}" 2>/dev/null &
HTTPS_PID="$!"
python3 "${ROOT_DIR}/tests/server.py" --quiet --state-dir "${STATE_DIR}" >"${SERVER2_OUT}" 2>/dev/null &
SERVER2_PID="$!"

sleep 0.2
HTTPS_URL="$(head -n 1 "${HTTPS_OUT}" | tr -d '\r\n')"
BASE2_URL="$(head -n 1 "${SERVER2_OUT}" | tr -d '\r\n')"
python3 "${ROOT_DIR}/tests/server.py" --quiet --state-dir "${STATE_DIR}" \
  --peer-base-url "${BASE2_URL}" --peer-https-base-url "${HTTPS_URL}" \
  >"${SERVER_OUT}" 2>/dev/null &
SERVER_PID="$!"
sleep 0.2
BASE_URL="$(head -n 1 "${SERVER_OUT}" | tr -d '\r\n')"
if [[ -z "${BASE_URL}" ]]; then
  echo "ERROR: failed to get base url from server" >&2
  exit 2
fi
if [[ -z "${HTTPS_URL}" ]]; then
  echo "ERROR: failed to get https base url from server" >&2
  exit 2
fi
if [[ -z "${BASE2_URL}" ]]; then
  echo "ERROR: failed to get base2 url from server" >&2
  exit 2
fi

TEST_CA_PEM="${ROOT_DIR}/tests/fixtures/tls/ca.pem"
TEST_CA_DIR="${ROOT_DIR}/tests/fixtures/tls/ca_dir"
if [[ ! -f "${TEST_CA_PEM}" ]]; then
  echo "ERROR: missing test CA at ${TEST_CA_PEM}" >&2
  exit 2
fi
# GNU wget 的 `--ca-directory` 依赖 OpenSSL「哈希」目录布局。仓库内 *.0 为 PEM 普通文件（非软链）以便 dpkg-source；
# 若已有哈希条目则跳过 rehash，否则 rehash 会把 *.0 改回指向 ca.pem 的软链并弄脏工作区。
shopt -s nullglob
_ca_hashes=( "${TEST_CA_DIR}"/*.0 )
shopt -u nullglob
if (( ${#_ca_hashes[@]} == 0 )); then
  openssl rehash "${TEST_CA_DIR}" 2>/dev/null || true
fi
unset _ca_hashes

echo "Base URL: ${BASE_URL}"
echo "HTTPS URL: ${HTTPS_URL}"
echo "Base2 URL: ${BASE2_URL}"

python3 "${ROOT_DIR}/tests/ftp_server.py" --port 0 --user u --password p >"${FTP_OUT}" 2>/dev/null &
FTP_PID="$!"
FTP_URL=""
for _ in 1 2 3 4 5 6 7 8 9 10; do
  sleep 0.1
  FTP_URL="$(head -n 1 "${FTP_OUT}" 2>/dev/null | tr -d '\r\n' || true)"
  if [[ -n "${FTP_URL}" ]]; then
    break
  fi
done
if [[ -z "${FTP_URL}" ]]; then
  echo "ERROR: failed to get ftp base url from server" >&2
  exit 2
fi
echo "FTP URL: ${FTP_URL}"

PROXY_FWD_OUT="$(mktemp)"
PROXY_EVIL_OUT="$(mktemp)"
PROXY_AUTH_OUT="$(mktemp)"
python3 "${ROOT_DIR}/tests/proxy.py" --mode forward --quiet >"${PROXY_FWD_OUT}" 2>/dev/null &
PROXY_FWD_PID="$!"
python3 "${ROOT_DIR}/tests/proxy.py" --mode evil --quiet >"${PROXY_EVIL_OUT}" 2>/dev/null &
PROXY_EVIL_PID="$!"
python3 "${ROOT_DIR}/tests/proxy.py" --mode forward --quiet \
  --proxy-auth-user wgettest --proxy-auth-password secret4test \
  >"${PROXY_AUTH_OUT}" 2>/dev/null &
PROXY_AUTH_PID="$!"

cleanup() {
  kill "${SERVER_PID}" >/dev/null 2>&1 || true
  kill "${HTTPS_PID}" >/dev/null 2>&1 || true
  kill "${SERVER2_PID}" >/dev/null 2>&1 || true
  kill "${FTP_PID}" >/dev/null 2>&1 || true
  kill "${PROXY_FWD_PID}" >/dev/null 2>&1 || true
  kill "${PROXY_EVIL_PID}" >/dev/null 2>&1 || true
  kill "${PROXY_AUTH_PID}" >/dev/null 2>&1 || true
  rm -f "${SERVER_OUT}" "${HTTPS_OUT}" "${SERVER2_OUT}" "${FTP_OUT}" || true
  rm -f "${PROXY_FWD_OUT}" "${PROXY_EVIL_OUT}" "${PROXY_AUTH_OUT}" || true
  rm -rf "${STATE_DIR}" || true
}
trap cleanup EXIT

sleep 0.2
PROXY_URL="$(head -n 1 "${PROXY_FWD_OUT}" | tr -d '\r\n')"
PROXY_EVIL_URL="$(head -n 1 "${PROXY_EVIL_OUT}" | tr -d '\r\n')"
PROXY_AUTH_URL="$(head -n 1 "${PROXY_AUTH_OUT}" | tr -d '\r\n')"
if [[ -z "${PROXY_URL}" || -z "${PROXY_EVIL_URL}" || -z "${PROXY_AUTH_URL}" ]]; then
  echo "ERROR: failed to start test proxies" >&2
  exit 2
fi
echo "Forward proxy: ${PROXY_URL}"
echo "Evil proxy: ${PROXY_EVIL_URL}"
echo "Auth proxy: ${PROXY_AUTH_URL}"

# 阶段 7.3.2：默认同套用例连跑 3 次；快速本地调试可 REGRESSION_PASSES=1
REGRESSION_PASSES="${REGRESSION_PASSES:-3}"
for ((pass = 1; pass <= REGRESSION_PASSES; pass++)); do
  echo "=== regression pass ${pass}/${REGRESSION_PASSES} ==="
  # 清空 flaky 等状态，避免多轮之间计数累积
  rm -f "${STATE_DIR}"/* 2>/dev/null || true
  python3 "${ROOT_DIR}/tests/compare.py" \
    --orig "${ORIG_WGET}" \
    --rust "${RUST_WGET}" \
    --base-url "${BASE_URL}" \
    --base2-url "${BASE2_URL}" \
    --https-base-url "${HTTPS_URL}" \
    --ftp-base-url "${FTP_URL}" \
    --ca-pem "${TEST_CA_PEM}" \
    --ca-dir "${TEST_CA_DIR}" \
    --proxy-url "${PROXY_URL}" \
    --proxy-evil-url "${PROXY_EVIL_URL}" \
    --proxy-auth-url "${PROXY_AUTH_URL}" || exit 1
done

