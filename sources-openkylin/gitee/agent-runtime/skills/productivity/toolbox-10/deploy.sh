#!/usr/bin/env bash
# toolbox-10 readiness check. This script does not install packages or start a daemon.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PYTHON_BIN="${PYTHON_BIN:-python3}"

command -v "$PYTHON_BIN" >/dev/null 2>&1 || {
  echo "Error: Python 3 is required."
  exit 1
}

if ! "$PYTHON_BIN" -c "import json5, numpy, qrcode, requests, scipy, sounddevice; from PIL import Image" 2>/dev/null; then
  echo "toolbox-10 dependencies are incomplete."
  echo "Create an isolated Python environment, then install the exact versions from:"
  echo "  $SCRIPT_DIR/requirements.txt"
  exit 1
fi

"$PYTHON_BIN" "$SCRIPT_DIR/toolbox-skill.py"
echo "toolbox-10 readiness check passed."
