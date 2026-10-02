// #include <X11/extensions/Xrandr.h>
#include "agent.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkdisplay.h>

static T_UIAgentHandle *s_self = NULL;

static const char *app_name = "投屏";
static const char *app_icon = "kylin-miracast";
static const int notify_id = 0;

static bool if_frontend_alive()
{    
    return system("checkFrontendRunning.sh") ? true : false;
}

static int send_notify(const char *title,
					const char *body,
					int timeout)
{
#define CHECK_STEP_FAILED() if(!bRet){iFailed = 1;}

	bool bRet = 0;
	int iFailed = 0;

    DBusMessage *message = dbus_message_new_method_call(
            "org.freedesktop.Notifications",
            "/org/freedesktop/Notifications",
            "org.freedesktop.Notifications",
            "Notify");

    if(message)
    {
    	DBusMessageIter iter, actions, hints;

    	dbus_message_iter_init_append(message, &iter);

	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &app_name);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &notify_id);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &app_icon);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &title);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &body);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &actions);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_close_container(&iter, &actions);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_open_container(&iter,
	                                                   DBUS_TYPE_ARRAY,
	                                                   DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
	                                                   DBUS_TYPE_STRING_AS_STRING
	                                                   DBUS_TYPE_VARIANT_AS_STRING
	                                                   DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
	                                                   &hints);
		CHECK_STEP_FAILED();
		bRet = dbus_message_iter_close_container(&iter, &hints);
		CHECK_STEP_FAILED();
		bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &timeout);
		CHECK_STEP_FAILED();
    	bRet = dbus_connection_send(s_self->ptDbushandle->sessBusConn, message, NULL);
		CHECK_STEP_FAILED();
    	dbus_connection_flush(s_self->ptDbushandle->sessBusConn);

    	dbus_message_unref(message);
    }
    else
        goto error;

    if(!iFailed)
        return 0;

error:
    fprintf(stderr, "[ERROR] miracle-agent send notification %s failed\n", title);
	return -1;
}


static int emit_signal_to_ui(DBusConnection *conn, const char* signame, int sigui_result, const char* dscp)
{
#define CHECK_STEP_FAILED() if(!bRet){iFailed = 1;}

	bool bRet = 0;
	int iFailed = 0;	
	DBusMessage *msg;
	DBusMessageIter iter, dict_iter;

	msg = dbus_message_new_signal("/org/freedesktop/miracleagent",
					"org.freedesktop.miracleagent.op",
					signame);

	if(!msg)
		goto error;

	dbus_message_iter_init_append(msg, &iter);

	if(SIGUI_INVALID != sigui_result)
	{
		bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &sigui_result);
		CHECK_STEP_FAILED();
	}
	bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &dscp);
	CHECK_STEP_FAILED();
	bRet = dbus_connection_send(conn, msg, NULL);
	CHECK_STEP_FAILED();

	dbus_connection_flush(conn);

	dbus_message_unref(msg);

    if(!iFailed)
        return 0;

error:
    fprintf(stderr, "[ERROR] miracle-agent emit UI signal %s failed\n", signame);
	return -1;
}

static void check_chose_frontend(void)
{
    s_self->useFrontend = if_frontend_alive();
    fprintf(stderr, "%s : useFrontend is %d\n", __func__, s_self->useFrontend);
}

static int agt_send_ui_peer_action(const char* signame, void *sigpara, 
        const char *noti_title, const char *noti_body, int timeout)
{
    int r = 0;

    if(!s_self)
    {
        fprintf(stderr, "[FATAL ERROR] ui-agent has null handle\n");
        return -1;
    }

    if(s_self->useFrontend)
    {    
        const char *peerName = sigpara;

        r = emit_signal_to_ui(s_self->ptDbushandle->sessBusConn, signame, SIGUI_INVALID, peerName);

        if(r)
            fprintf(stderr, "[ERROR] miracle-agent emit %s signal failed\n", signame); 
        else
            fprintf(stderr, "miracle-agent emit %s signal succ\n", signame);
    }
    else
    {
        r = s_self->send_notification(noti_title, noti_body, timeout);
        if(r)
            fprintf(stderr, "[ERROR] miracle-agent notify %s failed\n", noti_title);
        else
            fprintf(stderr, "miracle-agent notify %s succ\n", noti_title);
    }

    return r;
}

static int agt_send_ui_action_result(int result, const char* signame, void *sigpara,
        const char *noti_title, const char *noti_body, int timeout)
{
    int r = 0;

    if(!s_self)
    {
        fprintf(stderr, "[FATAL ERROR] ui-agent has null handle\n");
        return -1;
    }

    if(s_self->useFrontend)
    {   
        const char *dscp = sigpara;

        r = emit_signal_to_ui(s_self->ptDbushandle->sessBusConn, signame, result, dscp);
        if(r)
            fprintf(stderr, "[ERROR] miracle-agent emit %s:%d failed\n", signame, result);
        else
            fprintf(stderr, "miracle-agent emit %s:%d succ\n", signame, result);
    }
    else
    {
        r = s_self->send_notification(noti_title, noti_body, timeout);
        if(r)
            fprintf(stderr, "[ERROR] miracle-agent notify %s failed\n", noti_title);
        else
            fprintf(stderr, "[ERROR] miracle-agent notify %s succ\n", noti_title);
    }
}

static void on_screen_size_changed(GdkScreen *screen, void *data)
{
    T_UIAgentHandle *handle = data;
    GdkDisplay *display;
    GdkMonitor *monitor;
    GdkRectangle dest;

    fprintf(stderr, "[INFO] ENTER %s\n", __func__);
	display = gdk_display_get_default();
	if(!display)
		return;

	monitor = gdk_display_get_primary_monitor(display);
	if(!monitor)
		return;

	gdk_monitor_get_geometry(monitor, &dest);

	handle->width = dest.width;
	handle->height = dest.height;

    notify_wifid_resolution_changed(handle);

    fprintf(stderr, "[INFO] LEAVE %s, size changed to %d * %d\n", __func__, handle->width, handle->height);
}

int init_local_resolution(T_UIAgentHandle *handle)
{
	// Display *dpy = XOpenDisplay(":0");

	// if(dpy)
	// {
	// 	int num_sizes;
	// 	Rotation rotation;
	// 	Window root = DefaultRootWindow(dpy);
	// 	XRRScreenConfiguration *cfg = XRRGetScreenInfo(dpy, root);
	// 	XRRScreenSize *sizes = XRRConfigSizes(cfg, &num_sizes);
	// 	SizeID id = XRRConfigCurrentConfiguration(cfg, &rotation);

	// 	handle->width = sizes[id].width;
	// 	handle->height = sizes[id].height;

	// 	XCloseDisplay(dpy);

	// 	return 0;
	// }
    // return -1;
	
    GdkScreen *screen;
    GdkDisplay *display;
	GdkMonitor *monitor;
	GdkRectangle dest;

	display = gdk_display_get_default();
	if(!display)
		return -1;

    screen = gdk_display_get_default_screen(display);
	if(!screen)
		return -1;

   g_signal_connect (screen, "size-changed",
                     G_CALLBACK (on_screen_size_changed), handle);

	monitor = gdk_display_get_primary_monitor(display);
	if(!monitor)
		return -1;

	gdk_monitor_get_geometry(monitor, &dest);

	handle->width = dest.width;
	handle->height = dest.height;
    notify_wifid_resolution_changed(handle);

    return 0;
}

void init_ui_handle(int argc, char **argv, T_UIAgentHandle *handle, T_DbusHandle *pt_dbushandle)
{   
    gdk_init_check(&argc, &argv);//we use gtk temporary, next step, we'll use ukui-DaemonSetting dbus instead.

    handle->ptDbushandle = pt_dbushandle;

    handle->refresh = check_chose_frontend;
    handle->send_notification = send_notify;
    handle->send_ui_action_result = agt_send_ui_action_result;
    handle->send_ui_peer_action = agt_send_ui_peer_action;

    handle->width = -1;
    handle->height = -1;

    s_self = handle;
    // handle->useFrontend = if_frontend_alive();
}