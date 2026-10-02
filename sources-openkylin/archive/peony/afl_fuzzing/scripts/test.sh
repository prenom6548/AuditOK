#!/bin/bash
# 运行Peony AFL模糊测试

set -e

# 获取参数（运行时间，单位：分钟）
DURATION=${1:-60}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AFL_DIR="$SCRIPT_DIR/.."

# 检查测试目标是否存在
if [ ! -f "$AFL_DIR/targets/peony_sync_test" ]; then
    echo "Error: Test target not found. Run build.sh first"
    exit 1
fi

# 检查AFL
if ! command -v afl-fuzz &> /dev/null; then
    echo "Error: AFL not found. Install AFL++: sudo apt-get install afl++"
    exit 1
fi

# 处理AFL core dump警告
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1

# 运行AFL模糊测试
echo "Starting AFL fuzzing for $DURATION minutes..."
echo "Input directory: $AFL_DIR/corpus"
echo "Output directory: $AFL_DIR/results"

timeout "${DURATION}m" afl-fuzz -i "$AFL_DIR/corpus" \
                                -o "$AFL_DIR/results" \
                                -x "$AFL_DIR/corpus" \
                                -t 1000 \
                                "$AFL_DIR/targets/peony_sync_test" @@ || true

echo "✅ Fuzzing complete. Results in $AFL_DIR/results"
