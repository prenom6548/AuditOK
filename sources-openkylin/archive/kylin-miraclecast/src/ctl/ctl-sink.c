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

#include "ctl-sink.h"

/*
 * RTSP Session
 */

static int sink_req_fn(struct rtsp *bus, struct rtsp_message *m, void *data)
{
	cli_debug("INCOMING: %s\n", rtsp_message_get_raw(m));

	return 0;
}

static void sink_handle_options(struct ctl_sink *s,
				struct rtsp_message *m)
{
	_rtsp_message_unref_ struct rtsp_message *rep = NULL;
	int r;

	r = rtsp_message_new_reply_for(m, &rep, RTSP_CODE_OK, NULL);
	if (r < 0)
		return cli_vERR(r);

	r = rtsp_message_append(rep, "<s>",
				"Public",
				"org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER");
	if (r < 0)
		return cli_vERR(r);

	rtsp_message_seal(rep);
	cli_log("RTSP OPTIONS 1 OUTGOING: <<<<\n %s >>>>\n", rtsp_message_get_raw(rep));

	r = rtsp_send(s->rtsp, rep);
	if (r < 0)
		return cli_vERR(r);

	rtsp_message_unref(rep);
	rep = NULL;

	r = rtsp_message_new_request(s->rtsp,
				     &rep,
				     "OPTIONS",
				     "*");
	if (r < 0)
		return cli_vERR(r);

	r = rtsp_message_append(rep, "<s>",
				"Require",
				"org.wfa.wfd1.0");
	if (r < 0)
		return cli_vERR(r);

	rtsp_message_seal(rep);
	cli_log("RTSP OPTIONS 2 OUTGOING: <<<<\n %s >>>>\n", rtsp_message_get_raw(rep));

	r = rtsp_call_async(s->rtsp, rep, sink_req_fn, NULL, 0, NULL);
	if (r < 0)
		return cli_vERR(r);
}

static void pickup_video_params(struct ctl_sink *s, uint32_t *pCea, uint32_t *pVesa, uint32_t *pHH)
{
	int local_w = s->scr_w, local_h = s->scr_h;

	cli_log("Current resolution is %dx%d", s->scr_w, s->scr_h);
	pickup_cea(local_w, local_h, pCea);
	pickup_vesa(local_w, local_h, pVesa);
	pickup_hh(local_w, local_h, pHH);
	cli_log("We choose video param %08x, %08x, %08x", *pCea, *pVesa, *pHH);
}

static void sink_handle_get_parameter(struct ctl_sink *s,
				      struct rtsp_message *m)
{
	_rtsp_message_unref_ struct rtsp_message *rep = NULL;
	int r;
	uint32_t video_cea = 0, video_vesa = 0, video_hh = 0;

	// cli_log("[rtsp-establish]GET-PARAMETER INCOMING: %s\n", rtsp_message_get_raw(m));

	r = rtsp_message_new_reply_for(m, &rep, RTSP_CODE_OK, NULL);
	if (r < 0)
		return cli_vERR(r);

	pickup_video_params(s, &video_cea, &video_vesa, &video_hh);
	
	if(strstr(rtsp_message_get_raw(m), "microsoft"))
	{
		/* wfd_content_protection */
		check_and_response_option("wfd_content_protection", "none");

		/* wfd_video_formats */
		check_and_response_option("wfd_video_formats", "40 00 01 10 0001bdeb 051557ff 00000fff 00 0001 0003 11 0780 0438, 02 10 0001bdeb 155557ff 00000fff 00 0001 0003 11 0780 0438");
		// check_and_response_option("wfd_video_formats", "40 00 01 10 00000300 051557ff 00000fff 00 0001 0003 11 0780 0438, 02 10 00010001 155557ff 00000fff 00 0001 0003 11 0780 0438");
		// check_and_response_option("wfd_video_formats", "40 00 01 10 00000040 00200000 00000080 00 0000 001f 01 0780 0438, 02 10 00000040 00200000 00000080 00 0000 001f 01 0780 0438");
		
		/* wfd_audio_codecs */
		// check_and_response_option("wfd_audio_codecs", "wfd_audio_codecs: AAC 00000007 00");
		check_and_response_option("wfd_audio_codecs", "LPCM 00000003 00, AAC 0000000f 00, AC3 00000007 00");

		/* wfd_client_rtp_ports */
		char wfd_client_rtp_ports[128];
		sprintf(wfd_client_rtp_ports, "RTP/AVP/UDP;unicast %d 0 mode=play", rstp_port);
		check_and_response_option("wfd_client_rtp_ports", wfd_client_rtp_ports);
		check_and_response_option("wfd_content_protection", "none");
		check_and_response_option("wfd_display_edid", "0001 00ffffffffffff0051f38f50010000000e100104a51d10ff2f0000a057499b2610484f000000010101010101010101010101010101011a36809c70381f403020350025a510000018000000fc00496e7465726e616c204c43440a000000fd003c3c9a9a0e00000000000000000000000000000000000000000000000000000030");
		check_and_response_option("wfd_connector_type", "05");
		check_and_response_option("microsoft_cursor", "none 0x0100 0x0100 0x4abe");
		check_and_response_option("microsoft_rtcp_capability", "none");
		check_and_response_option("wfd_idr_request_capability", "1");
		check_and_response_option("microsoft_latency_management_capability", "none");
		check_and_response_option("microsoft_format_change_capability", "none");
		check_and_response_option("microsoft_diagnostics_capability", "none");
		check_and_response_option("intel_friendly_name", "miraclecast");
		check_and_response_option("intel_sink_manufacturer_name", "GNU Linux");
		check_and_response_option("intel_sink_model_name", "Arch linux");
		check_and_response_option("intel_sink_device_URL", "http://github.com/albfan/miraclecast");

		/* wfd_uibc_capability */
		if (uibc_option) {
			check_and_response_option("wfd_uibc_capability",
										"input_category_list=GENERIC, HIDC;"
										"generic_cap_list=Keyboard;"
										"hidc_cap_list=Keyboard/USB, Mouse/USB, MultiTouch/USB, Gesture/USB, RemoteControl/USB;"
										"port=none");
		}
	}
	else
	{		
		/* wfd_content_protection */
		if (rtsp_message_read(m, "{<>}", "wfd_content_protection") >= 0) {
			r = rtsp_message_append(rep, "{&}",
						"wfd_content_protection: none");
			if (r < 0)
				return cli_vERR(r);
		}
		/* wfd_video_formats */
		if (rtsp_message_read(m, "{<>}", "wfd_video_formats") >= 0) {
			char wfd_video_formats[128];
			sprintf(wfd_video_formats,
				"wfd_video_formats: 00 00 03 10 %08x %08x %08x 00 0000 0000 10 none none",
				video_cea, video_vesa, video_hh);
				// CEA_1080P_25FPS_CFG, VESA_19201200_30FPS_CFG, HH_960540_30FPS_CFG);//we use low fps due to the poor soft decoding capability, otherwise latency will obvious!
				// s->resolutions_cea, s->resolutions_vesa, s->resolutions_hh);
			// cli_log("[rtsp-establish]: response to GET-PARAMETER: %s", wfd_video_formats);
			
			r = rtsp_message_append(rep, "{&}", wfd_video_formats);
			if (r < 0)
				return cli_vERR(r);
		}
		/* wfd_audio_codecs */
		if (rtsp_message_read(m, "{<>}", "wfd_audio_codecs") >= 0) {
			r = rtsp_message_append(rep, "{&}",
						"wfd_audio_codecs: AAC 00000007 00");
			if (r < 0)
				return cli_vERR(r);
		}
		/* wfd_client_rtp_ports */
		if (rtsp_message_read(m, "{<>}", "wfd_client_rtp_ports") >= 0) {
			char wfd_client_rtp_ports[128];
			sprintf(wfd_client_rtp_ports,
						"wfd_client_rtp_ports: RTP/AVP/UDP;unicast %d 0 mode=play", rstp_port);
			r = rtsp_message_append(rep, "{&}",
						wfd_client_rtp_ports);
			if (r < 0)
				return cli_vERR(r);
		}

		/* wfd_uibc_capability */
		if (rtsp_message_read(m, "{<>}", "wfd_uibc_capability") >= 0 && uibc_option) {
			char wfd_uibc_capability[512];
			sprintf(wfd_uibc_capability,
				"wfd_uibc_capability: input_category_list=HIDC;"
				"generic_cap_list=none;"
				"hidc_cap_list=Keyboard/USB, Mouse/USB;port=none");
			r = rtsp_message_append(rep, "{&}", wfd_uibc_capability);
			if (r < 0)
				return cli_vERR(r);
		}
		else
		{
			//tell User, the source device dose not support UIBC
		}
	}
	rtsp_message_seal(rep);
	
	cli_log("RTSP GET_PARAMETER OUTGOING: <<<<\n %s >>>>\n", rtsp_message_get_raw(rep));

	r = rtsp_send(s->rtsp, rep);
	if (r < 0)
		return cli_vERR(r);
}

bool check_rtsp_option(struct rtsp_message *m, char *option) {
	return rtsp_message_read(m, "{<>}", option) >= 0;
}

static int sink_setup_fn(struct rtsp *bus, struct rtsp_message *m, void *data)
{
	_rtsp_message_unref_ struct rtsp_message *rep = NULL;
	struct ctl_sink *s = data;
	const char *session;
	char *ns, *next;
	int r;

	// cli_debug("INCOMING: %s\n", rtsp_message_get_raw(m));

	r = rtsp_message_read(m, "<s>", "Session", &session);
	if (r < 0)
		return cli_ERR(r);

	ns = strdup(session);
	if (!ns)
		return cli_ENOMEM();

	next = strchr(ns, ';');
	if (next)
		*next = '\0';

	free(s->session);
	s->session = ns;

	r = rtsp_message_new_request(s->rtsp,
				     &rep,
				     "PLAY",
				     s->url);
	if (r < 0)
		return cli_ERR(r);

	r = rtsp_message_append(rep, "<s>", "Session", s->session);
	if (r < 0)
		return cli_ERR(r);

	rtsp_message_seal(rep);
	fprintf(stderr, "Send M7 request: <<<<\n %s \n>>>>\n", (char*)(rtsp_message_get_raw(rep)));

	r = rtsp_call_async(s->rtsp, rep, sink_req_fn, NULL, 0, NULL);
	if (r < 0)
		return cli_ERR(r);

	return 0;
}

static int sink_set_format(struct ctl_sink *s,
					  unsigned int cea_res,
					  unsigned int vesa_res,
					  unsigned int hh_res)
{
	int hres, vres;

	if ((vfd_get_cea_resolution(cea_res, &hres, &vres) == 0) ||
		(vfd_get_vesa_resolution(vesa_res, &hres, &vres) == 0) ||
		(vfd_get_hh_resolution(hh_res, &hres, &vres) == 0)) {
		if (hres && vres) {
			s->hres = hres;
			s->vres = vres;
			ctl_fn_sink_resolution_set(s);
			cli_debug("[rtsp-establish] received SET_PARAMETER: cea %x, vesa %x, hh %x, resolution %d*%d", cea_res, vesa_res, hh_res, hres, vres);
			return 0;
		}
	}

	return -EINVAL;
}

static void sink_handle_set_parameter(struct ctl_sink *s,
				      struct rtsp_message *m)
{
	_rtsp_message_unref_ struct rtsp_message *rep = NULL;
	const char *trigger;
	const char *url;
	char *uibc_config;
	const char *uibc_setting;
	char *nu;
	unsigned int cea_res, vesa_res, hh_res;
	int r;

	// cli_log("[rtsp-establish]SET-PARAMETER INCOMING: %s\n", rtsp_message_get_raw(m));
	r = rtsp_message_new_reply_for(m, &rep, RTSP_CODE_OK, NULL);
	if (r < 0)
		return cli_vERR(r);

	rtsp_message_seal(rep);
	cli_log("RTSP SET_PARAMETER 1 OUTGOING: <<<<\n %s \n>>>>\n", rtsp_message_get_raw(rep));

	r = rtsp_send(s->rtsp, rep);
	if (r < 0)
		return cli_vERR(r);

	rtsp_message_unref(rep);
	rep = NULL;

	/* M4 (or any other) can pass presentation URLs */
	r = rtsp_message_read(m, "{<s>}", "wfd_presentation_URL", &url);
	if (r >= 0) {
		if (!s->url || strcmp(s->url, url)) {
			nu = strdup(url);
			if (!nu)
				return cli_vENOMEM();

			free(s->url);
			s->url = nu;
			cli_log("M4 Got URL: %s\n", s->url);
		}
	}	

	/* M4 (or any other) can pass presentation URLs */
	r = rtsp_message_read(m, "{<&>}", "wfd_uibc_capability", &uibc_config);
	if (r >= 0) {
		if (!s->uibc_config || strcmp(s->uibc_config, uibc_config)) {
            nu = strdup(uibc_config);
            if (!nu)
                return cli_vENOMEM();

            free(s->uibc_config);
            s->uibc_config = nu;

            if (!strcasecmp(uibc_config, "none")) {
                uibc_enabled = false;
            } else {
                char* token = strtok(uibc_config, ";");
				fprintf(stderr,"token = %s\n",token);
				// if(NULL == strstr(token,"GENERIC"))//TODO implement in a function
				// {
				// 	s->hidcEn = true;
				// 	fprintf(stderr,"No:input_category_list:s->hidcEn = %d\n",s->hidcEn);
				// }else{
				// 	s->hidcEn = false;
				// 	fprintf(stderr,"Yes:input_category_list:s->hidcEn = %d\n",s->hidcEn);
				// }

				s->hidcEn = true;

				while (token) {
                    if (sscanf(token, "port=%d", &uibc_port)) {
                        log_debug("UIBC port: %d\n", uibc_port);
                        if (uibc_option) {
                            uibc_enabled = true;
						}
                        break;
                    }
                    token = strtok(0, ";");
                }
            }
		}
	}

	/* M4 (or any other) can pass presentation URLs */
	r = rtsp_message_read(m, "{<s>}", "wfd_uibc_setting", &uibc_setting);
	if (r >= 0) {
		if (!s->uibc_setting || strcmp(s->uibc_setting, uibc_setting)) {
			nu = strdup(uibc_setting);
			if (!nu)
				return cli_vENOMEM();

			free(s->uibc_setting);
			s->uibc_setting = nu;
			cli_debug("uibc setting: %s\n", s->uibc_setting);
		}
	}
	/* M4 again */
	r = rtsp_message_read(m, "{<****hhh>}", "wfd_video_formats",
							&cea_res, &vesa_res, &hh_res);
	if (r == 0) {
		r = sink_set_format(s, cea_res, vesa_res, hh_res);
		if (r)
			return cli_vERR(r);
	}

	/* M5 */
	char *raw = rtsp_message_get_raw(m);
	r = rtsp_message_read(m, "{<s>}", "wfd_trigger_method", &trigger);
	if (r < 0)
	{
		cli_log("M5 wfd_trigger_method readfailed: ((((((((((\n %s \n))))))))))\n", raw);
		return;
	}

	if (!strcmp(trigger, "SETUP")) {
		if (!s->url) {
			cli_error("No valid wfd_presentation_URL\n");
			return;
		}
		fprintf(stderr, "Recv M5 request: <<<<\n %s \n>>>>\n", (char*)(rtsp_message_get_raw(m)));
		r = rtsp_message_new_request(s->rtsp,
					     &rep,
					     "SETUP",
					     s->url);
		if (r < 0)
			return cli_vERR(r);

		char rtsp_setup[128];
		sprintf(rtsp_setup, "RTP/AVP/UDP;unicast;client_port=%d", rstp_port);
		r = rtsp_message_append(rep, "<s>", "Transport", rtsp_setup);
		if (r < 0)
			return cli_vERR(r);

		rtsp_message_seal(rep);
		fprintf(stderr, "Send M6 request: <<<<\n %s \n>>>>\n", (char*)(rtsp_message_get_raw(rep)));

		r = rtsp_call_async(s->rtsp, rep, sink_setup_fn, s, 15ULL * 1000ULL * 1000ULL, NULL);
		if (r < 0)
			return cli_vERR(r);
	}
}

static void sink_handle(struct ctl_sink *s,
			struct rtsp_message *m)
{
	const char *method;
	char * raw = rtsp_message_get_raw(m);
	time_t ctm = time(NULL);

	fprintf(stderr, "RTSP INCOMING: ((((((((((\n %s \n%s))))))))))\n", raw, ctime(&ctm));

	method = rtsp_message_get_method(m);
	if (!method)
		return;

	if (!strcmp(method, "OPTIONS")) {
		sink_handle_options(s, m);
	} else if (!strcmp(method, "GET_PARAMETER")) {
		sink_handle_get_parameter(s, m);
	} else if (!strcmp(method, "SET_PARAMETER")) {
		sink_handle_set_parameter(s, m);
	}
}

static int sink_rtsp_fn(struct rtsp *bus,
			struct rtsp_message *m,
			void *data)
{
	struct ctl_sink *s = data;

	if (!m)
		s->hup = true;
	else
		sink_handle(s, m);

	if (s->hup) {
		ctl_sink_close(s);
		cli_printf("%s sink_disconnect", __func__);
		if(s->isR2Sink)
			ctl_link_notify_agent_r2_session_stopped(s->running_link);
		else
			ctl_fn_sink_disconnected(s);
	}

	return 0;
}

/*
 * Sink I/O
 */

static void sink_connected(struct ctl_sink *s)
{
	int r, val;
	socklen_t len;

	if (s->connected || s->hup)
		return;

	sd_event_source_set_enabled(s->fd_source, SD_EVENT_OFF);

	len = sizeof(val);
	r = getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &val, &len);
	if (r < 0) {
		s->hup = true;
		cli_vERRNO();
		return;
	} else if (val) {
		s->hup = true;
		errno = val;
		cli_error("cannot connect to remote host (%d): %m",
			  errno);
		return;
	}

	cli_log("L4 connection established! Now begin rtsp negotiation....");

	r = rtsp_open(&s->rtsp, s->fd);
	if (r < 0)
	{
		cli_log("rtsp open failed with retval %d", r);
		goto error;
	}

	r = rtsp_attach_event(s->rtsp, s->event, 0);
	if (r < 0)
	{
		cli_log("rtsp attach event failed with retval %d", r);
		goto error;
	}

	r = rtsp_add_match(s->rtsp, sink_rtsp_fn, s);
	if (r < 0)
	{
		cli_log("rtsp add match failed with retval %d", r);
		goto error;
	}

	s->connected = true;
	ctl_fn_sink_connected(s);
	return;

error:
	s->hup = true;
	cli_vERR(r);
}

static void sink_io(struct ctl_sink *s, uint32_t mask)
{
	if (mask & EPOLLOUT)
		sink_connected(s);

	if (mask & (EPOLLHUP | EPOLLERR)) {
		cli_notice("HUP/ERR on socket");
		s->hup = true;
	}

	if (s->hup) {
		ctl_sink_close(s);
		cli_printf("%s sink_disconnect", __func__);
		if(s->isR2Sink)
			ctl_link_notify_agent_r2_session_stopped(s->running_link);
		else
			ctl_fn_sink_disconnected(s);
	}
}

static int sink_io_fn(sd_event_source *source,
		      int fd,
		      uint32_t mask,
		      void *data)
{
	sink_io(data, mask);
	return 0;
}

static int sink_connect(struct ctl_sink *s)
{
	int fd, r;

	if (!s)
		return cli_EINVAL();
	if (s->fd >= 0)
		return 0;
	if (!s->addr.ss_family || !s->addr_size)
		return cli_EINVAL();

	fd = socket(s->addr.ss_family,
		    SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
		    0);
	if (fd < 0)
		return cli_ERRNO();

	r = connect(fd, (struct sockaddr*)&s->addr, s->addr_size);
	if (r < 0) {
		r = -errno;
		cli_log("%s L4 connect return err %d", __func__, r);
		if (r != -EINPROGRESS) {
			cli_vERR(r);
			goto err_close;
		}
	}

	r = sd_event_add_io(s->event,
			    &s->fd_source,
			    fd,
			    EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLOUT | EPOLLET,
			    sink_io_fn,
			    s);
	if (r < 0) {
		cli_vERR(r);
		goto err_close;
	}

	s->fd = fd;
	return 0;

err_close:
	close(fd);
	return r;
}

static void sink_close(struct ctl_sink *s)
{
	if (!s || s->fd < 0)
		return;

	rtsp_remove_match(s->rtsp, sink_rtsp_fn, s);
	rtsp_detach_event(s->rtsp);
	rtsp_unref(s->rtsp);
	s->rtsp = NULL;
	sd_event_source_unref(s->fd_source);
	s->fd_source = NULL;
	close(s->fd);
	s->fd = -1;
	s->connected = false;
	s->hup = false;
	s->hres = 0;
	s->vres = 0;
}

/*
 * Sink Management
 */

int ctl_sink_new(struct ctl_sink **out,
		 sd_event *event)
{
	struct ctl_sink *s;

	if (!out || !event)
		return cli_EINVAL();

	s = calloc(1, sizeof(*s));
	if (!s)
		return cli_ENOMEM();

	s->event = sd_event_ref(event);
	s->fd = -1;
	s->resolutions_cea = wfd_supported_res_cea;
	s->resolutions_vesa = wfd_supported_res_vesa;
	s->resolutions_hh = wfd_supported_res_hh;
	
	s->scr_w = 1920;
	s->scr_h = 1080;

	*out = s;
	return 0;
}

void ctl_sink_free(struct ctl_sink *s)
{
	if (!s)
		return;

	ctl_sink_close(s);
	free(s->target);
	free(s->session);
	free(s->url);
	sd_event_unref(s->event);
	free(s);
}

int ctl_sink_connect(struct ctl_sink *s, const char *target)
{
	struct sockaddr_in addr = { };
	char *t;
	int r;

	if (!s || !target || s->fd >= 0)
		return cli_EINVAL();

	addr.sin_family = AF_INET;
	addr.sin_port = htons(7236);
	r = inet_pton(AF_INET, target, &addr.sin_addr);
	if (r != 1)
		return cli_EINVAL();

	t = strdup(target);
	if (!t)
		return cli_ENOMEM();

	free(s->target);
	s->target = t;

	memcpy(&s->addr, &addr, sizeof(addr));
	s->addr_size = sizeof(addr);

	return sink_connect(s);
}

int ctl_sink_r2_session_start(struct ctl_sink *s, const char *target, const unsigned short port)
{
	struct sockaddr_in addr = { };
	char *t;
	int r;

	if (!s || !target || s->fd >= 0)
		return cli_EINVAL();

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	r = inet_pton(AF_INET, target, &addr.sin_addr);
	if (r != 1)
		return cli_EINVAL();

	t = strdup(target);
	if (!t)
		return cli_ENOMEM();

	free(s->target);
	s->target = t;

	memcpy(&s->addr, &addr, sizeof(addr));
	s->addr_size = sizeof(addr);

	r = sink_connect(s);
	if(r)
		ctl_link_notify_agent_r2_session_stopped(s->running_link);
	
	return r;
}


void ctl_sink_close(struct ctl_sink *s)
{
	if (!s)
		return;

	sink_close(s);
}

bool ctl_sink_is_connecting(struct ctl_sink *s)
{
	return s && s->fd >= 0 && !s->connected;
}

bool ctl_sink_is_connected(struct ctl_sink *s)
{
	return s && s->connected;
}

bool ctl_sink_is_closed(struct ctl_sink *s)
{
	return !s || s->fd < 0;
}
