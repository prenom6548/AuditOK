#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

PASS=0
FAIL=0

pass() {
  printf 'PASS: %s\n' "$1"
  PASS=$((PASS + 1))
}

fail() {
  printf 'FAIL: %s\n' "$1"
  FAIL=$((FAIL + 1))
}

assert_exists() {
  local path="$1"
  if [[ -e "$PROJECT_ROOT/$path" ]]; then
    pass "$path exists"
  else
    fail "$path is missing"
  fi
}

assert_contains() {
  local path="$1"
  local pattern="$2"
  if [[ -f "$PROJECT_ROOT/$path" ]] && grep -Fq "$pattern" "$PROJECT_ROOT/$path"; then
    pass "$path contains '$pattern'"
  else
    fail "$path does not contain '$pattern'"
  fi
}

assert_exists "scripts/build-kylinbot-linux.sh"
assert_exists "scripts/build-kylinbot-desktop-appimage.sh"
assert_exists "Dockerfile.build.kylinbot"
assert_exists "Dockerfile.build.desktop"

assert_contains "scripts/build-kylinbot-linux.sh" "docker buildx build"
assert_contains "scripts/build-kylinbot-linux.sh" "dist/kylinbot-linux-"
assert_contains "scripts/build-kylinbot-linux.sh" "Dockerfile.build.kylinbot"

assert_contains "scripts/build-kylinbot-desktop-appimage.sh" "docker buildx build"
assert_contains "scripts/build-kylinbot-desktop-appimage.sh" "dist/kylinbot-desktop-linux-"
assert_contains "scripts/build-kylinbot-desktop-appimage.sh" "Dockerfile.build.desktop"

assert_contains "Dockerfile.build.kylinbot" "FROM rust:"
assert_contains "Dockerfile.build.kylinbot" "cargo build --release --locked --bin kylinbot"

assert_contains "Dockerfile.build.desktop" "cargo tauri build"
assert_contains "Dockerfile.build.desktop" "AppImage"

printf '\nPassed: %d\n' "$PASS"
printf 'Failed: %d\n' "$FAIL"

if (( FAIL > 0 )); then
  exit 1
fi
