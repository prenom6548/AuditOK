#include "agent.h"

#ifdef WFD_R2

static const unsigned short r2_port = 7250;

typedef struct
{
	char type;
	char len;
	unsigned short port;
}T_R2_REQ_CMD;

static int r2_connection_close(T_Handle *ptHandle)
{
    if(ptHandle->r2_connection_Handle.socket)
    {
        GError *error = NULL;

        if(false == g_socket_close(ptHandle->r2_connection_Handle.socket, &error))
        {
            g_autofree gchar *peer_addr = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(ptHandle->r2_connection_Handle.sockaddr)));
            fprintf(stderr, "connection close failed with %s\n", peer_addr);
        }
        
        if(error)
            g_free(error);
        
        // g_object_unref(ptHandle->r2_connection_Handle.socket);
        ptHandle->r2_connection_Handle.socket = NULL;
    }

    if(ptHandle->r2_connection_Handle.sockaddr)
    {
        g_object_unref(ptHandle->r2_connection_Handle.sockaddr);
        ptHandle->r2_connection_Handle.sockaddr = NULL;
    }

    if(ptHandle->r2_connection_Handle.conn)
    {
        g_object_unref(ptHandle->r2_connection_Handle.conn);
        ptHandle->r2_connection_Handle.conn = NULL;
    }

    // if(ptHandle->r2_connection_Handle.channel)
    // {        
    //     g_source_remove(ptHandle->r2_connection_Handle.io_id);
    //     g_io_channel_shutdown(ptHandle->r2_connection_Handle.channel, true, NULL);
    //     g_io_channel_unref(ptHandle->r2_connection_Handle.channel);        
    //     ptHandle->r2_connection_Handle.channel = NULL;
    //     ptHandle->r2_connection_Handle.io_id = 0;
    // }

    return 0;
}

static int r2_connection_receive_data(T_Handle *ptHandle, GSocketConnection *connection, 
    gboolean(*pReadfunc)(GIOChannel *channel, GIOCondition condition, gpointer data))
{
    g_object_ref(connection);

    GSocketAddress *sockaddr = g_socket_connection_get_remote_address(connection, NULL);
    GSocket *conn_socket;

    guint16 peer_port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(sockaddr));    
    g_autofree gchar *g_str_addr = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(sockaddr)));
    fprintf(stderr, "r2 service receved a new Connection from %s:%d\n", g_str_addr, peer_port);

    //read client
    conn_socket = g_socket_connection_get_socket(connection);
    ptHandle->r2_connection_Handle.channel = g_io_channel_unix_new(g_socket_get_fd(conn_socket));
    if(ptHandle->r2_connection_Handle.channel) {
		ptHandle->r2_connection_Handle.socket = conn_socket;
		ptHandle->r2_connection_Handle.sockaddr = sockaddr;
        ptHandle->r2_connection_Handle.conn = connection;
        ptHandle->r2_connection_Handle.io_id = g_io_add_watch(ptHandle->r2_connection_Handle.channel, G_IO_IN, (GIOFunc) pReadfunc, ptHandle);
        g_io_channel_unref(ptHandle->r2_connection_Handle.channel);
        //ptHandle->r2_connection_Handle.start_receive_timeout(&ptHandle->r2_connection_Handle);
    }
    else
    {
        g_object_unref(sockaddr);
        g_object_unref(conn_socket);
        g_object_unref(connection);
        fprintf(stderr, "r2 service add source watch failed\n");
        return -1;
    }

    return 0;
}

static gboolean read_msg(GIOChannel *channel, GIOCondition condition, gpointer data)
{    
    T_Handle *ptHandle = data;
	T_R2_REQ_CMD tR2Command = {0};
    gsize len = 0;	
    gboolean bret = false;
	GIOStatus ioRet = g_io_channel_read_chars(channel, (gchar*)&tR2Command, sizeof(tR2Command), &len, NULL);

    if (ioRet == G_IO_STATUS_ERROR){        
        fprintf(stderr, "Error reading r2 connection\n");
        // g_object_unref(data);
    }
    else if (ioRet == G_IO_STATUS_EOF) {
        fprintf(stderr, "r2 peer connection read out eof\n");
    }
    else {
        // if(len == sizeof(tR2Command)
        //     && 0 == tR2Command.type
        //     && sizeof(tR2Command.port) == tR2Command.len)
		{
            GError *error = NULL;
	        gchar *remote_addr = NULL;	
            T_LinkList *ent = shl_dlist_last_entry(&ptHandle->link_list, T_LinkList, list);

            if(!ent
                || (long)ent == (long)&ptHandle->link_list)
            {
                fprintf(stderr, "[FATAL ERROR]Agent hold no iface when start r2 session!!\n");
                goto exit;
            }
            remote_addr = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(ptHandle->r2_connection_Handle.sockaddr)));
            tR2Command.port = 7236;
            fprintf(stderr, "r2 peer sent cmd with addr %s, port %u\n", remote_addr, tR2Command.port);
            g_dbus_proxy_call_sync(ptHandle->dbushandle.sysBusProxy,
                                "AgtStartR2Sess",
                                g_variant_new("(sii)", (char*)remote_addr, tR2Command.port, ent->ifindex),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

            g_free(remote_addr);

            if(error)
            {
                fprintf(stderr, "r2 connection call sess-start failed\n");
                g_free(error);
            }
            else
            {
                ptHandle->r2_connection_Handle.bR2InUse = true;
                // g_socket_listener_close(G_SOCKET_LISTENER(ptHandle->r2_connection_Handle.service));
                bret = true;
            }
        }
    }

exit:
    r2_connection_close(ptHandle);
	return bret;
}

static gboolean new_connection_income(GSocketService *service,
              GSocketConnection *connection,
              GObject *source_object,
              gpointer user_data)
{
	T_Handle *ptHandle = user_data;
    int r;

    if(!ptHandle->r2_connection_Handle.isStarted)
    {
        GSocket *conn_socket = g_socket_connection_get_socket(connection);
        g_socket_close(conn_socket, NULL);
        fprintf(stderr, "DROP A INCOMING CONNECTION\n");
        return false;
    }

    fprintf(stderr, "A NEW CONNECTION is COMING\n");
    if(ptHandle->r2_connection_Handle.conn)
    {
        fprintf(stderr, "r2 service already handle a connection, don't accept a new one\n");
        return false;
    }

    r = r2_connection_receive_data(ptHandle, connection, read_msg);

    return r ? false : true;
}

static int publish_display_service(T_Handle *ptHandle)
{
	int r;
	char hostname[256] = {0};
	char command[512] = {0};

	r = gethostname(hostname, sizeof(hostname)-1);
	if(r || ENAMETOOLONG == errno)
	{
		fprintf(stderr, "publish display service failed with %d\n", r);
		return r;
	}

	snprintf(command, sizeof(command)-1, "avahi-publish-service %s _display._tcp %u &", hostname, ptHandle->r2_connection_Handle.port);

	r = system(command);
	
	fprintf(stderr, "publish display service %s \n", r ? "failed" : "success");

	return r;
}

static void r2_version_socket_init(T_Handle *ptHandle)
{
    ptHandle->r2_connection_Handle.service = g_socket_service_new();   
#if 1
    GInetAddress *address = g_inet_address_new_from_string("0.0.0.0");
    gboolean bret;

    ptHandle->r2_connection_Handle.listen_addr = g_inet_socket_address_new(address, ptHandle->r2_connection_Handle.port);
    bret = g_socket_listener_add_address(G_SOCKET_LISTENER(ptHandle->r2_connection_Handle.service), 
                                ptHandle->r2_connection_Handle.listen_addr, G_SOCKET_TYPE_STREAM,
                                G_SOCKET_PROTOCOL_TCP, NULL, NULL, NULL);
    if(!bret)
    {
        fprintf(stderr, "r2 service init failed @ socket listen\n");
        return;
    }

	g_signal_connect(ptHandle->r2_connection_Handle.service, "incoming", G_CALLBACK(new_connection_income), ptHandle);

    g_object_unref(ptHandle->r2_connection_Handle.listen_addr);
    g_object_unref(address);
#endif
}


void wfd_r2_support_init(T_Handle *ptHandle)
{
	int r;

	ptHandle->r2_connection_Handle.port = r2_port;

	r = publish_display_service(ptHandle);
	if(r)
		return;

	r2_version_socket_init(ptHandle);
}

int r2_service_start(T_Handle *ptHandle)
{       
    ptHandle->r2_connection_Handle.isStarted = true;
    ptHandle->r2_connection_Handle.bR2InUse = false;
    // g_socket_service_start(ptHandle->r2_connection_Handle.service);

    fprintf(stderr, "r2 service is started\n");

    return 0;
}

void r2_service_stop(T_Handle *ptHandle)
{    
    // g_socket_service_stop(ptHandle->r2_connection_Handle.service);

    r2_connection_close(ptHandle);

    ptHandle->r2_connection_Handle.isStarted = false;
    ptHandle->r2_connection_Handle.bR2InUse = false;
    fprintf(stderr, "r2 service is stopped\n");
}


#endif