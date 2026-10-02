#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

PACKAGE_NAME="$(awk '/^Source:/ {print $2; exit}' "${SOURCE_DIR}/debian/control" 2>/dev/null)"
if [ -z "${PACKAGE_NAME}" ]; then
    PACKAGE_NAME="$(basename "${SOURCE_DIR}")"
fi

COVERAGE_XML="${SOURCE_DIR}/${PACKAGE_NAME}.xml"
COVERAGE_OUTPUT="${SOURCE_DIR}/${PACKAGE_NAME}.output"

cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DCMAKE_C_FLAGS_DEBUG="-O0 -g --coverage" \
    -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g --coverage" \
    -DCMAKE_EXE_LINKER_FLAGS_DEBUG="--coverage" \
    -DCMAKE_SHARED_LINKER_FLAGS_DEBUG="--coverage"

cmake --build "${BUILD_DIR}" -j"$(nproc)"

find "${BUILD_DIR}" -name '*.gcda' -type f -delete

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"
export QT_FATAL_WARNINGS="${QT_FATAL_WARNINGS:-0}"
export QT_LOGGING_RULES="${QT_LOGGING_RULES:-*.debug=false;qt.qpa.*=false}"
export CTEST_OUTPUT_ON_FAILURE=1

ctest --test-dir "${BUILD_DIR}" --output-on-failure

if ! command -v gcovr >/dev/null 2>&1; then
    echo "gcovr not found, cannot generate coverage xml."
    exit 1
fi

gcovr -r "${SOURCE_DIR}" \
    -x \
    --object-directory "${BUILD_DIR}" \
    -o "${COVERAGE_OUTPUT}" \
    --sonarqube "${COVERAGE_XML}" \
    --exclude-branches-by-pattern=".*"

echo "Coverage xml generated: ${COVERAGE_XML}"
exit 0
