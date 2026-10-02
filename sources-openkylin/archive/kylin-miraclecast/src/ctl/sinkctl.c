/*
 * MiracleCast - Wifi-Display/Miracast Implementation
 *
 * Copyright (c) 2013-2014 David Herrmann <dh.herrmann@gmail.com>
 *
 * MiracleCast is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * MiracleCast is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiracleCast; If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/signalfd.h>
#include <sys/time.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>
#include <systemd/sd-journal.h>
#include <time.h>
#include <unistd.h>
#include "ctl.h"
#include "ctl-sink.h"
#include "wfd.h"
#include "shl_macro.h"
#include "shl_util.h"
#include "util.h"
#include "config.h"

static sd_bus *bus;
static struct ctl_wifi *wifi;
static struct ctl_sink *sink;
static struct ctl_sink *r2_sink;
static sd_event_source *scan_timeout;
static sd_event_source *sink_timeout;
static sd_event_source *rtsp_timeout = NULL;
static unsigned int sink_timeout_time;
static bool sink_connected;
static pid_t sink_pid;

static char *bound_link;
static struct ctl_link *running_link;
static struct ctl_peer *running_peer;
static struct ctl_peer *pending_peer;
static struct gst_child_info st_gst_child_info;

// void launch_player(struct ctl_sink *s);

char *gst_scale_res;
int gst_audio_en = 1;
static const int DEFAULT_RSTP_PORT = 1991;
bool uibc_option;
bool uibc_enabled;
bool external_player;
int rstp_port;
int uibc_port;
char* player;

unsigned int wfd_supported_res_cea  = 0x0001ffff;
unsigned int wfd_supported_res_vesa = 0x1fffffff;
unsigned int wfd_supported_res_hh   = 0x00001fff;

/*
 * cmd list
 */

static int cmd_list(char **args, unsigned int n)
{
	size_t link_cnt = 0, peer_cnt = 0;
	struct shl_dlist *i, *j;
	struct ctl_link *l;
	struct ctl_peer *p;

	/* list links */

	cli_printf("%6s %-24s %-30s %-10s\n",
		   "LINK", "INTERFACE", "FRIENDLY-NAME", "MANAGED");

	shl_dlist_for_each(i, &wifi->links) {
		l = link_from_dlist(i);
		++link_cnt;

		cli_printf("%6s %-24s %-30s %-10s\n",
			   l->label,
			   shl_isempty(l->ifname) ?
			       "<unknown>" : l->ifname,
			   shl_isempty(l->friendly_name) ?
			       "<unknown>" : l->friendly_name,
			   l->managed ? "yes": "no");
	}

	cli_printf("\n");

	/* list peers */

	cli_printf("%6s %-24s %-30s %-10s\n",
		   "LINK", "PEER-ID", "FRIENDLY-NAME", "CONNECTED");

	shl_dlist_for_each(i, &wifi->links) {
		l = link_from_dlist(i);

		shl_dlist_for_each(j, &l->peers) {
			p = peer_from_dlist(j);
			++peer_cnt;

			cli_printf("%6s %-24s %-30s %-10s\n",
				   p->l->label,
				   p->label,
				   shl_isempty(p->friendly_name) ?
				       "<unknown>" : p->friendly_name,
				   p->connected ? "yes" : "no");
		}
	}

	cli_printf("\n %u peers and %u links listed.\n", peer_cnt, link_cnt);

	return 0;
}

/*
 * cmd: show
 */

static int cmd_show(char **args, unsigned int n)
{
	struct ctl_link *l = NULL;
	struct ctl_peer *p = NULL;

	if (n > 0) {
		if (!(l = ctl_wifi_find_link(wifi, args[0])) &&
		    !(p = ctl_wifi_find_peer(wifi, args[0])) &&
		    !(l = ctl_wifi_search_link(wifi, args[0])) &&
		    !(p = ctl_wifi_search_peer(wifi, args[0]))) {
			cli_error("unknown link or peer %s", args[0]);
			return 0;
		}
	}

	if (l) {
		cli_printf("Link=%s\n", l->label);
		if (l->ifindex > 0)
			cli_printf("InterfaceIndex=%u\n", l->ifindex);
		if (l->ifname && *l->ifname)
			cli_printf("InterfaceName=%s\n", l->ifname);
		if (l->friendly_name && *l->friendly_name)
			cli_printf("FriendlyName=%s\n", l->friendly_name);
		cli_printf("P2PScanning=%d\n", l->p2p_scanning);
		if (l->wfd_subelements && *l->wfd_subelements)
			cli_printf("WfdSubelements=%s\n", l->wfd_subelements);
		cli_printf("Managed=%d\n", l->managed);
	} else if (p) {
		cli_printf("Peer=%s\n", p->label);
		if (p->p2p_mac && *p->p2p_mac)
			cli_printf("P2PMac=%s\n", p->p2p_mac);
		if (p->friendly_name && *p->friendly_name)
			cli_printf("FriendlyName=%s\n", p->friendly_name);
		cli_printf("Connected=%d\n", p->connected);
		if (p->interface && *p->interface)
			cli_printf("Interface=%s\n", p->interface);
		if (p->local_address && *p->local_address)
			cli_printf("LocalAddress=%s\n", p->local_address);
		if (p->remote_address && *p->remote_address)
			cli_printf("RemoteAddress=%s\n", p->remote_address);
		if (p->wfd_subelements && *p->wfd_subelements)
			cli_printf("WfdSubelements=%s\n", p->wfd_subelements);
	} else {
		cli_printf("Show what?\n");
		return 0;
	}

	return 0;
}

/*
 * cmd: run
 */

static void run_on(struct ctl_link *l)//deprecated
{
	// if (running_link)
	// 	return;

	// running_link = l;	
	// ctl_link_set_wfd_subelements(l, "000600111c4400c8");
	// ctl_link_set_p2p_scanning(l, true);
	// cli_printf("now running on link %s\n", running_link->label);
}

static int cmd_run(char **args, unsigned int n)
{
	struct ctl_link *l;

	if (running_link) {
		cli_error("already running on %s", running_link->label);
		return 0;
	}

	l = ctl_wifi_search_link(wifi, args[0]);
	if (!l) {
		cli_error("unknown link %s", args[0]);
		return 0;
	}

	if (!l->managed) {
		cli_printf("link %s not managed\n", l->label);
		return 0;
	}

	run_on(l);

	return 0;
}

/*
 * cmd: bind
 */

static int cmd_bind(char **args, unsigned int n)
{
	struct ctl_link *l;
	char *t;

	if (running_link) {
		cli_error("already running on %s", running_link->label);
		return 0;
	}

	t = strdup(args[0]);
	if (!t)
		cli_vENOMEM();

	free(bound_link);
	bound_link = t;

	l = ctl_wifi_search_link(wifi, bound_link);
	if (!l)
		return 0;

	if (!l->managed) {
		cli_printf("link %s not managed\n", l->label);
		return 0;
	}

	run_on(l);

	return 0;
}

/*
 * cmd: set-managed
 */

static int cmd_set_managed(char **args, unsigned int n)
{
	struct ctl_link *l = NULL;
	bool managed = true;

	l = ctl_wifi_search_link(wifi, args[0]);
	if (!l) {
		cli_error("unknown link %s", args[0]);
		return 0;
	}

	if (!strcmp(args[1], "no")) {
		managed = false;
	}
	return ctl_link_set_managed(l, managed);
}

/*
 * cmd: quit/exit
 */

static int cmd_quit(char **args, unsigned int n)
{
	cli_exit();
	return 0;
}

/*
 * main
 */

static void schedule_timeout(sd_event_source **out,
			     uint64_t rel_usec,
			     sd_event_time_handler_t timeout_fn,
			     void *data)
{
	int r;

	rel_usec += shl_now(CLOCK_MONOTONIC);

	if (*out) {
		r = sd_event_source_set_time(*out, rel_usec);
		if (r < 0)
			cli_vERR(r);
	} else {
		r = sd_event_add_time(cli_event,
				      out,
				      CLOCK_MONOTONIC,
				      rel_usec,
				      0,
				      timeout_fn,
				      data);
		if (r < 0)
			cli_vERR(r);
	}
}

static void stop_timeout(sd_event_source **out)
{
	if (*out) {
		sd_event_source_set_enabled(*out, SD_EVENT_OFF);
		sd_event_source_unref(*out);
		*out = NULL;
	}
}

static int scan_timeout_fn(sd_event_source *s, uint64_t usec, void *data)
{
	stop_timeout(&scan_timeout);

	if (pending_peer) {
		if (cli_running()) {
			cli_printf("[" CLI_RED "TIMEOUT" CLI_DEFAULT "] waiting for %s\n",
				pending_peer->friendly_name);
		}
		pending_peer = NULL;
	}

	if (running_link)
		ctl_link_set_p2p_scanning(running_link, true);

	return 0;
}

static int rtsp_timeout_fn(sd_event_source *s, uint64_t usec, void *data)
{
	struct ctl_peer *p = data;

	cli_printf("[FATAL ERROR] peer %s rtsp negotiation timeout\n", p->label);
	if(rtsp_timeout)
	{
		ctl_peer_disconnect(p);
		stop_timeout(&rtsp_timeout);
	}
	return 0;
}

static int sink_timeout_fn(sd_event_source *s, uint64_t usec, void *data)
{
	int r;

	stop_timeout(&sink_timeout);

	if (running_peer &&
	    running_peer->connected &&
	    ctl_sink_is_closed(sink)) {
		cli_log("\nremote_peer %s L2-L3 link established, now trying L4....\n", running_peer->remote_address);

		r = ctl_sink_connect(sink, running_peer->remote_address);//dhcp for remote is finished @ this time				
		if (r < 0) {			
			if (sink_timeout_time++ >= 3)
				cli_vERR(r);
			else
				schedule_timeout(&sink_timeout,
						 sink_timeout_time * 1000ULL * 1000ULL,
						 sink_timeout_fn,
						 NULL);
			cli_log("sink connect to %s failed for %d times", running_peer->friendly_name, sink_timeout_time);
		}
	}

	return 0;
}

int sinkctl_r2_session_start_on_dbus_notify(const char* addr, int port, struct ctl_link *l)
{
	cli_log("A R2 session procedure with %s:%d is started\n", addr, port);

	r2_sink->running_link = l;
	r2_sink->isR2Sink = true;

	return ctl_sink_r2_session_start(r2_sink, addr, port);
}


static const struct cli_cmd cli_cmds[] = {
	{ "list",		NULL,					CLI_M,	CLI_LESS,	0,	cmd_list,		"List all objects" },
	{ "show",		"<link|peer>",				CLI_M,	CLI_LESS,	1,	cmd_show,		"Show detailed object information" },
	{ "run",		"<link>",				CLI_M,	CLI_EQUAL,	1,	cmd_run,		"Run sink on given link" },
	{ "bind",		"<link>",				CLI_M,	CLI_EQUAL,	1,	cmd_bind,		"Like 'run' but bind the link name to run when it is hotplugged" },
	{ "set-managed",	"<link> <yes|no>",	CLI_M,	CLI_EQUAL,	2,	cmd_set_managed,	"Manage or unmnage a link" },
	{ "quit",		NULL,					CLI_Y,	CLI_MORE,	0,	cmd_quit,		"Quit program" },
	{ "exit",		NULL,					CLI_Y,	CLI_MORE,	0,	cmd_quit,		NULL },
	{ "help",		NULL,					CLI_M,	CLI_MORE,	0,	NULL,			"Print help" },
	{ },
};

void ctl_fn_sink_connected(struct ctl_sink *s)
{
	sink_connected = true;
}

void ctl_fn_sink_disconnected(struct ctl_sink *s)
{
	if (!sink_connected) {
		/* treat HUP as timeout */
		sink_timeout_fn(sink_timeout, 0, NULL);
	} else {
		cli_notice("SINK disconnected");
		sink_connected = false;
	}
}

void ctl_fn_sink_resolution_set(struct ctl_sink *s)
{
	if (sink_connected && s->hres && s->vres)
	{
		cli_printf("SINK set resolution %dx%d, and stop rtsp timeout\n", s->hres, s->vres);
		stop_timeout(&rtsp_timeout);
		ctl_notify_spawn_player(s->hres, s->vres, uibc_enabled,s->hidcEn, rstp_port, uibc_port, 
				s->target, running_peer ? running_peer->label : "R2Session", running_link ? : s->running_link);
	}
	else
		cli_printf("SINK invalid resolution setting %dx%d\n", s->hres, s->vres);
}

void ctl_fn_peer_new(struct ctl_peer *p)
{
	if (p->l != running_link || shl_isempty(p->wfd_subelements))
		return;

	if (cli_running())
		cli_printf("[" CLI_GREEN "ADD" CLI_DEFAULT "] Peer: %s\n",
			   p->label);
}

void ctl_fn_peer_free(struct ctl_peer *p)
{
	if (p->l != running_link || shl_isempty(p->wfd_subelements))
		return;

	if (p == pending_peer) {
		cli_printf("no longer waiting for peer %s (%s)\n",
			   p->friendly_name, p->label);
		pending_peer = NULL;
		stop_timeout(&scan_timeout);
		ctl_link_set_p2p_scanning(p->l, true);
	}

	if (p == running_peer) {
		cli_printf("freed::no longer running on peer %s\n",
			   running_peer->label);
		stop_timeout(&sink_timeout);
		stop_timeout(&rtsp_timeout);
		// ctl_notify_kill_player(running_link);
		ctl_sink_close(sink);
		running_peer = NULL;
		stop_timeout(&scan_timeout);
		ctl_link_set_p2p_scanning(p->l, true);
	}

	if (cli_running())
		cli_printf("[" CLI_RED "REMOVE" CLI_DEFAULT "] Peer: %s\n",
			   p->label);
}

void ctl_fn_peer_provision_discovery(struct ctl_peer *p,
				     const char *prov,
				     const char *pin)
{
	if (p->l != running_link || shl_isempty(p->wfd_subelements))
		return;

	if (cli_running())
		cli_printf("[" CLI_YELLOW "PROV" CLI_DEFAULT "] Peer: %s Type: %s PIN: %s\n",
			   p->label, prov, pin);
}

void ctl_fn_peer_go_neg_request(struct ctl_peer *p,
				     const char *prov,
				     const char *pin,
					 int local_go_intent)
{
	if (p->l != running_link || shl_isempty(p->wfd_subelements))
		return;

	if (cli_running())
		cli_printf("[" CLI_YELLOW "GO NEG" CLI_DEFAULT "] Peer: %s Type: %s PIN: %s\n",
			   p->label, prov, pin);

	if (!running_peer) {
		/* auto accept any incoming connection attempt */
		int ret = ctl_peer_connect(p, "auto", "", local_go_intent);
		cli_printf("auto accept any incoming connection attempt, ret %d\n", ret);
		pending_peer = p;

		/* 60s timeout in case the connect fails. Yes, stupid wpas does
		 * not catch this and notify us.. and as it turns out, DHCP
		 * negotiation with some proprietary devices can take up to 30s
		 * so lets be safe. */
		schedule_timeout(&scan_timeout,
				 60 * 1000ULL * 1000ULL,
				 scan_timeout_fn,
				 NULL);
	}
}

void ctl_fn_peer_formation_failure(struct ctl_peer *p, const char *reason)
{
	if (p->l != running_link || shl_isempty(p->wfd_subelements))
		return;

	if (cli_running())
		cli_printf("[" CLI_YELLOW "FAIL" CLI_DEFAULT "] runningPeer %s, Peer: %s Reason: %s\n", running_peer,
			   p->label, reason);

	if (!running_peer) {
		stop_timeout(&scan_timeout);
		ctl_link_set_p2p_scanning(p->l, true);
	}
}

void ctl_fn_peer_connected(struct ctl_peer *p)
{
	if (p->l != running_link || shl_isempty(p->wfd_subelements))
		return;

	if (cli_running())
		cli_printf("[" CLI_GREEN "CONNECT" CLI_DEFAULT "] Peer: %s\n",
			   p->label);

	pending_peer = NULL;

	if (!running_peer) {
		running_peer = p;
		st_gst_child_info.gst_peer = p;
		cli_log("now running on peer %s, connected status %d\n", running_peer->label, running_peer->connected);
		stop_timeout(&scan_timeout);

		sink_connected = false;
		sink_timeout_time = 2;
		schedule_timeout(&sink_timeout,
				 sink_timeout_time * 1000ULL * 1000ULL,
				 sink_timeout_fn,
				 NULL);

		if(!rtsp_timeout)
		{
			cli_printf("set rtsp timeout");
			schedule_timeout(&rtsp_timeout,
						25 * 1000ULL * 1000ULL,
						rtsp_timeout_fn,
						p);
		}
	}
}

void ctl_fn_peer_disconnected(struct ctl_peer *p)
{
	if ((p->l != running_link) || shl_isempty(p->wfd_subelements))
		return;

	if (p == running_peer) {
		cli_printf("disconnected::no longer running on peer %s\n",
			   running_peer->label);
		stop_timeout(&sink_timeout);
		stop_timeout(&rtsp_timeout);
		// ctl_notify_kill_player(running_link);
		ctl_sink_close(sink);
		running_peer = NULL;
		stop_timeout(&scan_timeout);
		ctl_link_set_p2p_scanning(p->l, true);
	}

	if (cli_running())
		cli_printf("[" CLI_YELLOW "DISCONNECT" CLI_DEFAULT "] Peer: %s\n",
			   p->label);
}

void ctl_fn_link_new(struct ctl_link *l)
{
	if (cli_running())
		cli_printf("[" CLI_GREEN "ADD" CLI_DEFAULT "] Link: %s\n",
			   l->label);

	/* If we're not running but have a bound link, try to find it now and
	 * start running if the link is now available. */
	if (!running_link && bound_link) {
		l = ctl_wifi_search_link(wifi, bound_link);
		if (l)
			run_on(l);
	}
}

void ctl_fn_link_free(struct ctl_link *l)
{
	if (l == running_link) {
		cli_printf("no longer running on link %s\n",
			   running_link->label);
		running_link = NULL;
		stop_timeout(&scan_timeout);
	}

	if (cli_running())
		cli_printf("[" CLI_RED "REMOVE" CLI_DEFAULT "] Link: %s\n",
			   l->label);
}

void ctl_sink_set_screen_resolution(int w, int h)
{
	if(!sink)
		return ;
		
	sink->scr_w = w;
	sink->scr_h = h;
	cli_log("Sink resolution is set to %d x %d", w, h);
}

void ctl_link_running_set(struct ctl_link *l, int val)
{
	if(val)
	{
		if(l && ctl_wifi_search_link(wifi, l->label))
		{
			if(running_link && running_link != l) {				
				cli_printf("Old running link %s dropped\n", running_link->label);
				ctl_link_set_managed(running_link, false);
			}
			running_link = l;
			ctl_link_set_managed(l, true);
			cli_printf("Now running on link %s\n", l->label);
		}
	}
	else
	{
		if (l == running_link) {
			cli_printf("no longer running on link %s\n",
				running_link->label);
			// running_link = NULL; keep it for old logic
			ctl_link_set_managed(l, false);
		}
	}
}
void cli_fn_help()
{
	/*
	 * 80-char barrier:
	 *      01234567890123456789012345678901234567890123456789012345678901234567890123456789
	 */
	printf("%s [OPTIONS...] ...\n\n"
	       "Control a dedicated local sink via MiracleCast.\n"
	       "  -h --help                      Show this help\n"
	       "     --help-commands             Show avaliable commands\n"
	       "     --version                   Show package version\n"
	       "     --log-level <lvl>           Maximum level for log messages\n"
	       "     --log-journal-level <lvl>   Maximum level for journal log messages\n"
	       "     --gst-debug [cat:]lvl[,...] List of categories an level of debug\n"
	       "     --audio <0/1>               Enable audio support (default %d)\n"
	       "     --scale WxH                 Scale to resolution\n"
	       "  -p --port <port>                  Port for rtsp (default %d)\n"
	       "     --uibc                         Enables UIBC\n"
	       "  -e --external-player           Configure player to use\n"
	       "     --res <n,n,n>               Supported resolutions masks (CEA, VESA, HH)\n"
	       "                                    default CEA  %08X\n"
	       "                                    default VESA %08X\n"
	       "                                    default HH   %08X\n"
	       "     --help-res                  Shows avaliable values for res\n"
	       "\n"
	       , program_invocation_short_name, gst_audio_en, DEFAULT_RSTP_PORT,
		   wfd_supported_res_cea, wfd_supported_res_vesa, wfd_supported_res_hh
	       );
	/*
	 * 80-char barrier:
	 *      01234567890123456789012345678901234567890123456789012345678901234567890123456789
	 */
}

static int ctl_interactive(char **argv, int argc)
{
	int r;

	memset(&st_gst_child_info, 0 ,sizeof(st_gst_child_info));

	r = cli_init(bus, cli_cmds, &st_gst_child_info);
	if (r < 0)
		return r;

	r = ctl_sink_new(&sink, cli_event);
	if (r < 0)
		goto error;

	r = ctl_sink_new(&r2_sink, r2_event);
	if (r < 0)
		goto error;

	ctl_wifi_fetch(wifi);

	if (argc > 0) {
		r = cli_do(cli_cmds, argv, argc);
		if (r == -EAGAIN)
			cli_error("unknown operation %s", argv[0]);
	}

	r = cli_run();

error:
	ctl_sink_free(sink);
	cli_destroy();
	return r;
}

static int ctl_main(int argc, char *argv[])
{
	struct shl_dlist *i;
	struct ctl_link *l;
	int r, left;

   if (getuid() != 0) {
      r = EACCES;
		log_notice("Must run as root");
      return r;
	}

	r = ctl_wifi_new(&wifi, bus);
	if (r < 0)
		return r;

   left = argc - optind;
   left = left <= 0 ? 0 : left;
	r = ctl_interactive(argv + optind, left);

	/* stop all scans */
	shl_dlist_for_each(i, &wifi->links) {
		l = link_from_dlist(i);
		if (l->have_p2p_scan)
			ctl_link_set_p2p_scanning(l, false);
	}

	ctl_wifi_free(wifi);
	return r;
}

static int parse_argv(int argc, char *argv[])
{
	enum {
		ARG_VERSION = 0x100,
		ARG_LOG_LEVEL,
		ARG_JOURNAL_LEVEL,
		ARG_GST_DEBUG,
		ARG_AUDIO,
		ARG_SCALE,
		ARG_RES,
		ARG_HELP_RES,
		ARG_UIBC,
      ARG_HELP_COMMANDS,
	};
	static const struct option options[] = {
		{ "help",	no_argument,		NULL,	'h' },
		{ "help-commands",	no_argument,		NULL,	ARG_HELP_COMMANDS },
		{ "version",	no_argument,		NULL,	ARG_VERSION },
		{ "log-level",	required_argument,	NULL,	ARG_LOG_LEVEL },
		{ "log-journal-level",	required_argument,	NULL,	ARG_JOURNAL_LEVEL },
		{ "gst-debug",	required_argument,	NULL,	ARG_GST_DEBUG },
		{ "audio",	required_argument,	NULL,	ARG_AUDIO },
		{ "scale",	required_argument,	NULL,	ARG_SCALE },
		{ "res",	required_argument,	NULL,	ARG_RES },
		{ "help-res",	no_argument,	NULL,	ARG_HELP_RES },
		{ "port",		required_argument,	NULL,	'p' },
		{ "uibc",		no_argument,		NULL,	ARG_UIBC },
		{ "external-player",		required_argument,		NULL,	'e' },
		{}
	};
	int c;

	uibc_option = false;
	uibc_enabled = false;
	external_player = false;
	rstp_port = DEFAULT_RSTP_PORT;

	while ((c = getopt_long(argc, argv, "he:p:", options, NULL)) >= 0) {
		switch (c) {
		case 'h':
		   cli_fn_help();
         return 0;
		case ARG_HELP_COMMANDS:
			return cli_help(cli_cmds, 20);
		case ARG_HELP_RES:
			wfd_print_resolutions("");
         return 0;
		case ARG_VERSION:
			puts(PACKAGE_STRING);
			return 0;
		case ARG_LOG_LEVEL:
			cli_max_sev = log_parse_arg(optarg);
			break;
		case ARG_GST_DEBUG:
			gst_debug = optarg;
			break;
		case ARG_JOURNAL_LEVEL:
			log_max_sev = log_parse_arg(optarg);
			break;
		case ARG_AUDIO:
			gst_audio_en = atoi(optarg);
			break;
		case ARG_SCALE:
			gst_scale_res = optarg;
			break;
		case ARG_RES:
			sscanf(optarg, "%x,%x,%x",
				&wfd_supported_res_cea,
				&wfd_supported_res_vesa,
				&wfd_supported_res_hh);
			break;
		case 'p':
			rstp_port = atoi(optarg);
			break;
		case 'e':
			external_player = true;
			player = optarg;
			break;
		case ARG_UIBC:
			uibc_option = true;
			break;
		case '?':
			return -EINVAL;
		}
	}

	return 1;
}

int main(int argc, char **argv)
{
	int r;
   bool free_argv = false;

	setlocale(LC_ALL, "");

   GKeyFile* gkf = load_ini_file();

   gchar** autocmds_free = NULL;
   if (gkf) {
      player = g_key_file_get_string (gkf, "sinkctl", "external-player", NULL);
      if (player) {
			external_player = true;
      }
      gchar* log_level;
      log_level = g_key_file_get_string (gkf, "sinkctl", "log-journal-level", NULL);
      if (log_level) {
         log_max_sev = log_parse_arg(log_level);
         g_free(log_level);
      }
      log_level = g_key_file_get_string (gkf, "sinkctl", "log-level", NULL);
      if (log_level) {
         cli_max_sev = log_parse_arg(log_level);
         g_free(log_level);
      }
      gchar* rstp_port_str = g_key_file_get_string (gkf, "sinkctl", "rstp-port", NULL);
      if (rstp_port_str) {
         rstp_port = atoi(rstp_port_str);
         g_free(rstp_port_str);
      }
      gchar* autocmd;
      autocmd = g_key_file_get_string (gkf, "sinkctl", "autocmd", NULL);
      if (autocmd && argc == 1) {
         gchar** autocmds = g_strsplit(autocmd, " ", -1);
         autocmds_free = autocmds;
         while (*autocmds) {
            if (strcmp(*autocmds, "") != 0) {
               gchar **newv = malloc((argc + 2) * sizeof(gchar*));
               memmove(newv, argv, sizeof(gchar*) * argc);
               newv[argc] = *autocmds;
               newv[argc+1] = NULL;
               argc++;
               if (free_argv) {
                  free(argv);
               }
               argv = newv;
               free_argv = true;
            }
            autocmds++;
         }
         g_free(autocmd);
      }
      g_key_file_free(gkf);
   }

	r = parse_argv(argc, argv);
	if (r < 0)
		return EXIT_FAILURE;
	if (!r)
		return EXIT_SUCCESS;

	r = sd_bus_default_system(&bus);
	if (r < 0) {
		cli_error("cannot connect to system bus: %s", strerror(-r));
		return EXIT_FAILURE;
	}

	r = ctl_main(argc, argv);
   g_strfreev(autocmds_free);
   if (free_argv) {
      free(argv);
   }
	sd_bus_unref(bus);

	return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
