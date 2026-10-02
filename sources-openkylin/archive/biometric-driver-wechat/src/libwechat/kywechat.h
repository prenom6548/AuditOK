/*
 * Copyright (C) 2022 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author: Fnoily <liuyang@kylinos.cn>
 *         wangweiran <wangweiran@kylinos.cn>
 *         wumuti <wumuti@kylinos.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pthread.h>
#include <sys/syslog.h>


#define REAL_WECHAT_CALL            "https://open.weixin.qq.com/connect/qrconnect?appid=wxfd0de93f7bd963be&redirect_uri=http%3A%2F%2Fwww.kylinos.cn%2F&response_type=code&scope=snsapi_login"
#define WECHAT_DEVELOP_INTERFACE    "https://lp.open.weixin.qq.com/connect/l/qrconnect?uuid=%s"
#define WECHAT_LISTEN_INTERFACE     "https://lp.open.weixin.qq.com/connect/l/qrconnect?uuid=%s&last=%d"
#define TOKEN_URL_BASE              "https://api.weixin.qq.com/sns/oauth2/access_token?appid=%s&secret=%s&code=%s&grant_type=authorization_code"
// #define TOKEN_URL_BASE              "https://api.weixin.qq.com/sns/oauth2/access_token?appid=%s&secret=%s&code=%s&grant_type=authorization_code"
#define USERINFO_BASE               "https://api.weixin.qq.com/sns/userinfo?access_token=%s&openid=%s"


#define LEN                     1024
#define URL_LEN                 2048
#define CODE_LEN                512


#define KYWECHAT_NOERR              0
#define KYWECHAT_INVALID_PARAM      1
#define KYWECHAT_INVALID_URL        2
#define KYWECHAT_NETWORK            3
#define KYWECHAT_INTERNAL           4


struct url_data {
    size_t size;
    char* data;
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 
 * @param
 */
typedef int (*wechatLoginCallBackFunction_t) (int state, char *authcode);
extern int web_flag;

/**
 * @brief 获取微信登录二维码，及后续状态
 * @param  qr_jpg           二维码图片
 * @param  callback_context 回调函数
 * @return int 
 */
int     GetLoginQR(char *qr_jpg_path, wechatLoginCallBackFunction_t callback_context);
int     GetWechatUserInfo(char *auth_code, char *open_id, char *nickname);
void* CheckWebError(void *argv);
int StopLoginQR();

#ifdef __cplusplus
}
#endif