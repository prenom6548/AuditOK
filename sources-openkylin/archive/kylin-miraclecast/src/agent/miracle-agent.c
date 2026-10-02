#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <systemd/sd-journal.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */
#include "shl_log.h"
#include "agent.h"

typedef void (*SIG_EMIT_HANDLE)(GDBusConnection *,
                             const gchar *,
                             const gchar *,
                             const gchar *,
                             const gchar *,
                             GVariant *,
                             gpointer);

T_Handle *s_ptHandle = NULL;

enum wifid_user_approve_state {
	IDLE = 0,
	WAIT_APPROVE,
	USER_REJECTED ,
	USER_APPROVED ,
	TIMEOUT ,
	IGNORED
};

const char *version = "1.0";
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

	"  <interface name='org.freedesktop.miracleagent.op'>\n"
	"    <property name='Version' type='s' access='read' />\n"
	"    <method name='PreCheck'>\n"
	"    </method>\n"
	"    <method name='Start'>\n"
	"      <arg name='name' type='s' direction='in' />\n"
	"    </method>\n"
	"    <method name='Confirmed'>\n"
	"      <arg name='choice' type='i' direction='in' />\n"
	"    </method>\n"
	"    <signal name='Confirming'>\n"
	"      <arg name='name' type='s' direction='out' />\n"
	"    </signal>\n"
	"    <signal name='disconnected'>\n"
	"    </signal>\n"
	"    <signal name='WarnNotify'>\n"
	"    </signal>\n"
	"    <signal name='Formationing'>\n"
	"      <arg name='name' type='s' direction='out' />\n"
	"    </signal>\n"
	"    <signal name='FormationOver'>\n"
	"      <arg name='result' type='i' direction='out' />\n"
	"      <arg name='reason' type='s' direction='out' />\n"
	"    </signal>\n"
	"    <signal name='RtspOver'>\n"
	"      <arg name='result' type='i' direction='out' />\n"
	"      <arg name='reason' type='s' direction='out' />\n"
	"    </signal>\n"	
	"  </interface>\n"

	"</node>\n";

DBusHandlerResult server_get_properties_handler(const char *property, DBusConnection *conn, DBusMessage *reply)
{
	if (!strcmp(property, "Version")) {
		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &version,
					 DBUS_TYPE_INVALID);
	} else
		/* Unknown property */
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (!dbus_connection_send(conn, reply, NULL))
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult server_get_all_properties_handler(DBusConnection *conn, DBusMessage *reply)
{
	DBusHandlerResult result;
	DBusMessageIter array, dict, iter, variant;
	const char *property = "Version";

	/*
	 * All dbus functions used below might error due to out of
	 * memory error. If one of them fails, we assume that all
	 * following functions will error too, including
	 * dbus_connection_send().
	 */
	result = DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &array);

	/* Append all properties name/value pairs */
	property = "Version";
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);

	dbus_message_iter_close_container(&iter, &array);

	if (dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_HANDLED;
	return result;
}

static int create_uibc_pipe(void)
{
	struct stat st;
	char pipename[128] = {0};
	int socket;

	snprintf(pipename, sizeof(pipename), "/tmp/uibc-%d-pipe", getpid());
	if (lstat (pipename, &st) == 0
		&& ((st.st_mode & S_IFMT) == S_IFREG) )
		remove(pipename);

	unlink (pipename);

	if (mkfifo (pipename, 0777) == -1) 
	{
		fprintf(stderr,"make uibc pipe failed\n");
		return -1;
	}

	socket = open(pipename, O_RDWR | O_NONBLOCK, 0);
	if (socket == -1) 
	{
		fprintf(stderr,"open uibc pipe failed\n");
		return -1;
	}

	return socket;
}

static void remove_uibc_pipe(T_Handle *ptHandle)
{
	if(ptHandle->uibc_pipe_fd > 0)
		close(ptHandle->uibc_pipe_fd);
	
	ptHandle->uibc_pipe_fd = 0;
}

static void launch_uibc_daemon(int pipefd, int uibcPort, char *uibcServerAddr,int hidcEn)
{		
	int i = 0;
	char *argv[64];
	char uibc_portStr[64] = {0};
	char uibc_pipefd[64] = {0};
	char hidcEnStr[64] = {0};

	dup2(pipefd, 0);//daemon stdin redir to pipe	

	argv[i++] = "miracle-uibcctl";
	argv[i++] = uibcServerAddr;
	sprintf(uibc_portStr, "%d", uibcPort);
	argv[i++] = uibc_portStr;
	sprintf(uibc_pipefd, "%d", pipefd);
	argv[i++] = uibc_pipefd;

	sprintf(hidcEnStr, "%d", hidcEn);
	argv[i++] = hidcEnStr;

	argv[i++] = "--daemon";
	argv[i] = NULL;

	if (execvpe(argv[0], argv, environ) < 0) {
		fprintf(stderr,"start uibc-daemon failed (%d): %m", errno);
		int i = 0;
		fprintf(stderr,"printing environment: ");
		while (environ[i]) {
			fprintf(stderr,"%s", environ[i++]);
		}
	}
}

static void launch_player(T_Handle *ptHandle, int hres, int vres, int uibcEn, int rtspPort, int hidcEn, char *manufacturer,int portnm) 
{
	char *argv[64];
	char resolution[64];
	char scale[64];
	char port[64];	
	int i = 0;

	argv[i++] = "castplayer";

	// if (gst_debug) 
	{
		argv[i++] = "-d";
		argv[i++] = "1";
	}

	argv[i++] = "-a";

	if(uibcEn)
	{
		argv[i++] = "--uibc";
		argv[i++] = "1";
	}
	if(hidcEn)
	{
		argv[i++] = "--hidc";
		argv[i++] = "1";
	}
	if(portnm == 24605)
	{
		argv[i++] = "--kylinpc";
		argv[i++] = "1";
	}

	//scale 4k
	if (s_ptHandle->uiAgtHandle.width > 1920 && s_ptHandle->uiAgtHandle.height > 1080)
	{
		sprintf(scale, "%dx%d", s_ptHandle->uiAgtHandle.width, s_ptHandle->uiAgtHandle.height);
		argv[i++] = "-s";
		argv[i++] = scale;
	}

	argv[i++] = "-p";
	sprintf(port, "%d", rtspPort);
	argv[i++] = port;

	if (hres && vres) {
		sprintf(resolution, "%dx%d", hres, vres);
		argv[i++] = "-r";
		argv[i++] = resolution;
	}

	if (!strcmp(manufacturer, "manufacturer")){
		argv[i++] = "-m";
		argv[i++] = "5";
	}else if (!strcmp(manufacturer, "huawei")){
		argv[i++] = "-m";
		argv[i++] = "6";
	}else if (!strcmp(manufacturer, "xiaomi")){
		argv[i++] = "-m";
		argv[i++] = "7";
	}else{
		argv[i++] = "-m";
		argv[i++] = "8";
	}

	argv[i] = NULL;

	if (execvpe(argv[0], argv, environ) < 0) {
		fprintf(stderr,"start player failed (%d): %m", errno);
		int i = 0;
		fprintf(stderr,"printing environment: ");
		while (environ[i]) {
			fprintf(stderr,"%s", environ[i++]);
		}
	}
}

static void kill_uibc_daemon(T_Handle *ptHandle)
{
	if (ptHandle->uibc_daemon_pid <= 0)
		return;

	int r = kill(ptHandle->uibc_daemon_pid, SIGTERM);
	if(r < 0)
	{
		fprintf(stderr,"SIGTERM kill uibc daemon failed\n");
		r = kill(ptHandle->uibc_daemon_pid, SIGKILL);
	}

	if(r < 0)
		fprintf(stderr,"SIGKILL kill uibc daemon failed\n");

	remove_uibc_pipe(ptHandle);
}

static void spawn_gst(T_Handle *ptHandle, int hres, int vres, 
			int uibcEn, int hidcEn,int rtspPort, int uibcPort, char *manufacturer, char *uibcServerAddr)
{
	pid_t pid;
	int fd_journal;
	sigset_t mask;

	if (ptHandle->player_pid > 0)
		return;

	if(ptHandle->uibc_daemon_pid > 0)
	{
		kill_uibc_daemon(ptHandle);
		ptHandle->uibc_daemon_pid = 0;
		fprintf(stderr,"There's still old uibc daemon alive, kill it\n");
	}

	ptHandle->uibc_pipe_fd = create_uibc_pipe();
	if(ptHandle->uibc_pipe_fd < 0)
	{
		ptHandle->uibc_pipe_fd = 0;
		fprintf(stderr,"Create uibc pipe failed\n");
		return;
	}


	pid = fork();
	if (pid < 0) {
		return;
	} else if (!pid) {
		/* child */
		sigemptyset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		prctl(PR_SET_PDEATHSIG, SIGKILL);
		/* redirect stderr to journal */
		fd_journal = sd_journal_stream_fd("miracle-player",
						  LOG_DEBUG,
						  false);
		if (fd_journal >= 0) {
			/* dup journal-fd to stderr */
			dup2(fd_journal, 2);
		}

		dup2(ptHandle->uibc_pipe_fd, 1);//castplayer stdout redir to pipe

		launch_player(ptHandle, hres, vres, uibcEn, rtspPort, hidcEn, manufacturer,uibcPort);
		_exit(1);
	} else {
		ptHandle->player_pid = pid;
	}
	if(uibcEn)
	{		
		pid = fork();
		if (pid < 0) {
			return;
		} else if (!pid) {
			/* child */
			sigemptyset(&mask);
			sigprocmask(SIG_SETMASK, &mask, NULL);
			prctl(PR_SET_PDEATHSIG, SIGKILL);
			/* redirect stderr to journal */
			fd_journal = sd_journal_stream_fd("miracle-uibc",
							LOG_DEBUG,
							false);
			if (fd_journal >= 0) {
				/* dup journal-fd to stderr */
				dup2(fd_journal, 2);
				dup2(fd_journal, 1);
			}

			dup2(ptHandle->uibc_pipe_fd, 0);//daemon stdin redir to pipe

			launch_uibc_daemon(ptHandle->uibc_pipe_fd, uibcPort, uibcServerAddr,hidcEn);

			_exit(1);
		} else {
			ptHandle->uibc_daemon_pid = pid;
		}
	}
}

static void kill_gst(pid_t player_pid)
{
	if (player_pid <= 0)
		return;

	fprintf(stderr,"Agent kill castplayer as peer disconnected\n");
	int r = kill(player_pid, SIGTERM);
	if(r < 0)
	{
		fprintf(stderr,"SIGTERM kill gst failed\n");
		r = kill(player_pid, SIGKILL);
	}

	if(r < 0)
		fprintf(stderr,"SIGKILL kill gst failed\n");
}

static int activate_screenlock(T_Handle *ptHandle)
{
#define CHECK_STEP_FAILED() if(!bRet){iRet = -1;}
	bool bRet = 0;
	int iRet = 0;
	DBusMessage *message = NULL;

	if(!ptHandle->screen_block_cookie)
	{
		fprintf(stderr,"Screenlock is at avtived state, quit duplicated operation\n");
		return iRet;
	}

    message = dbus_message_new_method_call(
            "org.gnome.SessionManager",
            "/org/gnome/SessionManager",
            "org.gnome.SessionManager",
            "Uninhibit");

    if(message)
    {
    	DBusMessageIter iter;

    	dbus_message_iter_init_append(message, &iter);

	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ptHandle->screen_block_cookie);
		CHECK_STEP_FAILED();

    	bRet = dbus_connection_send(ptHandle->dbushandle.sessBusConn, message, NULL);
		CHECK_STEP_FAILED();
    	dbus_connection_flush(ptHandle->dbushandle.sessBusConn);

    	dbus_message_unref(message);
		fprintf(stderr,"Enable screenblock, cookie value is %u\n", ptHandle->screen_block_cookie);
		ptHandle->screen_block_cookie = 0;
    }

	return iRet;	
}

static int inactivate_screenlock(T_Handle *ptHandle)
{
#define CHECK_STEP_FAILED() if(!bRet){iRet = -1;}
	bool bRet = 0;
	int iRet = 0;
	int xid = 0;
	const char *app_name = "投屏";
	const char* reason = "投屏播放";
	int opcode = 8;
	DBusPendingCall *pending_call = NULL;
	DBusError err;
	DBusMessage *reply = NULL;
	DBusMessage *message = NULL;

	if(ptHandle->screen_block_cookie)
	{
		fprintf(stderr,"Screenlock is at inavtived state, quit duplicated operation\n");
		return iRet;
	}

    message = dbus_message_new_method_call(
            "org.gnome.SessionManager",
            "/org/gnome/SessionManager",
            "org.gnome.SessionManager",
            "Inhibit");

    if(message)
    {
    	DBusMessageIter iter;

		dbus_error_init(&err);

    	dbus_message_iter_init_append(message, &iter);

	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &app_name);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &xid);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &reason);
		CHECK_STEP_FAILED();
	    bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &opcode);
		CHECK_STEP_FAILED();

    	bRet = dbus_connection_send_with_reply(ptHandle->dbushandle.sessBusConn, message, &pending_call, 3000);
		CHECK_STEP_FAILED();
    	dbus_connection_flush(ptHandle->dbushandle.sessBusConn);

		if(!pending_call)
			goto exit;

		dbus_pending_call_block(pending_call);
		reply = dbus_pending_call_steal_reply(pending_call);

		if(!reply)
			goto exit;

		dbus_message_get_args(reply, &err, DBUS_TYPE_UINT32, &ptHandle->screen_block_cookie, DBUS_TYPE_INVALID);
		fprintf(stderr,"Disable screenblock, cookie value is %u\n", ptHandle->screen_block_cookie);
exit:
		if(pending_call)
			dbus_pending_call_unref(pending_call);
		if(reply)
			dbus_message_unref(reply);
		if(dbus_error_is_set(&err))
			dbus_error_free(&err);
		
    	dbus_message_unref(message);
    }
	return iRet;
}

void notify_wifid_resolution_changed(T_UIAgentHandle *ptUiHandle)
{
	GError *gErr = NULL;
	T_LinkList *ent = shl_dlist_last_entry(&s_ptHandle->link_list, T_LinkList, list);

	if(!ent
		|| (long)ent == (long)&s_ptHandle->link_list)
	{
		fprintf(stderr,"[FATAL ERROR]Agent hold no iface when update resolution!!\n");
		return;
	}

	g_dbus_proxy_call_sync(ptUiHandle->ptDbushandle->sysBusProxy,
						"ResolutionUpdate",
						g_variant_new("(iii)", ptUiHandle->width, ptUiHandle->height, ent->ifindex),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&gErr);

	if(gErr)
	{
		fprintf(stderr,"[FATAL]: dbus_sync_call resolution update failed for %s\n", gErr->message);
		g_error_free(gErr);
	}	
}

static int get_hardware_capability(const char* ifname, int *piCoexistWithNM, int *piSupportP2p)
{
	int res, ret, status, hardwareSupport;
	char cmd[128] = {0};

	snprintf(cmd, sizeof(cmd)-1, "checkhardware.sh %s", ifname);
	
	do{
		res = system(cmd);
		if(-1 == res)
		{
			fprintf(stderr,"checkhardware failed, will try again...\n");
			sleep(1);
		}
	}while(-1 == res);

	switch(res) {
		case 0:
			hardwareSupport = SUPPORT_P2P_PERFECT;
			fprintf(stderr,"checkhardware returned SUPPORT_P2P_PERFECT\n");
			break;
		case 256:
			hardwareSupport = NOT_SUPPORT_P2P;
			fprintf(stderr,"checkhardware returned NOT_SUPPORT_P2P\n");
			break;
		case 512:
			hardwareSupport = SUPPORT_P2P_WITHOUT_DEV;
			fprintf(stderr,"checkhardware returned SUPPORT_P2P_WITHOUT_DEV\n");
			break;
		default:
			fprintf(stderr,"checkhardware returned unexpected value %d\n", res);
			return OP_NO_RESPONSE;			
	}

	if(OP_NO_RESPONSE != hardwareSupport)
	{
		*piSupportP2p = (NOT_SUPPORT_P2P == hardwareSupport) ? 0 : 1;
		*piCoexistWithNM = *piSupportP2p ? ((SUPPORT_P2P_PERFECT == hardwareSupport) ? 1 : 0) : 0;
	}
	
	return hardwareSupport;
}

static int enable_nm_link_manage(const char* ifname)
{
	char command[128] = {0};
	int r = 0;

	snprintf(command, sizeof(command)-1, "nmcli device set %s managed yes", ifname);

	do{
		r = system(command);
	}while(-1 == r);

	if(r)
		fprintf(stderr, "Error: Set %s managed by NetworkManager failed!!!!, ret %d\n", ifname, r);
	else
		fprintf(stderr, "%s is managed by NetworkManager Now!!!!\n", ifname);
	
	return r;
}

static int disable_nm_link_manage(const char* ifname)
{
	char command[128] = {0};
	int r = 0;
	
	snprintf(command, sizeof(command)-1, "nmcli device set %s managed no", ifname);

	do{
		r = system(command);
	}while(-1 == r);

	if(r)
		fprintf(stderr,"Error: Set %s unmanaged from NetworkManager failed!!!!, ret %d\n", ifname, r);
	else
		fprintf(stderr,"%s is unmanaged from NetworkManager Now!!!!\n", ifname);
	
	return r;
}

static int disable_system_wpas(T_Handle *ptHandle)
{
	GError *gErr = NULL;

	g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
						"StopWpas",
						NULL,
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&gErr);

	if(gErr)
	{
		fprintf(stderr,"[FATAL]: dbus_sync_call Stop wpas failed for %s\n", gErr->message);
		g_error_free(gErr);
		return -1;
	}
	else
		fprintf(stderr,"Agent call Stop wpas...\n");
	
	return 0;
}

static int enable_system_wpas(T_Handle *ptHandle)
{
	GError *gErr = NULL;

	g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
						"StartWpas",
						NULL,
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&gErr);

	if(gErr)
	{
		fprintf(stderr,"[FATAL]: dbus_sync_call Start wpas failed for %s\n", gErr->message);
		g_error_free(gErr);
		return -1;
	}
	else
		fprintf(stderr,"Agent call Start wpas...\n");
	
	return 0;
}

static int takecharge_interface_from_networkmanager(T_Handle *ptHandle, const char* ifname, int bCoexistWithNM)
{
	if(bCoexistWithNM)
		return disable_nm_link_manage(ifname);
	else
		return disable_system_wpas(ptHandle);
}

static int giveback_interface_to_networkmanager(T_Handle *ptHandle, const char* ifname, int bCoexistWithNM, bool bProjectionOff)
{
	if(!ifname)
		return enable_system_wpas(ptHandle);//In case of that: usb wireless card is pullout, then we cannot recover wpa_supplicant

	if(bCoexistWithNM)
		return enable_nm_link_manage(ifname);
	else if(bProjectionOff)
		return enable_system_wpas(ptHandle);
	else
		return 0;
}

static void kill_frontend(T_Handle *ptHandle)
{
	if (ptHandle->front_pid <= 0)
		return;

	fprintf(stderr,"Agent will kill frontend [%d] now\n", ptHandle->front_pid);

	int r = kill(ptHandle->front_pid, SIGTERM);
	if(r < 0)
	{
		fprintf(stderr,"SIGTERM kill frontend failed\n");
		r = kill(ptHandle->front_pid, SIGKILL);
	}

	if(r < 0)
		fprintf(stderr,"SIGKILL kill frontend failed...\n");
}

static void launch_frontend(T_Handle *ptHandle, const char* peerName, const char* signalName) {
	char *argv[64];
	int i = 0;
	int fd_journal;
	pid_t pid;
	sigset_t mask;

	if(ptHandle->destroyTimerid > 0)
	{
		fprintf(stderr,"stop UI destroy timer, here comes a new work\n");
		g_source_remove(ptHandle->destroyTimerid);
		ptHandle->destroyTimerid = 0;
	}

	if (ptHandle->front_pid > 0)
	{
		kill_frontend(ptHandle);
		fprintf(stderr,"frontend is running, so kill it firstly\n");
	}

	pid = vfork();
	if (pid < 0) {
		return;
	} else if (!pid) {
		/* child */
		sigemptyset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		prctl(PR_SET_PDEATHSIG, SIGKILL);

		/* redirect stdout/stderr to journal */
		fd_journal = sd_journal_stream_fd("miraclecast-frontend",
						  LOG_DEBUG,
						  false);
		if (fd_journal >= 0) {
			/* dup journal-fd to stdout and stderr */
			dup2(fd_journal, 1);
			dup2(fd_journal, 2);
		} else {
			/* no journal? redirect stdout to parent's stderr */
			dup2(2, 1);
		}

		argv[i++] = "/usr/bin/miraclecast-frontend";
		argv[i++] = peerName;
		argv[i++] = signalName;

		argv[i] = NULL;

		if (execvpe(argv[0], argv, environ) < 0) {
			fprintf(stderr,"launch frontend failed (%d): %m\n", errno);
			int i = 0;
			fprintf(stderr,"printing environment: \n");
			while (environ[i]) {
				fprintf(stderr,"%s\n", environ[i++]);
			}
		}

		_exit(1);
	} else {
		ptHandle->front_pid = pid;
	}
}



static gboolean kill_frontend_with_delay_cbk(gpointer data)
{
	T_Handle *ptHandle = data;

	kill_frontend(ptHandle);

	ptHandle->destroyTimerid = 0;

	return false;
}

static void kill_frontend_with_delay(T_Handle *ptHandle)
{
	if(ptHandle->front_pid
		&& !ptHandle->destroyTimerid)
		ptHandle->destroyTimerid = g_timeout_add(3000, kill_frontend_with_delay_cbk, ptHandle);
}


static void notify_wifid_user_choice(T_Handle *ptHandle, int user_choice)
{
	GError *gErr = NULL;
	T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);
	GVariant *result;
	int approve_result = IDLE;

	if(USER_DISALLOW ==user_choice)
		approve_result = USER_REJECTED;
	else if(USER_ALLOW ==user_choice)
		approve_result = USER_APPROVED;
	else if(TIMEOUT ==user_choice)
		approve_result = TIMEOUT;	

	g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
						"userChosen",
						g_variant_new("(ii)", ent->ifindex, approve_result),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&gErr);

	if(gErr)
	{
		fprintf(stderr, "[FATAL]: dbus_sync_call handshake failed for %s\n", gErr->message);
		g_error_free(gErr);
	}
	else
		fprintf(stderr, "Agent report user approvement result %d\n", approve_result);
	
}


static gboolean user_operation_timeout_cbk(gpointer data)
{
	T_Handle *ptHandle = data;

	kill_frontend(ptHandle);
	ptHandle->confirmTimerid = 0;
	ptHandle->state = IDLE_STATE;
	memset(ptHandle->pincode, 0, MAX_PIN_LEN);
	fprintf(stderr,"[ERROR] user confirmation timeout, DISCONNECT\n");
	notify_wifid_user_choice(ptHandle, TIMEOUT);

	return false;
}

static void projection_recycle(T_Handle *ptHandle, const char* ifname, int bCoexistWithNM)
{
	kill_frontend_with_delay(ptHandle);

	activate_screenlock(ptHandle);
	
	giveback_interface_to_networkmanager(ptHandle, ifname, bCoexistWithNM, false);

	if(ptHandle->peer_label)
	{
		g_free(ptHandle->peer_label);
		ptHandle->peer_label = NULL;
	}

	if(ptHandle->peername)
	{
		g_free(ptHandle->peername);
		ptHandle->peername = NULL;
	}

	ptHandle->state = IDLE_STATE;	
}


static gboolean prov_disc_pin_filter_cbk(gpointer data)
{
	T_Handle *ptHandle = data;

	ptHandle->pinFilterTimerid = 0;	

	launch_frontend(ptHandle, ptHandle->peername ? : "未知设备", "Confirming");

	ptHandle->uiAgtHandle.refresh();

	ptHandle->uiAgtHandle.send_ui_peer_action("Confirming", ptHandle->peername ? : "未知设备", "请输入投屏 PIN ", ptHandle->pincode, -1);

	if(!ptHandle->confirmTimerid)
		ptHandle->confirmTimerid = g_timeout_add(10000, user_operation_timeout_cbk, data);

	ptHandle->state = PROJECTION_STATE;

	fprintf(stderr,"miracle-agent enter projection state: PinCode is %s.\n", ptHandle->pincode);

	return false;
}

static void start_prov_disc_pin_filter(T_Handle *ptHandle)
{
	if(!ptHandle->pinFilterTimerid)
		ptHandle->pinFilterTimerid = g_timeout_add(1500, prov_disc_pin_filter_cbk, ptHandle);
}

static void signal_provdisc_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	const char *pin;
	const char *type;
	const char *peerName;
	u_int32_t loginUid = 0;
	T_Handle *ptHandle = data;

	//keep refreshing pincode, some devices will send some prov-disc-show-pin in one second, so, need filtering here
	g_variant_get(parameters, "(sssu)", &type, &pin, &peerName, &loginUid);

	if(INVALID_UID != loginUid && loginUid != getuid())
	{
		fprintf(stderr,"Not for me, don't process prov-disc\n");
		return;
	}

	strncpy(ptHandle->pincode, pin, MAX_PIN_LEN);
	
	if(ptHandle->peername)
		free(ptHandle->peername);
	ptHandle->peername = strdup(peerName);

	start_prov_disc_pin_filter(ptHandle);
}

static void signal_disconn_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	fprintf(stderr,"miracle-agent signal received: Peer disconnected\n");

	if (ptHandle->front_pid > 0)
		ptHandle->uiAgtHandle.send_ui_peer_action("disconnected", "", "通知", "投屏设备已请求断开", -1);	
	
	kill_gst(ptHandle->player_pid);
	kill_uibc_daemon(ptHandle);

	T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

	if(!ent
		|| (long)ent == (long)&ptHandle->link_list)
	{
		fprintf(stderr,"[FATAL ERROR]Agent hold no iface when disconnect!!\n");
		projection_recycle(ptHandle, NULL, 0);
	}
	else
		projection_recycle(ptHandle, ent->ifname, ent->coexistWithNM);	
}

static void signal_spawn_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	int h, v, ue, hidcEn, rtspPort, uibcPort, r;
	char *uibcServerAddr = NULL;
	u_int32_t loginUid = 0;
	T_Handle *ptHandle = data;
	char* peer_label = NULL;
	char *manufacturer = NULL;

	ptHandle->uiAgtHandle.send_ui_action_result(SIGUI_SUCC, "RtspOver", "", "协商成功", "投屏播放启动", -1);

	g_variant_get(parameters, "(iiiiiisssu)", &h, &v, &ue, &hidcEn,&rtspPort, &uibcPort, &manufacturer, &uibcServerAddr, &peer_label, &loginUid);

	if(ptHandle->peer_label)
	{
		g_free(ptHandle->peer_label);
		ptHandle->peer_label = NULL;
	}
	
	if(0 != strcmp(peer_label, "R2Session") )
	{
		ptHandle->peer_label = strdup(peer_label);
		g_free(peer_label);
		fprintf(stderr,"miracle-agent spawn player: args h %d, v %d, ue %d,  rtspPrt %d, uibcPort %d, manufacturer %s, server %s, peer_lable %s, userId %u\n", 
			h, v, ue, rtspPort, uibcPort, manufacturer, uibcServerAddr, ptHandle->peer_label, loginUid);
	}
	else
	{
		fprintf(stderr,"Spawn a player for a R2 source!!!!\n");
	}
	
	// kill_frontend(ptHandle);
	uid_t agent_userId = getuid();

	if(INVALID_UID != loginUid 
		&& loginUid != agent_userId)
	{
		fprintf(stderr,"Not for me(%u), don't spawn the player!!!!\n", agent_userId);
		return;
	}

	spawn_gst(ptHandle, h, v, ue,hidcEn, rtspPort, uibcPort, manufacturer, uibcServerAddr);
	inactivate_screenlock(ptHandle);
}

static void signal_rtsp_failed_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;

	ptHandle->uiAgtHandle.send_ui_action_result(SIGUI_FAIL, "RtspOver", "连接超时", "协商失败", "连接超时", -1);

	kill_frontend_with_delay(ptHandle);
	ptHandle->state = IDLE_STATE;
}

#ifdef WFD_R2
static void signal_r2_session_stopped(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;

	if(!ptHandle->r2_connection_Handle.bR2InUse)
	{
		fprintf(stderr,"miracle-agent isn't connected, Reject R2SessStopped\n");
		return;
	}

	ptHandle->uiAgtHandle.send_ui_action_result(SIGUI_FAIL, "R2SessStopped", "", "R2测试", "会话结束", -1);
	fprintf(stderr,"R2 session stopped\n");
	ptHandle->r2_connection_Handle.bR2InUse = false;
}
#endif

static void signal_formation_succ_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;

	fprintf(stderr,"miracle-agent received formation success notify\n");

	ptHandle->uiAgtHandle.send_ui_action_result(SIGUI_SUCC, "FormationOver", "", "协商成功", "启动多媒体协商", -1);
}

static void signal_kill_agent_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;

	fprintf(stderr,"recv KillAgent ready to kill frontend\n");
	ptHandle->state = IDLE_STATE;
	kill_frontend(ptHandle);
}

static void signal_formation_fail_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	const char* reason;

	g_variant_get(parameters, "(s)", &reason);
	fprintf(stderr,"miracle-agent received formation failure notify, reason %s\n", reason);

	ptHandle->uiAgtHandle.send_ui_action_result(SIGUI_FAIL, "FormationOver", reason, "协商失败", reason, -1);

	kill_frontend_with_delay(ptHandle);
	ptHandle->state = IDLE_STATE;
}

static void signal_goneg_user_approve_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	const char* peer_name;
	char title[512]  ={0};

	g_variant_get(parameters, "(s)", &peer_name);
	log_info("miracle-agent received go-neg user approvment, peer %s\n", peer_name);

	fprintf(stderr,"get signal goneg from kylinPC,clear PIN\n");
	memset(ptHandle->pincode, 0, sizeof(ptHandle->pincode));//电脑投屏时清空PIN码

	if(ptHandle->peername)
		free(ptHandle->peername);

	ptHandle->peername = strdup(peer_name);

	start_prov_disc_pin_filter(ptHandle);
}


static void signal_rcv_goneg_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	const char* peer_name;
	u_int32_t loginUid = 0;
	char title[512]  ={0};

	g_variant_get(parameters, "(su)", &peer_name, &loginUid);
	fprintf(stderr,"recv goneg, enter projection state, peer %s, login uid %u\n", peer_name, loginUid);

	if(INVALID_UID != loginUid && loginUid != getuid())
	{
		fprintf(stderr,"Not for me, don't process invitation\n");
		return;
	}

	
	if(ptHandle->peername)
		free(ptHandle->peername);
	ptHandle->peername = strdup(peer_name);
	
	ptHandle->uiAgtHandle.refresh();
	snprintf(title, sizeof(title)-1, "%s 请求投屏", peer_name);
	ptHandle->uiAgtHandle.send_ui_peer_action("Formationing", peer_name, title, "开启频段协商与扫描连接", -1);

	ptHandle->state = PROJECTION_STATE;
}

static void signal_rcv_invite_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	const char* peer_name;
	u_int32_t loginUid = 0;
	char title[512]  ={0};

	g_variant_get(parameters, "(su)", &peer_name, &loginUid);
	fprintf(stderr,"recv invitation, enter projection state, peer %s, login uid %u\n", peer_name, loginUid);

	if(INVALID_UID != loginUid && loginUid != getuid())
	{
		fprintf(stderr,"Not for me, don't process invitation\n");
		return;
	}

	launch_frontend(ptHandle, peer_name, "Formationing");

	ptHandle->uiAgtHandle.refresh();

	snprintf(title, sizeof(title)-1, "持有认证的设备 %s 再次请求投屏", peer_name);
	ptHandle->uiAgtHandle.send_ui_peer_action("Formationing", peer_name, title, "开启频段协商与扫描连接", -1);
	
	ptHandle->state = PROJECTION_STATE;
}

static void signal_ssid_invalid_warn_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	const char* peer_name;
	const char *title = "注意：设备名称不符合系统规范";
	char body[512] = {0};
	int timeout = 40000;

	g_variant_get(parameters, "(s)", &peer_name);
	fprintf(stderr,"miracle-agent received ssid invalid warn notify, peer %s\n", peer_name);
	snprintf(body, sizeof(body), "接入设备名称 %s，建议不使用中文，且字母数字与符号不超过20个字符", peer_name);
	ptHandle->uiAgtHandle.send_notification(title, body, timeout);
}

static bool check_link_in_list_and_update_if_needed(struct shl_dlist *list, int ifindex, const char* ifname)
{
	if(!shl_dlist_empty(list))
	{
		struct shl_dlist *i, *t;
		T_LinkList *node;

		shl_dlist_for_each_safe(i, t, list)			
		{
			node = shl_dlist_entry(i, T_LinkList, list);
			if (node->ifindex == ifindex
			&& !strcmp(ifname, node->ifname))
			{
				return true;//link already exists in list
			}
			else if(node->ifindex == ifindex)
			{
				if(node->ifname)
					free(node->ifname);
				node->ifname = strdup(ifname);
				fprintf(stderr,"miracle-agent received link ready signal, with label %d-%s, update list\n", ifindex, ifname);
				return true;
			}
			else if(!strcmp(ifname, node->ifname))
			{
				node->ifindex = ifindex;
				fprintf(stderr,"miracle-agent received link ready signal, with label %d-%s, update list\n", ifindex, ifname);
				return true;
			}
		}
	}

	return false;
}

static void signal_name_report_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	const char* device_name;
	T_Handle *ptHandle = data;
	g_variant_get(parameters, "(s)", &device_name);

	if(NULL == device_name)
		return;
	
	if(ptHandle->friendly_name)
		free(ptHandle->friendly_name);
	ptHandle->friendly_name = strdup(device_name);

	fprintf(stderr,"miracle-agent received friendly name report signal, %s\n", ptHandle->friendly_name);
}

static void signal_link_report_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_LinkList *ent;
	int ifindex;
	const char* ifname;
	T_Handle *ptHandle = data;

	g_variant_get(parameters, "(is)", &ifindex, &ifname);	

	if(NULL == ifname)
		return;

	if(!check_link_in_list_and_update_if_needed(&ptHandle->link_list, ifindex, ifname))
	{
		ent = malloc(sizeof(*ent));

		if(ent)
		{
			memset(ent, 0, sizeof(*ent));
			ent->ifindex = ifindex;
			ent->ifname = strdup(ifname);

			if(OP_NO_RESPONSE == get_hardware_capability(ent->ifname, &ent->coexistWithNM, &ent->iSupportP2p))
			{
				ent->coexistWithNM = -1;
				ent->iSupportP2p = -1;
			}

			shl_dlist_link(&ptHandle->link_list, &ent->list);
			fprintf(stderr,"miracle-agent received link report signal, with label %d-%s, p2p %d, bCoexistWithNM %d, add to list\n", 
				ifindex, ifname, ent->iSupportP2p, ent->coexistWithNM);
		}
		else
			fprintf(stderr,"[ERROR]: miracle-agent malloc link node for %d-%s failed\n", ifindex, ifname);
	}
}

static void signal_link_is_ready_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	int ifindex;
	const char* ifname;
	T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

	if(!ent
		|| (long)ent == (long)&ptHandle->link_list)
	{
		fprintf(stderr,"[FATAL ERROR]Agent hold no iface when signal ready-emit!!\n");
		return;//since there is no wireless card, don't care about 'enable_nm_link_manage' further more
	}

	g_variant_get(parameters, "(is)", &ifindex, &ifname);

	if(NULL == ifname)
		return;

	if(strcmp(ent->ifname, ifname))
		fprintf(stderr,"[WARNING] wifid chose a different link from agent !!!!\n");

	if(ent->coexistWithNM)
		enable_nm_link_manage(ifname);
}

static void signal_link_is_added_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	T_LinkList *ent;
	int ifindex;
	const char* ifname;

	g_variant_get(parameters, "(is)", &ifindex, &ifname);	

	if(NULL == ifname)
		return;

	if(!check_link_in_list_and_update_if_needed(&ptHandle->link_list, ifindex, ifname))
	{
		ent = malloc(sizeof(*ent));

		if(ent)
		{
			memset(ent, 0, sizeof(*ent));
			ent->ifindex = ifindex;
			ent->ifname = strdup(ifname);

			if(OP_NO_RESPONSE == get_hardware_capability(ent->ifname, &ent->coexistWithNM, &ent->iSupportP2p))
			{
				ent->coexistWithNM = -1;
				ent->iSupportP2p = -1;
			}

			shl_dlist_link(&ptHandle->link_list, &ent->list);
			fprintf(stderr,"miracle-agent received link added signal, with label %d-%s, p2p %d, bCoexistWithNM %d, add to list\n", 
				ifindex, ifname, ent->iSupportP2p, ent->coexistWithNM);
		}
		else
			fprintf(stderr,"[ERROR]: miracle-agent malloc link node for %d-%s failed\n", ifindex, ifname);
	}
}

static void remove_link_node(T_LinkList *node)
{
	shl_dlist_unlink(&node->list);

	if(node->ifname)
		free(node->ifname);
	free(node);
}

static void signal_link_is_removed_emit_cb(GDBusConnection *gdConn,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer data)
{
	T_Handle *ptHandle = data;
	int ifindex;
	const char* ifname;

	g_variant_get(parameters, "(is)", &ifindex, &ifname);	

	if(NULL == ifname)
		return;

	if(!shl_dlist_empty(&ptHandle->link_list))
	{
		struct shl_dlist *i, *t;
		T_LinkList *node;

		shl_dlist_for_each_safe(i, t, &ptHandle->link_list)			
		{
			node = shl_dlist_entry(i, T_LinkList, list);
			if (node->ifindex == ifindex
			&& !strcmp(ifname, node->ifname))
			{
				remove_link_node(node);
				fprintf(stderr,"miracle-agent received link removed signal, with label %d-%s, remove from list\n", ifindex, ifname);
				return;
			}
		}
	}

	fprintf(stderr,"[ERROR]: miracle-agent remove link node %d-%s failed, not found\n", ifindex, ifname);
}


static void dbus_notify_projection_disconnected_by_user(T_Handle *ptHandle)
{
	GError *gErr = NULL;
	// if (ptHandle->front_pid <= 0)
	// 	return;
	if (ptHandle->front_pid > 0)
		ptHandle->uiAgtHandle.send_ui_peer_action("disconnected", "", "通知", "投屏设备已请求断开", -1);
	
	if(ptHandle->peer_label && strlen(ptHandle->peer_label))
	{
		g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
							"UsrDisconnect",
							g_variant_new("(s)", ptHandle->peer_label),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&gErr);

		if(gErr)
		{
			fprintf(stderr,"%s dbus_sync_call with peer %s failed for %s\n", __func__, ptHandle->peer_label, gErr->message);
			g_error_free(gErr);
		}
		else
			fprintf(stderr,"Notify wifid Peer %s disconnected with user ui operation\n", ptHandle->peer_label);
	}
	else
	{
		fprintf(stderr,"No peer label stored, do not send dbus notify -- disconnected by user\n");
	}

	T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);
	if(!ent
		|| (long)ent == (long)&ptHandle->link_list)
	{
		fprintf(stderr,"[FATAL ERROR]Agent hold no iface when switch!!, manually\n");
		projection_recycle(ptHandle, NULL, 0);
	}
	else
		projection_recycle(ptHandle, ent->ifname, ent->coexistWithNM);
}

static int dbus_notify_projection_switch(T_Handle *ptHandle,
		bool set, const char *device_name, const char *config_methods)
{
	GError *gErr = NULL;
	bool bNoDevice = false;
	char title[256] = {0};
	char body[256] = {0};
	int timeout = 20000;

	//TODO ifindex and ifname should be submitted from UI as params
	//Now for v1.x, we just chose the 1st one in link_list to use. Mostly, there is only one link in list.
	T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

	if(!ent
		|| (long)ent == (long)&ptHandle->link_list)
	{
		fprintf(stderr,"[ERROR]Agent hold no iface when switch!!\n");
		bNoDevice = true;
	}

	if(set)
	{
		if(bNoDevice)
			return -1;
		
		if(!ptHandle->bWlanEnabled)
		{
			fprintf(stderr,"[ERROR]WLAN is switch off, can not switch on projection!!\n");
			return -1;
		}

		takecharge_interface_from_networkmanager(ptHandle, ent->ifname, ent->coexistWithNM);

		if(-1 == ptHandle->uiAgtHandle.width && -1 == ptHandle->uiAgtHandle.height
			&& init_local_resolution(&ptHandle->uiAgtHandle))
		{
			ptHandle->uiAgtHandle.width = 1920;//default
			ptHandle->uiAgtHandle.height = 1080;
		}
#ifdef WFD_R2
		r2_service_start(ptHandle);
#endif
		fprintf(stderr,"Start with screen resolution %dx%d \n", ptHandle->uiAgtHandle.width, ptHandle->uiAgtHandle.height);    	
    	
	}
	else
	{
		giveback_interface_to_networkmanager(ptHandle, bNoDevice ? NULL : ent->ifname, bNoDevice ? 0 : ent->coexistWithNM, !set);
		dbus_notify_projection_disconnected_by_user(ptHandle);			
		kill_gst(ptHandle->player_pid);
		kill_frontend(ptHandle);
		kill_uibc_daemon(ptHandle);
#ifdef WFD_R2
		r2_service_stop(ptHandle);
#endif
	}

	ptHandle->state = IDLE_STATE;

	snprintf(title, sizeof(title)-1, "投屏接收功能已%s", set ? "开启" : "关闭");
	snprintf(body, sizeof(body)-1, "%s", set ? "请在投屏设备端开启或刷新扫描" : "需要投屏请再重新开启");
	ptHandle->bStarted = set ? true : false;

	if(bNoDevice)//if no device, we don't need to stop running link, as it's already been handled by wifid and sinkctl
		return -1;

	g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
						"ProjectionOp",
						g_variant_new("(biss)", set, ent->ifindex, device_name, config_methods),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&gErr);
	if(gErr)
	{
		fprintf(stderr,"%s failed, ifindex %d \n", __func__, ent->ifindex);
		g_error_free(gErr);

		projection_recycle(ptHandle, ent->ifname, ent->coexistWithNM);
		if(set)
			giveback_interface_to_networkmanager(ptHandle, ent->ifname, ent->coexistWithNM, set);//recover network on dbus-call exception

		return -1;
	}

	fprintf(stderr,"Notify projection switch %s succeed, ifindex %d, device_name %s, config_methods %s\n", 
		set ? "ON" : "OFF", ent->ifindex, device_name ? device_name : "default", config_methods ? config_methods : "default");

	return 0;
}

static int dbus_handshake_with_wifid(T_Handle *ptHandle)
{
	GError *gErr = NULL;

	//Handshake: agent will request all udev-links managed by miracle-wifid, all those will be transmitted on signal LinkReport
	g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
						"HandShake",
						NULL,
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&gErr);

	if(gErr)
	{
		fprintf(stderr,"[FATAL]: dbus_sync_call handshake failed for %s\n", gErr->message);
		g_error_free(gErr);
		return -1;
	}
	else
		fprintf(stderr,"Agent handshake with wifid to fetch available links...\n");
	
	return 0;
}

static int check_hardware_support(T_Handle *ptHandle)
{
	T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

	if(!ent
		|| (long)ent == (long)&ptHandle->link_list)
	{
		fprintf(stderr,"No wireless card found !!!\n");
		return NOT_SUPPORT_P2P;
	}
	
	if(-1 == ent->iSupportP2p
		|| -1 == ent->coexistWithNM)
	{
		fprintf(stderr,"query wireless card again , should not reach here!!!\n");
		return get_hardware_capability(ent->ifname, &ent->coexistWithNM, &ent->iSupportP2p);
	}

	if(!ent->iSupportP2p)
	{
		fprintf(stderr,"Wireless card do not support P2P!!!\n");
		return NOT_SUPPORT_P2P;
	}
	
	if(!ent->coexistWithNM)
	{
		fprintf(stderr,"Wireless card do not support wifi and widi same time!!!\n");
		return SUPPORT_P2P_WITHOUT_DEV;
	}
	else
	{
		fprintf(stderr,"Wireless card support P2P perfectly !!!\n");
		return SUPPORT_P2P_PERFECT;
	}
}

DBusHandlerResult dbus_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
	DBusHandlerResult result;
    DBusMessage *reply = NULL;
	DBusError err;
	int res = 0;
	T_Handle *ptHandle = data;

	fprintf(stderr,"Got D-Bus request: %s.%s on %s\n",
		dbus_message_get_interface(message),
		dbus_message_get_member(message),
		dbus_message_get_path(message));

	/*
	 * Does not allocate any memory; the error only needs to be
	 * freed if it is set at some point.
	 */
	dbus_error_init(&err);

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &server_introspection_xml,
					 DBUS_TYPE_INVALID);

	}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {
		const char *interface, *property;

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &interface,
					   DBUS_TYPE_STRING, &property,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		result = server_get_properties_handler(property, conn, reply);
		dbus_message_unref(reply);
		return result;

	}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll")) {
		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		result = server_get_all_properties_handler(conn, reply);
		dbus_message_unref(reply);
		return result;

	}  else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "PreCheck")) {

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		res = check_hardware_support(ptHandle);

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_STRING, &ptHandle->friendly_name,
					 DBUS_TYPE_INVALID);		

	}  else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "Start")) {
		const char *device_name, *config_methods;
		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &device_name,
					   DBUS_TYPE_STRING, &config_methods,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		if(!ptHandle->bStarted)
		{
			res = dbus_notify_projection_switch(ptHandle, ON, device_name, config_methods);

			if(device_name && strlen(device_name))
			{
				if(!ptHandle->friendly_name)
				{
					ptHandle->friendly_name = strdup(device_name);
				}
				else if(strcmp(ptHandle->friendly_name, device_name))
				{
					free(ptHandle->friendly_name);
					ptHandle->friendly_name = strdup(device_name);
				}
			}
			if(config_methods && strlen(config_methods))
			{
				if(!ptHandle->config_methods)
				{
					ptHandle->config_methods = strdup(config_methods);
				}
				else if(strcmp(ptHandle->config_methods, config_methods))
				{
					free(ptHandle->config_methods);
					ptHandle->config_methods = strdup(config_methods);
				}
			}
		}
		else
		{
			fprintf(stderr,"[WARN] Projection is already started\n");
			res = -1;
		}

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "Stop")) {

		if(ptHandle->bStarted)
			dbus_notify_projection_switch(ptHandle, OFF, "", "");

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "UiSetName")) {
		const char *device_name;
		GError *synCallErr = NULL;
		T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

		if(!ent
			|| (long)ent == (long)&ptHandle->link_list)
		{
			fprintf(stderr,"[FATAL ERROR]Agent hold no iface when set name!!\n");
			if ((reply = dbus_message_new_method_return(message)))
			{
				res = 0;
				dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);
			}
			else
				fprintf(stderr,"dbus reply message is null");
			goto error;
		}

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &device_name,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		if(device_name && strlen(device_name))
		{
			if(!ptHandle->friendly_name)
			{
				ptHandle->friendly_name = strdup(device_name);
			}
			else if(strcmp(ptHandle->friendly_name, device_name))
			{
				free(ptHandle->friendly_name);
				ptHandle->friendly_name = strdup(device_name);
			}
		}
		
		g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
							"SetSinkName",
							g_variant_new("(is)", ent->ifindex, ptHandle->friendly_name),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&synCallErr);

		if(synCallErr)
		{
			fprintf(stderr,"[FATAL]: dbus_sync_call SetSinkName to %s fail for %s\n", ptHandle->friendly_name, synCallErr->message);
			g_error_free(synCallErr);
			goto error;
		}
		else
			fprintf(stderr,"UI SetSinkName to %s notify backend succ...\n", ptHandle->friendly_name);

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "UiSetConfigMethods")) {
		const char *config_methods;
		GError *synCallErr = NULL;
		T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

		if(!ent
			|| (long)ent == (long)&ptHandle->link_list)
		{
			fprintf(stderr,"[FATAL ERROR]Agent hold no iface when set ConfigMethods!!\n");
			if ((reply = dbus_message_new_method_return(message)))
			{
				res = 0;
				dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);
			}
			else
				fprintf(stderr,"dbus reply message is null");
			goto error;
		}

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &config_methods,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		if(config_methods && strlen(config_methods))
		{
			if(!ptHandle->config_methods)
			{
				ptHandle->config_methods = strdup(config_methods);
			}
			else if(strcmp(ptHandle->config_methods, config_methods))
			{
				free(ptHandle->config_methods);
				ptHandle->config_methods = strdup(config_methods);
			}
		}
		
		g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
							"SetSinkConfigMethods",
							g_variant_new("(is)", ent->ifindex, ptHandle->config_methods),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&synCallErr);

		if(synCallErr)
		{
			fprintf(stderr,"[FATAL]: dbus_sync_call SetSinkConfigMethods to %s fail for %s\n", ptHandle->config_methods, synCallErr->message);
			g_error_free(synCallErr);
			goto error;
		}
		else
			fprintf(stderr,"UI SetSinkConfigMethods to %s notify backend succ...\n", ptHandle->config_methods);

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);
	
	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "UiGetConfigMethods")) {
		const char *config_methods;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;
		
		if(!ptHandle->config_methods)
		{
			res = 0;
		}
		else if (!strcmp(ptHandle->config_methods, "display"))
		{
			res = 0;
		}else 
		{
			res = 1;
		}

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "QueryConnectedDevice")) {
		GError *synCallErr = NULL;
		T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);
		GVariant *result;

		if(!ent
			|| (long)ent == (long)&ptHandle->link_list)
		{
			fprintf(stderr,"[FATAL ERROR]Agent hold no iface when query ConnectedDevice!!\n");
			if ((reply = dbus_message_new_method_return(message)))
			{
				res = 0;
				dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);
			}
			else
				fprintf(stderr,"dbus reply message is null");
			goto error;
		}

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;
		
		result = g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
							"QueryConnectedDevice",
							NULL,
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&synCallErr);
		
		if(synCallErr)
		{
			fprintf(stderr,"[FATAL]: dbus_sync_call QueryConnectedDevice fail for %s\n", synCallErr->message);
			g_error_free(synCallErr);
			goto error;
		}
		else
			fprintf(stderr,"UI QueryConnectedDevice notify backend succ...\n");

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_VARIANT,&result,
					 DBUS_TYPE_INVALID);
		
		g_variant_unref(result);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "DeleteConnectedDevice")) {
		const char *mac;
		GError *synCallErr = NULL;
		T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);
		GVariant *result;
		int isdel = -1;

		if(!ent
			|| (long)ent == (long)&ptHandle->link_list)
		{
			fprintf(stderr,"[FATAL ERROR]Agent hold no iface when delete ConnectedDevice!!\n");
			if ((reply = dbus_message_new_method_return(message)))
			{
				res = 0;
				dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);
			}
			else
				fprintf(stderr,"dbus reply message is null");
			goto error;
		}

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &mac,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;
		
		result = g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
							"DeleteConnectedDevice",
							g_variant_new("(is)", ent->ifindex, mac),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&synCallErr);
		
		g_variant_get(result,"(i)",&isdel);

		g_variant_unref(result);

		if(synCallErr)
		{
			fprintf(stderr,"[FATAL]: dbus_sync_call DeleteConnectedDevice fail for %s\n", synCallErr->message);
			g_error_free(synCallErr);
			goto error;
		}
		else
			fprintf(stderr,"UI DeleteConnectedDevice notify backend succ...\n");

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INT32, &isdel,
					 DBUS_TYPE_INVALID);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "Confirmed")) {
		int user_choice;
		char *pin;

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_INT32, &user_choice,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		fprintf(stderr,"miracle-agent: User confirmed is %d\n", user_choice);

		if(USER_DISALLOW == user_choice)
		{
			kill_frontend(ptHandle);
			ptHandle->state = IDLE_STATE;
			memset(ptHandle->pincode, 0, sizeof(ptHandle->pincode));//NULL pincode if user disallowed
		}

		if(ptHandle->confirmTimerid)
		{
			g_source_remove(ptHandle->confirmTimerid);//stop the timer
			ptHandle->confirmTimerid = 0;
		}
		pin = &ptHandle->pincode[0];
		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_STRING, &pin,
					 DBUS_TYPE_INVALID);
		
		notify_wifid_user_choice(ptHandle, user_choice);

	} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "SetManufacturer")) {
		int manufacturer;
		char *pin;
		GError *synCallErr = NULL;
		T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

		if(!ent
			|| (long)ent == (long)&ptHandle->link_list)
		{
			fprintf(stderr,"[FATAL ERROR]Agent hold no iface when set manufacturer!!\n");
			if ((reply = dbus_message_new_method_return(message)))
			{
				res = 0;
				dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);
			}
			else
				fprintf(stderr,"dbus reply message is null");
			goto error;
		}

		if (!dbus_message_get_args(message, &err,
					   DBUS_TYPE_INT32, &manufacturer,
					   DBUS_TYPE_INVALID))
			goto error;

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
							"SetManufacturer",
							g_variant_new("(ii)", ent->ifindex, manufacturer),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&synCallErr);

		if(synCallErr)
		{
			fprintf(stderr,"[FATAL]: dbus_sync_call SetManufacturer fail for %s\n", synCallErr->message);
			g_error_free(synCallErr);
			goto error;
		}
		else
			fprintf(stderr,"UI SetManufacturer notify backend succ...\n");

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);

} else if (dbus_message_is_method_call(message, "org.freedesktop.miracleagent.op", "disconnect")) {

		dbus_notify_projection_disconnected_by_user(ptHandle);

		if (!(reply = dbus_message_new_method_return(message)))
			goto error;

		dbus_message_append_args(reply,
					 DBUS_TYPE_INT32, &res,
					 DBUS_TYPE_INVALID);

} else
{
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

error:
	if (dbus_error_is_set(&err)) {
		if (reply)
			dbus_message_unref(reply);
		reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}

	/*
	 * In any cases we should have allocated a reply otherwise it
	 * means that we failed to allocate one.
	 */
	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	/* Send the reply which might be an error one too. */
	result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	dbus_message_unref(reply);

	return result;
}

const DBusObjectPathVTable dbus_vtable = {
	.message_function = dbus_message_handler
};

static void sigchild_frontend_proc(T_Handle *ptHandle, siginfo_t* ptInfo)
{	
	if(1 == ptInfo->si_status
	&& CLD_EXITED == ptInfo->si_code)
	{
		fprintf(stderr,"[WARN] Frontend sigchld not exited or killed: si_code %d, si_status %d, pid %d \n", 
					ptInfo->si_code, ptInfo->si_status, ptHandle->front_pid);
		return;
	}

	if(ptHandle->front_pid <= 0)
	{
		fprintf(stderr,"[WARN] Frontend sigchld lost its pid: si_code %d, si_status %d, pid %d \n", 
					ptInfo->si_code, ptInfo->si_status, ptHandle->front_pid);
		return;
	}

	if(CLD_EXITED == ptInfo->si_code)
		ptHandle->state = IDLE_STATE;

	fprintf(stderr,"child frontend [%d] is killed\n", ptHandle->front_pid);
	ptHandle->front_pid = 0;
}

static void sigchild_player_proc(T_Handle *ptHandle, siginfo_t* ptInfo)
{
	if(ptHandle->player_pid <= 0)
	{
		fprintf(stderr,"[WARN] No castplayer child process actually running or already killed\n");
		return;
	}

	ptHandle->player_pid = 0;

	kill_uibc_daemon(ptHandle);//uibc daemon must be killed with player

	if(CLD_EXITED == ptInfo->si_code)//close by user manually
	{
		fprintf(stderr,"gst player is closed by user-operation\n");
	}
	else
	{
		fprintf(stderr,"gst player is closed by signal\n");
		dbus_notify_projection_disconnected_by_user(ptHandle);
	}
}

static void sigchild_uibc_daemon_proc(T_Handle *ptHandle, siginfo_t* ptInfo)
{
	if(ptHandle->uibc_daemon_pid <= 0)
	{
		fprintf(stderr,"[WARN] No uibc daemon child process actually running or already killed\n");
		return;
	}

	fprintf(stderr,"uibc daemon is closed.....\n");
	ptHandle->uibc_daemon_pid = 0;
	remove_uibc_pipe(ptHandle);
}

static void sys_signal_handler(int sig, siginfo_t *info, void *ctx)
{
	

	fprintf(stderr,"miracle-agent SIGCHLD proc, signal matchs %d, si_code %d, uid is %d\n", (sig==SIGCHLD), info->si_code, info->si_status);

	int iStatus = 0;

	pid_t child_pid = waitpid(-1, &iStatus, WNOHANG);

	if(child_pid >= 0)
	{
		if(child_pid == s_ptHandle->player_pid)
			sigchild_player_proc(s_ptHandle, info);
		else if(child_pid == s_ptHandle->front_pid)
			sigchild_frontend_proc(s_ptHandle, info);
		else if(child_pid == s_ptHandle->uibc_daemon_pid)
			sigchild_uibc_daemon_proc(s_ptHandle, info);
		else
			fprintf(stderr,"[NOTICE] SIGCHLD get a ignored child pid %d \n", child_pid);
	}
	else
		fprintf(stderr,"[WARN] SIGCHLD waitpid ret %d, not exited normally\n", child_pid);
	
}

static void dbus_wifid_signals_subscribe(T_Handle *ptHandle, const char* signame, SIG_EMIT_HANDLE handler)
{
	guint id = g_dbus_connection_signal_subscribe(ptHandle->dbushandle.sysBusConn,
						"org.freedesktop.miracle.wifi",
						"org.freedesktop.miracle.wifi.ui",
						signame,
						"/org/freedesktop/miracle/wifi/ui",
						NULL,
						G_DBUS_SIGNAL_FLAGS_NONE,
						handler,
						ptHandle,
						NULL);
}

static void wlan_switch_changed_signal_cb(GDBusConnection *gdConn,
								const gchar *sender_name,
								const gchar *object_path,
								const gchar *interface_name,
								const gchar *signal_name,
								GVariant *parameters,
								gpointer data)
{
	T_Handle *ptHandle = data;
	gchar *key;
	GVariant *value;
	GVariantIter *iter;

	g_variant_get (parameters, "(a{sv})", &iter);

    while (iter
		&& g_variant_iter_loop(iter, "{sv}", &key, &value))
	{
        if (key && value
			&& !g_strcmp0(key, "WirelessEnabled"))
		{
			if(ptHandle->bWlanEnabled != g_variant_get_boolean(value))
			{
				ptHandle->bWlanEnabled = g_variant_get_boolean(value);
				fprintf(stderr,"WLAN switch status changed to %s, %s\n", ptHandle->bWlanEnabled ? "ON" : "OFF", ptHandle->bStarted ? "so switch off projection" : "but projection is off");
				if(!ptHandle->bWlanEnabled && ptHandle->bStarted)
					dbus_notify_projection_switch(ptHandle, OFF, "","");
			}
        }
    }

	g_variant_iter_free(iter);
}

static void dbus_wlan_status_signals_subscribe(T_Handle *ptHandle, SIG_EMIT_HANDLE handler)
{
	    guint id = g_dbus_connection_signal_subscribe(ptHandle->dbushandle.sysBusConn,
		                        "org.freedesktop.NetworkManager",                        
								"org.freedesktop.NetworkManager",                        
								"PropertiesChanged",                        
								"/org/freedesktop/NetworkManager",                        
								NULL,
								G_DBUS_SIGNAL_FLAGS_NONE,
								handler, 
								ptHandle,                        
								NULL);
}

static void dbus_query_init_wlan_status(T_Handle *ptHandle)
{
	GError *synCallErr = NULL;
	GVariant *value;
	GVariant *result;
	bool bEnabled = false;

	GDBusProxy* sysBusProxy = g_dbus_proxy_new_sync(ptHandle->dbushandle.sysBusConn,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.NetworkManager",
			"/org/freedesktop/NetworkManager",
			"org.freedesktop.DBus.Properties",
			NULL,
			&synCallErr);

	if(!sysBusProxy || synCallErr)
	{
		fprintf(stderr,"[FATAL]:Agent get NetworkManager proxy failed\n");
		goto exit;
	}

	result = g_dbus_proxy_call_sync(sysBusProxy,
						"Get",
						g_variant_new("(ss)", "org.freedesktop.NetworkManager", "WirelessEnabled"),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&synCallErr);

	if(synCallErr)
	{
		fprintf(stderr,"[FATAL]: dbus_sync_call NetworkManager Get fail for %s\n", synCallErr->message);
		goto exit;
	}
	else
	{
		g_variant_get(result, "(v)", &value);
		bEnabled = g_variant_get_boolean(value);
		fprintf(stderr,"dbus_sync_call NetworkManager Get succ, with enabled %d\n", bEnabled);
	}
	
	ptHandle->bWlanEnabled = bEnabled;

exit:
	if(sysBusProxy)
		g_object_unref(sysBusProxy);

	if(synCallErr) 
		g_error_free(synCallErr);
}

static int dbus_with_wifid_init(T_Handle *ptHandle)
{
	GError *gdError = NULL;
	int trycnt = 0;
	const int TRYMAX = 20;
	int r = 0;	

	while(!ptHandle->dbushandle.sysBusConn && trycnt++ < TRYMAX)
	{
		ptHandle->dbushandle.sysBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &gdError);
	}

	if(trycnt >= TRYMAX)
	{
		fprintf(stderr,"[FATAL]:Agent get system-bus handle failed\n");
		r = -1;
		goto error;
	}

	trycnt = 0;

	while(!ptHandle->dbushandle.sysBusProxy && trycnt++ < TRYMAX)
	{
		ptHandle->dbushandle.sysBusProxy = g_dbus_proxy_new_sync(ptHandle->dbushandle.sysBusConn,
				G_DBUS_PROXY_FLAGS_NONE,
				NULL,
				"org.freedesktop.miracle.wifi",
				"/org/freedesktop/miracle/wifi/ui",
				"org.freedesktop.miracle.wifi.ui",
				NULL,
				&gdError);
	}

	if(trycnt >= TRYMAX)
	{
		fprintf(stderr,"[FATAL]:Agent get system-bus proxy failed\n");
		r = -1;
		goto error;
	}

	dbus_query_init_wlan_status(ptHandle);
	dbus_wlan_status_signals_subscribe(ptHandle, wlan_switch_changed_signal_cb);

	dbus_wifid_signals_subscribe(ptHandle, "ProvDisc", signal_provdisc_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "GoNegReq", signal_rcv_goneg_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "GoNegUsrApprove", signal_goneg_user_approve_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "Invitation", signal_rcv_invite_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "PeerQuit", signal_disconn_emit_cb);

	dbus_wifid_signals_subscribe(ptHandle, "SpawnPlayer", signal_spawn_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "StopPlayerFast", signal_disconn_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "RtspFailed", signal_rtsp_failed_cb);
#ifdef WFD_R2	
	dbus_wifid_signals_subscribe(ptHandle, "R2SessStopped", signal_r2_session_stopped);
#endif
	dbus_wifid_signals_subscribe(ptHandle, "FormationOk", signal_formation_succ_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "KillAgent", signal_kill_agent_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "FormationFailure", signal_formation_fail_emit_cb);
	
	//warning or information
	dbus_wifid_signals_subscribe(ptHandle, "ssidInvalid", signal_ssid_invalid_warn_emit_cb);

	//handshake
	dbus_wifid_signals_subscribe(ptHandle, "LinkReport", signal_link_report_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "NameReport", signal_name_report_emit_cb);

	//link management
	dbus_wifid_signals_subscribe(ptHandle, "LinkAdded", signal_link_is_added_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "LinkRemoved", signal_link_is_removed_emit_cb);
	dbus_wifid_signals_subscribe(ptHandle, "LinkIsReady", signal_link_is_ready_emit_cb);
	
	r = dbus_handshake_with_wifid(ptHandle);
	
	if(r)
		goto error;

	return r;

error:
	if(ptHandle->dbushandle.sysBusConn)
	{
		g_object_unref(ptHandle->dbushandle.sysBusConn);
		ptHandle->dbushandle.sysBusConn = NULL;
	}

	if(ptHandle->dbushandle.sysBusProxy)
	{
		g_object_unref(ptHandle->dbushandle.sysBusProxy);
		ptHandle->dbushandle.sysBusProxy = NULL;
	}
	
	return r;
}

static void signal_init(void)
{
	// 	SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGCHLD
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(sigact));
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_handler = NULL;
	sigact.sa_sigaction = sys_signal_handler;

	if(sigaction(SIGCHLD, &sigact, NULL))
		fprintf(stderr,"signal handler install failed\n");		
	
	
}

static int handle_init(int argc, char **argv, T_Handle **out)
{
	T_Handle *ptHandle = malloc(sizeof(*ptHandle));
	if(!ptHandle)
	{		
		return -1;
	}

	memset(ptHandle, 0, sizeof(*ptHandle));

	shl_dlist_init(&ptHandle->link_list);

	init_ui_handle(argc, argv, &ptHandle->uiAgtHandle, &ptHandle->dbushandle);

	ptHandle->bWlanEnabled = true;//default is true

	*out = ptHandle;

	return 0;
}

int main(int argc, char **argv)
{
	int r = 0;
	DBusError err;
    GMainLoop *mainloop;
	int fd_journal = -1;

	if(handle_init(argc, argv, &s_ptHandle))
	{		
		return EXIT_FAILURE;
	}
	
/* dbus init(session) */
	// 1. comunicate with control-center :: "Start" and "Stop" method
	// 2. comunicate with org.freedesktop.Notifications

	fd_journal = sd_journal_stream_fd("miracle-agent", LOG_DEBUG, false);
	if(0 <= fd_journal)
	{
		dup2(fd_journal, 1);
		dup2(fd_journal, 2);
	}
	else
	{
		dup2(2, 1);
	}

    dbus_error_init(&err);

	s_ptHandle->dbushandle.sessBusConn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (!s_ptHandle->dbushandle.sessBusConn) {
		fprintf(stderr,"Failed to get a session DBus connection: %s\n", err.message);
		goto error;
	}

	r = dbus_bus_request_name(s_ptHandle->dbushandle.sessBusConn, "org.freedesktop.miracleagent", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (r != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		fprintf(stderr,"Failed to request name on bus: %s\n", err.message);
		goto error;
	}

	if(dbus_with_wifid_init(s_ptHandle))
		goto error;

	if (!dbus_connection_register_object_path(s_ptHandle->dbushandle.sessBusConn, "/org/freedesktop/miracleagent", &dbus_vtable, s_ptHandle)) {
		fprintf(stderr,"Failed to register a object path for 'TestObject'\n");
		goto error;
	}
	
	/* Set up the DBus connection to work in a GLib event loop */
	dbus_connection_setup_with_g_main(s_ptHandle->dbushandle.sessBusConn, NULL);

#ifdef WFD_R2
	wfd_r2_support_init(s_ptHandle);
#endif

	fprintf(stderr,"Now miracle-agent prepare to run main_loop\n");

	/* event-loop */
	mainloop = g_main_loop_new(NULL, false);

	signal_init();

	g_main_loop_run(mainloop);

    g_main_loop_unref(mainloop);

	return EXIT_SUCCESS;

error:
	dbus_error_free(&err);
	return EXIT_FAILURE;
}