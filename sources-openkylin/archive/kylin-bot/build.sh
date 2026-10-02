#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

MODE="native"
ARCH="amd64"
TARGET=""

usage() {
  cat <<'EOF'
Usage: ./build.sh [-m native|container] [-a amd64|arm64] [-t web|bot|app|all]

Options:
  -m MODE      Build mode. native uses host toolchain, container uses Docker buildx.
  -a ARCH      Target architecture: amd64 or arm64.
  -t TARGET    Build target: web, bot, app, or all.
  -h           Show this help.

Examples:
  ./build.sh
  ./build.sh -m container -a amd64 -t bot
  ./build.sh -m container -a arm64 -t app
  ./build.sh -m native -t all
  ./build.sh -m native -t web

Container helpers:
  scripts/build-kylinbot-linux.sh
  scripts/build-kylinbot-desktop-appimage.sh
EOF
}

require_cmd() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Missing required command: $cmd" >&2
    exit 1
  fi
}

arch_to_rust_target() {
  case "$1" in
    amd64) echo "x86_64-unknown-linux-gnu" ;;
    arm64) echo "aarch64-unknown-linux-gnu" ;;
    *)
      echo "Unsupported architecture: $1" >&2
      exit 1
      ;;
  esac
}

prepare_web_native() {
  require_cmd npm

  if [[ -n "${NVM_DIR:-}" && -s "${NVM_DIR}/nvm.sh" ]]; then
    # shellcheck disable=SC1090
    . "${NVM_DIR}/nvm.sh"
  elif [[ -s "$HOME/.nvm/nvm.sh" ]]; then
    # shellcheck disable=SC1091
    . "$HOME/.nvm/nvm.sh"
  fi

  if command -v nvm >/dev/null 2>&1; then
    nvm use default >/dev/null
  fi

  (
    cd "$REPO_ROOT/web"
    npm ci --ignore-scripts
    npm run build
  )
}

build_native_kylinbot() {
  local rust_target="$1"
  require_cmd cargo
  prepare_web_native
  rustup target add "$rust_target" >/dev/null 2>&1 || true
  (
    cd "$REPO_ROOT"
    cargo build --release --target "$rust_target" --bin kylinbot
  )
}

build_native_desktop() {
  local rust_target="$1"
  require_cmd cargo
  prepare_web_native
  rustup target add "$rust_target" >/dev/null 2>&1 || true

  if ! command -v cargo-tauri >/dev/null 2>&1; then
    echo "Installing cargo-tauri CLI..."
    cargo install tauri-cli --version "^2"
  fi

  (
    cd "$REPO_ROOT/apps/tauri"
    cargo tauri build --target "$rust_target" --bundles appimage
  )
}

build_container_kylinbot() {
  "$REPO_ROOT/scripts/build-kylinbot-linux.sh" "$ARCH"
}

build_container_desktop() {
  "$REPO_ROOT/scripts/build-kylinbot-desktop-appimage.sh" "$ARCH"
}

while getopts ":m:a:t:h" opt; do
  case "$opt" in
    m) MODE="$OPTARG" ;;
    a) ARCH="$OPTARG" ;;
    t) TARGET="$OPTARG" ;;
    h)
      usage
      exit 0
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      usage >&2
      exit 1
      ;;
    \?)
      echo "Unknown option: -$OPTARG" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ $# -eq 0 || -z "$TARGET" ]]; then
  usage
  exit 0
fi

case "$MODE" in
  native|container) ;;
  *)
    echo "Unsupported mode: $MODE" >&2
    usage >&2
    exit 1
    ;;
esac

case "$ARCH" in
  amd64|arm64) ;;
  *)
    echo "Unsupported architecture: $ARCH" >&2
    usage >&2
    exit 1
    ;;
esac

case "$TARGET" in
  web|bot|app|all) ;;
  *)
    echo "Unsupported target: $TARGET" >&2
    usage >&2
    exit 1
    ;;
esac

RUST_TARGET="$(arch_to_rust_target "$ARCH")"

if [[ "$MODE" == "container" ]]; then
  require_cmd docker
else
  require_cmd rustup
fi

case "$TARGET" in
  web)
    prepare_web_native
    ;;
  bot)
    if [[ "$MODE" == "container" ]]; then
      build_container_kylinbot
    else
      build_native_kylinbot "$RUST_TARGET"
    fi
    ;;
  app)
    if [[ "$MODE" == "container" ]]; then
      build_container_desktop
    else
      build_native_desktop "$RUST_TARGET"
    fi
    ;;
  all)
    if [[ "$MODE" == "container" ]]; then
      prepare_web_native
      build_container_kylinbot
      build_container_desktop
    else
      prepare_web_native
      build_native_kylinbot "$RUST_TARGET"
      build_native_desktop "$RUST_TARGET"
    fi
    ;;
esac
