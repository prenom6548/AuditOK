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
#include <string.h>
#include <stdlib.h>
#include <sys/syslog.h>

#include "ksb_http.h"

#define REQUIRE_TIMEOUT_S   20
//#define BUSINESS_TIMEOUT_S  5

size_t write_data(void *ptr, size_t size, size_t nmemb, Get_Data *data);
int Do_Http_Get(char *url, char *getcontent);
int Do_Http_Post(char *url, char *postcontent, char *getcontent);
int Do_Download(char *url, char *filename);

int safe_strcpy(char *dst, const char *src, size_t dstsize) {
    size_t srclen = strnlen(src,40860);
    size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;

    if (dstsize > 0) {
        memcpy(dst, src, copylen);
        dst[copylen] = '\0';
    }
    return 0;
}
size_t write_data(void *ptr, size_t size, size_t nmemb, Get_Data *data){
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;
 
    data->size += (size * nmemb);
 
    tmp = (char*)realloc(data->data, data->size + 1); /* +1 for '\0' */
 
    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        printf( "Failed to allocate HTTP-GET data memory.");

        return 0;
    }
 
    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';
 
    return size * nmemb;
}

int Do_Http_Get(char *url, char* getcontent)
{
    // syslog(LOG_DEBUG,"getting %s\n",url);

    int rc = 0;

    CURL *handler = NULL;
    CURLcode ops_res;
    Get_Data ret_data;
    ret_data.data = NULL;

    ret_data.size = 0;
    ret_data.data = (char *)malloc(1024*1024);
    if (NULL ==ret_data.data)
    {
        syslog(LOG_DEBUG,"[%s:%d]Do_Http_Get ret_data failed to allocate memory",__FUNCTION__,__LINE__);
        rc = 301;  //301 - K_EDU_MALLOC
        goto fail;
    }
    ret_data.data[0] = '\0';

    handler = curl_easy_init();
    if (!handler)
    {
        rc = 1;
        printf("HTTP-GET handler create failed");
        goto fail;
    }
    curl_easy_setopt(handler, CURLOPT_URL, url);
    curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handler, CURLOPT_WRITEDATA, &ret_data); 
    curl_easy_setopt(handler, CURLOPT_TIMEOUT, REQUIRE_TIMEOUT_S);

    ops_res = curl_easy_perform(handler);
    if (CURLE_OK != ops_res )
    {   
        //rc = ops_res;
        if (CURLE_OPERATION_TIMEDOUT != ops_res)
        {
        rc=7;
        printf("http do GET failed! errcode = %d", ops_res);
        goto fail;
        }
        
    }
    safe_strcpy(getcontent, ret_data.data, 1024*1024);

fail:
    if(handler)
        curl_easy_cleanup(handler);
    if(ret_data.data)
        free(ret_data.data);
    return rc;
}


int Do_Http_Post(char *url, char *postcontent, char *getcontent)
{
    int  rc = 0;
    
    CURL *handler = NULL;
    CURLcode ops_res;

    Get_Data ret_data;
    ret_data.data = NULL;
    ret_data.size = 0;
    struct curl_slist *headers = NULL;

    ret_data.data = (char *)malloc(1024*1024);
    if (NULL ==ret_data.data)
    {
        printf( "[%s:%d ]Do_Http_Post ret_data failed to allocate memory",__FUNCTION__,__LINE__);
        rc = 301;
        goto fail;
    }
    ret_data.data[0] = '\0';

    handler = curl_easy_init();
    if (!handler)
    {
        rc = 1;
        printf( "http post handler create failed!");
        goto fail;

    }
    curl_easy_setopt(handler, CURLOPT_URL, url);

    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");

    curl_easy_setopt(handler, CURLOPT_POST, 1);
    curl_easy_setopt(handler, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(handler, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(handler, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(handler, CURLOPT_POSTFIELDS, postcontent);

    curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handler, CURLOPT_WRITEDATA, &ret_data);
    curl_easy_setopt(handler, CURLOPT_TIMEOUT, REQUIRE_TIMEOUT_S);


    ops_res = curl_easy_perform(handler);
    if (CURLE_OK !=  ops_res)
    {
        
        rc = ops_res;
        printf( "[%s:%d]http do POST failed!errcode = %d", __FUNCTION__,__LINE__,ops_res);
        goto fail;
    }

    safe_strcpy(getcontent, ret_data.data, 1024*1024);

fail:
   if (headers)
       curl_slist_free_all(headers);
    if (handler)
        curl_easy_cleanup(handler);
    if (ret_data.data)
        free(ret_data.data);
    
    return rc;
}

int Do_Download(char *url, char *filename)
{

    int rc = 0;
    CURLcode ops_res;

    CURL* download_handler = curl_easy_init();
    curl_easy_setopt( download_handler, CURLOPT_URL, url ) ;

    FILE* file = fopen( filename, "wb");
    curl_easy_setopt(download_handler, CURLOPT_WRITEDATA, file) ;
    curl_easy_setopt(download_handler, CURLOPT_TIMEOUT, REQUIRE_TIMEOUT_S);
    //curl_easy_setopt(download_handler, CURLOPT_VERBOSE, 1);
    //curl_easy_setopt(download_handler, CURLOPT_FOLLOWLOCATION, 1);

    ops_res = curl_easy_perform(download_handler);
     if (CURLE_OK != ops_res)
    {   
        rc = ops_res;
        printf( "http do download failed! errcode = %d", ops_res);
        goto fail;
    }

    int jpg_len;
	fseek(file, 0, SEEK_END);
	jpg_len = ftell(file);

    if (jpg_len==0)
    {   
        rc=-1;
        printf( "download failed!");
        goto fail;
    }

   
fail:
    if(download_handler)
        curl_easy_cleanup(download_handler);
    fclose(file);
    return rc;
}