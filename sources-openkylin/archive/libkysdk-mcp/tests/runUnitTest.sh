#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TEST_DIR="$PROJECT_ROOT/build"

if [ ! -d "${TEST_DIR}" ]; then
    echo "Error: build directory not found. Please run auto_test.sh first."
    exit 1
fi

cd "${TEST_DIR}" || exit 1

# Check if ctest is available
if command -v ctest &> /dev/null; then
    echo "Running all tests via ctest..."
    echo "========================================"
    ctest --parallel 4 --output-on-failure
    RESULT=$?
else
    echo "ctest not found, running test binaries directly..."
    echo "========================================"

    # List of test binaries
    TEST_BINARIES=(
        "client_gtest"
        "protocol_gtest"
        "transport_test"
        "logger_test"
        "pipe_communication_test"
        "netio_test"
        "server_gtest"
        "server_logger_gtest"
        "server_netio_gtest"
        "pipe_communication_gtest"
        "server_transport_gtest"
        "session_manager_gtest"
    )

    TOTAL_PASSED=0
    TOTAL_FAILED=0

    for binary in "${TEST_BINARIES[@]}"; do
        if [ -x "./${binary}" ]; then
            echo ""
            echo "Running ${binary}..."
            echo "----------------------------------------"
            if ./${binary}; then
                ((TOTAL_PASSED++))
                echo "${binary}: PASSED"
            else
                ((TOTAL_FAILED++))
                echo "${binary}: FAILED"
            fi
        else
            echo "Warning: ${binary} not found or not executable"
        fi
    done

    echo ""
    echo "========================================"
    echo "Test Summary"
    echo "========================================"
    echo "Passed: ${TOTAL_PASSED}"
    echo "Failed: ${TOTAL_FAILED}"

    if [ ${TOTAL_FAILED} -eq 0 ]; then
        RESULT=0
    else
        RESULT=1
    fi
fi

echo ""
echo "Unit test execution completed."
exit ${RESULT}
