#ifndef AGENT_H
#define AGENT_H

#include <dbus/dbus.h>
#include <gio/gio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include "shl_dlist.h"

//#define WFD_R2

#define MAX_PIN_LEN 16

#define INVALID_UID (uint32_t)0xffffffff

enum{
	OFF = 0,
	ON,
};

enum{
	USER_DISALLOW = 0,
	USER_ALLOW,
};

enum {
	SIGUI_FAIL = 0,
	SIGUI_SUCC,
	SIGUI_INVALID
};

enum {
	IDLE_STATE = 0,
    PROJECTION_STATE
};

enum {
	NOT_SUPPORT_P2P = 0,
	SUPPORT_P2P_WITHOUT_DEV,
	SUPPORT_P2P_PERFECT,
	OP_NO_RESPONSE
};


typedef void (*PF_REFRESH) (void);
typedef int (*PF_SEND_UI_NOTIFICATION) (const char*, const char*, int);
typedef int (*PF_SEND_UI_PEER_ACTION) (const char*, void *para, const char*, const char*, int);
typedef int (*PF_SEND_UI_ACTION_RESULT) (int res, const char* , void *para, const char*, const char*, int);

typedef struct
{
	struct shl_dlist list;
	int ifindex;
	char *ifname;
	int coexistWithNM;
	int iSupportP2p;
}T_LinkList;

typedef struct
{
	GDBusProxy* sysBusProxy;
	DBusConnection *sessBusConn;
	GDBusConnection *sysBusConn;
}T_DbusHandle;

typedef struct {
    bool useFrontend;
    
    T_DbusHandle *ptDbushandle;
    PF_REFRESH refresh;
    PF_SEND_UI_PEER_ACTION send_ui_peer_action;
    PF_SEND_UI_ACTION_RESULT send_ui_action_result;
    // PF_SEND_UI_NORMAL notify_init;
    // PF_SEND_UI_NORMAL notify_formation;
    // PF_SEND_UI_NORMAL notify_mm_neg;
    // PF_SEND_UI_NORMAL notify_play;
    // PF_SEND_UI_NORMAL notify_disconnect;
    // PF_NOTIFY_UI_EXCEPT notify_except;

    PF_SEND_UI_NOTIFICATION send_notification;
	
	int width;
	int height;

    // void *self;
}T_UIAgentHandle;

#ifdef WFD_R2
typedef struct
{
	bool isStarted;
	bool bR2InUse;	

	int port;
	GSocketAddress *listen_addr;
	GSocketService *service;
	GSocket *socket;
	GSocketAddress *sockaddr;
	GSocketConnection *conn;
	GIOChannel *channel;
	guint io_id;
}T_g_socket;
#endif

typedef struct
{
	char* friendly_name;
	char* config_methods;
	char* peer_label;
	char* peername;
	char pincode[MAX_PIN_LEN];
    
    T_DbusHandle dbushandle;
    T_UIAgentHandle uiAgtHandle;

	struct shl_dlist link_list;

	guint confirmTimerid;
	guint pinFilterTimerid;
	guint destroyTimerid;

	pid_t front_pid;
	pid_t uibc_daemon_pid;
	pid_t player_pid;	

	unsigned int screen_block_cookie;
	int uibc_pipe_fd;
	int coexistWithNM;
	int state;

	bool bStarted;
	bool bWlanEnabled;

#ifdef WFD_R2
	T_g_socket r2_connection_Handle;
#endif
}T_Handle;

void init_ui_handle(int argc, char **argv, T_UIAgentHandle *handle, T_DbusHandle *pt_dbushandle);
void notify_wifid_resolution_changed(T_UIAgentHandle *ptUiHandle);
int init_local_resolution(T_UIAgentHandle *handle);

void wfd_r2_support_init(T_Handle *ptHandle);
void r2_service_stop(T_Handle *ptHandle);
int r2_service_start(T_Handle *ptHandle);
#endif