#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# 白盒门禁：安装依赖并编译单元测试相关产物（见 docs/白盒单元测试指导手册.docx）。
# 成功 exit 0，失败 exit -1。勿在 apt 前加 sudo；apt install 须带 -y。

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT" || exit -1

echo "[auto_test] ROOT=$ROOT"

# 与 debian/control Build-Depends 对齐；补充 cmake/cargo 链常用包。
apt-get update -qq || {
  echo "Error: apt-get update failed." >&2
  exit -1
}

apt-get install -y -qq \
  build-essential \
  ca-certificates \
  cargo \
  cmake \
  debhelper \
  dpkg-dev \
  doxygen \
  extra-cmake-modules \
  gettext \
  libc-dev \
  pkg-config \
  python3 \
  qtbase5-dev \
  qtbase5-doc \
  qtbase5-private-dev \
  qtdeclarative5-dev \
  qttools5-dev \
  rustc \
  || {
    echo "Error: apt-get install failed." >&2
    exit -1
  }

echo "[auto_test] cargo build --release (Rust 静态库，供 CMake 链接)"
if ! cargo build --release --manifest-path "$ROOT/Cargo.toml"; then
  echo "Error: cargo build --release failed." >&2
  exit -1
fi

echo "[auto_test] cargo test --no-run (编译全部集成测试二进制)"
if ! cargo test --no-run --manifest-path "$ROOT/Cargo.toml"; then
  echo "Error: cargo test --no-run failed." >&2
  exit -1
fi

if [[ -d "$ROOT/build" ]]; then
  echo "[auto_test] Removing existing build directory..."
  rm -rf "$ROOT/build"
fi

mkdir -p "$ROOT/build"
cd "$ROOT/build" || exit -1

echo "[auto_test] cmake .."
if ! cmake ..; then
  echo "Error: cmake failed." >&2
  exit -1
fi

echo "[auto_test] cmake --build . -j$(nproc 2>/dev/null || echo 4)"
if ! cmake --build . -j"$(nproc 2>/dev/null || echo 4)"; then
  echo "Error: cmake --build failed." >&2
  exit -1
fi

echo "[auto_test] Build successful."
exit 0
