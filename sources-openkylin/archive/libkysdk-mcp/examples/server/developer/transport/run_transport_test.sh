#!/bin/bash

# 循环执行 transport_server_client_example 的测试脚本
# 每3秒执行一次，每次并行启动5-10个进程

# 可执行文件路径（相对于脚本所在目录）
EXECUTABLE="./bin/transport_server_client_example"

# 检查可执行文件是否存在
if [ ! -f "$EXECUTABLE" ]; then
    echo "错误: 找不到可执行文件 $EXECUTABLE"
    echo "请确保在项目根目录下运行此脚本"
    exit 1
fi

# 确保日志目录存在
mkdir -p logs

# 计数器
ROUND=1

# 信号处理函数，用于优雅退出
cleanup() {
    echo ""
    echo "正在停止所有后台进程..."
    # 等待所有后台进程完成
    wait
    echo "脚本已停止"
    exit 0
}

# 捕获 Ctrl+C 信号
trap cleanup SIGINT SIGTERM

echo "开始循环测试 transport_server_client_example"
echo "每3秒执行一次，每次并行启动5-10个进程"
echo "按 Ctrl+C 停止"
echo "=========================================="

# 主循环
while true; do
    # 随机生成5-10之间的进程数
    NUM_PROCESSES=$((RANDOM % 6 + 5))
    
    echo ""
    echo "[第 $ROUND 轮] 启动 $NUM_PROCESSES 个并行进程..."
    
    # 存储进程ID
    PIDS=()
    
    # 启动并行进程
    for ((i=1; i<=NUM_PROCESSES; i++)); do
        # 在后台运行，使用tee同时输出到终端和日志文件，并添加进程标识
        (
            LOG_FILE="logs/test_round${ROUND}_process${i}.log"
            "$EXECUTABLE" 2>&1 | while IFS= read -r line; do
                echo "[Round${ROUND}-Process${i}] $line" | tee -a "$LOG_FILE"
            done
        ) &
        PID=$!
        PIDS+=($PID)
        echo "  进程 $i 已启动 (PID: $PID)"
    done
    
    echo "  已启动 $NUM_PROCESSES 个进程，等待3秒后开始下一轮..."
    
    # 等待3秒
    sleep 10
    
    # 可选：等待当前轮次的所有进程完成（如果不想等待，可以注释掉这部分）
    # echo "  等待当前轮次进程完成..."
    # for PID in "${PIDS[@]}"; do
    #     wait $PID 2>/dev/null
    # done
    
    ROUND=$((ROUND + 1))
done

