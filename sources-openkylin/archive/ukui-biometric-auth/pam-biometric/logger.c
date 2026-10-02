/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int pam_enable_debug;
char *pam_log_prefix;

void pam_logger(char *format, ...)
{
    if(!pam_enable_debug){
        return;
    }

	va_list args;

	time_t t = time(NULL);
	char timestr[32] = {0};
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&t)); //产生"YYYYMMDD hh:mm:ss"格式的字符串。

    fprintf(stderr, "[%s] %s - ", pam_log_prefix, timestr);
	va_start(args, format); /* 初始化 args */
	vfprintf(stderr, format, args);
}
