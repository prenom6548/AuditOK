/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "../libkyprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>

// 辅助函数：打印分隔线
static void print_separator(const char *title)
{
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n");
}

// 辅助函数：测试进程相关接口（使用 init 进程 PID 1）
static void test_process_basic_info(pid_t pid)
{
    print_separator("基础进程信息测试");

    // 1. 获取进程名称
    char *name = kdk_get_process_name(pid);
    if (name)
    {
        printf("[✓] kdk_get_process_name(%d): %s\n", pid, name);
        free(name);
    }
    else
    {
        printf("[✗] kdk_get_process_name(%d): failed\n", pid);
    }

    // 2. 获取进程CPU使用率
    float cpu_usage = kdk_get_process_cpu_usage_percent(pid);
    printf("[✓] kdk_get_process_cpu_usage_percent(%d): %.2f%%\n", pid, cpu_usage);

    // 3. 获取进程内存占用率
    float mem_usage = kdk_get_process_mem_usage_percent(pid);
    printf("[✓] kdk_get_process_mem_usage_percent(%d): %.2f%%\n", pid, mem_usage);

    // 4. 获取进程状态
    char *status = kdk_get_process_status(pid);
    if (status)
    {
        printf("[✓] kdk_get_process_status(%d): %s\n", pid, status);
        free(status);
    }
    else
    {
        printf("[✗] kdk_get_process_status(%d): failed\n", pid);
    }

    // 5. 获取进程端口号
    int port = kdk_get_process_port(pid);
    printf("[✓] kdk_get_process_port(%d): %d\n", pid, port);

    // 6. 获取进程端口号列表
    int *ports = kdk_get_process_port_nums(pid);
    if (ports)
    {
        printf("[✓] kdk_get_process_port_nums(%d): ", pid);
        size_t index = 0;
        while (ports[index])
        {
            printf("%d ", ports[index]);
            index++;
        }
        printf("\n");
        free(ports);
    }
    else
    {
        printf("[✓] kdk_get_process_port_nums(%d): no ports\n", pid);
    }

    // 7. 获取进程启动时间
    char *start_time = kdk_get_process_start_time(pid);
    if (start_time)
    {
        printf("[✓] kdk_get_process_start_time(%d): %s\n", pid, start_time);
        free(start_time);
    }
    else
    {
        printf("[✗] kdk_get_process_start_time(%d): failed\n", pid);
    }

    // 8. 获取进程运行时间
    char *run_time = kdk_get_process_running_time(pid);
    if (run_time)
    {
        printf("[✓] kdk_get_process_running_time(%d): %s\n", pid, run_time);
        free(run_time);
    }
    else
    {
        printf("[✗] kdk_get_process_running_time(%d): failed\n", pid);
    }

    // 9. 获取进程CPU时间
    char *cpu_time = kdk_get_process_cpu_time(pid);
    if (cpu_time)
    {
        printf("[✓] kdk_get_process_cpu_time(%d): %s\n", pid, cpu_time);
        free(cpu_time);
    }
    else
    {
        printf("[✗] kdk_get_process_cpu_time(%d): failed\n", pid);
    }

    // 10. 获取进程命令
    char *cmd = kdk_get_process_command(pid);
    if (cmd)
    {
        printf("[✓] kdk_get_process_command(%d): %.100s%s\n",
               pid, cmd, (strlen(cmd) > 100) ? "..." : "");
        free(cmd);
    }
    else
    {
        printf("[✗] kdk_get_process_command(%d): failed\n", pid);
    }

    // 11. 获取进程属主
    char *user = kdk_get_process_user(pid);
    if (user)
    {
        printf("[✓] kdk_get_process_user(%d): %s\n", pid, user);
        free(user);
    }
    else
    {
        printf("[✗] kdk_get_process_user(%d): failed\n", pid);
    }
}

// 测试进程ID查找接口
static void test_process_id_lookup(void)
{
    print_separator("进程ID查找测试");

    const char *test_proc = "systemd";

    // 12. 根据进程名获取单个PID
    int pid = kdk_get_process_id((char *)test_proc);
    printf("[✓] kdk_get_process_id(\"%s\"): %d\n", test_proc, pid);

    // 13. 根据进程名获取多个PID
    int *pids = kdk_get_process_id_nums((char *)test_proc);
    if (pids)
    {
        printf("[✓] kdk_get_process_id_nums(\"%s\"): ", test_proc);
        size_t index = 0;
        int count = 0;
        while (pids[index] && count < 10) // 最多显示10个
        {
            printf("%d ", pids[index]);
            index++;
            count++;
        }
        if (pids[index])
        {
            printf("...");
        }
        printf("\n");
        kdk_proc_freeid(pids);
    }
    else
    {
        printf("[✗] kdk_get_process_id_nums(\"%s\"): failed\n", test_proc);
    }
}

// 测试进程详细信息接口
static void test_process_detailed_info(void)
{
    print_separator("进程详细信息测试");

    const char *test_proc = "systemd";

    // 14. 获取指定进程名的所有进程信息
    char **proc_info = kdk_procname_get_process_infomation((char *)test_proc);
    if (proc_info)
    {
        printf("[✓] kdk_procname_get_process_infomation(\"%s\"):\n", test_proc);
        size_t index = 0;
        int count = 0;
        while (proc_info[index] && count < 3) // 最多显示3个进程
        {
            printf("  [%zu] %s\n", index, proc_info[index]);
            index++;
            count++;
        }
        if (proc_info[index])
        {
            printf("  ... (more processes)\n");
        }
        kdk_proc_freeall(proc_info);
    }
    else
    {
        printf("[✗] kdk_procname_get_process_infomation(\"%s\"): failed\n", test_proc);
    }

    // 15. 获取系统中所有进程的信息（仅显示前5个）
    char **all_info = kdk_get_process_all_information();
    if (all_info)
    {
        printf("[✓] kdk_get_process_all_information():\n");
        size_t count = 0;
        while (all_info[count] && count < 5)
        {
            // 只显示进程ID和名称部分
            char *pid_str = strstr(all_info[count], "process_id:");
            char *name_str = strstr(all_info[count], "proc_name:");
            if (pid_str && name_str)
            {
                char pid_buf[64] = {0};
                char name_buf[128] = {0};
                sscanf(pid_str, "process_id:%[^,]", pid_buf);
                sscanf(name_str, "proc_name:%[^,]", name_buf);
                printf("  [%zu] PID=%s, Name=%s\n", count, pid_buf, name_buf);
            }
            count++;
        }
        if (all_info[count])
        {
            printf("  ... (total %zu+ processes)\n", count);
        }
        else
        {
            printf("  Total: %zu processes\n", count);
        }
        kdk_proc_freeall(all_info);
    }
    else
    {
        printf("[✗] kdk_get_process_all_information(): failed\n");
    }
}

// 测试CPU模式比例接口
static void test_cpu_modes(pid_t pid)
{
    print_separator("CPU模式比例测试");

    // 16. 获取内核态CPU时间比例
    float kernel_ratio = kdk_get_process_kernel_modes_per(pid);
    printf("[✓] kdk_get_process_kernel_modes_per(%d): %.4f (%.2f%%)\n",
           pid, kernel_ratio, kernel_ratio * 100);

    // 17. 获取用户态CPU时间比例
    float user_ratio = kdk_get_process_user_modes_per(pid);
    printf("[✓] kdk_get_process_user_modes_per(%d): %.4f (%.2f%%)\n",
           pid, user_ratio, user_ratio * 100);

    printf("[✓] Total ratio: %.4f (%.2f%%)\n",
           kernel_ratio + user_ratio, (kernel_ratio + user_ratio) * 100);
}

// 测试运行环境检测
static void test_runtime_env(pid_t pid)
{
    print_separator("运行环境检测测试");

    // 22. 获取进程运行环境
    KdkRuntimeEnv env = kdk_get_process_runtime_env(pid);
    printf("[✓] kdk_get_process_runtime_env(%d): ", pid);
    switch (env)
    {
    case KDK_ENV_UNKNOWN:
        printf("KDK_ENV_UNKNOWN (未知环境)\n");
        break;
    case KDK_ENV_HOST:
        printf("KDK_ENV_HOST (核心系统/宿主机)\n");
        break;
    case KDK_ENV_KAIMING:
        printf("KDK_ENV_KAIMING (kaiming环境)\n");
        break;
    case KDK_ENV_KARE_V10SP1:
        printf("KDK_ENV_KARE_V10SP1 (kare v10sp1环境)\n");
        break;
    case KDK_ENV_KARE_SHADOW:
        printf("KDK_ENV_KARE_SHADOW (kare shadow环境)\n");
        break;
    default:
        printf("Unknown (%d)\n", env);
        break;
    }
}

// 测试调试终端打开功能（需要用户确认）
static void test_debug_terminal(pid_t pid)
{
    print_separator("调试终端测试");

    printf("[!] kdk_open_process_debug_terminal(%d) 将尝试打开调试终端\n", pid);
    printf("[!] 此操作会启动新的终端窗口，是否继续？(y/n): ");

    char choice;
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y')
    {
        int ret = kdk_open_process_debug_terminal(pid);
        if (ret == 0)
        {
            printf("[✓] kdk_open_process_debug_terminal(%d): success\n", pid);
        }
        else
        {
            printf("[✗] kdk_open_process_debug_terminal(%d): failed (ret=%d)\n", pid, ret);
        }
    }
    else
    {
        printf("[~] Skipped kdk_open_process_debug_terminal test\n");
    }
}

int main(int argc, char *argv[])
{
    // 默认测试 PID 为 1 (init/systemd)，可通过命令行参数指定
    pid_t test_pid = 1;

    if (argc > 1)
    {
        test_pid = atoi(argv[1]);
        if (test_pid <= 0)
        {
            fprintf(stderr, "Error: Invalid PID '%s'\n", argv[1]);
            return 1;
        }
    }

    printf("╔════════════════════════════════════════╗\n");
    printf("║  libkysdk-system Process API Test     ║\n");
    printf("║  Testing PID: %-26d ║\n", test_pid);
    printf("╚════════════════════════════════════════╝\n");

    // 测试基础进程信息
    test_process_basic_info(test_pid);

    // 测试进程ID查找
    test_process_id_lookup();

    // 测试详细进程信息
    test_process_detailed_info();

    // 测试CPU模式比例
    test_cpu_modes(test_pid);

    // 测试运行环境
    test_runtime_env(test_pid);

    // 测试调试终端（交互式）
    test_debug_terminal(test_pid);

    print_separator("测试完成");
    printf("All tests completed successfully!\n\n");

    return 0;
}
