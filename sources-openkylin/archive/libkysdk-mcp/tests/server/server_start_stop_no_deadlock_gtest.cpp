/**
 * @file server_start_stop_no_deadlock_gtest.cpp
 * @brief 回归测试 —— MCPServer 在 IPC server 模式下连续 start/stop 不应死锁。
 *
 * 历史问题:
 *   在 IPC server 模式下，PipeCommunication 的 read 线程会阻塞在 read(STDIN_FILENO)，
 *   stop() 时仅置 running_ 标志、随即 join 读线程，导致主线程在 pthread_join 上死锁。
 *   首次运行常因 stdin 时序"碰巧 EOF"而看似正常，但重复运行就会复现。
 *
 * 验证策略:
 *   - 在子进程中执行多次 start → isRunning → stop 循环；
 *   - 父进程用 waitpid + 软超时观察；
 *   - 若超时仍未退出，强制杀掉子进程并 FAIL，保留可定位的栈位置（PID 输出在错误消息里）。
 *   - 用子进程是因为：
 *       1. 即使本测试触发回归，gtest 主进程也不会被一起锁住；
 *       2. 子进程的 stdin 污染不会影响其他测试。
 */

#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>

#include "common/transport.h"
#include "server.h"

using namespace mcp;
using namespace mcp_transport;

namespace {

// 在子进程中执行 N 次 IPC server start/stop 循环；
// 任何一步失败立即退出对应错误码。仅由子进程调用。
[[noreturn]] void runIpcServerStartStopChild(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        MCPServer server;
        server.setServerInfo("deadlock-regression-server", "1.0.0");

        ServerCapabilities caps;
        caps.tools = {{"listChanged", true}};
        server.setCapabilities(caps);

        auto config = mcp_transport::utils::createIpcServerConfig(
            "deadlock-regression-server", "1.0.0");
        config.enable_logging = false;  // 失败诊断时改为 true 即可看完整 IPC 日志

        if (!server.start(config)) {
            std::_Exit(10 + i);  // start 失败
        }
        if (!server.isRunning()) {
            std::_Exit(40 + i);  // start 后状态不对
        }
        try {
            server.stop();  // 历史死锁点
        } catch (...) {
            std::_Exit(70 + i);  // stop 抛异常
        }
    }
    std::_Exit(0);
}

// 等待子进程退出，超时返回 false。
// 用循环 + WNOHANG 轮询而不是 alarm()/SIGALRM，避免干扰子进程信号语义。
bool waitForChildWithTimeout(pid_t pid, std::chrono::milliseconds timeout,
                              int& out_status) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        pid_t r = ::waitpid(pid, &out_status, WNOHANG);
        if (r == pid) {
            return true;
        }
        if (r < 0 && errno != EINTR) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return false;
}

// 用 fork 隔离执行一次"N 轮 start/stop"回归。
// 死锁时父进程 SIGKILL 子进程并 FAIL，附上 PID 与轮数方便事后分析。
//
// 关键：必须保证子进程的 STDIN 是"打开但永远阻塞"的状态，否则
//      fork 出来的子进程默认继承的 stdin 在 gtest 进程里通常已经是 EOF/
//      非交互式状态，read(0) 会立刻返回 0，掩盖死锁。
//      所以这里构造一对管道，父进程持有写端但什么都不写，把读端
//      dup2 到子进程的 STDIN_FILENO —— 这样 server 模式的 read 线程
//      就会真实阻塞在 read(STDIN_FILENO)，与生产环境一致。
void runIpcServerStartStopRegression(int iterations,
                                      std::chrono::milliseconds timeout) {
    int stdin_pipe[2] = {-1, -1};
    ASSERT_EQ(::pipe(stdin_pipe), 0)
        << "pipe() failed: " << std::strerror(errno);

    pid_t pid = ::fork();
    if (pid < 0) {
        ::close(stdin_pipe[0]);
        ::close(stdin_pipe[1]);
        FAIL() << "fork() failed: " << std::strerror(errno);
        return;
    }

    if (pid == 0) {
        // 子进程：用管道读端替换 STDIN，让 read(0) 真实阻塞
        ::close(stdin_pipe[1]);
        if (::dup2(stdin_pipe[0], STDIN_FILENO) < 0) {
            std::_Exit(99);
        }
        ::close(stdin_pipe[0]);
        runIpcServerStartStopChild(iterations);
        // 不应到达
    }

    // 父进程：保留写端不写不关，让子进程的 stdin 永远 readable=false
    ::close(stdin_pipe[0]);

    int status = 0;
    const bool finished = waitForChildWithTimeout(pid, timeout, status);
    if (!finished) {
        // 死锁回归：保留进程方便 gdb -p，输出后再 kill。
        ::kill(pid, SIGKILL);
        int dummy = 0;
        ::waitpid(pid, &dummy, 0);
        ::close(stdin_pipe[1]);
        FAIL() << "IPC server start/stop deadlock regression: child pid=" << pid
               << " did not finish within " << timeout.count() << "ms over "
               << iterations << " iterations";
        return;
    }
    ::close(stdin_pipe[1]);

    ASSERT_TRUE(WIFEXITED(status))
        << "child terminated abnormally (signal=" << WTERMSIG(status) << ")";
    ASSERT_EQ(WEXITSTATUS(status), 0)
        << "child returned non-zero exit code " << WEXITSTATUS(status)
        << " (see runIpcServerStartStopChild for code → step mapping)";
}

}  // namespace

// 单轮：等同于历史 bug 复现用例 examples/bug/start_01.cpp。
TEST(IpcServerLifecycleRegression, SingleStartStopDoesNotDeadlock) {
    runIpcServerStartStopRegression(/*iterations=*/1,
                                     std::chrono::seconds(5));
}

// 关键回归：bug 在"连续 start/stop"时复现，5 次足以稳定触发原死锁。
TEST(IpcServerLifecycleRegression, RepeatedStartStopDoesNotDeadlock) {
    runIpcServerStartStopRegression(/*iterations=*/5,
                                     std::chrono::seconds(15));
}
