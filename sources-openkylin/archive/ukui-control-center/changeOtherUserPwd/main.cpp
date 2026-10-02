/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
/*change otheruser password on pam*/
#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <glib.h>

/*
 * 管道通信协议（二进制安全）：
 *
 * 调用方（父进程）-> 本程序（子进程）：
 *   [4字节: username_len][N字节: username]
 *   [4字节: new_password_len][N字节: new_password]
 *
 * 本程序（子进程）-> 调用方（父进程）：
 *   [4字节: result_code]      // PAM_SUCCESS = 0 表示成功，其他值表示失败
 *   [4字节: msg_len][N字节: msg]  // 结果消息（UTF-8）
 *
 * 注意：本程序只能通过 stdin/stdout 管道进行通信。
 */

static char *username = NULL;
static char *new_password = NULL;
static char *error_msg = NULL;

/* 从 stdin 读取指定长度的数据（二进制安全） */
static int read_exact(int fd, void *buf, size_t count)
{
    size_t total = 0;
    while (total < count) {
        ssize_t n = read(fd, (char *)buf + total, count - total);
        if (n <= 0) {
            return -1;  /* EOF 或错误 */
        }
        total += n;
    }
    return 0;
}

/* 向 stdout 写入指定长度的数据 */
static int write_exact(int fd, const void *buf, size_t count)
{
    size_t total = 0;
    while (total < count) {
        ssize_t n = write(fd, (const char *)buf + total, count - total);
        if (n < 0) {
            return -1;
        }
        total += n;
    }
    return 0;
}

/* 发送结果到 stdout */
static void send_result(int result_code, const char *msg)
{
    uint32_t code = (uint32_t)result_code;
    uint32_t msg_len = msg ? (uint32_t)strlen(msg) : 0;

    write_exact(STDOUT_FILENO, &code, sizeof(code));
    write_exact(STDOUT_FILENO, &msg_len, sizeof(msg_len));
    if (msg_len > 0) {
        write_exact(STDOUT_FILENO, msg, msg_len);
    }
    fflush(stdout);
}

/* 从 stdin 读取用户名和新密码 */
static int read_credentials_from_pipe(void)
{
    uint32_t user_len = 0;
    uint32_t new_len = 0;

    /* 读取用户名长度 */
    if (read_exact(STDIN_FILENO, &user_len, sizeof(user_len)) < 0) {
        send_result(-1, "Failed to read username length from pipe");
        return -1;
    }

    /* 安全检查：用户名长度限制 */
    if (user_len > 4096) {
        send_result(-1, "Username too long");
        return -1;
    }

    /* 读取用户名 */
    char *user = (char *)calloc(user_len + 1, sizeof(char));
    if (!user) {
        send_result(-1, "Memory allocation failed");
        return -1;
    }
    if (user_len > 0 && read_exact(STDIN_FILENO, user, user_len) < 0) {
        free(user);
        send_result(-1, "Failed to read username from pipe");
        return -1;
    }
    user[user_len] = '\0';

    /* 读取新密码长度 */
    if (read_exact(STDIN_FILENO, &new_len, sizeof(new_len)) < 0) {
        free(user);
        send_result(-1, "Failed to read new password length from pipe");
        return -1;
    }

    if (new_len > 4096) {
        free(user);
        send_result(-1, "New password too long");
        return -1;
    }

    /* 读取新密码 */
    char *new_pwd = (char *)calloc(new_len + 1, sizeof(char));
    if (!new_pwd) {
        free(user);
        send_result(-1, "Memory allocation failed");
        return -1;
    }
    if (new_len > 0 && read_exact(STDIN_FILENO, new_pwd, new_len) < 0) {
        free(user);
        free(new_pwd);
        send_result(-1, "Failed to read new password from pipe");
        return -1;
    }
    new_pwd[new_len] = '\0';

    username = user;
    new_password = new_pwd;
    return 0;
}

static int ni_conv (int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *responses;
    int count;

    if (num_msg <= 0 || num_msg >= PAM_MAX_NUM_MSG) {
        fprintf(stderr, "bad number of messages %d <= 0 || >= %d\n", num_msg, PAM_MAX_NUM_MSG);
        *resp = NULL;
        return PAM_CONV_ERR;
    }

    responses = (struct pam_response *) calloc ((size_t) num_msg, sizeof (*responses));
    if (NULL == responses) {
        return PAM_BUF_ERR;
    }

    //循环每个消息
    for (count=0; count < num_msg; count++) {
        //初始化响应变量
        responses[count].resp_retcode = 0;

        //根据消息类型处理消息
        switch (msg[count]->msg_style) {

        //回显消息，从标准输入获取数据并显示在屏幕上，一般是交互的名称信息，如用户名等
        case PAM_PROMPT_ECHO_ON:
            // 对于更改其他用户密码，可能需要确认用户名
            responses[count].resp = strdup(username);
            if (NULL == responses[count].resp) {
                goto failed_conversation;
            }
            break;

        //从标准输入获取不回显数据，一般是输入密码
        case PAM_PROMPT_ECHO_OFF:
            responses[count].resp = strdup(new_password);

            if (NULL == responses[count].resp) {
                goto failed_conversation;
            }
            break;

        //回显PAM模块传递的错误消息
        case PAM_ERROR_MSG:
            if ((NULL == msg[count]->msg)) {
                goto failed_conversation;
            } else {
                error_msg = strdup(msg[count]->msg);
            }
            responses[count].resp = NULL;
            break;

        //回显PAM模块传递的文本消息
        case PAM_TEXT_INFO:
            if ((NULL == msg[count]->msg)) {
                goto failed_conversation;
            } else {
                error_msg = strdup(msg[count]->msg);
            }
            responses[count].resp = NULL;
            break;

        default:
            fprintf (stderr, "conversation type %d not supported.\n", msg[count]->msg_style);
            goto failed_conversation;
        }
    }

    *resp = responses;

    return PAM_SUCCESS;

failed_conversation:
    for (count=0; count < num_msg; count++) {
        if (NULL != responses[count].resp) {
            memset (responses[count].resp, 0, strnlen(responses[count].resp, sizeof(responses[count].resp)));
            free (responses[count].resp);
            responses[count].resp = NULL;
        }
    }

    free (responses);
    *resp = NULL;

    return PAM_CONV_ERR;
}

int main(int argc, char *argv[])
{
    /* ============================================================
     * 安全策略：禁止通过命令行参数接收密码
     * 调用方只能通过管道（stdin/stdout）与本程序通信
     * ============================================================ */
    if (argc > 1) {
        fprintf(stderr, "Error: This program can only be invoked via pipe.\n");
        fprintf(stderr, "Usage: Do not pass arguments. Write binary data to stdin instead.\n");
        fprintf(stderr, "Protocol: [4B:user_len][user][4B:new_len][new_pwd] -> stdin\n");
        fprintf(stderr, "          [4B:result][4B:msg_len][msg] <- stdout\n");
        return EXIT_FAILURE;
    }

    /* 设置 stdin/stdout 为二进制模式（Windows 兼容性） */
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    struct pam_conv conv;
    int ret = 0;
    pam_handle_t *pamh = NULL;

    /* 从管道读取凭据 */
    if (read_credentials_from_pipe() < 0) {
        return EXIT_FAILURE;
    }

    /* 会话函数传递到 PAM 模块中 */
    conv.conv = ni_conv;
    conv.appdata_ptr = NULL;

    if ((pam_start("passwd", username, &conv, &pamh)) != PAM_SUCCESS) {
        send_result(-1, "PAM start failed");
        /* 清理内存 */
        if (username) {
            free(username);
            username = NULL;
        }
        if (new_password) {
            free(new_password);
            new_password = NULL;
        }
        return EXIT_FAILURE;
    }

    ret = pam_chauthtok(pamh, 0);
    if (ret == PAM_SUCCESS) {
        if (error_msg && strlen(error_msg) > 0) {
            send_result(ret, error_msg);
            free(error_msg);
            error_msg = NULL;
        } else {
            send_result(ret, "Password changed successfully");
        }
    } else {
        if (!error_msg || strlen(error_msg) == 0) {
            send_result(ret, "Unable to modify password!");
        } else {
            send_result(ret, error_msg);
            free(error_msg);
            error_msg = NULL;
        }
    }

    /* 结束 PAM */
    ret = pam_end(pamh, ret);

    /* 清理内存（安全擦除） */
    if (username) {
        free(username);
        username = NULL;
    }
    if (new_password) {
        size_t len = strlen(new_password);
        explicit_bzero(new_password, len);
        free(new_password);
        new_password = NULL;
    }

    return (ret == PAM_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*end pam*/
