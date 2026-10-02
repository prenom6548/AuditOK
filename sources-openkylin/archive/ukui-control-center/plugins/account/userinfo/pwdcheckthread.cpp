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
#include "pwdcheckthread.h"
#include <stdio.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <glib.h>
#include <string.h>
#include <QDebug>

// 封装回调需要的所有数据，避免全局变量
struct PwdCheckData {
    const char *password;   // 新密码
    char *error;            // PAM 返回的错误/提示消息
};

PwdCheckThread::PwdCheckThread(QObject *parent):QThread(parent)
{
}

PwdCheckThread::~PwdCheckThread()
{
}

void PwdCheckThread::setArgs(const QString &userName, const QString &userPwd)
{
    uname = userName;
    upwd = userPwd;
}

static int ni_conv(int num_msg, const struct pam_message **msg,
                   struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *responses;
    int count;

    // 从 appdata_ptr 获取数据，替代全局变量
    struct PwdCheckData *data = static_cast<struct PwdCheckData *>(appdata_ptr);
    if (NULL == data) {
        return PAM_CONV_ERR;
    }

    if (num_msg <= 0 || num_msg >= PAM_MAX_NUM_MSG) {
        printf("bad number of messages %d <= 0 || >= %d\n", num_msg, PAM_MAX_NUM_MSG);
        *resp = NULL;
        return PAM_CONV_ERR;
    }

    responses = (struct pam_response *) calloc((size_t) num_msg, sizeof(*responses));
    if (NULL == responses) {
        return PAM_BUF_ERR;
    }

    for (count = 0; count < num_msg; count++) {
        responses[count].resp_retcode = 0;

        printf("message[%d]: %d %s\n", count, msg[count]->msg_style, msg[count]->msg);

        switch (msg[count]->msg_style) {

        case PAM_PROMPT_ECHO_ON:
            fprintf(stderr, "PAM modules requesting echoing are not supported.\n");
            goto failed_conversation;

        case PAM_PROMPT_ECHO_OFF:
            // 使用 data->password 替代全局 new_password
            responses[count].resp = strdup(data->password);
            if (NULL == responses[count].resp) {
                goto failed_conversation;
            }
            break;

        case PAM_ERROR_MSG:
        case PAM_TEXT_INFO:
            if (NULL == msg[count]->msg) {
                goto failed_conversation;
            } else {
                // 先释放旧的 error，防止内存泄漏
                if (data->error != NULL) {
                    free(data->error);
                }
                data->error = strdup(msg[count]->msg);
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
            memset(responses[count].resp, 0, strnlen(responses[count].resp, PAM_MAX_RESP_SIZE));
            free(responses[count].resp);
            responses[count].resp = NULL;
        }
    }
    free(responses);
    *resp = NULL;
    return PAM_CONV_ERR;
}

void PwdCheckThread::run()
{
    int ret = 0;
    pam_handle_t *pamh = NULL;

    // 数据全部放在栈上，通过 appdata_ptr 传递
    struct PwdCheckData data;
    data.error = NULL;

    // 用户名和密码转为 std::string 持久化，避免 c_str() 悬空
    std::string str1 = uname.toStdString();
    const char *username = str1.c_str();

    std::string str2 = upwd.toStdString();
    data.password = str2.c_str();   // str2 在 run() 结束前一直存活

    struct pam_conv conv;
    conv.conv = ni_conv;
    conv.appdata_ptr = &data;       // 传递结构体指针

    if ((pam_start("control-center", username, &conv, &pamh)) != PAM_SUCCESS) {
        printf("failed to start PAM: %s\n", pam_strerror(NULL, ret));
    }

    ret = pam_authenticate(pamh, 0);
    QString errorMsg = "";
    qDebug() << data.error;

    if (ret == PAM_SUCCESS) {
        if (data.error != NULL) {
            errorMsg = data.error;
            free(data.error);
            data.error = NULL;
        }
    } else {
        if (data.error != NULL) {
            errorMsg = data.error;
            free(data.error);
            data.error = NULL;
        } else {
            errorMsg = tr("Unable to modify password!\n");
        }
    }

    qDebug() << errorMsg;

    ret = pam_end(pamh, ret);

    emit complete(errorMsg);
}
