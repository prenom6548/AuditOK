#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

ARCH="${1:-amd64}"

case "$ARCH" in
  amd64)
    DOCKER_PLATFORM="linux/amd64"
    RUST_TARGET="x86_64-unknown-linux-gnu"
    ;;
  arm64)
    DOCKER_PLATFORM="linux/arm64"
    RUST_TARGET="aarch64-unknown-linux-gnu"
    ;;
  *)
    echo "Unsupported architecture: $ARCH" >&2
    echo "Usage: $0 [amd64|arm64]" >&2
    exit 1
    ;;
esac

if ! docker buildx version >/dev/null 2>&1; then
  echo "docker buildx is required" >&2
  exit 1
fi

OUTPUT_DIR="$REPO_ROOT/dist/kylinbot-linux-$ARCH"
mkdir -p "$OUTPUT_DIR"

docker buildx build \
  --platform "$DOCKER_PLATFORM" \
  --build-arg "RUST_TARGET=$RUST_TARGET" \
  --build-arg "KYLINBOT_CARGO_FEATURES=${KYLINBOT_CARGO_FEATURES:-}" \
  --file "$REPO_ROOT/Dockerfile.build.kylinbot" \
  --target artifact \
  --output "type=local,dest=$OUTPUT_DIR" \
  "$REPO_ROOT"

echo "Linux kylinbot artifact exported to: $OUTPUT_DIR/kylinbot"
