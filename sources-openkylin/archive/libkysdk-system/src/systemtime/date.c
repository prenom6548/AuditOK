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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "libkydate.h"
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/inotify.h>
#include <libintl.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/types.h>

char path[512] = {0};	//用户修改后配置文件路径
#define PATH_MAX_H 4096

#define LOCALEDIR "/usr/share/locale/"
#define GETTEXT_PACKAGE "kysdk-date"

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif


pthread_mutex_t lock;
u_int8_t g_Flag; // 控制启用常驻定时器还是临时定时器

u_int8_t g_Quit; //退出信号
sem_t g_Wait;

u_int8_t g_DateChanged; // 发生了文件变更

const char *server_introspection_xml =
	DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
	"<node>\n"

	"  <interface name='org.freedesktop.DBus.Introspectable'>\n"
	"    <method name='Introspect'>\n"
	"      <arg name='data' type='s' direction='out' />\n"
	"    </method>\n"
	"  </interface>\n"

	"  <interface name='org.freedesktop.DBus.Properties'>\n"
	"    <method name='Get'>\n"
	"      <arg name='interface' type='s' direction='in' />\n"
	"      <arg name='property'  type='s' direction='in' />\n"
	"      <arg name='value'     type='s' direction='out' />\n"
	"    </method>\n"
	"    <method name='GetAll'>\n"
	"      <arg name='interface'  type='s'     direction='in'/>\n"
	"      <arg name='properties' type='a{sv}' direction='out'/>\n"
	"    </method>\n"
	"  </interface>\n"

	"  <interface name='com.kylin.kysdk.DateInterface'>\n"
	"    <signal name='DateSignal'>\n"
	"       <arg name='date' type='s' />\n"
	"    </signal>"
	"    <signal name='LongDateSignal'>\n"
	"       <arg name='date' type='s' />\n"
	"    </signal>"
	"    <signal name='ShortDateSignal'>\n"
	"       <arg name='date' type='s' />\n"
	"    </signal>"
	"    <signal name='TimeSignal'>\n"
	"       <arg name='date' type='s' />\n"
	"    </signal>"
	"  </interface>\n"

	"</node>\n";

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
}

static void *printDate(DBusConnection *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char value[64] = "\0";
	char tvalue[64] = "\0";
	char tmpvalue[64] = "\0";
	char tmp[40] = {0};
	GKeyFile *config = g_key_file_new();
	char *lang = getenv("LANG");
	char canonical_filename[PATH_MAX_H] = "\0";
	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}

	// if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
	// 	goto out;

	g_key_file_load_from_file(config, path, 0, NULL);

	char *gkey = g_key_file_get_string(config, "DATEFORMAT", "DATE_FORMAT", NULL);
	if(gkey == NULL)
	{
		strncpy(value, "**/**/**", sizeof(value) - 1);
	}
	else{
		strncpy(value, gkey, sizeof(value) - 1);
	}

	strncpy(tmpvalue, value, sizeof(tmpvalue) - 1);
	if(strstr(value, "*-*-*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", now->tm_mon+1, now->tm_mday, (now->tm_year)%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d-%d-%d", (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
		}
	}
	if(strstr(value, "*/*/*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", now->tm_mon+1, now->tm_mday, (now->tm_year)%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d/%d/%d", (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
		}
	}
	if(strstr(value, "*.*.*"))
	{
		if(strstr(env_time, "en_US"))
		{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", now->tm_mon+1, now->tm_mday, (now->tm_year)%100);
		}else{
			snprintf(tmp, sizeof(tmp), "%d.%d.%d", (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
		}
	}
	if(strstr(value, "*年*月*日"))
	{
		if(strstr(lang, "en_US"))
		{
			char tdate[20];
			strftime(tdate, sizeof(tdate), "%b", localtime(&current));
			snprintf(tmp, sizeof(tmp), "%s %d,%d", tdate, now->tm_mday, (1900 + now->tm_year)%100 );
			strncpy(tmpvalue, "*year*mon*day", sizeof(tmpvalue) - 1);
		}else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
			strncpy(tmpvalue, gettext("*year*mon*day"), sizeof(tmpvalue) - 1);
		}
	}

	if(strstr(value, "**-**-**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m-%d-%Y", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&current));
		}
	}
	if(strstr(value, "**/**/**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m/%d/%Y", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), "%Y/%m/%d", localtime(&current));
		}
	}
	if(strstr(value, "**.**.**"))
	{
		if(strstr(env_time, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%m.%d.%Y", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), "%Y.%m.%d", localtime(&current));
		}
	}
	if(strstr(value, "**年**月**日"))
	{
		if(strstr(lang, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%b %d,%Y", localtime(&current));
			strncpy(tmpvalue, "**year**mon**day", sizeof(tmpvalue) - 1);
		}else{
			strftime(tmp, sizeof(tmp), gettext("%Y_year%m_mon%d_day"), localtime(&current));
			strncpy(tmpvalue, gettext("**year**mon**day"), sizeof(tmpvalue) - 1);
		}
	}

	char *gvalue = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
	if(gvalue == NULL)
	{
		strncpy(value, "24", sizeof(value) - 1);
	}
	else{
		strncpy(value, gvalue, sizeof(value) - 1);
	}
	
	if(strstr(value, "24"))
	{
		if(strstr(lang, "en_US"))
		{
			strncpy(tvalue, "24-hour clock", sizeof(tvalue) - 1);
		}else{
			strncpy(tvalue, gettext("24-hour clock"), sizeof(tvalue) - 1);
		}
	}else{
		if(strstr(lang, "en_US"))
		{
			strncpy(tvalue, "12-hour clock", sizeof(tvalue) - 1);
		}else{
			strncpy(tvalue, gettext("12-hour clock"), sizeof(tvalue) - 1);
		}
	}

	if (g_DateChanged == 1)
	{
		char *buf = calloc(1, 128);
		snprintf(buf, 128, "%s %s %s", tmpvalue, tmp, tvalue);

		msg = dbus_message_new_signal("/com/kylin/kysdk/Date",
									"com.kylin.kysdk.DateInterface",
									"DateSignal");
		if (NULL == msg)
		{
			fprintf(stderr, "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &buf);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		free(buf);
		g_key_file_free(config);
	}
out:
	return NULL;
}

static void print_longDate(DBusConnection *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char value[64] = "\0";
	char *tvalue = (char *)malloc(sizeof(char) * 64);
	char *tmpvalue = (char *)malloc(sizeof(char) * 64);
	if(!tvalue || !tmpvalue)
	{
		goto out;
	}
	char tmp[64] = "\0";
	GKeyFile *config = g_key_file_new();
	char *lang = getenv("LANG");
	char canonical_filename[PATH_MAX_H] = "\0";

	g_key_file_load_from_file(config, path, 0, NULL);

	char *gkey = g_key_file_get_string(config, "DATEFORMAT", "LONG_DATE_FORMAT", NULL);
	if(gkey == NULL)
	{
		strncpy(value, "yyyy MM dd", sizeof(value) - 1);
	}
	else{
		strncpy(value, gkey, sizeof(value) - 1);
	}

	strncpy(tmpvalue, value, 63);
	
	if(strstr(value, "yy M d"))
	{
		if(strstr(lang, "en_US"))
		{
			char tdate[20];
			strftime(tdate, sizeof(tdate), "%b", localtime(&current));
			snprintf(tmp, sizeof(tmp), "%s %d,%d", tdate, now->tm_mday, (1900 + now->tm_year)%100 );
		}else{
			snprintf(tmp, sizeof(tmp), gettext("%d_year%d_mon%d_day"), (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
		}
	}

	if(strstr(value, "yyyy MM dd"))
	{
		if(strstr(lang, "en_US"))
		{
			strftime(tmp, sizeof(tmp), "%b %d,%Y", localtime(&current));
		}else{
			strftime(tmp, sizeof(tmp), gettext("%Y_year%m_mon%d_day"), localtime(&current));
		}
	}
	strncpy(tvalue, tmp, 63);

	if (g_DateChanged == 1)
	{
		msg = dbus_message_new_signal("/com/kylin/kysdk/Date",
									"com.kylin.kysdk.DateInterface",
									"LongDateSignal");
		if (NULL == msg)
		{
			fprintf(stderr, "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &tmpvalue);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &tvalue);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		g_key_file_free(config);
	}
out:
	if(tmpvalue)
		free(tmpvalue);
	if(tvalue)
		free(tvalue);
	return ;
}

static void print_shortDate(DBusConnection *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char value[64] = "\0";
	char tvalue[64] = "\0";
	char *tmpvalue = (char *)malloc(sizeof(char) * 64);
	char tmp[64] = "\0";
	char *str_tmp = (char *)malloc(sizeof(char) * 64);
	if(!tmpvalue || !str_tmp)
	{
		goto out;
	}
	GKeyFile *config = g_key_file_new();
	char canonical_filename[PATH_MAX_H] = "\0";
	char *lang = getenv("LANG");
	char *env_time = getenv("LC_TIME");
	if(!env_time)
	{
		env_time = " ";
	}

	// if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
	// 	goto out;

	g_key_file_load_from_file(config, path, 0, NULL);

	char *gkey = g_key_file_get_string(config, "DATEFORMAT", "SHORT_DATE_FORMAT", NULL);
	if(gkey == NULL)
	{
		if(strstr(env_time, "en_US"))
		{
			strncpy(value, "MM/dd/yyyy", sizeof(value) - 1);
		}
		else{
			strncpy(value, "yyyy/MM/dd", sizeof(value) - 1);
		}
	}
	else{
		strncpy(value, gkey, sizeof(value) - 1);
	}

	strncpy(tmpvalue, value, 63);
	if(strstr(value, "yy-M-d"))
	{
		snprintf(tmp, sizeof(tmp), "%d-%d-%d", (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
	}
	else if(strstr(value, "M-d-yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d-%d-%d", now->tm_mon+1, now->tm_mday, (now->tm_year)%100);
	}

	else if(strstr(value, "yy/M/d"))
	{
		snprintf(tmp, sizeof(tmp), "%d/%d/%d", (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
	}
	else if(strstr(value, "M/d/yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d/%d/%d", now->tm_mon+1, now->tm_mday, (now->tm_year)%100);
	}
	
	if(strstr(value, "yy.M.d"))
	{
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", (now->tm_year)%100, now->tm_mon+1, now->tm_mday);
	}
	else if(strstr(value, "M.d.yy"))
	{
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", now->tm_mon+1, now->tm_mday, (now->tm_year)%100);
	}

	if(strstr(value, "yyyy-MM-dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&current));
	}
	else if(strstr(value, "MM-dd-yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m-%d-%Y", localtime(&current));
	}

	if(strstr(value, "yyyy/MM/dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y/%m/%d", localtime(&current));
	}
	else if(strstr(value, "MM/dd/yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m/%d/%Y", localtime(&current));
	}

	if(strstr(value, "yyyy.MM.dd"))
	{
		strftime(tmp, sizeof(tmp), "%Y.%m.%d", localtime(&current));
	}
	else if(strstr(value, "MM.dd.yyyy"))
	{
		strftime(tmp, sizeof(tmp), "%m.%d.%Y", localtime(&current));
	}

	strncpy(str_tmp, tmp, 63);

	if (g_DateChanged == 1)
	{
		msg = dbus_message_new_signal("/com/kylin/kysdk/Date",
									"com.kylin.kysdk.DateInterface",
									"ShortDateSignal");
		if (NULL == msg)
		{
			fprintf(stderr, "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &tmpvalue);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str_tmp);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		g_key_file_free(config);
	}
out:
	if(tmpvalue)
		free(tmpvalue);
	if(str_tmp)
		free(str_tmp);
	return ;
}

static void print_Time(DBusConnection *ptr)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	char value[64] = "\0";
	char tvalue[64] = "\0";
	char tmp[64] = "\0";
	char *str_tmp = (char *)malloc(sizeof(char) * 64);
	if(!str_tmp)
	{
		goto out;
	}
	GKeyFile *config = g_key_file_new();
	char *lang = getenv("LANG");

	g_key_file_load_from_file(config, path, 0, NULL);
	char *gvalue = g_key_file_get_string(config, "DATEFORMAT", "TIME_FORMAT", NULL);
	if(gvalue == NULL)
	{
		strncpy(value, "24", sizeof(value) - 1);
	}
	else{
		strncpy(value, gvalue, sizeof(value) - 1);
	}
	
	if(strstr(value, "24"))
	{
		if(strstr(lang, "en_US"))
		{
			strncpy(tvalue, "24-hour clock", sizeof(tvalue) - 1);
		}else{
			strncpy(tvalue, gettext("24-hour clock"), sizeof(tvalue) - 1);
		}
	}else{
		if(strstr(lang, "en_US"))
		{
			strncpy(tvalue, "12-hour clock", sizeof(tvalue) - 1);
		}else{
			strncpy(tvalue, gettext("12-hour clock"), sizeof(tvalue) - 1);
		}
	}
	if(tvalue[0] != '\0')
	{
		strncpy(str_tmp, tvalue, 63); 
	}
	else{
		free(str_tmp);
		return;
	}

	if (g_DateChanged == 1)
	{
		msg = dbus_message_new_signal("/com/kylin/kysdk/Date",
									"com.kylin.kysdk.DateInterface",
									"TimeSignal");
		if (NULL == msg)
		{
			fprintf(stderr, "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str_tmp);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		g_key_file_free(config);
	}
out:
	if(str_tmp)
		free(str_tmp);
	return ;
}

int monitorSystemDateChange(int fd){

	char *homeDir = NULL;
	char *realpath_res = NULL;
	char filename[PATH_MAX_H] = "\0";
 
	homeDir = getenv("HOME");
	if (!realpath(homeDir, filename) || !verify_file(filename))
	{
		return -1;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", filename);

	char buf[BUFSIZ];
	buf[sizeof(buf) - 1] = 0;
	struct inotify_event *event;

	if (fd < 0)
	{
		return -1;
	}
	int ret = read(fd, buf, sizeof(buf) - 1);
	event = (struct inotify_event *)&buf[0];
	if (ret)
	{
		fprintf(stdout, "%s --- %s\n", event->name, "IN_DELETE | IN_MODIFY");
		if (event->mask & (IN_DELETE | IN_DELETE_SELF))
		{
			int fdate = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF );
		}
		return 1;
	}
	return 0;
}

void *actionDateChanged(void *ptr)
{
	DBusConnection *conn = NULL;
	conn = ptr;
	char strpath[512] = "\0";
	char canonical_filename[PATH_MAX_H] = "\0";
	char filename[PATH_MAX_H] = "\0";
	char fname[PATH_MAX_H] = "\0";

	int fd = inotify_init();
	
	char *homeDir = NULL;

	homeDir = getenv("HOME");
	if (!realpath(homeDir, fname) || !verify_file(fname))
	{
		return NULL;
	}
	snprintf(path, sizeof(path), "%s/.config/kydate/dateformat.conf", fname);
	snprintf(strpath, sizeof(strpath), "%s/.config/kydate/", fname);
	if (access(strpath, F_OK) != 0)
	{
		if (!realpath(strpath, filename))
		{
			if(!verify_file(filename))
			{
				close(fd);
				return NULL;
			}
			int status = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (status != 0) {
				close(fd);
				return NULL;
			}
		}
		// if(!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        // {
		// 	close(fd);
        //     return NULL;
        // }
		if (!realpath(path, canonical_filename))
		{
			if(!verify_file(canonical_filename))
			{
				close(fd);
				return NULL;
			}
		}

		FILE *fp = fopen(canonical_filename, "w");
		if(!fp)
		{
			close(fd);
			return NULL;
		}
		fclose(fp);
	}
	else{
		if(access(path, F_OK) != 0)
		{
			// if(!realpath(path, canonical_filename) || !verify_file(canonical_filename))
			// {
			// 	close(fd);
			// 	return NULL;
			// }
			if (!realpath(path, canonical_filename))
			{
				if(!verify_file(canonical_filename))
				{
					close(fd);
					return NULL;
				}
			}
			FILE *fp = fopen(canonical_filename, "w");
			if(!fp)
			{
				close(fd);
				return NULL;
			}
			fclose(fp);
		}
	}

	int fdate = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF );
	if (fdate < 0)
	{
		close(fd);
		return NULL;
	}
	while (!g_Quit)
	{
		if (monitorSystemDateChange(fd) == 1)
		{
			g_DateChanged = 1;
			printDate(conn);
			print_longDate(conn);
			print_shortDate(conn);
			print_Time(conn);
		}
	}
	inotify_rm_watch(fd, fdate);
	close(fd);
	return NULL;
}

const char *version = "0.1";
GMainLoop *mainloop;

/*
 * This implements 'Get' method of DBUS_INTERFACE_PROPERTIES so a
 * client can inspect the properties/attributes of 'TestInterface'.
 */
DBusHandlerResult kdk_date_server_get_properties_handler(const char *property, DBusConnection *conn, DBusMessage *reply)
{
	if (!strcmp(property, "Version"))
	{
		dbus_message_append_args(reply,
								 DBUS_TYPE_STRING, &version,
								 DBUS_TYPE_INVALID);
	}
	else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (!dbus_connection_send(conn, reply, NULL))
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult kdk_date_server_get_all_properties_handler(DBusConnection *conn, DBusMessage *reply)
{
	DBusHandlerResult result;
	DBusMessageIter date_array, date_dict, iter, variant;
	const char *property = "Version";

	result = DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &date_array);

	/* Append all properties name/value pairs */
	property = "Version";
	dbus_message_iter_open_container(&date_array, DBUS_TYPE_DICT_ENTRY, NULL, &date_dict);
	dbus_message_iter_append_basic(&date_dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&date_dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&date_dict, &variant);
	dbus_message_iter_close_container(&date_array, &date_dict);

	dbus_message_iter_close_container(&iter, &date_array);

	if (dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_HANDLED;
	return result;
}

DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
	DBusError err;

	DBusHandlerResult result;
	DBusMessage *date_reply = NULL;
	bool quit = false;

	// kdk_log("Got D-Bus request: %s.%s on %s\n",
	// 		dbus_message_get_interface(message),
	// 		dbus_message_get_member(message),
	// 		dbus_message_get_path(message));


	dbus_error_init(&err);

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{

		if (!(date_reply = dbus_message_new_method_return(message)))
			goto fail;

		dbus_message_append_args(date_reply,
								 DBUS_TYPE_STRING, &server_introspection_xml,
								 DBUS_TYPE_INVALID);
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get"))
	{
		const char *interface, *property;

		if (!dbus_message_get_args(message, &err,
								   DBUS_TYPE_STRING, &interface,
								   DBUS_TYPE_STRING, &property,
								   DBUS_TYPE_INVALID))
			goto fail;

		if (!(date_reply = dbus_message_new_method_return(message)))
			goto fail;
		result = kdk_date_server_get_properties_handler(property, conn, date_reply);
		dbus_message_unref(date_reply);
		return result;
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll"))
	{

		if (!(date_reply = dbus_message_new_method_return(message)))
			goto fail;

		result = kdk_date_server_get_all_properties_handler(conn, date_reply);
		dbus_message_unref(date_reply);
		return result;
	}
	else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

fail:
	if (dbus_error_is_set(&err))
	{
		if (date_reply)
			dbus_message_unref(date_reply);
		date_reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}

	if (!date_reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	/* Send the reply which might be an error one too. */
	result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, date_reply, NULL))
		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	dbus_message_unref(date_reply);

	if (quit)
	{
		// kdk_log("Server exiting...\n");
		g_main_loop_quit(mainloop);
	}
	return result;
}

const DBusObjectPathVTable date_server_vtable = {
	.message_function = server_message_handler};

int main()
{
	DBusConnection *conn;
	DBusError err;
	int rv;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (!conn)
	{
		// kdk_err("Failed to get a session DBus connection: %s\n", err.message);
		goto fail;
	}

	rv = dbus_bus_request_name(conn, "com.kylin.kysdk.DateServer", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		// kdk_err("Failed to request name on bus: %s\n", err.message);
		goto fail;
	}

	if (!dbus_connection_register_object_path(conn, "/com/kylin/kysdk/Date", &date_server_vtable, NULL))
	{
		// kdk_err("Failed to register a object path for 'TestObject'\n");
		goto fail;
	}

	sem_init(&g_Wait, 0, 1);

	pthread_mutex_init(&lock, NULL);
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, actionDateChanged, conn);

	mainloop = g_main_loop_new(NULL, false);
	/* Set up the DBus connection to work in a GLib event loop */
	dbus_connection_setup_with_g_main(conn, NULL);
	/* Start the glib event loop */
	g_main_loop_run(mainloop);

	return EXIT_SUCCESS;
fail:
	dbus_error_free(&err);
	return EXIT_FAILURE;
}
