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

#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */

#include "m_systime.h"
#include <libkytimer.h>
#include <libkylog.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/timerfd.h>
#include <errno.h>
// 20020721新增时区变化监听
#include <sys/inotify.h>

#ifndef TFD_TIMER_CANCEL_ON_SET
#define TFD_TIMER_CANCEL_ON_SET 1 << 1
#endif
#define TIME_T_MAX (time_t)((1UL << ((sizeof(time_t) << 3) - 1)) - 1)

pthread_mutex_t lock, dbus_lock;
u_int8_t g_Quit; // 退出信号
size_t periodicTimerID = 0;

const char *tiem_server_introspection_xml =
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

	"  <interface name='com.kylin.kysdk.TimeInterface'>\n"
	"    <signal name='TimeSignal'>\n"
	"       <arg name='tim' type='s' />\n"
	"    </signal>"
	"    <signal name='TimeChangeSignal'>\n"
	"       <arg name='tim' type='s' />\n"
	"    </signal>"
	"  </interface>\n"

	"</node>\n";

void sig_Handler(int sig)
{
	g_Quit = 1;
}

static void schedule_minute_timer(void *ptr);

static void *onTimeChange(void *ptr)
{
	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;

	dbus_uint32_t serial = 0;

	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);

	char *buf = calloc(1, 128);
	snprintf(buf, 128, "%04d/%02d/%02d %02d:%02d:%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

	msg = dbus_message_new_signal("/com/kylin/kysdk/Timer",
								  "com.kylin.kysdk.TimeInterface",
								  "TimeChangeSignal");
	if (NULL == msg)
	{
		klog_err("dbus_message_new_signal msg error!\n");
	}

	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &buf);

	pthread_mutex_lock(&dbus_lock);
	dbus_connection_send(conn, msg, &serial);
	dbus_connection_flush(conn);
	pthread_mutex_unlock(&dbus_lock);

	dbus_message_unref(msg);

	klog_info("Emit signal [TimeChangeSignal] success.Time change signal value: %s!\n", buf);

	free(buf);
	return NULL;
}

static void emit_time_signal(void *ptr)
{
	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;

	time_t current;
	time(&current);
	struct tm *now = localtime(&current);

	char *buf = calloc(1, 128);
	snprintf(buf, 128, "%04d/%02d/%02d %02d:%02d:%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

	msg = dbus_message_new_signal("/com/kylin/kysdk/Timer",
								  "com.kylin.kysdk.TimeInterface",
								  "TimeSignal");
	if (NULL == msg)
	{
		klog_err("dbus_message_new_signal msg error!\n");
		free(buf);
		return;
	}

	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &buf);

	pthread_mutex_lock(&dbus_lock);
	dbus_connection_send(conn, msg, &serial);
	dbus_connection_flush(conn);
	pthread_mutex_unlock(&dbus_lock);

	dbus_message_unref(msg);

	klog_info("Emit signal [TimeSignal] success.Time signal value: %s!\n", buf);

	free(buf);
}

static void *onMinuteTick(void *ptr)
{
	emit_time_signal(ptr);

	// Re-align to next minute boundary (handles drift after S3/S4 resume)
	time_t current;
	time(&current);
	struct tm *now = localtime(&current);
	int ms = (60 - now->tm_sec) * 1000;
	if (ms <= 0)
		ms = 60000;

	// Read timer ID under lock, then reset without holding lock
	// to avoid deadlock with schedule_minute_timer calling kdk_timer_stop
	pthread_mutex_lock(&lock);
	size_t id = periodicTimerID;
	pthread_mutex_unlock(&lock);

	if (id != 0)
		kdk_timer_reset(id, ms);

	return NULL;
}

static void schedule_minute_timer(void *ptr)
{
	time_t current;
	time(&current);
	struct tm *now = localtime(&current);
	int ms = (60 - now->tm_sec) * 1000;
	if (ms <= 0)
		ms = 60000;

	size_t old_id;
	pthread_mutex_lock(&lock);
	old_id = periodicTimerID;
	periodicTimerID = 0;
	pthread_mutex_unlock(&lock);

	if (old_id != 0)
		kdk_timer_stop(old_id);

	size_t new_id = kdk_timer_start(ms, onMinuteTick, KTIMER_PERIODIC, KTIMER_RELATIVE, ptr, 0);

	pthread_mutex_lock(&lock);
	periodicTimerID = new_id;
	pthread_mutex_unlock(&lock);

	if (new_id <= 0)
	{
		klog_err("kdk_timer_start error!\n");
		pthread_mutex_lock(&lock);
		periodicTimerID = 0;
		pthread_mutex_unlock(&lock);
	}

	klog_info("Schedule minute timer: %d ms, timerfd: %zu\n", ms, new_id);
}

void *startBroadcastSystemTimePerMin(void *ptr)
{
	onTimeChange(ptr);
	schedule_minute_timer(ptr);

	while (!g_Quit)
		sleep(60);

	if (periodicTimerID != 0)
		kdk_timer_stop(periodicTimerID);

	return NULL;
}

int monitorSystemTimeChange()
{
	// klog_info("Monitor system time change in!\n");

	struct itimerspec its = {.it_value.tv_sec = TIME_T_MAX};
	int fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	if (fd < 0)
	{
		klog_err("timerfd create error !\n");
		return -1;
	}

	if (timerfd_settime(fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &its, NULL) < 0)
	{
		klog_err("timerfd settime error!\n");
		close(fd);
		return -1;
	}

	char dep[8] = "\0";
	ssize_t ret = read(fd, dep, sizeof(dep));
	close(fd);
	if (ret == -1 && errno == ECANCELED)
	{
		klog_info("System Time Change!\n");
		return 1;
	}

	return 0;
}

void *actionTimeChanged(void *ptr)
{
	while (!g_Quit)
	{
		if (monitorSystemTimeChange() == 1)
		{
			onTimeChange(ptr);
			schedule_minute_timer(ptr);
		}
	}

	return NULL;
}

// 20020721新增时区变化监听
int monitorSystemTimeZoneChange()
{
	// klog_info("Monitor system time zone change in!\n");

	char buf[BUFSIZ];
	int fd = inotify_init();
	buf[sizeof(buf) - 1] = 0;

	struct inotify_event *event;

	if (fd < 0)
	{
		return -1;
	}

	int ftimezone = inotify_add_watch(fd, "/etc/localtime", IN_DONT_FOLLOW);

	if (ftimezone < 0)
	{
		close(fd);
		return -1;
	}
	int ret = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	event = (struct inotify_event *)&buf[0];
	if (ret)
	{
		klog_info("System time zone change!\n");
		return 1;
	}
	return 0;
}
// 20020721新增时区变化监听
void *actionTimeZoneChanged(void *ptr)
{
	while (!g_Quit)
	{
		if (monitorSystemTimeZoneChange() == 1)
		{
			onTimeChange(ptr);
			schedule_minute_timer(ptr);
		}
	}
	return NULL;
}

const char *version = "0.1";
GMainLoop *mainloop;

DBusHandlerResult kdk_time_server_get_all_properties_handler(DBusConnection *conn, DBusMessage *reply)
{
	DBusHandlerResult time_result;
	DBusMessageIter time_array, time_dict, time_iter, time_variant;
	const char *property = "Version";

	time_result = DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init_append(reply, &time_iter);
	dbus_message_iter_open_container(&time_iter, DBUS_TYPE_ARRAY, "{sv}", &time_array);

	property = "Version";
	dbus_message_iter_open_container(&time_array, DBUS_TYPE_DICT_ENTRY, NULL, &time_dict);
	dbus_message_iter_append_basic(&time_dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&time_dict, DBUS_TYPE_VARIANT, "s", &time_variant);
	dbus_message_iter_append_basic(&time_variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&time_dict, &time_variant);
	dbus_message_iter_close_container(&time_array, &time_dict);

	dbus_message_iter_close_container(&time_iter, &time_array);

	if (dbus_connection_send(conn, reply, NULL))
		time_result = DBUS_HANDLER_RESULT_HANDLED;
	return time_result;
}

DBusHandlerResult kdk_time_server_get_properties_handler(const char *property, DBusConnection *conn, DBusMessage *reply)
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

DBusHandlerResult kdk_time_server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
	DBusError err;

	DBusHandlerResult time_result;
	DBusMessage *time_reply = NULL;
	bool quit = false;

	// kdk_err("Got D-Bus request: %s.%s on %s\n",
	// 		dbus_message_get_interface(message),
	// 		dbus_message_get_member(message),
	// 		dbus_message_get_path(message));

	dbus_error_init(&err);

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{

		if (!(time_reply = dbus_message_new_method_return(message)))
			goto fail;

		dbus_message_append_args(time_reply,
								 DBUS_TYPE_STRING, &tiem_server_introspection_xml,
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

		if (!(time_reply = dbus_message_new_method_return(message)))
			goto fail;

		time_result = kdk_time_server_get_properties_handler(property, conn, time_reply);
		dbus_message_unref(time_reply);
		return time_result;
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll"))
	{

		if (!(time_reply = dbus_message_new_method_return(message)))
			goto fail;

		time_result = kdk_time_server_get_all_properties_handler(conn, time_reply);
		dbus_message_unref(time_reply);
		return time_result;
	}
	else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

fail:
	if (dbus_error_is_set(&err))
	{
		if (time_reply)
			dbus_message_unref(time_reply);
		time_reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}

	if (!time_reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	time_result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, time_reply, NULL))
		time_result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	dbus_message_unref(time_reply);

	if (quit)
	{
		// kdk_err("Server exiting...\n");
		g_main_loop_quit(mainloop);
	}
	return time_result;
}

const DBusObjectPathVTable time_server_vtable = {
	.message_function = kdk_time_server_message_handler};

int main(void)
{
	DBusConnection *conn;
	DBusError err;
	int time_rv;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn)
	{
		// kdk_err("Failed to get a system DBus connection: %s\n", err.message);
		goto fail;
	}

	time_rv = dbus_bus_request_name(conn, "com.kylin.kysdk.TimeServer", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (time_rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		// kdk_err("Failed to request name on bus: %s\n", err.message);
		goto fail;
	}

	if (!dbus_connection_register_object_path(conn, "/com/kylin/kysdk/Timer", &time_server_vtable, NULL))
	{
		// kdk_err("Failed to register a object path for 'TestObject'\n");
		goto fail;
	}

	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&dbus_lock, NULL);
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, actionTimeChanged, conn);

	// 20020721新增时区变化监听
	pthread_attr_t timezone_attr;
	pthread_t timezone_id;
	pthread_attr_init(&timezone_attr);
	pthread_attr_setdetachstate(&timezone_attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&timezone_id, &timezone_attr, actionTimeZoneChanged, conn);

	pthread_attr_t tt;
	pthread_t dd;
	pthread_attr_init(&tt);
	pthread_attr_setdetachstate(&tt, PTHREAD_CREATE_DETACHED);
	pthread_create(&dd, &tt, startBroadcastSystemTimePerMin, conn);
	mainloop = g_main_loop_new(NULL, false);
	dbus_connection_setup_with_g_main(conn, NULL);
	g_main_loop_run(mainloop);

	return EXIT_SUCCESS;
fail:
	dbus_error_free(&err);
	return EXIT_FAILURE;
}
