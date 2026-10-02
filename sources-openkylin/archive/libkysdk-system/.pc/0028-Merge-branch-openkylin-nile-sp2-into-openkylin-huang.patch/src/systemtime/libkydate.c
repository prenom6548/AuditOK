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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkydate.h"
#include <stdio.h>
#include <string.h>
#include <libkyconf.h>
#include <glib-object.h>
#include <glib.h>
#include <errno.h>
#include <kysdk/kysdk-base/sdkmarcos.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libintl.h>
#include <locale.h>
#include <string.h>
#include <libkylog.h>

#define DATEFORMAT "DATEFORMAT"

#define CONF_FILE_PATH  "/etc/kydate/dateform.conf"
#define LOCALEDIR "/usr/share/locale/"
#define GETTEXT_PACKAGE "kysdk-date"
#define MAXSIZE 80
#define PATH_MAX_H 4096

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

char path[512] = {0};	//用户修改后配置文件路径

static uint verify_file(char *pFileName)
{
	int ret =  strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
    return !ret;
}

char *en_long_mon(int mon)
{
	char *en_mon = (char *)malloc(32 * sizeof(char));
	switch (mon)
    {
    case 1:
        strncpy(en_mon, "January", 32);
        break;
    case 2:
        strncpy(en_mon, "February", 32);
        break;
    case 3:
        strncpy(en_mon, "March", 32);
        break;
    case 4:
        strncpy(en_mon, "April", 32);
		break;
	case 5:
        strncpy(en_mon, "May", 32);
		break;
	case 6:
        strncpy(en_mon, "June", 32);
		break;
	case 7:
        strncpy(en_mon, "July", 32);
		break;
	case 8:
        strncpy(en_mon, "August", 32);
		break;
	case 9:
        strncpy(en_mon, "September", 32);
		break;
	case 10:
        strncpy(en_mon, "October", 32);
		break;
	case 11:
        strncpy(en_mon, "November", 32);
		break;
	case 12:
        strncpy(en_mon, "December", 32);
		break;
	default:
		break;
    }
    return en_mon;
}

// char *en_short_mon(int mon)
// {
// 	char *en_mon = (char *)malloc(32 * sizeof(char));
// 	switch (mon)
//     {
//     case 1:
//         strcpy(en_mon,"Jan");
//         break;
//     case 2:
//         strcpy(en_mon,"Feb");
//         break;
//     case 3:
//         strcpy(en_mon,"Mar");
//         break;
//     case 4:
//         strcpy(en_mon,"Apr");
// 		break;
// 	case 5:
//         strcpy(en_mon,"May");
// 		break;
// 	case 6:
//         strcpy(en_mon,"Jun");
// 		break;
// 	case 7:
//         strcpy(en_mon,"Jul");
// 		break;
// 	case 8:
//         strcpy(en_mon,"Aug");
// 		break;
// 	case 9:
//         strcpy(en_mon,"Sept");
// 		break;
// 	case 10:
//         strcpy(en_mon,"Oct");
// 		break;
// 	case 11:
//         strcpy(en_mon,"Nov");
// 		break;
// 	case 12:
//         strcpy(en_mon,"Dec");
// 		break;
// 	default:
// 		break;
//     }
//     return en_mon;
// }

char** kdk_system_get_dateformat()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
    time_t timep;
    time(&timep);
	struct tm *p;
	p = localtime(&timep);
    char tmp[128] = {0};

	size_t index = 0;
	char **res = (char **)malloc(sizeof(tmp) * 256);
	if(!res)
		goto out;
    res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
	if(!res[index])
		goto out;
	char *lang = getenv("LANG");
	GKeyFile *config = g_key_file_new();

	FILE *fp = fopen(CONF_FILE_PATH, "r");
	if(!fp)
		goto out;
	g_key_file_load_from_file(config, CONF_FILE_PATH, 0, NULL);
	char *tmpval = g_key_file_get_string(config, "DATE", "DATE_FORMAT", NULL);

	if (strstr(tmpval, "**/**/**"))
	{
		strftime(tmp, sizeof(tmp), "%Y/%m/%d", localtime(&timep));
		strncpy(res[index], tmp, sizeof(tmp));
	}

	if (strstr(tmpval, "**年**月**日"))
	{
		char tmpe[128] = {0};
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmpe) + 1));
		if(!res[index])
			goto out;

		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(1+p->tm_mon);
			if(p->tm_mday > 9)
			{
				snprintf(tmpe, sizeof(tmpe), "%s %d, %d", mon, (p->tm_mday), (1900 + p->tm_year));
			}else{
				snprintf(tmpe, sizeof(tmpe), "%s 0%d, %d", mon, (p->tm_mday), (1900 + p->tm_year));
			}
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
		{
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day"); 
            if(p->tm_mday > 9)
			{
                if((p->tm_mon + 1) > 9)
                {
                    snprintf(tmpe, sizeof(tmpe), "%d%s%d%s%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
                else{
                    snprintf(tmpe, sizeof(tmpe), "%d%s0%d%s%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
				
			}else{
                if((p->tm_mon + 1) > 9)
                {
				    snprintf(tmpe, sizeof(tmpe), "%d%s%d%s0%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
                else{
                    snprintf(tmpe, sizeof(tmpe), "%d%s0%d%s0%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
			}
        }
        else{
			strftime(tmpe, sizeof(tmpe), gettext("%Y_year%m_mon%d_day"), localtime(&timep));
		}
		strncpy(res[index], tmpe, sizeof(tmpe));
		
	}

    if (strstr(tmpval, "*/*/*"))
	{
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
		if(!res[index])
			goto out;

		snprintf(tmp, sizeof(tmp), "%d/%d/%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		strncpy(res[index], tmp, sizeof(tmp));
	}

    if (strstr(tmpval, "**-**-**"))
	{
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
		if (!res[index])
			goto out;

		strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
		strncpy(res[index], tmp, sizeof(tmp));
	}

    if (strstr(tmpval, "*-*-*"))
	{
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
		if (!res[index])
			goto out;

		snprintf(tmp, sizeof(tmp), "%d-%d-%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		strncpy(res[index], tmp, sizeof(tmp));
	}

	if (strstr(tmpval, "**.**.**"))
	{
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
		if (!res[index])
			goto out;
		strftime(tmp, sizeof(tmp), "%Y.%m.%d", localtime(&timep));
		strncpy(res[index], tmp, sizeof(tmp));
	}

    if (strstr(tmpval, "*.*.*"))
	{
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
		if (!res[index])
			goto out;
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		strncpy(res[index], tmp, sizeof(tmp));
	}

	if (strstr(tmpval, "*年*月*日"))
	{
		index++;
		res[index] = malloc(sizeof(char) * (sizeof(tmp) + 1));
		if (!res[index])
			goto out;
		if (strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(1+p->tm_mon);
			snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, (p->tm_mday), (1900 + p->tm_year)%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
		{
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

			snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", (1900 + p->tm_year)%100, trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
			
		}
		strncpy(res[index], tmp, sizeof(tmp));
	}

	index++;
	res[index] = NULL;
	fclose(fp);
	g_free(tmpval);
    return res;
out:
	while(index)
	{
		free(res[--index]);
	}
	free(res);
	fclose(fp);
	g_free(tmpval);
	return NULL;
}

static int strtok_date(char *tmp, char *str, int mark)
{
	int i = 0;
	int len = 0;
	char *buf[3], *p = NULL;
	p = strtok(tmp, str);
	while(p)
	{
		buf[i] = p;
		i++;
		p = strtok(NULL,str);
	}
	
	len = strnlen(buf[0], STRNLEN_MAX);
	return len;
}

static int strtok_short_date(char *tmp, char *str)
{
	int i = 0;
	int len = 0;
	char *buf[3], *p = NULL;
	p = strtok(tmp, str);
	while(p)
	{
		buf[i] = p;
		i++;
		p = strtok(NULL,str);
	}
	
	len = strnlen(buf[0], STRNLEN_MAX);
	return len;
}

int kdk_system_set_dateformat(char *date)
{
	int i = 0;
	char tmp[64] = {0};
	char *homeDir = NULL;
	strncpy(tmp, date, sizeof(tmp) - 1);
	GKeyFile *config = g_key_file_new();
	GError* gerr = NULL;
	int status;
	int mark = 0;
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}

	if(strstr(env_time, "en_US"))
	{
		mark = 1;
	}
	else{
		mark = 0;
	}

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		return -1;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/", filename);
	if (!realpath(path, canonical_filename))
	{
		if(!verify_file(canonical_filename))
		{
			return -1;
		}
		status = mkdir(canonical_filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status != 0) {
			return -1;
		}
	}

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	char tpath[512] = {0};
	
	char *home = getlogin();
	
	snprintf(tpath, sizeof(tpath), "/var/lib/lightdm-data/%s/dateformat.conf", home);
	
	g_key_file_load_from_file(config, path, 0, NULL);
	if(strstr(tmp, "/"))
	{
		int len =  strtok_date(tmp, "/", mark);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "**/**/**");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "*/*/*");
		}
	}
	else if(strstr(tmp, "-"))
	{
		int len =  strtok_date(tmp, "-", mark);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "**-**-**");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "*-*-*");
		}
	}
	else if(strstr(tmp, "."))
	{
		int len =  strtok_date(tmp, ".", mark);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "**.**.**");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "*.*.*");
		}
	}
	else if(strstr(tmp, "年"))
	{
		int len =  strtok_date(tmp, "年", 0);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "**年**月**日");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "*年*月*日");
		}
	}
	else if(strstr(tmp, ","))
	{
		char *buf[16], *p = NULL;
		p = strtok(tmp, ",");
		while(p)
		{
			buf[i] = p;
			i++;
			p = strtok(NULL,"/");
		}
		int len = strnlen(buf[1], STRNLEN_MAX);
		if(len == 4 || len == 3)
		{
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "**年**月**日");
		}
		else if(len == 2 || len == 3){
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "*年*月*日");
		}
		else{
			goto err;
		}
	}else{
		char *p = NULL;
		p = strtok(tmp, "ལོ།");
		int len = strnlen(p, STRNLEN_MAX);
		
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "**年**月**日");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "DATE_FORMAT", "*年*月*日");
		}
		else{
			goto err;
		}
	}
	g_key_file_save_to_file(config, path, &gerr);

	g_key_file_save_to_file(config, tpath, &gerr);

	g_key_file_free(config);
	return 0;
err:
	g_key_file_free(config);
	
	return -1;
}

int kdk_system_set_24_timeformat()
{
	char *homeDir = NULL;
	int status = 0;

	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		return -1;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/", filename);

	if (!realpath(path, canonical_filename))
	{
		if(!verify_file(canonical_filename))
		{
			return -1;
		}
		status = mkdir(canonical_filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status != 0) {
			return -1;
		}
	}
	
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	GKeyFile *config = g_key_file_new();
	GError *gerr = NULL;
	char tpath[512] = {0};
	
	char *home = getlogin();

	snprintf(tpath, sizeof(tpath), "/var/lib/lightdm-data/%s/dateformat.conf", home);
	
	g_key_file_load_from_file(config, path, 0, NULL);
	g_key_file_set_value(config, DATEFORMAT, "TIME_FORMAT", "24小时制");
	g_key_file_save_to_file(config, path, &gerr);
	g_key_file_save_to_file(config, tpath, &gerr);
	
	g_key_file_free(config);
	return 0;
}

int kdk_system_set_12_timeformat()
{
	char *homeDir = NULL;
	GError *gerr = NULL;
	int status = 0;

	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		return -1;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/", filename);

	if (!realpath(path, canonical_filename))
	{
		if(!verify_file(canonical_filename))
		{
			return -1;
		}
		status = mkdir(canonical_filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status != 0) {
			return -1;
		}
	}
	
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);
	GKeyFile *config = g_key_file_new();
	char tpath[512] = {0};
	
	char *home = getlogin();
	
	snprintf(tpath, sizeof(tpath), "/var/lib/lightdm-data/%s/dateformat.conf", home);
	
	g_key_file_load_from_file(config, path, 0, NULL);		
	g_key_file_set_value(config, DATEFORMAT, "TIME_FORMAT", "12小时制");
	g_key_file_save_to_file(config, path, &gerr);

	g_key_file_save_to_file(config, tpath, &gerr);

	g_key_file_free(config);
	return 0;
}

char* kdk_system_get_now_dateformat()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *homeDir = NULL;
	char *value = NULL;
	char *tvalue = malloc(sizeof(char) * 128);
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(tvalue);
		return NULL;
	}
	char *lang = getenv("LANG");
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(tvalue);
			return NULL;
		}
	}

	GKeyFile *config = g_key_file_new();
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "**/**/**", 127);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		value = g_key_file_get_string(config, "DATEFORMAT", "DATE_FORMAT", NULL);
		if(value == NULL)
		{
			strncpy(tvalue, "**/**/**", 127);
			g_key_file_free(config);
			return tvalue;
		}
		else{
			strncpy(tvalue, value, 127);
		}

		if(strstr(value, "**年**月**日"))
		{
			if(strstr(lang, "en_US"))
			{
				strncpy(tvalue, "**year**mon**day", 127);
			}
            else if(strstr(lang, "kk") || strstr(lang, "ky"))
            {
                char *trnYear = gettext("year");
                char *trnMon = gettext("mon"); 
                char *trnDay = gettext("day");

                snprintf(tvalue, sizeof(char) * 128, "**%s**%s**%s", trnYear, trnMon, trnDay);
            }
            else{
				strncpy(tvalue, gettext("**year**mon**day"), 127);
			}
		}
		if(strstr(value, "*年*月*日"))
		{
			if(strstr(lang, "en_US"))
			{
				strncpy(tvalue, "*year*mon*day", 127);
			}
            else if(strstr(lang, "kk") || strstr(lang, "ky"))
            {
                char *trnYear = gettext("year");
                char *trnMon = gettext("mon"); 
                char *trnDay = gettext("day");

                snprintf(tvalue, sizeof(char) * 128, "*%s*%s*%s", trnYear, trnMon, trnDay);
            }
            else{
				strncpy(tvalue, gettext("*year*mon*day"), 127);
			}
		}
		fclose(fp);
		g_free(value);
	}
	g_key_file_free(config);
	return tvalue;
}

char* kdk_system_get_now_timeformat()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *homeDir = NULL;
	char value[128] = "\0";
	char *tvalue = malloc(sizeof(char) * 128);
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";
 
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(tvalue);
		return NULL;
	}
	char *lang = getenv("LANG");
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(tvalue);
			return NULL;
		}
	}
	
	GKeyFile *config = g_key_file_new();
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		if(strstr(lang, "en_US"))
		{
			strncpy(tvalue, "24-hour clock", 127);
		}else{
			strncpy(tvalue, gettext("24-hour clock"), 127);
		}
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
		if(gkey == NULL)
		{
			if(strstr(lang, "en_US"))
			{
				strncpy(tvalue, "24-hour clock", 127);
			}else{
				strncpy(tvalue, gettext("24-hour clock"), 127);
			}
		}
		else{
			strncpy(value, gkey, sizeof(value) - 1);
		}
		if(strstr(value, "12"))
		{
			if(strstr(lang, "en_US"))
			{
				strncpy(tvalue, "12-hour clock", 127);
			}else{
				strncpy(tvalue, gettext("12-hour clock"), 127);
			}
		}
		else if(strstr(value, "24")) 
		{
			if(strstr(lang, "en_US"))
			{
				strncpy(tvalue, "24-hour clock", 127);
			}else{
				strncpy(tvalue, gettext("24-hour clock"), 127);
			}
		}
		fclose(fp);
		g_free(gkey);
		g_key_file_free(config);
	}
	
	return tvalue;
}

kdk_dateinfo *kdk_system_tran_dateformat(struct tm *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *homeDir = NULL;
	char *value = (char *)malloc(sizeof(char) * 128);
	char *tvalue = (char *)malloc(sizeof(char) * 128);
	char tmp[128] = "\0";
	char tmpe[128] = "\0";
	char stmp[128] = "\0";
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);
	int ptr_year = ptr->tm_year;
	ptr->tm_year = ptr->tm_year - 1900;
	int ptr_mon = ptr->tm_mon;
	ptr->tm_mon = ptr->tm_mon - 1;

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			free(tvalue);
			return NULL;
		}
	}

	GKeyFile *config = g_key_file_new();
	kdk_dateinfo *res = (kdk_dateinfo *)calloc(1, sizeof(kdk_dateinfo));
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(value, "**/**/**", 127);
		strncpy(tvalue, "24小时制", 127);
	}
	else{
		res->date = malloc(sizeof(struct tm) + 1);
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(value, "**/**/**", 127);
		}
		else{
			strncpy(value, gkey, 127);
		}
		fclose(fp);
		g_free(gkey);
	}
	if(strstr(value, "*-*-*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", ptr_mon, ptr->tm_mday, ptr_year%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", ptr_year%100, ptr_mon, ptr->tm_mday);
		}
	}
	else if(strstr(value, "*/*/*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", ptr_mon, ptr->tm_mday, ptr_year%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", ptr_year%100, ptr_mon, ptr->tm_mday);
		}
	}
	else if(strstr(value, "*.*.*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", ptr_mon, ptr->tm_mday, ptr_year%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", ptr_year%100, ptr_mon, ptr->tm_mday);
		}
	}
	else if(strstr(value, "*年*月*日"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(ptr_mon);
			snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, ptr->tm_mday, ptr_year%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", ptr_year%100, trnYear, ptr_mon, trnMon, ptr->tm_mday, trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), ptr_year%100, ptr_mon, ptr->tm_mday);
		}
	}
	else if(strstr(value, "**-**-**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m-%d-%Y", ptr);
		}else{
			strftime(tmp, sizeof(tmp), "%Y-%m-%d", ptr);
		}
	}
	else if(strstr(value, "**/**/**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m/%d/%Y", ptr);
		}else{
			strftime(tmp, sizeof(tmp), "%Y/%m/%d", ptr);
		}
	}
	else if(strstr(value, "**.**.**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m.%d.%Y", ptr);
		}else{
			strftime(tmp, sizeof(tmp), "%Y.%m.%d", ptr);
		}
	}
	else if(strstr(value, "**年**月**日"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(ptr_mon);
			if(ptr->tm_mday < 10)
			{
				snprintf(tmp, sizeof(tmp), "%s 0%d, %d", mon, ptr->tm_mday, ptr_year);
			}else{
				snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, ptr->tm_mday, ptr_year);
			}
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            if(ptr->tm_mday < 10)
			{
                if((ptr->tm_mon + 1) < 10)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s0%d%s", ptr_year, trnYear, ptr_mon, trnMon, ptr->tm_mday, trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s0%d%s", ptr_year, trnYear, ptr_mon, trnMon, ptr->tm_mday, trnDay);
                }
                
			}else{
                if((ptr->tm_mon + 1) < 10)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s%d%s", ptr_year, trnYear, ptr_mon, trnMon, ptr->tm_mday, trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", ptr_year, trnYear, ptr_mon, trnMon, ptr->tm_mday, trnDay);
                }
			}
        }
        else{
			strftime(tmp, sizeof(tmp), gettext("%Y_year%m_mon%d_day"), ptr);
		}
	}

	strncpy(res->date, tmp, sizeof(struct tm));
	res->time = malloc(sizeof(struct tm) + 1);
	char *gkey = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
	if(gkey == NULL)
	{
		strncpy(tvalue, "24小时制", 127);
	}
	else{
		strncpy(tvalue, gkey, 127);
	}
	g_free(gkey);
	
	if(strstr(tvalue, "12小时制"))
	{
		int ap_time = 0;	//0为上午，1为下午
		if (ptr->tm_hour > 12) {
			ap_time = 1;
		} 
		else if (ptr->tm_hour == 12) {
			if (ptr->tm_min > 0 || ptr->tm_sec > 0) {
				ap_time = 1;
			} else {
				ap_time = 0;
			}
		} else {
			ap_time = 0;
		}

		if(strstr(lang, "en_US"))
		{
			if (ap_time == 0)
			{
				strftime(tmpe, sizeof(tmpe), "%I:%M AM", ptr);
				strftime(stmp, sizeof(stmp), "%I:%M:%S AM", ptr);
			}
			else{
				strftime(tmpe, sizeof(tmpe), "%I:%M PM", ptr);
				strftime(stmp, sizeof(stmp), "%I:%M:%S PM", ptr);
			}
		}else{
			if (ap_time == 0)
			{
				strftime(tmpe, sizeof(tmpe), gettext("am%I:%M"), ptr);
				strftime(stmp, sizeof(stmp), gettext("am%I:%M:%S"), ptr);
			}
			else{
				strftime(tmpe, sizeof(tmpe), gettext("pm%I:%M"), ptr);
				strftime(stmp, sizeof(stmp), gettext("pm%I:%M:%S"), ptr);
			}
		}
			
	}
	else if(strstr(tvalue, "24小时制"))
	{
		strftime(tmpe, sizeof(tmpe), "%H:%M", ptr);
		strftime(stmp, sizeof(stmp), "%H:%M:%S", ptr);
	}
	strncpy(res->time, tmpe, sizeof(struct tm));

	res->timesec = malloc(sizeof(struct tm) + 1);
	strncpy(res->timesec, stmp, sizeof(struct tm));
	
	g_key_file_free(config);
	free(tvalue);
	free(value);
	return res;
}

char* kdk_system_nowtime()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char tmp[128] = "\0";
	char *homeDir = NULL;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char *tvalue = (char*)malloc(sizeof(char) * 128);
	char *value = (char*)malloc(sizeof(char) * 128);
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			free(tvalue);
			return NULL;
		}
	}

	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "24小时制", 127);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(tvalue, "24小时制", 127);
		}
		else{
			strncpy(tvalue, gkey, 127);
		}
		fclose(fp);
		g_free(gkey);
	}
	if(strstr(tvalue, "24小时制"))
	{
		strftime(tmp, sizeof(tmp), "%H:%M", localtime(&current));
	}
	else if(strstr(tvalue, "12小时制"))
	{
		int ap_time = 0;	//0为上午，1为下午
		if (now->tm_hour > 12) {
			ap_time = 1;
		} 
		else if (now->tm_hour == 12) {
			if (now->tm_min > 0 || now->tm_sec > 0) {
				ap_time = 1;
			} else {
				ap_time = 0;
			}
		} else {
			ap_time = 0;
		}

		if(strstr(lang, "en_US"))
		{
			if (ap_time == 0)
			{
				strftime(tmp, sizeof(tmp), "%I:%M AM", localtime(&current));
			}else{
				strftime(tmp, sizeof(tmp), "%I:%M PM", localtime(&current));
			}
			
		}else{
			if (ap_time == 0)
			{
				strftime(tmp, sizeof(tmp), gettext("am%I:%M"), localtime(&current));
			}else{
				strftime(tmp, sizeof(tmp), gettext("pm%I:%M"), localtime(&current));
			}
		}
	}
	strncpy(value, tmp, 127);
	g_key_file_free(config);
	free(tvalue);
	return value;
}

char* kdk_system_nowdate()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char tmp[128] = "\0";
	char *homeDir = NULL;
	struct tm *p;
	time_t current;
	time(&current);
	p = localtime(&current);
	char *tvalue = malloc(sizeof(char) * 128);
	char *value = malloc(sizeof(char) * 128);
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			free(tvalue);
			return NULL;
		}
	}

	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "**/**/**", 127);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(tvalue, "**/**/**", 127);
		}
		else{
			strncpy(tvalue, gkey, 127);
		}
		fclose(fp);
		g_free(gkey);
	}
	if (strstr(tvalue, "**/**/**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m/%d/%Y", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), "%Y/%m/%d", localtime(&current));
		}
	}
	else if (strstr(tvalue, "*/*/*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", (1+p->tm_mon), (p->tm_mday), (1900 + p->tm_year)%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		}
	}
	else if (strstr(tvalue, "**-**-**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m-%d-%Y", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&current));
		}
	}
	else if (strstr(tvalue, "*-*-*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", (1+p->tm_mon), (p->tm_mday), (1900 + p->tm_year)%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		}
	}
	else if (strstr(tvalue, "**.**.**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%Y.%m.%d", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), "%m.%d.%Y", localtime(&current));
		}
	}
	else if (strstr(tvalue, "*.*.*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", (1+p->tm_mon), (p->tm_mday), (1900 + p->tm_year)%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		}
	}
	else if (strstr(tvalue, "**年**月**日"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(1+p->tm_mon);
			if(p->tm_mday > 9)
			{
				snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, (p->tm_mday), (1900 + p->tm_year));
			}else{
				snprintf(tmp, sizeof(tmp), "%s 0%d, %d", mon, (p->tm_mday), (1900 + p->tm_year));
			}
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            if(p->tm_mday > 9)
			{
                if((p->tm_mon + 1) > 9)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, p->tm_mday, trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, p->tm_mday, trnDay);
                }
                
			}else{
                if((p->tm_mon + 1) > 9)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s0%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, p->tm_mday, trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s0%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, p->tm_mday, trnDay);
                }
			}
        }
        else{
			strftime(tmp, sizeof(tmp), gettext("%Y_year%m_mon%d_day"), localtime(&current));
		}
	}
	else if (strstr(tvalue, "*年*月*日"))
	{
		if (strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(1+p->tm_mon);
			snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, (p->tm_mday), (1900 + p->tm_year)%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", (1900 + p->tm_year)%100, trnYear, (p->tm_mon + 1), trnMon, p->tm_mday, trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		}
	}
	
	strncpy(value, tmp, 127);
	g_key_file_free(config);
	free(tvalue);
	return value;
}

char* kdk_system_shortweek()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *tmpe = malloc(sizeof(char) * 128);
	char *lang = getenv("LANG");
	struct tm *p;
	time_t current;
	time(&current);
	p = localtime(&current);
	if(strstr(lang, "en_US"))
	{
		switch (p->tm_wday)
		{
		case 0:
			strncpy(tmpe, "Sun", 127);
			break;
		case 1:
			strncpy(tmpe, "Mon", 127);
			break;
		case 2:
			strncpy(tmpe, "Tue", 127);
			break;
		case 3:
			strncpy(tmpe, "Wed", 127);
			break;
		case 4:
			strncpy(tmpe, "Thu", 127);
			break;
		case 5:
			strncpy(tmpe, "Fri", 127);
			break;
		case 6:
			strncpy(tmpe, "Sat", 127);
			break;
		default:
			break;
		}
	}else{
		switch (p->tm_wday)
		{
		case 0:
			strncpy(tmpe, gettext("_Sun"), 127);
			break;
		case 1:
			strncpy(tmpe, gettext("_Mon"), 127);
			break;
		case 2:
			strncpy(tmpe, gettext("_Tue"), 127);
			break;
		case 3:
			strncpy(tmpe, gettext("_Wed"), 127);
			break;
		case 4:
			strncpy(tmpe, gettext("_Thu"), 127);
			break;
		case 5:
			strncpy(tmpe, gettext("_Fri"), 127);
			break;
		case 6:
			strncpy(tmpe, gettext("_Sat"), 127);
			break;
		default:
			break;
		}
	}
	return tmpe;
}

char* kdk_system_longweek()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *tmpe = (char *)malloc(sizeof(char) * 256);
	char *lang = getenv("LANG");
	struct tm *p;
	time_t current;
	time(&current);
	p = localtime(&current);
	if(strstr(lang, "en_US"))
	{
		switch (p->tm_wday)
		{
		case 0:
			strncpy(tmpe, "Sunday", 255);
			break;
		case 1:
			strncpy(tmpe, "Monday", 255);
			break;
		case 2:
			strncpy(tmpe, "Tuesday", 255);
			break;
		case 3:
			strncpy(tmpe, "Wednesday", 255);
			break;
		case 4:
			strncpy(tmpe, "Thursday", 255);
			break;
		case 5:
			strncpy(tmpe, "Friday", 255);
			break;
		case 6:
			strncpy(tmpe, "Saturday", 255);
			break;
		default:
			break;
		}
	}else{
		switch (p->tm_wday)
		{
		case 0:
			strncpy(tmpe, gettext("_Sunday"), 255);
			break;
		case 1:
			strncpy(tmpe, gettext("_Monday"), 255);
			break;
		case 2:
			strncpy(tmpe, gettext("_Tuesday"), 255);
			break;
		case 3:
			strncpy(tmpe, gettext("_Wednesday"), 255);
			break;
		case 4:
			strncpy(tmpe, gettext("_Thursday"), 255);
			break;
		case 5:
			strncpy(tmpe, gettext("_Friday"), 255);
			break;
		case 6:
			strncpy(tmpe, gettext("_Saturday"), 255);
			break;
		default:
			break;
		}
	}
	return tmpe;
}

char* kdk_system_loginweek()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *tmpe = (char *)malloc(sizeof(char) * 256);
	char *lang = getenv("LANG");
	struct tm *p;
	time_t current;
	time(&current);
	p = localtime(&current);
	if(strstr(lang, "en"))
	{
		switch (p->tm_wday)
		{
		case 0:
			strncpy(tmpe, "Sunday", 255);
			break;
		case 1:
			strncpy(tmpe, "Monday", 255);
			break;
		case 2:
			strncpy(tmpe, "Tuesday", 255);
			break;
		case 3:
			strncpy(tmpe, "Wednesday", 255);
			break;
		case 4:
			strncpy(tmpe, "Thursday", 255);
			break;
		case 5:
			strncpy(tmpe, "Friday", 255);
			break;
		case 6:
			strncpy(tmpe, "Saturday", 255);
			break;
		default:
			break;
		}
	}else{
		switch (p->tm_wday)
		{
		case 0:
			strncpy(tmpe, gettext("_Sunday"), 255);
			break;
		case 1:
			strncpy(tmpe, gettext("_Monday"), 255);
			break;
		case 2:
			strncpy(tmpe, gettext("_Tuesday"), 255);
			break;
		case 3:
			strncpy(tmpe, gettext("_Wednesday"), 255);
			break;
		case 4:
			strncpy(tmpe, gettext("_Thursday"), 255);
			break;
		case 5:
			strncpy(tmpe, gettext("_Friday"), 255);
			break;
		case 6:
			strncpy(tmpe, gettext("_Saturday"), 255);
			break;
		default:
			break;
		}
	}
	return tmpe;
}

char* kdk_system_second()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char tmp[128] = "\0";
	char *homeDir = NULL;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char *tvalue = (char *)malloc(sizeof(char) * 128);
	char *value = (char *)malloc(sizeof(char) * 128);
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			free(tvalue);
			return NULL;
		}
	}

	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "24小时制", 127);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(tvalue, "24小时制", 127);
		}
		else{
			strncpy(tvalue, gkey, 127);
		}
		fclose(fp);
		g_free(gkey);
	}
	if(strstr(tvalue, "24小时制"))
	{
		strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&current));
	}
	else if(strstr(tvalue, "12小时制"))
	{
		int ap_time = 0;	//0为上午，1为下午
		if (now->tm_hour > 12) {
			ap_time = 1;
		} 
		else if (now->tm_hour == 12) {
			if (now->tm_min > 0 || now->tm_sec > 0) {
				ap_time = 1;
			} else {
				ap_time = 0;
			}
		} else {
			ap_time = 0;
		}

		if(strstr(lang, "en_US"))
		{
			if (ap_time == 0)
			{
				strftime(tmp, sizeof(tmp), "%I:%M:%S AM", localtime(&current));
			}else{
				strftime(tmp, sizeof(tmp), "%I:%M:%S PM", localtime(&current));
			}
		}else{
			if(ap_time == 0)
			{
				strftime(tmp, sizeof(tmp), gettext("am%I:%M:%S"), localtime(&current));
			}else{
				strftime(tmp, sizeof(tmp), gettext("pm%I:%M:%S"), localtime(&current));
			}
		}
	}
	strncpy(value, tmp, 127);

	g_key_file_free(config);
	free(tvalue);
	return value;
}

kdk_logn_dateinfo *kdk_system_logn_dateinfo(char *user)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char value[64] = "\0"; 
	char *tvalue = NULL;
	char tmp[128] = "\0";
	char tmpe[128] = "\0";
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char *lang = getenv("LANG");
	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	snprintf(path, sizeof(path), "/var/lib/lightdm-data/%s/dateformat.conf", user);
	GKeyFile *config = g_key_file_new();
	kdk_logn_dateinfo *res = (kdk_logn_dateinfo *)calloc(1, sizeof(kdk_logn_dateinfo));
	
	res->time = malloc(sizeof(char) * 128);

	g_key_file_load_from_file(config, path, 0, NULL);
	tvalue = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
	if(tvalue == NULL)
	{
		strftime(tmp, sizeof(tmp), "%H:%M", localtime(&current));
	}else{
		if(strstr(tvalue, "24小时制"))
		{
			strftime(tmp, sizeof(tmp), "%H:%M", localtime(&current));
		}
		else if(strstr(tvalue, "12小时制"))
		{
			if(strstr(lang, "en_US"))
			{
				// strftime(tmp, sizeof(tmp), "%I:%M %p", localtime(&current));
				if (now->tm_hour >= 12)
				{
					strftime(tmp, sizeof(tmp), "%I:%M PM", localtime(&current));
				}else{
					strftime(tmp, sizeof(tmp), "%I:%M AM", localtime(&current));
				}
			}else{
				if(now->tm_hour >= 12)
				{
					strftime(tmp, sizeof(tmp), gettext("pm%I:%M"), localtime(&current));
				}else{
					strftime(tmp, sizeof(tmp), gettext("am%I:%M"), localtime(&current));
				}
			}
		}
	}
	strncpy(res->time, tmp, 127);
	res->date = malloc(128);
	char *gkey = g_key_file_get_string(config, "DATEFORMAT", "DATE_FORMAT", NULL);
	if(gkey == NULL)
	{
		strncpy(value, "**/**/**", 63);
	}
	else{
		strncpy(value, gkey, 63);
	}
	if (strstr(value, "**/**/**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmpe, sizeof(tmpe), "%m/%d/%Y", localtime(&current));
		}else{
			strftime(tmpe, sizeof(tmpe), "%Y/%m/%d", localtime(&current));
		}
	}
	else if (strstr(value, "*/*/*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmpe, sizeof(tmpe), "%d/%d/%d", (1+now->tm_mon), (now->tm_mday), (1900 + now->tm_year)%100);
		}else{
			snprintf(tmpe, sizeof(tmpe), "%d/%d/%d", (1900 + now->tm_year)%100, (1+now->tm_mon), (now->tm_mday));
		}
	}
	else if (strstr(value, "**-**-**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmpe, sizeof(tmpe), "%Y-%m-%d", localtime(&current));
		}else{
			strftime(tmpe, sizeof(tmpe), "%Y-%m-%d", localtime(&current));
		}
	}
	else if (strstr(value, "*-*-*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmpe, sizeof(tmpe), "%d-%d-%d", (1+now->tm_mon), (now->tm_mday), (1900 + now->tm_year)%100);
		}else{
			snprintf(tmpe, sizeof(tmpe), "%d-%d-%d", (1900 + now->tm_year)%100, (1+now->tm_mon), (now->tm_mday));
		}
	}
	else if (strstr(value, "**.**.**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmpe, sizeof(tmpe), "%m.%d.%Y", localtime(&current));
		}else{
			strftime(tmpe, sizeof(tmpe), "%Y.%m.%d", localtime(&current));
		}
	}
	else if (strstr(value, "*.*.*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmpe, sizeof(tmpe), "%d.%d.%d", (1+now->tm_mon), (now->tm_mday), (1900 + now->tm_year)%100);
		}else{
			snprintf(tmpe, sizeof(tmpe), "%d.%d.%d", (1900 + now->tm_year)%100, (1+now->tm_mon), (now->tm_mday));
		}
	}
	else if (strstr(value, "**年**月**日"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(now->tm_mon+1);
			snprintf(tmpe, sizeof(tmpe), "%s %d, %d", mon, now->tm_mday, (1900 + now->tm_year));
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmpe, sizeof(tmpe), "%d%s%d%s%d%s", (1900 + now->tm_year), trnYear, (now->tm_mon + 1), trnMon, now->tm_mday, trnDay);
        }
        else{
			strftime(tmpe, sizeof(tmpe), gettext("%Y_year%m_mon%d_day"), localtime(&current));
		}
	}
	else if (strstr(value, "*年*月*日"))
	{
		if (strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(now->tm_mon+1);
			snprintf(tmpe, sizeof(tmpe), "%s %d, %d", mon, now->tm_mday, (1900 + now->tm_year)%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmpe, sizeof(tmpe), "%d%s%d%s%d%s", (1900 + now->tm_year)%100, trnYear, (now->tm_mon + 1), trnMon, now->tm_mday, trnDay);
        }
        else{
			snprintf(tmpe, sizeof(tmpe), gettext("%d_year%d_mon%d_day"), (1900 + now->tm_year)%100, (1+now->tm_mon), (now->tm_mday));
		}
	}
	strncpy(res->date, tmpe, 127);
	
	char *week = kdk_system_longweek();
	res->week = malloc(128);
	strncpy(res->week, week, 127);
	free(week);
	g_key_file_free(config);
	return res;
}

char* kdk_system_gjx_time(char *date)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char tmp[128] = "\0";
	char dtmp[128] = "\0";
	strncpy(dtmp, date, sizeof(dtmp));
	char *homeDir = NULL;
	char *tvalue = (char *)malloc(sizeof(char) * 128);
	char *value = (char*)malloc(sizeof(char) * 128);
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			free(tvalue);
			return NULL;
		}
	}

	int i = 0;
	char *buf[16], *p = NULL;
	p = strtok(dtmp, "/");
	while(p)
	{
		buf[i] = p;
		i++;
		p = strtok(NULL,"/");
	}
	int year = atoi(buf[2]);
	int mon = atoi(buf[0]);
	int day = atoi(buf[1]);
	char *years = (char*)malloc(sizeof(char) * 32);
	char *mons = (char*)malloc(sizeof(char) * 32);
	char *days = (char*)malloc(sizeof(char) * 32);
	strncpy(years, buf[2], 31);
	strncpy(mons, buf[0], 31);
	strncpy(days, buf[1], 31);
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "**/**/**", 127);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(tvalue, "**/**/**", 127);
		}
		else{
			strncpy(tvalue, gkey, 127);
		}
		fclose(fp);
		g_free(gkey);
	}
	struct tm ptr;
	ptr.tm_year = year;
	ptr.tm_mon = mon;
	ptr.tm_yday = day;
	time_t timep;
	timep = mktime(&ptr);
	strncpy(dtmp, ctime(&timep), sizeof(dtmp));
	char *buff[16], *q = NULL;
	q = strtok(dtmp, " ");
	while(q)
	{
		buff[i] = q;
		i++;
		q = strtok(NULL," ");
	}

	if (strstr(tvalue, "**/**/**"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%s/%s/%s", mons, days, years);
		}else{
			snprintf(tmp, sizeof(tmp), "%s/%s/%s", years, mons, days);
		}
	}
	else if (strstr(tvalue, "*/*/*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", mon, day, year%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", year%100, mon, day);
		}
	}
	else if (strstr(tvalue, "**-**-**"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%s-%s-%s", mons , days, years);
		}else{
			snprintf(tmp, sizeof(tmp), "%s-%s-%s", years, mons , days);
		}
	}
	else if (strstr(tvalue, "*-*-*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", mon, day, year%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", year%100, mon, day);
		}
	}
	else if (strstr(tvalue, "**.**.**"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%s.%s.%s", mons, days, years);
		}else{
			snprintf(tmp, sizeof(tmp), "%s.%s.%s", years, mons, days);
		}
	}
	else if (strstr(tvalue, "*.*.*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", mon, day, year%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", year%100, mon, day);
		}
	}
	else if (strstr(tvalue, "**年**月**日"))
	{
		if(strstr(lang, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%s %s, %s", buff[1], days, years);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmp, sizeof(tmp), "%s%s%s%s%s%s", years, trnYear, mons, trnMon, days, trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%s_year%s_mon%s_day"), years, mons, days);
		}
	}
	else if (strstr(tvalue, "*年*月*日"))
	{
		if (strstr(lang, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%s %d, %d", buff[1], day, year%100);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", year%100, trnYear, mon, trnMon, day, trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), year%100, mon, day);
		}
	}
	strncpy(value, tmp, 127);
	free(years);
	free(mons);
	free(days);
	free(tvalue);
	return value;
}

inline void kdk_date_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}

void kdk_free_dateinfo(kdk_dateinfo *date)
{
    SAFE_FREE(date->date);
    SAFE_FREE(date->time);
	SAFE_FREE(date->timesec);
    SAFE_FREE(date);
}

void kdk_free_logn_dateinfo(kdk_logn_dateinfo *date)
{
    SAFE_FREE(date->date);
    SAFE_FREE(date->time);
	SAFE_FREE(date->week);
    SAFE_FREE(date);
}

int kdk_system_set_long_dateformat(char *format)
{
	int i = 0;
	char tmp[128] = {0};
	char *homeDir = NULL;
	strncpy(tmp, format, sizeof(tmp));
	GKeyFile *config = g_key_file_new();
	GError* gerr = NULL;
	int status = 0;
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";
	char *lang = getenv("LANG");

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		return -1;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/", filename);

	if (!realpath(path, canonical_filename))
	{
		if(!verify_file(canonical_filename))
		{
			return -1;
		}
		status = mkdir(canonical_filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status != 0) {
			return -1;
		}
	}

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	char tpath[128] = {0};
	
	char *home = getlogin();
	
	snprintf(tpath, sizeof(tpath), "/var/lib/lightdm-data/%s/dateformat.conf", home);
	
	g_key_file_load_from_file(config, path, 0, NULL);
	
	if(strstr(tmp, "年"))
	{
		int len =  strtok_date(tmp, "年", 0);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yyyy MM dd");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yy M d");
		}
	}
	else if(strstr(tmp, ","))
	{
		char *buf[3], *p = NULL;
		p = strtok(tmp, ",");
		while(p)
		{
			buf[i] = p;
			i++;
			p = strtok(NULL," ");
		}
		int len = strnlen(buf[1], STRNLEN_MAX);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yyyy MM dd");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yy M d");
		}
		else{
			goto err;
		}
	}
	else if(strstr(tmp, "ལོ།"))
	{
		char *p = NULL;
		p = strtok(tmp, "ལོ།");
		int len = strnlen(p, STRNLEN_MAX);
		
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yyyy MM dd");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yy M d");
		}
		else{
			goto err;
		}
	}
	else if(strstr(lang, "mn_MN"))
	{
		char *p = NULL;
		p = strtok(tmp, " ");
		strstripspace(p);
		int len = strnlen(p, STRNLEN_MAX);
		
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yyyy MM dd");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yy M d");
		}
		else{
			goto err;
		}
	}
	else if(strstr(lang, "kk") || strstr(lang, "ky") || strstr(lang, "ug"))
	{
		klog_debug("tmp = %s\n", tmp);
		char mon[32] = "\0", day[32] = "\0", year[32] = "\0";
		sscanf(tmp, "%s %*s %s %*s %s %*s", year, mon, day);
		klog_debug("day = %s, mon = %s, year = %s\n", day, mon, year);

		int len = strnlen(year, STRNLEN_MAX);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yyyy MM dd");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yy M d");
		}
		else{
			goto err;
		}
	}
	else if(strstr(lang, "de") || strstr(lang, "es") || strstr(lang, "fr"))
	{
		klog_debug("tmp = %s\n", tmp);
		char mon[32] = "\0", day[32] = "\0", year[32] = "\0";
		sscanf(tmp, "%s %*s %s %*s %s %*s", year, mon, day);
		klog_debug("day = %s, mon = %s, year = %s\n", day, mon, year);

		int len = strnlen(year, STRNLEN_MAX);
		if(len == 4)
		{
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yyyy MM dd");
		}
		else if(len == 2){
			g_key_file_set_value(config, DATEFORMAT, "LONG_DATE_FORMAT", "yy M d");
		}
		else{
			goto err;
		}
	}

	g_key_file_save_to_file(config, path, &gerr);

	g_key_file_save_to_file(config, tpath, &gerr);

	g_key_file_free(config);
	return 0;
err:
	g_key_file_free(config);
	
	return -1;
}

int kdk_system_set_short_dateformat(char *format)
{
	char tmp[64] = {0};
	char *homeDir = NULL;
	strncpy(tmp, format, sizeof(tmp));
	GKeyFile *config = g_key_file_new();
	GError* gerr = NULL;
	int status = 0;
	int mark = 0;
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";
	int sdate = 0;
	int len = 0;

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}

	if(strstr(env_time, "en_US"))
	{
		mark = 1;
	}
	else{
		mark = 0;
	}

	homeDir = getenv("HOME");
	if (!realpath(homeDir, canonical_filename) || !verify_file(canonical_filename))
	{
		return -1;
	}

	snprintf(path, sizeof(path), "%s/.config/kydate/", canonical_filename);
	if (!realpath(path, filename))
	{
		if(!verify_file(filename))
		{
			return -1;
		}
		status = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status != 0) {
			return -1;
		}
	}

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", canonical_filename);
	char tpath[512] = {0};
	
	char *home = getlogin();
	
	snprintf(tpath, sizeof(tpath), "/var/lib/lightdm-data/%s/dateformat.conf", home);
	
	g_key_file_load_from_file(config, path, 0, NULL);
	if(strstr(tmp, "/"))
	{
		len = strnlen(tmp, STRNLEN_MAX);
		sdate = strtok_short_date(tmp, "/");
		if(len == 10)
		{
			if(sdate == 4)
			{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "yyyy/MM/dd");
			}else{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "MM/dd/yyyy");
			}
		}
		else {
			
			if(sdate == 2 && mark == 0)
			{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "yy/M/d");
			}else{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "M/d/yy");
			}
		}
	}
	else if(strstr(tmp, "-"))
	{
		len = strnlen(tmp, STRNLEN_MAX);
		sdate = strtok_short_date(tmp, "-");
		if(len == 10)
		{
			if(sdate == 4)
			{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "yyyy-MM-dd");
			}else{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "MM-dd-yyyy");
			}
		}
		else {
			if(sdate == 2 && mark == 0)
			{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "yy-M-d");
			}else{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "M-d-yy");
			}
		}
	}
	else if(strstr(tmp, "."))
	{
		len = strnlen(tmp, STRNLEN_MAX);
		sdate = strtok_short_date(tmp, ".");
		if(len == 10)
		{
			if(sdate == 4)
			{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "yyyy.MM.dd");
			}else{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "MM.dd.yyyy");
			}
		}
		else {
			if(sdate == 2 && mark == 0)
			{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "yy.M.d");
			}else{
				g_key_file_set_value(config, DATEFORMAT, "SHORT_DATE_FORMAT", "M.d.yy");
			}
		}
	}
	g_key_file_save_to_file(config, path, &gerr);

	g_key_file_save_to_file(config, tpath, &gerr);

	g_key_file_free(config);
	return 0;
}

char* kdk_system_get_longformat_date()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char tmp[128] = "\0";
	char *homeDir = NULL;
	struct tm *p;
	time_t current;
	time(&current);
	p = localtime(&current);
	char tvalue[64] = "\0";
	char *value = malloc(sizeof(char) * 128);
	if(!value)
	{
		return NULL;
	}
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			return NULL;
		}
	}

	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "yyyy MM dd", 63);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "LONG_DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(tvalue, "yyyy MM dd", 63);
		}
		else{
			strncpy(tvalue, gkey, 63);
		}
		fclose(fp);
		g_free(gkey);
	}

	if (strstr(tvalue, "yyyy MM dd"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(1+p->tm_mon);
			if(p->tm_mday > 9)
			{
				snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, (p->tm_mday), (1900 + p->tm_year));
			}else{
				snprintf(tmp, sizeof(tmp), "%s 0%d, %d", mon, (p->tm_mday), (1900 + p->tm_year));
			}
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            if(p->tm_mday > 9)
			{
                if((p->tm_mon + 1) > 9)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
            }
            else{
                if((p->tm_mon + 1) > 9)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s0%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s0%d%s", (1900 + p->tm_year), trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
                }
            }
        }
        else{
			strftime(tmp, sizeof(tmp), gettext("%Y_year%m_mon%d_day"), localtime(&current));
		}
	}
	else if (strstr(tvalue, "yy M d"))
	{
		if (strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(1+p->tm_mon);
			snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, (p->tm_mday), (1900 + p->tm_year)%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", (1900 + p->tm_year)%100, trnYear, (p->tm_mon + 1), trnMon, (p->tm_mday), trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
		}
	}
	
	strncpy(value, tmp, 127);
	g_key_file_free(config);
	return value;
}

char* kdk_system_get_shortformat_date()
{
	char tmp[32];
	char *homeDir = NULL;
	struct tm *p;
	time_t current;
	time(&current);
	p = localtime(&current);
	char tvalue[64] = "\0";
	char *value = malloc(sizeof(char) * 64);
	if(!value)
	{
		return NULL;
	}
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(value);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(value);
			return NULL;
		}
	}	

	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		if(strstr(env_time, "en_US"))
		{
			strncpy(tvalue, "MM/dd/yyyy", 63);
		}
		else{
			strncpy(tvalue, "yyyy/MM/dd", 63);
		}
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "SHORT_DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			if(strstr(env_time, "en_US"))
			{
				strncpy(tvalue, "MM/dd/yyyy", 63);
			}
			else{
				strncpy(tvalue, "yyyy/MM/dd", 63);
			}
		}
		else{
			strncpy(tvalue, gkey, 63);
		}
		fclose(fp);
		g_free(gkey);
	}

	if (strstr(tvalue, "yyyy/MM/dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y/%m/%d", localtime(&current));
	}
	else if(strstr(tvalue, "MM/dd/yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m/%d/%Y", localtime(&current));
	}
	else if (strstr(tvalue, "yy/M/d"))
	{
		snprintf(tmp, sizeof(tmp), "%d/%d/%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
	}
	else if (strstr(tvalue, "M/d/yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d/%d/%d", (1+p->tm_mon), (p->tm_mday), (1900 + p->tm_year)%100);
	}
	
	else if (strstr(tvalue, "yyyy-MM-dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&current));
	}
	else if (strstr(tvalue, "MM-dd-yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m-%d-%Y", localtime(&current));
	}
	else if (strstr(tvalue, "yy-M-d"))
	{
		snprintf(tmp, sizeof(tmp), "%d-%d-%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
	}
	else if (strstr(tvalue, "M-d-yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d-%d-%d", (1+p->tm_mon), (p->tm_mday), (1900 + p->tm_year)%100);
	}
	
	else if (strstr(tvalue, "yyyy.MM.dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y.%m.%d", localtime(&current));
	}
	else if (strstr(tvalue, "MM.dd.yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m.%d.%Y", localtime(&current));
	}
	else if (strstr(tvalue, "yy.M.d"))
	{
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", (1900 + p->tm_year)%100, (1+p->tm_mon), (p->tm_mday));
	}
	else if (strstr(tvalue, "M.d.yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", (1+p->tm_mon), (p->tm_mday), (1900 + p->tm_year)%100);
	}
	
	strncpy(value, tmp, 63);
	g_key_file_free(config);
	return value;
}

char* kdk_system_get_longformat()
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *homeDir = NULL;
	char *value = NULL;
	char *tvalue = malloc(sizeof(char) * 32);
	if(!tvalue)
	{
		return NULL;
	}
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(tvalue);
		return NULL;
	}
	// char *lang = getenv("LANG");
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(tvalue);
			return NULL;
		}
	}

	GKeyFile *config = g_key_file_new();
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "yyyy MM dd", 31);
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		value = g_key_file_get_string(config, "DATEFORMAT", "LONG_DATE_FORMAT", NULL);
		if(value == NULL)
		{
			strncpy(tvalue, "yyyy MM dd", 31);
		}
		else{
			strncpy(tvalue, value, 31);
		}
		fclose(fp);
		g_free(value);
	}
	g_key_file_free(config);
	return tvalue;
}

char* kdk_system_get_shortformat()
{
	char *homeDir = NULL;
	char *value = NULL;
	char *tvalue = malloc(sizeof(char) * 32);
	if(!tvalue)
	{
		return NULL;
	}
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}

	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(tvalue);
		return NULL;
	}
	// char *lang = getenv("LANG");
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(tvalue);
			return NULL;
		}
	}

	GKeyFile *config = g_key_file_new();
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		if(strstr(env_time, "en_US"))
		{
			strncpy(tvalue, "MM/dd/yyyy", 31);
		}
		else{
			strncpy(tvalue, "yyyy/MM/dd", 31); 
		}
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		value = g_key_file_get_string(config, "DATEFORMAT", "SHORT_DATE_FORMAT", NULL);
		if(value == NULL)
		{
			if(strstr(env_time, "en_US"))
			{
				strncpy(tvalue, "MM/dd/yyyy", 31);
			}
			else{
				strncpy(tvalue, "yyyy/MM/dd", 31); 
			}
		}
		else{
			strncpy(tvalue, value, 31);
		}
		fclose(fp);
		g_free(value);
	}
	g_key_file_free(config);
	return tvalue;
}

char* kdk_system_longformat_transform(struct tm *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *homeDir = NULL;
	char value[32] = "\0";
	struct tm stPtr;
	char *tvalue = (char *)malloc(sizeof(char) * 128);
	if(!tvalue)
	{
		return NULL;
	}
	char tmp[128] = "\0";
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);
	int ptr_year = ptr->tm_year;
	// ptr->tm_year = ptr->tm_year - 1900;
	int ptr_mon = ptr->tm_mon;
	// ptr->tm_mon = ptr->tm_mon - 1;

	struct tm date;
	int year = ptr_year;
	date.tm_year = year - 1900;
	int mon = ptr_mon;
	date.tm_mon = mon - 1;
	date.tm_mday = ptr->tm_mday;

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			free(tvalue);
			return NULL;
		}
	}

	GKeyFile *config = g_key_file_new();
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		strncpy(value, "yyyy MM dd", 31); 
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "LONG_DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(value, "yyyy MM dd", 31);
		}
		else{
			strncpy(value, gkey, 31);
		}
		fclose(fp);
		g_free(gkey);
	}

	if(strstr(value, "yy M d"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(ptr_mon);
			snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, ptr->tm_mday, ptr_year%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");

            snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", ptr_year%100, trnYear, ptr_mon, trnMon, (ptr->tm_mday), trnDay);
        }
        else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), ptr_year%100, ptr_mon, ptr->tm_mday);
		}
	}
	else if(strstr(value, "yyyy MM dd"))
	{
		if(strstr(lang, "en_US"))
		{
			char *mon = en_long_mon(ptr_mon);
			if(ptr->tm_mday < 10)
			{
				snprintf(tmp, sizeof(tmp), "%s 0%d, %d", mon, ptr->tm_mday, ptr_year);
			}else{
				snprintf(tmp, sizeof(tmp), "%s %d, %d", mon, ptr->tm_mday, ptr_year);
			}
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");
            
            if(ptr->tm_mday < 10)
			{
                if((ptr->tm_mon + 1) < 10)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s0%d%s", ptr_year, trnYear, ptr_mon, trnMon, (ptr->tm_mday), trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s0%d%s", ptr_year, trnYear, ptr_mon, trnMon, (ptr->tm_mday), trnDay);
                }
                
            }
            else{
                if((ptr->tm_mon + 1) < 10)
                {
                    snprintf(tmp, sizeof(tmp), "%d%s0%d%s%d%s", ptr_year, trnYear, ptr_mon, trnMon, (ptr->tm_mday), trnDay);
                }
                else{
                    snprintf(tmp, sizeof(tmp), "%d%s%d%s%d%s", ptr_year, trnYear, ptr_mon, trnMon, (ptr->tm_mday), trnDay);
                }
            }
        }
        else{
			strftime(tmp, sizeof(tmp), gettext("%Y_year%m_mon%d_day"), &date);
		}
	}
	
	g_key_file_free(config);
	strncpy(tvalue, tmp, 127);
	return tvalue;
}

char* kdk_system_shortformat_transform(struct tm *ptr)
{
	char *homeDir = NULL;
	char value[32] = "\0";
	char *tvalue = (char *)malloc(sizeof(char) * 64);
	char tmp[64] = {0};
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";
	struct tm stPrt;

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		strncpy(tvalue, tmp, 63);
		return tvalue;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);
	int ptr_year = ptr->tm_year;
	int ptr_mon = ptr->tm_mon;
	
	struct tm date;
	int year = ptr_year;
	date.tm_year = year - 1900;
	int mon = ptr_mon;
	date.tm_mon = mon - 1;
	date.tm_mday = ptr->tm_mday;

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
		{
			strncpy(tvalue, tmp, 63);
			return tvalue;
		}
	}

	GKeyFile *config = g_key_file_new();
	FILE *fp = fopen(canonical_filename, "r");
	if(!fp)
	{
		if(strstr(env_time, "en_US"))
		{
			strncpy(value, "MM/dd/yyyy", 31);
		}
		else{
			strncpy(value, "yyyy/MM/dd", 31);
		}
	}
	else{
		g_key_file_load_from_file(config, canonical_filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "SHORT_DATE_FORMAT", NULL);
		if(gkey == NULL)
		{
			if(strstr(env_time, "en_US"))
			{
				strncpy(value, "MM/dd/yyyy", 31);
			}
			else{
				strncpy(value, "yyyy/MM/dd", 31);
			}
		}
		else{
			strncpy(value, gkey, 31);
		}
		fclose(fp);
		g_free(gkey);
	}

	if(strstr(value, "yy-M-d"))
	{
		snprintf(tmp, sizeof(tmp), "%d-%d-%d", ptr_year%100, ptr_mon, ptr->tm_mday);
	}
	else if (strstr(value, "M-d-yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d-%d-%d", ptr_mon, ptr->tm_mday, ptr_year%100);
	}
	
	else if(strstr(value, "yy/M/d"))
	{
		snprintf(tmp, sizeof(tmp), "%d/%d/%d", ptr_year%100, ptr_mon, ptr->tm_mday);
	}
	else if (strstr(value, "M/d/yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d/%d/%d", ptr_mon, ptr->tm_mday, ptr_year%100);
	}
	
	else if(strstr(value, "yy.M.d"))
	{
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", ptr_year%100, ptr_mon, ptr->tm_mday);
	}
	else if (strstr(value, "M.d.yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", ptr_mon, ptr->tm_mday, ptr_year%100);
	}
	
	
	else if(strstr(value, "yyyy-MM-dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y-%m-%d", &date);
	}
	else if (strstr(value, "MM-dd-yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m-%d-%Y", &date);
	}
	
	else if(strstr(value, "yyyy/MM/dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y/%m/%d", &date);
	}
	else if (strstr(value, "MM/dd/yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m/%d/%Y", &date);
	}
	
	else if(strstr(value, "yyyy.MM.dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y.%m.%d", &date);
	}
	else if (strstr(value, "MM.dd.yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m.%d.%Y", &date);
	}
	
	g_key_file_free(config);
	strncpy(tvalue, tmp, 63);
	return tvalue;
}

kdk_logn_dateinfo *kdk_system_login_lock_dateinfo(char *user)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char value[64] = "\0";
	char *tvalue = NULL;
	char tmp[128] = "\0";
	char tmpe[128] = "\0";
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char *lang = getenv("LANG");
	snprintf(path, sizeof(path), "/var/lib/lightdm-data/%s/dateformat.conf", user);
	GKeyFile *config = g_key_file_new();
	kdk_logn_dateinfo *res = (kdk_logn_dateinfo *)calloc(1, sizeof(kdk_logn_dateinfo));
	if (!res)
	{
		return NULL;
	}
	
	res->time = malloc(sizeof(char) * 128);
	if(!res->time)
	{
		free(res);
		return NULL;
	}

	g_key_file_load_from_file(config, path, 0, NULL);
	tvalue = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
	if(tvalue == NULL)
	{
		strftime(tmp, sizeof(tmp), "%H:%M", localtime(&current));
	}else{
		if(strstr(tvalue, "24小时制"))
		{
			strftime(tmp, sizeof(tmp), "%H:%M", localtime(&current));
		}
		else if(strstr(tvalue, "12小时制"))
		{
			int ap_time = 0; //0为上午，1为下午
			if (now->tm_hour > 12) {
				ap_time = 1;
			} 
			else if (now->tm_hour == 12) {
				if (now->tm_min > 0 || now->tm_sec > 0) {
					ap_time = 1;
				} else {
					ap_time = 0;
				}
			} else {
				ap_time = 0;
			}

			if(strstr(lang, "en"))
			{
				if (ap_time == 0)
				{
					strftime(tmp, sizeof(tmp), "%I:%M AM", localtime(&current));
				}else{
					strftime(tmp, sizeof(tmp), "%I:%M PM", localtime(&current));
				}
			}else{
				if(now->tm_hour >= 12)
				{
					strftime(tmp, sizeof(tmp), gettext("pm%I:%M"), localtime(&current));
				}else{
					strftime(tmp, sizeof(tmp), gettext("am%I:%M"), localtime(&current));
				}
			}
		}
	}
	g_free(tvalue);
	strncpy(res->time, tmp, 127);
	res->date = malloc(128);
	if(!res->date)
	{
		free(res);
		free(res->time);
		return NULL;
	}
	char *key = g_key_file_get_string(config, "DATEFORMAT", "LONG_DATE_FORMAT", NULL);
	if(key == NULL)
	{
		strncpy(value, "yyyy MM dd", 63);
	}
	else{
		strncpy(value, key, 63);
	}
	g_free(key);
	
	if (strstr(value, "yyyy MM dd"))
	{
		if(strstr(lang, "en"))
		{
			char *mon = en_long_mon(now->tm_mon+1);
			if(now->tm_mday > 9)
			{
				snprintf(tmpe, sizeof(tmpe), "%s %d, %d", mon, (now->tm_mday), (1900 + now->tm_year));
			}else{
				snprintf(tmpe, sizeof(tmpe), "%s 0%d, %d", mon, (now->tm_mday), (1900 + now->tm_year));
			}
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");
            
            if(now->tm_mday > 9)
			{
                if((now->tm_mon + 1) > 9)
			    {
                    snprintf(tmpe, sizeof(tmpe), "%d%s%d%s%d%s", (1900 + now->tm_year), trnYear, (now->tm_mon + 1), trnMon, (now->tm_mday), trnDay);
                }
                else{
                    snprintf(tmpe, sizeof(tmpe), "%d%s0%d%s%d%s", (1900 + now->tm_year), trnYear, (now->tm_mon + 1), trnMon, (now->tm_mday), trnDay);
                }
            }
            else{
                if((now->tm_mon + 1) > 9)
			    {
                    snprintf(tmpe, sizeof(tmpe), "%d%s%d%s0%d%s", (1900 + now->tm_year), trnYear, (now->tm_mon + 1), trnMon, (now->tm_mday), trnDay);
                }
                else{
                    snprintf(tmpe, sizeof(tmpe), "%d%s0%d%s0%d%s", (1900 + now->tm_year), trnYear, (now->tm_mon + 1), trnMon, (now->tm_mday), trnDay);
                }
            }
        }
        else{
			strftime(tmpe, sizeof(tmpe), gettext("%Y_year%m_mon%d_day"), localtime(&current));
		}
	}
	else if (strstr(value, "yy M d"))
	{
		if (strstr(lang, "en"))
		{
			char *mon = en_long_mon(now->tm_mon+1);
			snprintf(tmpe, sizeof(tmpe), "%s %d, %d", mon, now->tm_mday, (1900 + now->tm_year)%100);
			free(mon);
		}
        else if(strstr(lang, "kk") || strstr(lang, "ky"))
        {
            char *trnYear = gettext("year");
            char *trnMon = gettext("mon"); 
            char *trnDay = gettext("day");
            
            snprintf(tmpe, sizeof(tmpe), "%d%s%d%s%d%s", (1900 + now->tm_year)%100, trnYear, (now->tm_mon + 1), trnMon, (now->tm_mday), trnDay);
        }
        else{
			snprintf(tmpe, sizeof(tmpe), gettext("%d_year%d_mon%d_day"), (1900 + now->tm_year)%100, (1+now->tm_mon), (now->tm_mday));
		}
	}
	strncpy(res->date, tmpe, 127);
	
	char *week = kdk_system_loginweek();
	res->week = malloc(128);
	if(!res->week)
	{
		free(res);
		free(res->time);
		free(res->date);
		free(week);
		return NULL;
	}
	strncpy(res->week, week, 127);
	free(week);
	g_key_file_free(config);
	return res;
}

kdk_timeinfo *kdk_system_timeformat_transform(struct tm *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *homeDir = NULL;
	char *tvalue = (char *)malloc(sizeof(char) * 64);
	// char tmp[64] = "\0";
	char tmpe[128] = "\0";
	char stmp[128] = "\0";
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";

	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}
	char *lang = getenv("LANG");
	homeDir = getenv("HOME");
	if (!realpath(homeDir, canonical_filename) || !verify_file(canonical_filename))
	{
		free(tvalue);
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", canonical_filename);
	// memset(canonical_filename, 0, sizeof(canonical_filename));

	if (access(path, F_OK) == 0)
	{
		if (!realpath(path, filename) || !verify_file(filename))
		{
			free(tvalue);
			return NULL;
		}
	}

	GKeyFile *config = g_key_file_new();
	kdk_timeinfo *res = (kdk_timeinfo *)calloc(1, sizeof(kdk_timeinfo));
	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		strncpy(tvalue, "24小时制", 63);
	}
	else{
		g_key_file_load_from_file(config, filename, 0, NULL);
		char *gkey = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
		if(gkey == NULL)
		{
			strncpy(tvalue, "24小时制", 63);
		}
		else{
			strncpy(tvalue, gkey, 63);
		}
		fclose(fp);
		g_free(gkey);
	}
	res->time = malloc(sizeof(struct tm) + 1);
	
	if(strstr(tvalue, "12小时制"))
	{
		int ap_time = 0; //0为上午，1为下午
		if (ptr->tm_hour > 12) {
			ap_time = 1;
		} 
		else if (ptr->tm_hour == 12) {
			if (ptr->tm_min > 0 || ptr->tm_sec > 0) {
				ap_time = 1;
			} else {
				ap_time = 0;
			}
		} else {
			ap_time = 0;
		}

		if(strstr(lang, "en_US"))
		{
			if (ap_time == 0)
			{
				strftime(tmpe, sizeof(tmpe), "%I:%M AM", ptr);
				strftime(stmp, sizeof(stmp), "%I:%M:%S AM", ptr);
			}
			else{
				strftime(tmpe, sizeof(tmpe), "%I:%M PM", ptr);
				strftime(stmp, sizeof(stmp), "%I:%M:%S PM", ptr);
			}
		}else{
			if (ap_time == 0)
			{
				strftime(tmpe, sizeof(tmpe), gettext("am%I:%M"), ptr);
				strftime(stmp, sizeof(stmp), gettext("am%I:%M:%S"), ptr);
			}
			else{
				strftime(tmpe, sizeof(tmpe), gettext("pm%I:%M"), ptr);
				strftime(stmp, sizeof(stmp), gettext("pm%I:%M:%S"), ptr);
			}
		}
			
	}
	else if(strstr(tvalue, "24小时制"))
	{
		strftime(tmpe, sizeof(tmpe), "%H:%M", ptr);
		strftime(stmp, sizeof(stmp), "%H:%M:%S", ptr);
	}
	strncpy(res->time, tmpe, sizeof(struct tm));

	res->timesec = malloc(sizeof(struct tm) + 1);
	strncpy(res->timesec, stmp, sizeof(struct tm));
	
	g_key_file_free(config);
	free(tvalue);
	return res;
}

char* kdk_system_tran_absolute_date(struct tm *ptr)
{
	if(ptr->tm_year < 0 || ptr->tm_mon < 0 || ptr->tm_mday < 0 || ptr->tm_mon > 12 || ptr->tm_mday > 31)
	{
		return NULL;
	}
	int absYear = ptr->tm_year;
	int absMon = ptr->tm_mon;
	int absDay = ptr->tm_mday;
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *absDate = calloc(1,64);
	if(!absDate)
	{
		return NULL;
	}
	memset(absDate, 0, 64);
	char *trans = NULL;
	char *lang = getenv("LANG");
	time_t timep;
    time(&timep);
	struct tm *p;
	p = localtime(&timep);
	int ySpace = (p->tm_year + 1900) - absYear;
	int mSpace = (p->tm_mon + 1) - absMon;
	int dSpace = p->tm_mday - absDay;
	struct tm asbPtr;
	asbPtr.tm_year = absYear;
	asbPtr.tm_mon = absMon;
	asbPtr.tm_mday = absDay;
	if(ySpace > 0 || mSpace > 0 || dSpace > 1)
	{
		trans = kdk_system_shortformat_transform(&asbPtr);
		strncpy(absDate, trans, 63);
		free(trans);
	}
	else if(ySpace == 0 && mSpace == 0 && dSpace == 1)
	{
		if(strstr(lang, "zh_CN") || strstr(lang, "zh_HK"))
		{
			strncpy(absDate, gettext("Yesterday"), 63);
		}
		else
		{
			trans = kdk_system_shortformat_transform(&asbPtr);
			strncpy(absDate, trans, 63);
			free(trans);
		}		
	}
	else if(ySpace == 0 && mSpace == 0 && dSpace == 0)
	{
		if(strstr(lang, "zh_CN") || strstr(lang, "zh_HK"))
		{
			strncpy(absDate, gettext("Today"), 63);
		}
		else
		{
			trans = kdk_system_shortformat_transform(&asbPtr);
			strncpy(absDate, trans, 63);
			free(trans);
		}
	}
	else
	{
		free(absDate);
		return NULL;
	}

	return absDate;
}

char* kdk_system_tran_absolute_date_longformat(struct tm *ptr)
{
	if(ptr->tm_year <= 0 || ptr->tm_mon <= 0 || ptr->tm_mday <= 0 || ptr->tm_mon > 12 || ptr->tm_mday > 31)
	{
		return NULL;
	}

	if(ptr->tm_mon == 2 && (ptr->tm_mday == 30 || ptr->tm_mday == 31))
	{
		return NULL;
	}

	int absYear = ptr->tm_year;
	int absMon = ptr->tm_mon;
	int absDay = ptr->tm_mday;
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);
	char *absDate = calloc(1,64);
	if(!absDate)
	{
		return NULL;
	}
	memset(absDate, 0, 64);
	char *trans = NULL;
	char *lang = getenv("LANG");
	time_t timep;
    time(&timep);
	struct tm *p;
	p = localtime(&timep);
	int ySpace = (p->tm_year + 1900) - absYear;
	int mSpace = (p->tm_mon + 1) - absMon;
	int dSpace = p->tm_mday - absDay;
	struct tm asbPtr;
	asbPtr.tm_year = absYear;
	asbPtr.tm_mon = absMon;
	asbPtr.tm_mday = absDay;

	if(ySpace == 0 && mSpace == 0 && dSpace == 1)
	{
		if(strstr(lang, "zh_CN") || strstr(lang, "zh_HK"))
		{
			strncpy(absDate, gettext("Yesterday"), 63);
		}
		else
		{
			trans = kdk_system_longformat_transform(&asbPtr);
			strncpy(absDate, trans, 63);
			free(trans);
		}		
	}
	else if(ySpace == 0 && mSpace == 0 && dSpace == 0)
	{
		if(strstr(lang, "zh_CN") || strstr(lang, "zh_HK"))
		{
			strncpy(absDate, gettext("Today"), 63);
		}
		else
		{
			trans = kdk_system_longformat_transform(&asbPtr);
			strncpy(absDate, trans, 63);
			free(trans);
		}
	}
	else
	{
		trans = kdk_system_longformat_transform(&asbPtr);
		strncpy(absDate, trans, 63);
		free(trans);
	}

	return absDate;
}

void kdk_free_timeinfo(kdk_timeinfo *time)
{
    SAFE_FREE(time->time);
	SAFE_FREE(time->timesec);
    SAFE_FREE(time);
}
