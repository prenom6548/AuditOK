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

assert_contains() {
  local path="$1"
  local pattern="$2"
  if [[ -f "$PROJECT_ROOT/$path" ]] && grep -Fq "$pattern" "$PROJECT_ROOT/$path"; then
    pass "$path contains '$pattern'"
  else
    fail "$path does not contain '$pattern'"
  fi
}

assert_contains "build.sh" "Usage:"
assert_contains "build.sh" "native"
assert_contains "build.sh" "container"
assert_contains "build.sh" "amd64"
assert_contains "build.sh" "arm64"
assert_contains "build.sh" "kylinbot"
assert_contains "build.sh" "desktop"
assert_contains "build.sh" "all"
assert_contains "build.sh" "scripts/build-kylinbot-linux.sh"
assert_contains "build.sh" "scripts/build-kylinbot-desktop-appimage.sh"
assert_contains "build.sh" 'MODE="container"'
assert_contains "build.sh" 'ARCH="amd64"'
assert_contains "build.sh" 'TARGET=""'

if [[ "$("$PROJECT_ROOT/build.sh")" == *"Usage:"* ]]; then
  pass "build.sh without args prints usage"
else
  fail "build.sh without args does not print usage"
fi

if [[ "$("$PROJECT_ROOT/build.sh" -m container -a amd64)" == *"Usage:"* ]]; then
  pass "build.sh without target prints usage"
else
  fail "build.sh without target does not print usage"
fi

printf '\nPassed: %d\n' "$PASS"
printf 'Failed: %d\n' "$FAIL"

if (( FAIL > 0 )); then
  exit 1
fi
