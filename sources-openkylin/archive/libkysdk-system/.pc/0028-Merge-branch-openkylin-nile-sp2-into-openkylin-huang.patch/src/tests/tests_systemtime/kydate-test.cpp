/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

#include "../../systemtime/libkydate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"

extern char *strstr_stub (char *__haystack, const char *__needle)
{
    return "1";
}

extern char *getenv_stub (const char *__name)
{
	if(strcmp(__name, "LANG") == 0)
	{
		return "en_US";
	}
	else
		return nullptr;
}

extern char *getenv_stub1 (const char *__name)
{
	if(strcmp(__name, "LANG") == 0)
	{
		return "kk";
	}
	else
		return nullptr;
}

TEST(KdkDateTest, DateInfoArgs)
{
	int ret = kdk_system_set_dateformat("22/1/2");
	printf("ret = %d\n",ret);
	int date = kdk_system_set_12_timeformat();
	struct tm ptr;
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	kdk_dateinfo *test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	char *tt = kdk_system_get_now_timeformat();
	printf("tt = %s\n", tt);
	free(tt);

	char* value = kdk_system_nowtime();
	printf("value = %s\n", value);
	free(value);

	char* tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	char *se = kdk_system_second();	
	printf("se = %s\n",se);
	free(se);

	kdk_logn_dateinfo *logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	ptr.tm_hour = 19;
	ptr.tm_min = 3;
	ptr.tm_sec = 2;
	kdk_timeinfo *ti = kdk_system_timeformat_transform(&ptr);
	printf("time = %s, timesec = %s\n", ti->time, ti->timesec);
	kdk_free_timeinfo(ti);

	char* timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	kdk_logn_dateinfo *info = kdk_system_login_lock_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",info->date,info->time,info->week);
	kdk_free_logn_dateinfo(info);

	ret = kdk_system_set_dateformat("22.1.2");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("22年1月2日");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	char *dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202-11-12");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202/11/12");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202.11.12");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202年11月12日");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("December 9, 2019");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	ret = kdk_system_set_long_dateformat("2230年3月4日");
	char *ld = kdk_system_get_longformat_date();
	printf("long res = %s\n", ld);
	free(ld);

	char *lt = kdk_system_longformat_transform(&ptr);
	printf("longformat_transform = %s\n", lt);
	free(lt);

	ret = kdk_system_set_short_dateformat("22-5-3");
	char *sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	char *st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("22.5.3");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("2202/15/13");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("2202-15-13");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("2202.15.13");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ptr.tm_year = 2024;
	ptr.tm_mon = 12;
	ptr.tm_mday = 6;
	ptr.tm_hour = 14;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	char *absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);

	ptr.tm_year = 2024;
	ptr.tm_mon = 12;
	ptr.tm_mday = 5;
	ptr.tm_hour = 19;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);
}

TEST(KdkDateTest, DateInfo)
{
    char **res = kdk_system_get_dateformat();
    size_t index = 0;
    while (res[index])
    {
        printf("系统支持格式返回日期：%s\n",res[index]);
        index ++;
    }
	kdk_date_freeall(res);

    int ret = kdk_system_set_dateformat("22-1-2");
	printf("ret = %d\n",ret);
	int date = kdk_system_set_12_timeformat();

	ret = kdk_system_set_24_timeformat();
	printf("ret = %d\n",ret);
	
	char *dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);
	char *tt = kdk_system_get_now_timeformat();
	printf("tt = %s\n", tt);
	free(tt);

    struct tm ptr;
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	kdk_dateinfo *test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	char* value = kdk_system_nowtime();
	printf("value = %s\n", value);
	free(value);

	char* tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	char* sw = kdk_system_shortweek();
	printf("sw = %s\n",sw);
	free(sw);
	
	char* lw = kdk_system_longweek();
	printf("lw = %s\n",lw);
	free(lw);

	char *se = kdk_system_second();	
	printf("se = %s\n",se);
	free(se);

	kdk_logn_dateinfo *logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	char* timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_long_dateformat("22年3月4日");
	ret = kdk_system_set_short_dateformat("22/5/3");

	char *ld = kdk_system_get_longformat_date();
	printf("long res = %s\n", ld);
	free(ld);
	
	char *sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	char *ll = kdk_system_get_longformat();
	printf("longformat = %s\n", ll);
	free(ll);

	char *ss = kdk_system_get_shortformat();
	printf("shortformat = %s\n", ss);
	free(ss);

	// struct tm ptr;
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;

	char *lt = kdk_system_longformat_transform(&ptr);
	printf("longformat_transform = %s\n", lt);
	free(lt);

	ptr.tm_year = 2022;
	ptr.tm_mon = 3;
	ptr.tm_mday = 2;

	char *st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	kdk_logn_dateinfo *info = kdk_system_login_lock_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",info->date,info->time,info->week);
	kdk_free_logn_dateinfo(info);

	ptr.tm_hour = 19;
	ptr.tm_min = 3;
	ptr.tm_sec = 2;
	kdk_timeinfo *ti = kdk_system_timeformat_transform(&ptr);
	printf("time = %s, timesec = %s\n", ti->time, ti->timesec);
	kdk_free_timeinfo(ti);

	// struct tm ptr;	

	ptr.tm_year = 2024;
	ptr.tm_mon = 3;
	ptr.tm_mday = 28;
	ptr.tm_hour = 14;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	char *absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);

	ptr.tm_year = 2025;
	ptr.tm_mon = 3;
	ptr.tm_mday = 13;
	ptr.tm_hour = 19;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);

	ptr.tm_year = 2025;
	ptr.tm_mon = 3;
	ptr.tm_mday = 12;
	ptr.tm_hour = 19;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);

	// free(tvalue);
}

TEST(KdkDateTest, DateInfoStub)
{
	Stub st1;
	char* (*original_strstr)(char*, const char*) = &strstr;
	st1.set(original_strstr, strstr_stub);


	int ret = kdk_system_set_dateformat("22/1/2");
	printf("ret = %d\n",ret);
	struct tm ptr;
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	kdk_dateinfo *test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	char **res = kdk_system_get_dateformat();
    size_t index = 0;
    while (res[index])
    {
        printf("系统支持格式返回日期：%s\n",res[index]);
        index ++;
    }
	kdk_date_freeall(res);

	char *dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);

	char* tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	char* sw = kdk_system_shortweek();
	printf("sw = %s\n",sw);
	free(sw);
	
	char* lw = kdk_system_longweek();
	printf("lw = %s\n",lw);
	free(lw);

	ret = kdk_system_set_dateformat("22/1/2");
	printf("ret = %d\n",ret);
	int date = kdk_system_set_12_timeformat();
	// struct tm ptr;
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	char *tt = kdk_system_get_now_timeformat();
	printf("tt = %s\n", tt);
	free(tt);

	char* value = kdk_system_nowtime();
	printf("value = %s\n", value);
	free(value);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	char *se = kdk_system_second();	
	printf("se = %s\n",se);
	free(se);

	kdk_logn_dateinfo *logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	ptr.tm_hour = 19;
	ptr.tm_min = 3;
	ptr.tm_sec = 2;
	kdk_timeinfo *ti = kdk_system_timeformat_transform(&ptr);
	printf("time = %s, timesec = %s\n", ti->time, ti->timesec);
	kdk_free_timeinfo(ti);

	char* timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	kdk_logn_dateinfo *info = kdk_system_login_lock_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",info->date,info->time,info->week);
	kdk_free_logn_dateinfo(info);

	ret = kdk_system_set_dateformat("22.1.2");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("22年1月2日");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202-11-12");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202/11/12");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202.11.12");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("2202年11月12日");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	kdk_free_dateinfo(test);

	dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	logn =  kdk_system_logn_dateinfo("kylin");
	printf("date = %s,time = %s,week = %s\n",logn->date,logn->time,logn->week);
	kdk_free_logn_dateinfo(logn);

	timt = kdk_system_gjx_time("08/01/2022");
    printf("timt = %s\n", timt);
	free(timt);

	ret = kdk_system_set_dateformat("December 9, 2019");
	printf("ret = %d\n",ret);
	date = kdk_system_set_24_timeformat();
	
	ptr.tm_year = 2322;
	ptr.tm_mon = 9;
	ptr.tm_mday = 3;
	ptr.tm_hour = 23;
	ptr.tm_min = 50;
	ptr.tm_sec = 21;
	ptr.tm_isdst = 0;
	printf("%d\n", ptr.tm_year);
	printf("test\n");
	test = kdk_system_tran_dateformat(&ptr);
	printf("%s ;%s ; %s\n",test->date, test->time, test->timesec);
	// kdk_free_dateinfo(test);

	tvalue = kdk_system_nowdate();
	printf("value = %s\n", tvalue);
	free(tvalue);

	ret = kdk_system_set_long_dateformat("2230年3月4日");
	char *ld = kdk_system_get_longformat_date();
	printf("long res = %s\n", ld);
	free(ld);

	char *lt = kdk_system_longformat_transform(&ptr);
	printf("longformat_transform = %s\n", lt);
	free(lt);

	ret = kdk_system_set_short_dateformat("22-5-3");
	char *sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	char *st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("22.5.3");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("2202/15/13");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("2202-15-13");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ret = kdk_system_set_short_dateformat("2202.15.13");
	sd = kdk_system_get_shortformat_date();
	printf("short res = %s\n", sd);
	free(sd);

	st = kdk_system_shortformat_transform(&ptr);
	printf("shortformat_transform = %s\n", st);
	free(st);

	ptr.tm_year = 2024;
	ptr.tm_mon = 12;
	ptr.tm_mday = 6;
	ptr.tm_hour = 14;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	char *absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);

	ptr.tm_year = 2024;
	ptr.tm_mon = 12;
	ptr.tm_mday = 5;
	ptr.tm_hour = 19;
	ptr.tm_min = 53;
	ptr.tm_sec = 21;
	absDate = kdk_system_tran_absolute_date(&ptr);
	printf("absDate = %s\n", absDate);
	free(absDate);

	dt = kdk_system_get_now_dateformat();
	printf("tt = %s\n", dt);
	free(dt);
}

TEST(KdkDateTest, DateInfoStub1)
{
	Stub st1;
	st1.set(getenv, getenv_stub);
	char **res = kdk_system_get_dateformat();
    size_t index = 0;
    while (res[index])
    {
        printf("系统支持格式返回日期：%s\n",res[index]);
        index ++;
    }
	// kdk_date_freeall(res);	
}