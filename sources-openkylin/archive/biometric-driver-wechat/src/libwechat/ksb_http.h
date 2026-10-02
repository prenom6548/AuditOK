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
#ifndef __KSB_HTTP_H__
#define __KSB_HTTP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <curl/curl.h>


typedef struct _Get_Data{
    size_t size;
    char* data;
}Get_Data;


enum fcurl_type_e {
  CFTYPE_NONE = 0,
  CFTYPE_FILE = 1,
  CFTYPE_CURL = 2
};
 
struct fcurl_data
{
  enum fcurl_type_e type;     /* type of handle */ 
  union {
    CURL *curl;
    FILE *file;
  } handle;                   /* handle */ 
 
  char *buffer;               /* buffer to store cached data*/ 
  size_t buffer_len;          /* currently allocated buffers length */ 
  size_t buffer_pos;          /* end of data in buffer*/ 
  int still_running;          /* Is background url fetch still in progress */ 
};
typedef struct fcurl_data URL_FILE;

extern int Do_Http_Post(char* url, char* postcontent, char* getcontent);
extern int Do_Http_Get(char* url, char* getcontent);
extern int Do_Download(char* url, char* filename);
size_t write_data(void *ptr, size_t size, size_t nmemb, Get_Data *data);
int safe_strcpy(char *dst, const char *src, size_t dstsize);

#ifdef __cplusplus
}
#endif

#endif