#include "utils/cmd_runner.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MODULE "cmd_runner"

static int last_exit_code = 0;

psp_error_t cmd_runner_execute(const char *command, const char **args,
                          char *output, size_t output_size)
{
    if (!command) {
        return ERR_INVALID_PARAM;
    }

    size_t arg_count = 0;
    if (args) {
        while (args[arg_count] != NULL) {
            arg_count++;
        }
    }

    const char **argv = (const char**)malloc((arg_count + 2) * sizeof(char*));
    if (!argv) {
        logger_error(MODULE, "Failed to allocate memory for arguments");
        return ERR_MEMORY;
    }

    argv[0] = command;
    for (size_t i = 0; i < arg_count; i++) {
        argv[i + 1] = args[i];
    }
    argv[arg_count + 1] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        logger_error(MODULE, "Failed to create pipe (error: %d)", errno);
        free(argv);
        return ERR_IO;
    }

    pid_t pid = fork();
    if (pid == -1) {
        logger_error(MODULE, "Failed to fork process (error: %d)", errno);
        close(pipefd[0]);
        close(pipefd[1]);
        free(argv);
        return ERR_GENERIC;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execv(command, (char *const *)argv);
        _exit(127);
    }

    /* 父进程 */
    close(pipefd[1]);

    if (output && output_size > 0) {
        /* 循环读取直到管道关闭或缓冲区满 */
        size_t total_read = 0;
        while (total_read < output_size - 1) {
            ssize_t bytes_read = read(pipefd[0], output + total_read,
                                      output_size - 1 - total_read);
            if (bytes_read <= 0) {
                break;  /* EOF 或错误 */
            }
            total_read += bytes_read;
        }
        output[total_read] = '\0';

        /* 如果缓冲区已满但管道仍有数据，排空管道防止子进程 SIGPIPE */
        if (total_read >= output_size - 1) {
            char drain[1024];
            while (read(pipefd[0], drain, sizeof(drain)) > 0) {
                /* 排空 */
            }
        }
    } else {
        /* 不关心输出，也要排空管道防止子进程 SIGPIPE */
        char drain[1024];
        while (read(pipefd[0], drain, sizeof(drain)) > 0) {
            /* 排空 */
        }
    }

    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        last_exit_code = WEXITSTATUS(status);
        if (last_exit_code != 0) {
            logger_debug(MODULE, "Command exited with code: %d", last_exit_code);
        }
    } else {
        last_exit_code = -1;
        logger_error(MODULE, "Command terminated abnormally");
    }

    free(argv);
    return (last_exit_code == 0) ? ERR_OK : ERR_EXECUTION;
}

psp_error_t cmd_runner_execute_shell(const char *shell_command, 
                                char *output, size_t output_size)
{
    if (!shell_command) {
        return ERR_INVALID_PARAM;
    }
    
    /* 使用 /bin/sh -c 执行 shell 命令 */
    const char *args[] = {"-c", shell_command, NULL};
    return cmd_runner_execute("/bin/sh", args, output, output_size);
}

bool cmd_runner_command_exists(const char *command)
{
    if (!command) {
        return false;
    }
    
    /* 如果是绝对路径，直接检查 */
    if (command[0] == '/') {
        return file_utils_is_regular_file(command) && access(command, X_OK) == 0;
    }
    
    /* 在 PATH 中查找 */
    const char *path_env = getenv("PATH");
    if (!path_env) {
        return false;
    }
    
    char *path_copy = string_utils_strdup(path_env);
    if (!path_copy) {
        return false;
    }
    
    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    
    while (dir) {
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        
        if (file_utils_is_regular_file(full_path) && access(full_path, X_OK) == 0) {
            free(path_copy);
            return true;
        }
        
        dir = strtok_r(NULL, ":", &saveptr);
    }
    
    free(path_copy);
    return false;
}

int cmd_runner_get_exit_code(void)
{
    return last_exit_code;
}
