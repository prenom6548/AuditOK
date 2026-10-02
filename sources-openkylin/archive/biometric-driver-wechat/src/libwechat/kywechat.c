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

#include "kywechat.h"
#include "ksb_http.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define REDIRECT_URL_ENCODE         "http%3A%2F%2Fwww.kylinos.cn%2F"
#define APPID                       "wxfd0de93f7bd963be"
#define APPSECRET                   "6076216a9df1779f0dff89762333c8ba"
#define SCOPE                       "snsapi_login"
int web_flag;


pthread_t thread_id;
pthread_t thread_id2;
pthread_attr_t attr;
char uni_uuid[128];

typedef struct{
	char *uuid_param;
	wechatLoginCallBackFunction_t  callback_context;
}wechat_param_t;


wechat_param_t wechat_param;

void	ParseWechatRetContent(char *origin_string, int *errcode, char *authcode);
int     GetLoginQR(char *qr_jpg_path, wechatLoginCallBackFunction_t callback_context);
int     GetWechatUserInfo(char *auth_code, char *open_id, char *nickname);
// int 	GetwechatUserName(char *auth_code, char *open_id, char *nickname);

void ParseWechatRetContent(char *origin_string, int *errcode, char *authcode)
{
	//window.wx_errcode=408;window.wx_code='';
	int pos;
	pos = 18;
	//拼接errcode
	*errcode = 0;
	for (;;)
	{
        if (origin_string[pos] == ';')
			break;
		*errcode = *errcode * 10 + (int)origin_string[pos] - 48;
		pos = pos + 1;
		
	}
	//指向wx_code内容的第一个单引号后
    pos = pos + 17;
	//判断没有带code的情况
	if(origin_string[pos]=='\'')
	{
		safe_strcpy(authcode,"",sizeof(""));
		return;
	}
	//带code的情况
    safe_strcpy(authcode, origin_string+pos,1024);	
    strtok(authcode, "\';");
	return;
}

void* ExcuteLoginListen(void *argv)
{

	wechat_param_t* wechat_param = (wechat_param_t *) argv;
	char *dev_uuid = wechat_param->uuid_param;
	wechatLoginCallBackFunction_t wx_callback = (wechatLoginCallBackFunction_t) wechat_param->callback_context;

	int rc = 0;
    char *wechat_ret_content = NULL;
	wechat_ret_content = (char *)malloc(1024*1024);
	if (!wechat_ret_content)
	{
		printf("wechat_ret_content 内存申请失败！\n");
		//return;
	}
	int		wx_errcode=0;
	char	wx_authcode[1024]={0};
	char wx_lp_url[1024] = {0};
	snprintf(wx_lp_url,1024, WECHAT_DEVELOP_INTERFACE, dev_uuid);
	while (web_flag)
	{
		// syslog(LOG_DEBUG,"listenning\n");

		wx_errcode = 0;
		memset(wechat_ret_content, 0, sizeof(wechat_ret_content) + 1);
		// pthread_testcancel();
		printf("wx_lp_url :%s \n",wx_lp_url);
		rc = Do_Http_Get(wx_lp_url, wechat_ret_content);
		// syslog(LOG_DEBUG,"after http get\n");
		if(rc!=0||wechat_ret_content==NULL||strcmp(wechat_ret_content,"\0")==0)
		{
			printf("Get wechat content failed\n");
			wx_callback(7, NULL);
			goto clean_up;
		}
		// pthread_testcancel();
		printf("wx_content : %s\n", wechat_ret_content);
		ParseWechatRetContent(wechat_ret_content, &wx_errcode, wx_authcode);
		printf("wx_errcode : %d\n", wx_errcode);
		switch (wx_errcode)
		{
			// 在时间内尚无相应
			case 408:{
				// printf( "before wx_callback");
				wx_callback(1, NULL);
				// printf( "after wx_callback");
				//(wechatLoginCallBackFunction_t &) wechat_param->callback_context(1,"null");
				printf("send signal of 408\n");
				snprintf(wx_lp_url,1024, WECHAT_DEVELOP_INTERFACE, dev_uuid);
			}break;
			
			//用户已扫码状态
			case 404:{
				wx_callback(2, NULL);
				printf(" send signal of 404\n");
				snprintf(wx_lp_url,1024, WECHAT_LISTEN_INTERFACE, dev_uuid,wx_errcode);
			}break;
			
			// 用户取消扫码，取消扫码后二维码仍可扫
			case 403:{
				wx_callback(3, NULL);
				printf("send signal of 403\n");
				// 更新二维码状态监听地址,微信lp地址带上次返回错误码
				snprintf(wx_lp_url,1024,WECHAT_LISTEN_INTERFACE,dev_uuid,wx_errcode);
			}break;

			// 用户扫码成功并确认后确认授权
			case 405:{
				wx_callback(5, wx_authcode);
				// 二维码无效终止监听	
				printf("send signal of 405\n");
				goto clean_up;
			}break;

			case 402:{
				wx_callback(6, NULL);
				printf("send signal of 402\n");
				goto clean_up;
			}
			// 此时需reload
			case 500:{
				// 抛出信号
				wx_callback(6,NULL);
				syslog(LOG_DEBUG,"send signal of 500\n");
				goto clean_up;
			}break;
			// 其他未知wx_errcode类型，一律按需重新刷新二维码处理
			default:
			{
				wx_callback(6,NULL);
				printf("send unkown\n");
				goto clean_up;
			}
		}
	}
clean_up:
	if (wechat_ret_content)
		free(wechat_ret_content);
	web_flag=0;
    // return ;
}

int	GetLoginQR(char *qr_jpg_path, wechatLoginCallBackFunction_t callback_context)
{
	int rc = 0;
	if(qr_jpg_path == NULL )
	{
		return KYWECHAT_INVALID_PARAM;
	}
	if (callback_context == NULL)
	{
		return KYWECHAT_INVALID_PARAM;
	}

	// pthread_testcancel();		
	CURL *handler1 = NULL;
    CURLcode ops_res1;
    Get_Data ret_data1;
    ret_data1.data = NULL;    	
	ret_data1.size = 0;
    ret_data1.data = (char *)malloc(1024*1024);
	//printf("check baidu1\n");
	
		// pthread_testcancel();
		//printf("check baidu2\n");
    if (NULL ==ret_data1.data)
	{
      	printf("Do_Http_Get ret_data failed to allocate memory");
     	rc = 301;  //301 - K_EDU_MALLOC
      	return 7;
    }
	ret_data1.data[0] = '\0';
    handler1 = curl_easy_init();
	if (!handler1)
	{
      	printf("HTTP-GET handler create failed");
      	return 7;
	}
	curl_easy_setopt(handler1, CURLOPT_URL, "https://www.baidu.com");
	curl_easy_setopt(handler1, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handler1, CURLOPT_WRITEDATA, &ret_data1); 
	curl_easy_setopt(handler1, CURLOPT_TIMEOUT, 3);

    ops_res1 = curl_easy_perform(handler1);
    if (CURLE_OK != ops_res1)
    {   
       	printf("http do GET failed! errcode = %d", ops_res1);
		if(ops_res1 == CURLE_PEER_FAILED_VERIFICATION)
		{
			return 9;
		}
       	return 7;
	}
	
	char *qr_login_web_content = NULL;
	char *web_ret = NULL;
	char qr_url[64] = {0};
	//char *uuid = NULL;
	qr_login_web_content = (char *)malloc(1024 * 1024);
	if (!qr_login_web_content)
	{
		printf("qr_web_login_content allocation error");
		rc = -1;
		return rc;
	}

	rc = Do_Http_Get(REAL_WECHAT_CALL, qr_login_web_content);
	if (rc)
	{
		if (qr_login_web_content)
			free(qr_login_web_content);
		printf("[%s:%d]do http get ops to get QR code failed, error code=%d",__FUNCTION__,__LINE__, rc);
		return rc;
	}
	// syslog(LOG_DEBUG,"get web\n");
	web_ret = (char *)malloc(1024*1024);
	if (!web_ret)
	{
		if (qr_login_web_content)
			free(qr_login_web_content);
		printf("web_ret allocation error");
		rc = -1;
		return rc;
	}

	safe_strcpy(web_ret,strstr(qr_login_web_content, "/connect/qrcode/"),1024);
	strtok(web_ret, "\"");
	snprintf(qr_url,64,"https://open.weixin.qq.com%s",web_ret);
	// syslog(LOG_DEBUG,"getting %s\n",qr_url);
    rc = Do_Download(qr_url, "/var/lib/biometric-auth/image.jpg");
	web_flag=1;
	safe_strcpy(uni_uuid, web_ret + 16,1024);
	wechat_param.uuid_param = uni_uuid;
	wechat_param.callback_context = callback_context;

	safe_strcpy(qr_jpg_path, "/var/lib/biometric-auth/image.jpg",40);

	rc = pthread_create(&thread_id, 0, ExcuteLoginListen, (void*) &wechat_param);
	if (rc)
	{
		if (qr_login_web_content)
			free(qr_login_web_content);
		if(web_ret)
			free(web_ret);
		printf( "wechat login failed, async thread create failed.");
		return rc;
	}
	rc = pthread_create(&thread_id2, 0, CheckWebError, (void*) &wechat_param);
	if (qr_login_web_content)
		free(qr_login_web_content);
	if(web_ret)
		free(web_ret);
	return rc;
	
}

int  GetWechatUserInfo(char *auth_code, char *open_id, char *nickname)
{
	
	// 获取微信用户基本信息
	if (!auth_code)
    {
    	//printf("授权code不存在\n");
        return KYWECHAT_INVALID_PARAM;
    }

	int rc=0;
	char access_token[248] = {0};
	char openid[128]={0};
	char userinfo_url[URL_LEN] = {0};
    // syslog(LOG_DEBUG,"auth_code : %s \n",auth_code);
    // 构建获取access_token的URL
    char access_token_url[URL_LEN];
    snprintf(access_token_url,2048, TOKEN_URL_BASE, APPID, APPSECRET,auth_code);
    syslog(LOG_DEBUG,"access_token_url : %s\n", access_token_url);

    // GET access_token_url

    struct url_data token_data;
    token_data.size = 0;
    token_data.data = (char *)malloc(4096);
    if (NULL ==token_data.data)
    {
        printf("token_data failed to allocate memory.\n");
        return 1;
    }

    token_data.data[0] = '\0';
	int errcode=0;
	rc = Do_Http_Get(access_token_url, token_data.data);
	syslog(LOG_DEBUG,"%s\n",token_data.data);
	if (rc||token_data.data==NULL)
	{
		printf("do http get ops to get QR code failed, error code=%d",rc);
		goto clean_up;
	}
    //CJSON 解析
    cJSON *parse_root;
    cJSON *parse_access_token;
    cJSON *parse_expire_in;
    cJSON *parse_refresh_token;
    cJSON *parse_openid;
    cJSON *parse_scope;
    cJSON *parse_unionid;
	// cJSON *parse_errcode;
  
    parse_root = cJSON_Parse(token_data.data);
	// parse_errcode = cJSON_GetObjectItem(parse_root, "errmsg");
	// if(parse_errcode->valuestring!=NULL)
	// {
	// 	rc=1;
	// 	syslog(LOG_DEBUG,"GET USER ERROR\n");
	// 	goto clean_up;
	// }
    parse_access_token = cJSON_GetObjectItem(parse_root, "access_token");
    parse_expire_in = cJSON_GetObjectItem(parse_root, "expires_in");
    parse_refresh_token = cJSON_GetObjectItem(parse_root,"refresh_token"); 
    parse_openid = cJSON_GetObjectItem(parse_root,"openid");
    parse_scope = cJSON_GetObjectItem(parse_root,"scope");
    parse_unionid = cJSON_GetObjectItem(parse_root,"unionid");

	safe_strcpy(openid, parse_openid->valuestring,10240);
	safe_strcpy(open_id,openid,10240);
	safe_strcpy(access_token, parse_access_token->valuestring,10240);

	syslog(LOG_DEBUG,"%s,%s\n",access_token,openid);
	// syslog(LOG_DEBUG,"%d,%d\n",sizeof(access_token),sizeof(openid));

	snprintf(userinfo_url,2048,"https://api.weixin.qq.com/sns/userinfo?access_token=%s&openid=%s",access_token,openid);

	struct url_data user_data;
	user_data.size = 0;
    user_data.data = (char *)malloc(4096);
	if (NULL ==user_data.data)
    {
        printf("user_data failed to allocate memory.\n");
        return 1;
    }
	user_data.data[0] = '\0';\

	rc = Do_Http_Get(userinfo_url, user_data.data);

	syslog(LOG_NOTICE,"Get user data :%s\n",user_data.data);

	if (NULL ==user_data.data)
    {
        printf("user_data failed to allocate memory.\n");
        return 1;
    }

	cJSON *info_root;
    cJSON *info_nickname;

	info_root = cJSON_Parse(user_data.data);
    info_nickname = cJSON_GetObjectItem(info_root, "nickname");
	// printf("nickname:%s\n",info_nickname->valuestring);
	safe_strcpy(nickname, info_nickname->valuestring,10240);
	printf("nickname:%s\n",nickname);

clean_up:

	if (parse_root)
		cJSON_Delete(parse_root);
	if (info_root)
		cJSON_Delete(info_root);
	return rc;
}

void* CheckWebError(void *argv)
{
	wechat_param_t* wechat_param = (wechat_param_t *) argv;
	char *dev_uuid = wechat_param->uuid_param;
	wechatLoginCallBackFunction_t wx_callback = (wechatLoginCallBackFunction_t) wechat_param->callback_context;
    char *chr = "nmcli connection show -active";
    while(web_flag)
    {
    	FILE *activefp;
    	int activenum=0;
    	char activeStrLine[1024];
    	if((activefp=popen(chr,"r"))==NULL)
    	{
        	printf("error!");
    	}
    	fgets(activeStrLine,1024,activefp);
    	while(!feof(activefp))
    	{
        	fgets(activeStrLine,1024,activefp);
        	activenum++;
    	}
		if (activenum == 1)
		{
			wx_callback(7, NULL);
			goto clean_up;
		}
    	printf("%d\n",activenum);
    	pclose(activefp);
    	sleep(1);
    }
	//printf("after check baidu\n");
clean_up:
	web_flag=0;
}

int	StopLoginQR()
{
	web_flag=0;
	return 0;
}