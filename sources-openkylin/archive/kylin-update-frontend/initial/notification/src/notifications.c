#include <stdio.h>
#include <unistd.h>
#include <glib-2.0/glib.h>
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <libnotify/notify-enum-types.h>
#include <libnotify/notify-features.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> //for mainloop
#include <error.h>
#include <stdlib.h>
#include <stdbool.h>

#include <libintl.h>//gettext
#include <locale.h>
#include <stdint.h>


#define	false	0
//#define _(string) gettext(string)

void closed_handler(NotifyNotification* notification,gpointer data);
void actionClose(NotifyNotification* notification,char *action, gpointer data);
void actionReboot(NotifyNotification* notification,char *action, gpointer data);
void actionLogout(NotifyNotification* notification,char *action, gpointer data);


GMainLoop* mainloop;

int notifications (char *sigvalue)
{

	setlocale(LC_ALL, "");//yuyanshezhi
    bindtextdomain("kylin-system-updater-notify-new","/usr/share/locale");
    textdomain("kylin-system-updater-notify-new");

	NotifyNotification* notification;
	gboolean            success;
	GError*             error = NULL;
	printf("2\n");
	if (!notify_init (gettext("Update Finished!")))//zhuti
		return 1;
 
	// /* try the icon-summary-body case */
	// notification = notify_notification_new (
	// 			gettext("Update Finished!"),//biaoti
	// 			gettext("It's suggested to reboot immediately."),//neirong
	// 			"");//icon

	// notify_notification_set_timeout(notification, 10*1000);//shijian yijing bei xiesi

	if(strcmp(sigvalue,"reboot") == 0)
	{
	printf("It should reboot\n");
	/* try the icon-summary-body case */
	notification = notify_notification_new (
				gettext("Update Finished!"),//biaoti
				gettext("It's suggested to reboot immediately."),//neirong
				"/usr/share/icons/ukui-icon-theme-default/16x16/apps/ukui-control-center.png");//icon

	notify_notification_set_timeout(notification, 0);//shijian yijing bei xiesi	
	notify_notification_add_action(notification, gettext("Reboot"), gettext("Reboot") , (NotifyActionCallback)actionReboot , NULL, NULL);
	notify_notification_add_action(notification, "Closed", gettext("Closed"), (NotifyActionCallback)actionClose , NULL, NULL);
    notify_notification_set_urgency(notification,NOTIFY_URGENCY_CRITICAL);
    //	notify_notification_set_hint_string(notification,"Reboot","reboot");
	//notify_notification_set_hint(notification,"","");
	}

	if(strcmp(sigvalue,"logout") == 0)
	{

	/* try the icon-summary-body case */
	notification = notify_notification_new (
				gettext("Update Finished!"),//title
				gettext("It's suggested to logout immediately."),//content
				"");//icon

	notify_notification_set_timeout(notification, 0);//shijian yijing bei xiesi

	notify_notification_add_action(notification, "Logout", gettext("Logout") , (NotifyActionCallback)actionLogout , NULL, NULL);
	notify_notification_add_action(notification, "Closed", gettext("Closed"), (NotifyActionCallback)actionClose , NULL, NULL);
//	notify_notification_set_hint_string(notification,"Logout","logout");
    notify_notification_set_urgency(notification,NOTIFY_URGENCY_CRITICAL);
	//notify_notification_set_hint(notification,"","");
	}

	error = NULL;
	success = notify_notification_show (notification, &error);
	if (!success)
	{
		g_print ("That did not work ... \"%s\".\n", error->message);
		g_error_free (error);
	}
 
	g_signal_connect (G_OBJECT (notification),
			  "closed",
			  G_CALLBACK (closed_handler),
			  NULL);
 
	mainloop=g_main_loop_new(NULL, false);
	g_main_loop_run(mainloop);

	notify_uninit ();
 
	return 0;
}


void closed_handler(NotifyNotification* notification, gpointer data)
{	notify_uninit ();
	g_print ("closed_handler() called\n");
	g_main_loop_quit(mainloop);
	return;
}

void actionClose(NotifyNotification* notification,char *action, gpointer data)
{
	g_print("Closed\n");
	return;
}

void actionReboot(NotifyNotification* notification,char *action, gpointer data) 
{
	g_print ("get signal to reboot\n");
	system("ukui-session-tools --reboot");
	return;
	// g_print("Reboot\n");
	// //dbus_reboot_method_call();
	// DBusConnection *connection;
	// DBusMessage *message;
	// DBusError err ;
	// DBusMessageIter iter;

	// dbus_bool_t reboot=false;//dbus  内部bool型

	// dbus_error_init(&err);
	// connection = dbus_bus_get(DBUS_BUS_SYSTEM,  &err);

	// if(!connection)
	// {
	// fprintf(stderr, "Fail to get a system dbus connection: %s\n", err.message);
	// exit(1);
	// }

	// message = dbus_message_new_method_call("org.freedesktop.login1",
	// 										"/org/freedesktop/login1", //object
	// 										"org.freedesktop.login1.Manager",//interface
	// 										"Reboot");//method name


	// if (NULL == message)
	// {
	// fprintf(stderr, "Message Null\n");
	// exit(1);
	// }

	// dbus_message_iter_init_append(message, &iter);
	// if( !dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN,  &reboot))
	// {
	// fprintf(stderr, "Out of Memory!\n");
	// exit(1);
	// }

	// dbus_connection_send(connection,message,NULL);
	// dbus_connection_flush(connection);
	// dbus_message_unref(message);
	// return;
}

void actionLogout(NotifyNotification* notification,char *action, gpointer data) 
{
	g_print("Logout\n");
	system("ukui-session-tools --logout");
	return;
}
