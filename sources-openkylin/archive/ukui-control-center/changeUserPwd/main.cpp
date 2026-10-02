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
#include <QCoreApplication>
#include <glib.h>

/*using pam change password*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>

static std::string old_password_str;
static std::string new_password_str;

/*
 * 管道通信协议（二进制安全）：
 *
 * 调用方（父进程）-> 本程序（子进程）：
 *   [4字节: old_password_len][N字节: old_password]
 *   [4字节: new_password_len][N字节: new_password]
 *
 * 本程序（子进程）-> 调用方（父进程）：
 *   [4字节: result_code]      // PAM_SUCCESS = 0 表示成功，其他值表示失败
 *   [4字节: msg_len][N字节: msg]  // 结果消息（UTF-8）
 *
 * 注意：本程序禁止通过命令行参数接收任何输入，
 *       只能通过 stdin/stdout 管道进行通信。
 */

static const char *pipe_old_password = "";
static const char *pipe_new_password = "";
static int isCurrentPwd = 0;
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

/* 从 stdin 读取密码对 */
static int read_passwords_from_pipe(void)
{
    uint32_t old_len = 0;
    uint32_t new_len = 0;

    /* 读取旧密码长度 */
    if (read_exact(STDIN_FILENO, &old_len, sizeof(old_len)) < 0) {
        send_result(-1, "Failed to read old password length from pipe");
        return -1;
    }

    /* 安全检查：密码长度限制 */
    if (old_len > 4096) {
        send_result(-1, "Old password too long");
        return -1;
    }

    /* 读取旧密码 */
    char *old_pwd = (char *)calloc(old_len + 1, sizeof(char));
    if (!old_pwd) {
        send_result(-1, "Memory allocation failed");
        return -1;
    }
    if (old_len > 0 && read_exact(STDIN_FILENO, old_pwd, old_len) < 0) {
        free(old_pwd);
        send_result(-1, "Failed to read old password from pipe");
        return -1;
    }
    old_pwd[old_len] = '\0';

    /* 读取新密码长度 */
    if (read_exact(STDIN_FILENO, &new_len, sizeof(new_len)) < 0) {
        free(old_pwd);
        send_result(-1, "Failed to read new password length from pipe");
        return -1;
    }

    if (new_len > 4096) {
        free(old_pwd);
        send_result(-1, "New password too long");
        return -1;
    }

    /* 读取新密码 */
    char *new_pwd = (char *)calloc(new_len + 1, sizeof(char));
    if (!new_pwd) {
        free(old_pwd);
        send_result(-1, "Memory allocation failed");
        return -1;
    }
    if (new_len > 0 && read_exact(STDIN_FILENO, new_pwd, new_len) < 0) {
        free(old_pwd);
        free(new_pwd);
        send_result(-1, "Failed to read new password from pipe");
        return -1;
    }
    new_pwd[new_len] = '\0';

    pipe_old_password = old_pwd;
    pipe_new_password = new_pwd;
    return 0;
}

static int changeCurrent_conv(int num_msg, const struct pam_message **msg,
                              struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *responses;
    int count;

    if (num_msg <= 0 || num_msg >= PAM_MAX_NUM_MSG) {
        fprintf(stderr, "bad number of messages %d <= 0 || >= %d\n", num_msg, PAM_MAX_NUM_MSG);
        *resp = NULL;
        return PAM_CONV_ERR;
    }

    responses = (struct pam_response *) calloc((size_t) num_msg, sizeof(*responses));
    if (NULL == responses) {
        return PAM_BUF_ERR;
    }

    for (count = 0; count < num_msg; count++) {
        responses[count].resp_retcode = 0;

        switch (msg[count]->msg_style) {

        case PAM_PROMPT_ECHO_ON:
            fprintf(stderr, "PAM modules requesting echoing are not supported.\n");
            goto failed_conversation;

        case PAM_PROMPT_ECHO_OFF:
            if (isCurrentPwd == 0) {
                isCurrentPwd = 1;
                responses[count].resp = strdup(pipe_old_password);
            } else {
                responses[count].resp = strdup(pipe_new_password);
            }

            if (NULL == responses[count].resp) {
                goto failed_conversation;
            }
            break;

        case PAM_ERROR_MSG:
            if ((NULL == msg[count]->msg)) {
                goto failed_conversation;
            } else {
                error_msg = strdup(msg[count]->msg);
            }
            responses[count].resp = NULL;
            break;

        case PAM_TEXT_INFO:
            if ((NULL == msg[count]->msg)) {
                goto failed_conversation;
            } else {
                error_msg = strdup(msg[count]->msg);
            }
            responses[count].resp = NULL;
            break;

        default:
            fprintf(stderr, "conversation type %d not supported.\n", msg[count]->msg_style);
            goto failed_conversation;
        }
    }

    *resp = responses;
    return PAM_SUCCESS;

failed_conversation:
    for (count = 0; count < num_msg; count++) {
        if (NULL != responses[count].resp) {
            memset(responses[count].resp, 0, strnlen(responses[count].resp, sizeof(responses[count].resp)));
            free(responses[count].resp);
            responses[count].resp = NULL;
        }
    }

    free(responses);
    *resp = NULL;
    return PAM_CONV_ERR;
}


static int changeCurrent_legacyConv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *responses;
    int count;

//    assert (NULL != non_interactive_password);

    if (num_msg <= 0 || num_msg >= PAM_MAX_NUM_MSG) {
        printf("bad number of messages %d <= 0 || >= %d\n", num_msg, PAM_MAX_NUM_MSG);
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

//        printf("message[%d]: %d %s\n", count, msg[count]->msg_style, msg[count]->msg);

        //根据消息类型处理消息
        switch (msg[count]->msg_style) {

        //回显消息，从标准输入获取数据并显示在屏幕上，一般是交互的名称信息，如用户名等
        case PAM_PROMPT_ECHO_ON:
            fprintf (stderr, "PAM modules requesting echoing are not supported.\n");
            goto failed_conversation;

        //从标准输入获取不回显数据，一般是输入密码
        case PAM_PROMPT_ECHO_OFF:
            if (isCurrentPwd == 0){
                isCurrentPwd = 1;
                responses[count].resp = strdup (old_password_str.c_str());
            } else {
                responses[count].resp = strdup (new_password_str.c_str());
            }

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
            (void) fprintf (stderr, ("conversation type %d not supported.\n"), msg[count]->msg_style);
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


// 兼容模式：从命令行参数读取（存在泄露风险）
int legacy_main(int argc, char *argv[]) {
    // 原有逻辑，解析命令行参数
    struct pam_conv conv;
    int ret = 0;
    pam_handle_t * pamh = NULL;
    const char *username = g_get_user_name();
    if (argc == 3) {
        old_password_str = QByteArray::fromBase64(QString(argv[1]).toLatin1()).constData();
        new_password_str = QByteArray::fromBase64(QString(argv[2]).toLatin1()).constData();
    } else {
        printf("missing parameter!\n");
        return -1;
    }

    //会话函数传递到PAM模块中，在模块中通过pam_get_item获取并调用
    conv.conv = changeCurrent_legacyConv;
    conv.appdata_ptr = NULL;

    //
    if ((pam_start("passwd", username, &conv, &pamh)) != PAM_SUCCESS){
        return 0;
    }

    //
    ret = pam_chauthtok(pamh, 0);
    if (ret == PAM_SUCCESS){
        if (error_msg != "") {
            printf("%s\n", error_msg);
            free(error_msg);
        }
    } else {
        if (error_msg == "") {
            printf("Unable to modify password!\n");
        } else {
            printf("%s\n", error_msg);
            free(error_msg);
        }
    }

    //结束PAM
    ret = pam_end(pamh, ret);

    return ret;
}

int secure_main(int argc, char *argv[])
{
    /* ============================================================
     * 安全策略：禁止通过命令行参数接收密码
     * 调用方只能通过管道（stdin/stdout）与本程序通信
     * ============================================================ */
    if (argc > 1) {
        fprintf(stderr, "Error: This program can only be invoked via pipe.\n");
        fprintf(stderr, "Usage: Do not pass arguments. Write binary data to stdin instead.\n");
        fprintf(stderr, "Protocol: [4B:old_len][old_pwd][4B:new_len][new_pwd] -> stdin\n");
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
    const char *username = g_get_user_name();

    /* 从管道读取密码 */
    if (read_passwords_from_pipe() < 0) {
        return EXIT_FAILURE;
    }

    /* 会话函数传递到 PAM 模块中 */
    conv.conv = changeCurrent_conv;
    conv.appdata_ptr = NULL;

    if ((pam_start("passwd", username, &conv, &pamh)) != PAM_SUCCESS) {
        send_result(-1, "PAM start failed");
        /* 清理内存 */
        if (pipe_old_password) {
            free((void *)pipe_old_password);
            pipe_old_password = NULL;
        }
        if (pipe_new_password) {
            free((void *)pipe_new_password);
            pipe_new_password = NULL;
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

    /* 清理密码内存（安全擦除） */
    if (pipe_old_password) {
        size_t len = strlen(pipe_old_password);
        memset((void *)pipe_old_password, 0, len);
        free((void *)pipe_old_password);
        pipe_old_password = NULL;
    }
    if (pipe_new_password) {
        size_t len = strlen(pipe_new_password);
        memset((void *)pipe_new_password, 0, len);
        free((void *)pipe_new_password);
        pipe_new_password = NULL;
    }

    return (ret == PAM_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    const char *secure_mode = getenv("UKCC_SECURE_MODE");
    if (secure_mode != NULL && strcmp(secure_mode, "1") == 0) {
        // 安全模式：走管道输入
        return secure_main(argc, argv);
    } else {
        // 兼容模式：走命令行参数（带警告）
        fprintf(stderr, "WARNING: Running in legacy mode. "
                "Password may be exposed in process list. "
                "Please set UKCC_SECURE_MODE=1 for secure mode.\n");
        return legacy_main(argc, argv);
    }
}
/* end pam */
