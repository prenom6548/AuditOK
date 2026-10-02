#!/usr/bin/env bash
# Run FTP-related compare.py cases (orig vs rust).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

ORIG="${WGET_ORIG:-/usr/bin/wget}"
RUST="${WGET_RUST:-$ROOT/rust/target/release/wget}"

if [[ ! -x "$ORIG" ]]; then
  echo "ERROR: GNU wget not found: $ORIG" >&2
  exit 2
fi
if [[ ! -x "$RUST" ]]; then
  echo "Building Rust wget ..."
  (cd "$ROOT/rust" && cargo build --release)
  RUST="$ROOT/rust/target/release/wget"
fi

HTTP_PORT="$(python3 -c "import socket; s=socket.socket(); s.bind(('127.0.0.1',0)); print(s.getsockname()[1]); s.close()")"
FTP_PORT="$(python3 -c "import socket; s=socket.socket(); s.bind(('127.0.0.1',0)); print(s.getsockname()[1]); s.close()")"

python3 tests/server.py --port "$HTTP_PORT" --quiet >/dev/null 2>&1 &
HTTP_PID=$!
python3 tests/ftp_server.py --port "$FTP_PORT" >/dev/null 2>&1 &
FTP_PID=$!
trap 'kill "$HTTP_PID" "$FTP_PID" 2>/dev/null || true' EXIT
sleep 0.5

BASE="http://127.0.0.1:${HTTP_PORT}"
# 勿加尾部 '/'：用例里已写 {FTP}/path，避免出现 ftp://host:port// 双斜杠。
FTP="ftp://127.0.0.1:${FTP_PORT}"

echo "HTTP base: $BASE"
echo "FTP base:  $FTP"
echo "Rust bin:  $RUST"
echo ""

python3 tests/compare.py \
  --orig "$ORIG" \
  --rust "$RUST" \
  --base-url "$BASE" \
  --ftp-base-url "$FTP" \
  --case-regex 'ftp|TC185|TC186'
