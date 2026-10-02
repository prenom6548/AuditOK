#!/bin/bash
set -euo pipefail
# auto_test.sh
# 目的：安装依赖，在 build-coverage/ 影子目录用 qmake + make 编译「全工程 + 单测」，供白盒测试与 gcovr。
#
# 并行编译：默认 make -j$(nproc)，可通过 MAKE_JOBS 覆盖，例如 MAKE_JOBS=4 bash tests/auto_test.sh
#
# 覆盖率说明：
# - 使用 peony-extensions.pro 编译全部子工程；各子工程在 EXTENSIONS_SKIP_TEST 为空时通常带 --coverage。
# - 默认 EXTENSIONS_SKIP_PLUGIN_COVERAGE=1：插件子工程不插桩，gcovr 分母以核心服务与单测可执行代码为主，更易达到白盒行覆盖目标（如 70%）。
#   若需统计全仓库含插件源码：EXTENSIONS_SKIP_PLUGIN_COVERAGE=0 bash tests/auto_test.sh
# 单测二进制路径：
#   <工程根>/build-coverage/tests/extensions-test/extensions-test

INSTALL_DEPS="${INSTALL_DEPS:-1}"
if [ "${INSTALL_DEPS}" = "1" ]; then
    APT_BIN="apt"
    if [ "${EUID}" -ne 0 ] && command -v sudo >/dev/null 2>&1; then
        APT_BIN="sudo apt"
    fi

    ${APT_BIN} install -y \
        build-essential \
        debhelper-compat \
        libglib2.0-dev \
        libgsettings-qt-dev \
        libnotify-dev \
        libpeony-dev \
        libpoppler-qt5-dev \
        libudisks2-dev \
        pkg-config \
        qt5-qmake \
        qtbase5-dev \
        qtbase5-dev-tools \
        qtchooser \
        libpolkit-gobject-1-dev \
        libkysdk-qtwidgets-dev \
        libukui-common-dev \
        qttools5-dev-tools \
        libukui-service-interface-dev \
        libukui-common-dev \
        libkysdk-alm-dev \
        libkysdk-filesystem-dev \
        libkysdk-ai-common-dev \
        libgudev-1.0-dev \
        libukui-search-dev \
        libkysdk-systime-dev \
        libkyai-data-management-client-dev \
        libqt5remoteobjects5-dev \
        libukui-log4qt-dev \
        libukui-file-metadata-dev \
        libkysdk-disk-dev \
        libgtest-dev \
        lcov \
        gcovr
else
    echo "Skip dependency install (INSTALL_DEPS=${INSTALL_DEPS})."
fi

cd "$(dirname "$0")" || exit 1

TESTS_DIR=$(pwd)
PROJECT_ROOT=$(cd .. && pwd)
BUILD_DIR="${PROJECT_ROOT}/build-coverage"
TEST_BIN="${BUILD_DIR}/tests/extensions-test/extensions-test"

unset EXTENSIONS_SKIP_TEST
if [ -z "${ENABLE_LEGACY_EXTENSIONS_TEST+x}" ]; then
    if [ -f "${TESTS_DIR}/extensions-test/extensions-test.pro" ]; then
        ENABLE_LEGACY_EXTENSIONS_TEST=1
    else
        ENABLE_LEGACY_EXTENSIONS_TEST=0
    fi
fi
export ENABLE_LEGACY_EXTENSIONS_TEST

# 默认可达成分母与单测执行范围更匹配；全仓插桩请显式设为 0。
export EXTENSIONS_SKIP_PLUGIN_COVERAGE="${EXTENSIONS_SKIP_PLUGIN_COVERAGE:-1}"

echo "Configuring and building with coverage (qmake, shadow build in build-coverage/)..."
echo "  PROJECT_ROOT=${PROJECT_ROOT}"
echo "  BUILD_DIR=${BUILD_DIR}"
echo "  ENABLE_LEGACY_EXTENSIONS_TEST=${ENABLE_LEGACY_EXTENSIONS_TEST}"
echo "  EXTENSIONS_SKIP_PLUGIN_COVERAGE=${EXTENSIONS_SKIP_PLUGIN_COVERAGE}"

CLEAN_BUILD="${CLEAN_BUILD:-1}"
if [ "${CLEAN_BUILD}" = "1" ] && [ -d "${BUILD_DIR}" ]; then
    echo "Removing existing build-coverage directory..."
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}" || exit 1

MAKE_JOBS="${MAKE_JOBS:-$(nproc)}"
if ! [[ "${MAKE_JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "警告：MAKE_JOBS=${MAKE_JOBS} 非法，回退为 1。" >&2
    MAKE_JOBS=1
fi

cd "${BUILD_DIR}" || exit 1
qmake "${PROJECT_ROOT}/peony-extensions.pro" CONFIG+=debug
make -j"${MAKE_JOBS}"

BUILD_DISCOVERED_TESTS="${BUILD_DISCOVERED_TESTS:-1}"
if [ "${BUILD_DISCOVERED_TESTS}" = "1" ]; then
    echo "Building discovered unit-test projects under tests/ ..."
    mapfile -t DISCOVERED_TEST_PROS < <(
        python3 - "${PROJECT_ROOT}" <<'PY'
from pathlib import Path
import sys
project_root = Path(sys.argv[1]).resolve()
root = project_root / "tests"
pros = sorted([p for p in root.rglob("*.pro") if p.name.endswith("-test.pro") or p.name.startswith("test_")])
for p in pros:
    print(p.relative_to(project_root).as_posix())
PY
    )

    built_count=0
    failed_count=0
    failed_projects=()
    for rel_pro in "${DISCOVERED_TEST_PROS[@]}"; do
        [ -n "${rel_pro}" ] || continue
        abs_pro="${PROJECT_ROOT}/${rel_pro}"
        rel_dir="$(dirname "${rel_pro#tests/}")"
        out_dir="${BUILD_DIR}/tests/${rel_dir}"
        mkdir -p "${out_dir}"
        echo "  -> ${rel_pro}"
        if (
            cd "${out_dir}" || exit 1
            qmake "${abs_pro}" CONFIG+=debug && make -j"${MAKE_JOBS}"
        ); then
            built_count=$((built_count + 1))
        else
            failed_count=$((failed_count + 1))
            failed_projects+=("${rel_pro}")
            echo "  !! build failed: ${rel_pro}"
        fi
    done
    echo "Discovered test projects built: ${built_count}, failed: ${failed_count}"
    if [ "${failed_count}" -gt 0 ]; then
        echo "Failed test projects:"
        for p in "${failed_projects[@]}"; do
            echo "  - ${p}"
        done
    fi
fi

echo "Build successful!"
if [ -x "${TEST_BIN}" ]; then
    echo "  Test binary: ${TEST_BIN}"
else
    echo "  Warning: expected test binary not found: ${TEST_BIN}"
fi
echo "  Run: bash ${TESTS_DIR}/runUnitTest.sh"
echo "  全量单测: UNITTEST_MAX_WALL_SEC=0 bash ${TESTS_DIR}/runUnitTest.sh"
echo "  本地 gcovr 摘要/HTML：EXTENSIONS_SKIP_GCOVR=0 bash ${TESTS_DIR}/runUnitTest.sh  # HTML 再加 EXTENSIONS_GCOVR_HTML=1"
echo "  插件 so-smoke：EXTENSIONS_SKIP_SO_SMOKE=0 bash ${TESTS_DIR}/runUnitTest.sh"
echo "  HTML 报告目录（需 EXTENSIONS_SKIP_GCOVR=0 且 EXTENSIONS_GCOVR_HTML=1）: ${BUILD_DIR}/coverage-report/"
exit 0
