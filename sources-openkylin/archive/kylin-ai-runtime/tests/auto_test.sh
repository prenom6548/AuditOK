#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${project_root}/build"

cmake -S "${project_root}" -B "${build_dir}"
cmake --build "${build_dir}"
