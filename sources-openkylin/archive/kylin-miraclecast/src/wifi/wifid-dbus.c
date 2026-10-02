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

#define LOG_SUBSYSTEM "dbus"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>
#include "shl_log.h"
#include "shl_util.h"
#include "util.h"
#include "wifid.h"

static char *peer_dbus_get_path(struct peer *p)
{
	char buf[128], *node;
	int r;

	sprintf(buf, "%s@%u", p->p2p_mac, p->l->ifindex);

	r = sd_bus_path_encode("/org/freedesktop/miracle/wifi/peer",
			       buf,
			       &node);
	if (r < 0) {
		log_vERR(r);
		return NULL;
	}

	return node;
}

static char *link_dbus_get_path(struct link *l)
{
	char buf[128] = {0};
	char *node;
	int r;

	snprintf(buf, sizeof(buf)-1, "%u", l->ifindex);

	r = sd_bus_path_encode("/org/freedesktop/miracle/wifi/link",
			       buf,
			       &node);
	if (r < 0) {
		log_vERR(r);
		return NULL;
	}

	return node;
}

static bool is_login_user(struct link *l, const char *objpath){
	const char *state;
	int r = 0;
	_cleanup_sd_bus_error_ sd_bus_error err = SD_BUS_ERROR_NULL;
	_cleanup_sd_bus_message_ sd_bus_message *m = NULL, *reply = NULL;

	r = sd_bus_message_new_method_call(l->m->bus,
					   &m,
					   "org.freedesktop.login1",
					   objpath,
					   "org.freedesktop.DBus.Properties",
					   "Get");
	if (r < 0)
	{
		log_EINVAL();
		return false;
	}

	r = sd_bus_message_append(m, "ss",
				"org.freedesktop.login1.User",
				"State");
	if (r < 0)
	{
		log_EINVAL();
		return false;
	}

	r = sd_bus_call(l->m->bus, m, 0, &err, &reply);
	if (r < 0) {
		log_error("cannot call system.login1.User: %s\n",
			bus_error_message(&err, r));
		return false;
	}

	r = sd_bus_message_enter_container(reply, 'v', "s");
	if (r < 0)
	{
		log_EINVAL();
		return false;
	}
	
	r = sd_bus_message_read(reply, "s", &state);
	if (r < 0)
	{
		log_EINVAL();
		return false;
	}

	r = sd_bus_message_exit_container(reply);
	if (r < 0)
		log_EINVAL();

	return !strcmp(state, "active") ? true : false;
}

static int get_current_login_user_info(struct link *l, uint32_t *pUid, char **pName){
	char *userObjPath = NULL, *userName = NULL;
	uint32_t uid;
	int r = 0;
	bool bFound = false;
	_cleanup_sd_bus_error_ sd_bus_error err = SD_BUS_ERROR_NULL;
	_cleanup_sd_bus_message_ sd_bus_message *m = NULL, *reply = NULL;

	r = sd_bus_message_new_method_call(l->m->bus,
					   &m,
					   "org.freedesktop.login1",
					   "/org/freedesktop/login1",
					   "org.freedesktop.login1.Manager",
					   "ListUsers");
	if (r < 0)
	{
		log_EINVAL();
		goto exit;
	}

	r = sd_bus_call(l->m->bus, m, 0, &err, &reply);
	if (r < 0) {
		log_error("!!!cannot call system.login1.Manager\n: %s",
			  bus_error_message(&err, r));
		goto exit;
	}

	r = sd_bus_message_enter_container(reply, 'a', "(uso)");
	if (r < 0)
	{
		log_EINVAL();
		goto exit;
	}

	for (;;) {
		r = sd_bus_message_read(reply, "(uso)", &uid, &userName, &userObjPath);
		if (r <= 0)
			break;

		if(!bFound && is_login_user(l, userObjPath))
		{
			if(pUid)		
				*pUid = uid;
			if(pName)
				*pName = userName;
			bFound = true;
		}
  	}

	r = sd_bus_message_exit_container(reply);
	if (r < 0)
		log_EINVAL();

exit:	
	return bFound ? 0 : -1;
}

static void get_manufacturer(struct link *l, char *mac, char *manufacturer){	
	FILE *fp;	
	int r;	
	char filepath[512] = {0};
	char device_info[256] = {0};
	char manu[256] = {0};
	char *line;

	snprintf(filepath, sizeof(filepath), WIFI_SAVE_PATH"klwfd_%s-%s.conf",  l->ifname, mac);						

	fp = fopen(filepath, "r");			
	if(fp)
	{
		while (!feof(fp) && !ferror(fp)){
			fgets(device_info, sizeof(device_info)-1, fp);
			line = strstr(device_info, "manufacturer=");
			if (line){
				line += strlen("manufacturer=");
				memcpy(manufacturer, line, strlen(line) - 1);	
				break;
			}

		}
		fclose(fp);
	}
	else
	{
		log_error("open network conf %s failed", filepath);
	}

	return;
}

static int link_spawn_player(sd_bus_message *msg,
			     void *data, sd_bus_error *err)
{
	int hres, vres, uibcEn, hidcEn, rstpPort, uibcPort;
	const char *peer_label, *uibcServerAddr;
	int r = 0;
	uint32_t self_uid = INVALID_UID;
	struct link *l = data;
	char manufacturer[256] = {0};
	char mac[256] = {0};
	
	log_debug("Link-DBus-MethodCall: SpawnPlayer begin\n");
	r = sd_bus_message_read(msg, "iiiiiiss", &hres, &vres, &uibcEn, &hidcEn, &rstpPort, &uibcPort, &uibcServerAddr, &peer_label);
	if (r < 0)
	{
		log_error("%s msg read failed", __func__);
		return r;
	}

	memcpy(mac, peer_label,17);
	get_manufacturer(l, mac, manufacturer);

	//Get current user UID
	r = supplicant_get_uid(l->s, &self_uid);
	if(r < 0)
		self_uid = INVALID_UID;

	r = sd_bus_emit_signal(l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"SpawnPlayer",
					"iiiiiisssu", hres, vres, uibcEn, hidcEn, rstpPort, uibcPort, manufacturer, uibcServerAddr, peer_label, self_uid);
	if (r < 0)
	{
		log_error("%s signal emit failed", __func__);
		return r;
	}

	log_info("Link-DBus-MethodCall: SpawnPlayer received with login user %u", self_uid);
	
	r = sd_bus_reply_method_return(msg, NULL);	
	return r;
}

static int link_kill_player(sd_bus_message *msg,
			     void *data, sd_bus_error *err)
{
	int r = 0;
	struct link *l = data;

	r = sd_bus_emit_signal(l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"KillPlayer",
					"");

	if (r < 0)
	{
		log_error("%s signal emit failed", __func__);
		return r;
	}

	log_debug("Link-DBus-MethodCall: KillPlayer received");
	return sd_bus_reply_method_return(msg, NULL);	
}

static int link_r2_session_stopped(sd_bus_message *msg,
			     void *data, sd_bus_error *err)
{
	int r = 0;
	struct link *l = data;

	r = sd_bus_emit_signal(l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"R2SessStopped",
					"");

	if (r < 0)
	{
		log_error("%s R2SessStopped emit failed", __func__);
		return r;
	}

	log_info("Link-DBus-MethodCall: SinkStopR2Session received");
	return sd_bus_reply_method_return(msg, NULL);
}

/*
 * Peer DBus
 */

static int peer_dbus_connect(sd_bus_message *msg,
			     void *data, sd_bus_error *err)
{
	struct peer *p = data;
	const char *prov, *pin;
	int local_go_intent = 0;
	int r;

	r = sd_bus_message_read(msg, "ssi", &prov, &pin, &local_go_intent);
	if (r < 0)
		return r;
	log_info("PEER connecting with ===== Prov [%s], PIN: [%s] go_intent %d ~~~~~~~~~~~~~\n", prov, pin, local_go_intent);
	if (!*prov || !strcmp(prov, "auto"))
		prov = NULL;
	if (!*pin)
		pin = NULL;

	r = peer_connect(p, prov, pin, local_go_intent);
	if (r < 0)
		return r;

	return sd_bus_reply_method_return(msg, NULL);
}

static int peer_dbus_disconnect(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct peer *p = data;
	int r = sd_bus_emit_signal(p->l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"RtspFailed",
					"");
	if (r < 0)
		log_error("wifid emit rtsp timeout signal failed");

	log_info("Peer %s disconnect from sink", p->p2p_mac);
	peer_disconnect(p);
	return sd_bus_reply_method_return(msg, NULL);
}

static int peer_dbus_get_link(sd_bus *bus,
			      const char *path,
			      const char *interface,
			      const char *property,
			      sd_bus_message *reply,
			      void *data,
			      sd_bus_error *err)
{
	_shl_free_ char *node = NULL;
	struct peer *p = data;
	int r;

	node = link_dbus_get_path(p->l);
	if (!node)
		return -ENOMEM;

	r = sd_bus_message_append_basic(reply, 'o', node);
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_p2p_mac(sd_bus *bus,
				 const char *path,
				 const char *interface,
				 const char *property,
				 sd_bus_message *reply,
				 void *data,
				 sd_bus_error *err)
{
	struct peer *p = data;
	int r;

	r = sd_bus_message_append_basic(reply, 's', p->p2p_mac);
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_friendly_name(sd_bus *bus,
				       const char *path,
				       const char *interface,
				       const char *property,
				       sd_bus_message *reply,
				       void *data,
				       sd_bus_error *err)
{
	struct peer *p = data;
	const char *name;
	int r;

	name = peer_get_friendly_name(p);
	if (!name)
		name = "<unknown>";

	r = sd_bus_message_append_basic(reply, 's', name);
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_connected(sd_bus *bus,
				   const char *path,
				   const char *interface,
				   const char *property,
				   sd_bus_message *reply,
				   void *data,
				   sd_bus_error *err)
{
	struct peer *p = data;
	int r;
	log_info("Connected got!!!!! @%s:%d", __func__, __LINE__);
	r = sd_bus_message_append(reply, "b", p->connected);
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_interface(sd_bus *bus,
				   const char *path,
				   const char *interface,
				   const char *property,
				   sd_bus_message *reply,
				   void *data,
				   sd_bus_error *err)
{
	struct peer *p = data;
	int r;

	r = sd_bus_message_append(reply, "s", peer_get_interface(p));
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_local_address(sd_bus *bus,
				       const char *path,
				       const char *interface,
				       const char *property,
				       sd_bus_message *reply,
				       void *data,
				       sd_bus_error *err)
{
	struct peer *p = data;
	int r;

	r = sd_bus_message_append(reply, "s", peer_get_local_address(p));
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_remote_address(sd_bus *bus,
					const char *path,
					const char *interface,
					const char *property,
					sd_bus_message *reply,
					void *data,
					sd_bus_error *err)
{
	struct peer *p = data;
	int r;

	r = sd_bus_message_append(reply, "s", peer_get_remote_address(p));
	if (r < 0)
		return r;

	return 1;
}

static int peer_dbus_get_wfd_subelements(sd_bus *bus,
					 const char *path,
					 const char *interface,
					 const char *property,
					 sd_bus_message *reply,
					void *data,
					sd_bus_error *err)
{
	struct peer *p = data;
	int r;

	r = sd_bus_message_append(reply, "s", peer_get_wfd_subelements(p));
	if (r < 0)
		return r;

	return 1;
}

static const sd_bus_vtable peer_dbus_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("Connect",
		      "ssi",
		      NULL,
		      peer_dbus_connect,
		      0),
	SD_BUS_METHOD("Disconnect",
		      NULL,
		      NULL,
		      peer_dbus_disconnect,
		      0),
	SD_BUS_PROPERTY("Link",
			"o",
			peer_dbus_get_link,
			0,
			SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("P2PMac",
			"s",
			peer_dbus_get_p2p_mac,
			0,
			SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("FriendlyName",
			"s",
			peer_dbus_get_friendly_name,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("Connected",
			"b",
			peer_dbus_get_connected,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("Interface",
			"s",
			peer_dbus_get_interface,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("LocalAddress",
			"s",
			peer_dbus_get_local_address,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("RemoteAddress",
			"s",
			peer_dbus_get_remote_address,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("WfdSubelements",
			"s",
			peer_dbus_get_wfd_subelements,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_SIGNAL("ProvisionDiscovery", "ss", 0),
	SD_BUS_SIGNAL("GoNegRequest", "ssi", 0),
	SD_BUS_SIGNAL("PeerFormationFailure", "s", 0),
	SD_BUS_VTABLE_END
};

static int ui_dbus_find_peer_by_label(char* label, void *data, struct peer **peerOut)
{
	struct manager *m = data;
	struct link *l;
	struct peer *p;	
	char *sep;

	sep = strchr(label, '@');
	if (sep) {
		*sep = 0;
		l = manager_find_link_by_label(m, sep + 1);
		if (!l || !l->public)
		{
			log_error("%s find link by @label %s from manager failed", __func__, label);
			return -1;
		}

		p = link_find_peer_by_label(l, label);
		if (!p || !p->public)
		{
			log_error("%s find peer by @label %s from link failed", __func__, label);
			return -1;
		}
	} else {
		p = NULL;

		MANAGER_FOREACH_LINK(l, m) {
			if (!l->public)
				continue;

			p = link_find_peer_by_label(l, label);
			if (p)
				break;
		}

		if (!p || !p->public)
		{
			log_error("%s find peer by label %s from link failed", __func__, label);
			return -1;
		}
	}

	*peerOut = p;
	return 0;
}

static int agent_user_dbus_disconnect(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	_shl_free_ char *node = NULL;
	char *peer_label;
	struct peer *p;
	int r;

	r = sd_bus_message_read(msg, "s", &peer_label);

	if (r < 0)
	{
		log_error("%s read dbus msg for peer p2p mac failed", __func__);
		return r;
	}

	log_info("Peer label %s disconnect from sink by user", peer_label);

	r = ui_dbus_find_peer_by_label(peer_label, data, &p);
	if (r < 0)
	{
		return r;
	}

	peer_disconnect(p);

	return sd_bus_reply_method_return(msg, NULL);
}

static int agent_user_dbus_update_sink_resolution(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	int w,h;
	int link_lable;
	int r;
	struct link *l = NULL;
	_shl_free_ char *node = NULL;	

	r = sd_bus_message_read(msg, "iii", &w, &h, &link_lable);
	if (r < 0)
	{
		log_error("read dbus msg for WxH failed");
		return r;
	}

	l = manager_find_link(m, link_lable);
	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		return -1;
	}

	node = link_dbus_get_path(l);

	r = sd_bus_emit_signal(l->m->bus,
				node,
				"org.freedesktop.miracle.wifi.Link",
				"SinkResolution",
				"ii", w, h);
	if (r < 0)
	{
		log_vERR(r);
		return r;
	}
	
	return 0;
}

static int agent_user_dbus_set_name(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
#define IS_SUPPLICANT_CFG_STR_VALID(s) (s && strlen(s))

	struct manager *m = data;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	int link_lable;	
	struct link *l = NULL;
	char *nameToSet;
	int r;

	r = sd_bus_message_read(msg, "is", &link_lable, &nameToSet);

	if (r < 0)
	{
		log_error("read dbus msg for name to set failed");
		return r;
	}	

	l = manager_find_link(m, link_lable);
	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		return -1;
	}

	if(IS_SUPPLICANT_CFG_STR_VALID(nameToSet))
		link_set_friendly_name(l, nameToSet);

	log_info("User try set sink name to %s", nameToSet);

	return sd_bus_reply_method_return(msg, NULL);
}

static int agent_user_dbus_set_config_methods(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	int link_lable;	
	struct link *l = NULL;
	char *configMethodsToSet;
	int r;

	r = sd_bus_message_read(msg, "is", &link_lable, &configMethodsToSet);

	if (r < 0)
	{
		log_error("read dbus msg for config_methods to set failed");
		return r;
	}	

	l = manager_find_link(m, link_lable);
	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		return -1;
	}

	if(IS_SUPPLICANT_CFG_STR_VALID(configMethodsToSet))
		link_set_config_methods(l, configMethodsToSet);

	log_info("User try set sink config_methods to %s", configMethodsToSet);

	return sd_bus_reply_method_return(msg, NULL);
}

static int agent_user_dbus_query_connected_devices(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	_shl_free_ char *path = NULL;
	DIR *dir;
	FILE *fp;
	struct dirent *dirInfo;	
	int r;	
	sd_bus_message *reply;

	r = sd_bus_message_new_method_return(msg, &reply);
	if (r < 0) {
		goto error;
	}

	r = sd_bus_message_open_container(reply, 'a', "{ss}");
	if (r < 0) {
		goto error;
	}

	r = asprintf(&path, WIFI_SAVE_PATH);

	dir = opendir(path);
	if(!dir)
	{
		log_warning("Open path %s failed!!!", path);
		return -1;
	}

	while( NULL != (dirInfo=readdir(dir)) )
	{
		// if(is_special_dir(dirInfo->d_name)) skip . and ..
		// 	continue;
		
		if(strstr(dirInfo->d_name, "conf"))
		{
			char abspath[512] = {0};
			int offset;
			struct stat filStat;
			char device_info[256] = {0};
			char device_name[256] = {0};
			char mac[256] = {0};
			char* line;

			snprintf(abspath, sizeof(abspath), WIFI_SAVE_PATH"%s", dirInfo->d_name);						

			fp = fopen(abspath, "r");			
			if(fp)
			{
				r = stat(abspath, &filStat);
				if(r < 0)
				{
					log_error("stat network conf %s failed,skip it", abspath);
					fclose(fp);
					continue;
				}

				while (!feof(fp) && !ferror(fp)){
					fgets(device_info, sizeof(device_info)-1, fp);

					line = strstr(device_info, "devicename=");
					if (line){
						line += strlen("devicename=");
						memcpy(device_name, line, strlen(line) - 1);
						continue;
					}

					line = strstr(device_info, "bssid=");
					if (line){
						line += strlen("bssid=");
						memcpy(mac, line, strlen(line) - 1);
						break;
					}

				}

				r = sd_bus_message_open_container(reply, 'e', "ss");
				if (r < 0) {
					goto error;
				}

				r = sd_bus_message_append(reply, "ss", device_name, mac);
				if (r < 0) {
					goto error;
				}

				r = sd_bus_message_close_container(reply);
				if (r < 0)
					goto error;


			}
			else
			{
				log_error("open network conf %s failed", abspath);
			}

			fclose(fp);
		}
	}

	r = sd_bus_message_close_container(reply);
	if (r < 0)
		goto error;

	r = sd_bus_send(NULL, reply, NULL);
	if (r < 0) {
		goto error;
	}
	sd_bus_message_unref(reply);

	closedir(dir);

	return 0;
error:
	if(fp){
		fclose(fp);
	}
	if(dir){
		closedir(dir);
	}
	if(reply){
		sd_bus_message_unref(reply);
	}
	return log_EINVAL();
}

static int agent_user_dbus_delete_connected_devices(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	_shl_free_ char *path = NULL;
	DIR *dir;
	struct dirent *dirInfo;	
	char filepath[512] = {0};
	char *mac;
	int link_lable;	
	int r;

	r = sd_bus_message_read(msg, "s", &mac);

	if (r < 0)
	{
		log_error("read dbus msc for mac to get failed");
		goto exit;
	}	

	r = asprintf(&path, WIFI_SAVE_PATH);

	dir = opendir(path);
	if(!dir)
	{
		log_error("Open path %s failed!!!", path);
		goto exit;
	}

	while( NULL != (dirInfo=readdir(dir)) )
	{
		// if(is_special_dir(dirInfo->d_name)) skip . and ..
		// 	continue;
		
		if(strstr(dirInfo->d_name, mac))
		{
			snprintf(filepath, sizeof(filepath), WIFI_SAVE_PATH"%s", dirInfo->d_name);
			r = remove(filepath);
			if(r)
			{	
				log_error("delete %s failed", filepath);
				goto exit;
			}
			return sd_bus_reply_method_return(msg, "i", 0);
		}
	}

	log_error("try to delete %s device", mac);
exit:
	sd_bus_reply_method_return(msg, "i", -1);
	return log_EINVAL();
}

static int agent_r2_session_start(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	int r = 0;
	int link_lable;
	char str_link_lable[16] = {0};
	struct link *l = NULL;	
	struct manager *m = data;
	_shl_free_ char *node = NULL;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	const char* r2_source_addr;
	int r2_source_port;

	r = sd_bus_message_read(msg, "sii", &r2_source_addr, &r2_source_port, &link_lable);
	if (r < 0)
	{
		log_error("%s msg read failed", __func__);
		return r;
	}

	l = manager_find_link(m, link_lable);
	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		return -1;
	}

	node = link_dbus_get_path(l);

	snprintf(str_link_lable, sizeof(str_link_lable)-1, "%d", link_lable);

	r = sd_bus_emit_signal(l->m->bus,
				node,
				"org.freedesktop.miracle.wifi.Link",
				"SinkR2SessStart",
				"sis", r2_source_addr, r2_source_port, str_link_lable);

	if (r < 0)
	{
		log_error("%s SinkR2SessStart emit failed", __func__);
		return r;
	}

	log_info("Link-DBus-MethodCall: AgtStartR2Sess received");
	return sd_bus_reply_method_return(msg, NULL);	
}

static int query_wpas_stopped_status(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	int wpasStopped = manager_get_wpasStopped_flag(m);

	return sd_bus_reply_method_return(msg, "i", wpasStopped);
}

static int query_established_group_info(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	struct link *l;
	struct peer *p;
	int r = 0;
	
	MANAGER_FOREACH_LINK(l, m) {
		if(l->peer_cnt)
		{
			LINK_FOREACH_PEER(p, l) {
				const char* local=NULL,*remote=NULL;

				if(supplicant_peer_get_group_info(p->sp, &local, &remote))
					continue;

				if(strlen(local)
					&& strlen(remote))
				{
					log_info("dbus-call-query group info: local %s, remote %s", local, remote);
					r = sd_bus_reply_method_return(msg, "ssu", local, remote);
					free(local);
					free(remote);
					return r;
				}
			}
		}
	}
	return sd_bus_reply_method_return(msg, "ssu", " ", " ", 0);
}


static int user_logout_handler(sd_bus_message *msg, void *data)
{
	struct link *l = NULL;
	struct manager *m = data;
	
	l = MANAGER_FIRST_LINK(m);
	if(!l)
	{
		log_error("%s : manager find link failed", __func__);
		return -1;
	}

	log_info("Projection Operation from LOGOUT: link %d", l->ifindex);

	l->running = false;
	manager_set_user_enabled(l->m, false);//bUserEnabled is a global switch to control all wpas-objects scanning
	link_supplicant_running_changed(l, false);//notify sinkctl to run and manage this link
	
	return 0;
}

static int set_manufacturer(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;
	_sd_bus_error_free_ sd_bus_error dbus_err = SD_BUS_ERROR_NULL;
	struct link *l = NULL;
	FILE *fpr;
	FILE *fpw;	
	char filepath[512] = {0};
	char temporary_path[512] = {0};
	char line[256] = {0};
	char updateline[256] = {0};
	char *mac;
	char *manufacturer;
	int link_lable;
	int device_manufacturer;	
	int r;

	r = sd_bus_message_read(msg, "ii", &link_lable, &device_manufacturer);

	if (r < 0)
	{
		log_error("read dbus msg for config_methods to set failed");
		log_EINVAL();
	}

	switch (device_manufacturer)
	{
	case MANUFACTURER:
		manufacturer = "manufacturer";
		break;
	case HUAWEI:
		manufacturer = "huawei";
		break;
	case XIAOMI:
		manufacturer = "xiaomi";
		break;
	case OTHER:
		manufacturer = "other";
		break;
	default:
		break;
	}

	l = manager_find_link(m, link_lable);
	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		return -1;
	}
	supplicant_get_mac(l->s, &mac);

	snprintf(filepath, sizeof(filepath), WIFI_SAVE_PATH"klwfd_%s-%s.conf", l->ifname, mac);
	snprintf(temporary_path, sizeof(temporary_path), WIFI_SAVE_PATH"temporaryfile.conf");

	fpr = fopen(filepath, "r");
	if (!fpr)
		return log_ERRNO();
	fpw = fopen(temporary_path, "w");
	if (!fpw)
		return log_ERRNO();

	while (fgets(line, sizeof(line)-1, fpr)){
		if (strstr(line, "manufacturer="))
		{
			snprintf(updateline, sizeof(updateline), "manufacturer=%s\n", manufacturer);
			fputs(updateline, fpw);
		}else
		{
			fputs(line, fpw);
		}
	}
	fclose(fpr);
	fclose(fpw);
	
	r = rename(temporary_path, filepath);
	if(r < 0)
		return log_EINVAL();
	
	return sd_bus_reply_method_return(msg, NULL);
}

static int agent_sent_user_choice(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	int user_choice;
	int link_lable;
	char* config_methods;
	char* friendly_name;
	int r;
	struct link *l = NULL;
	struct manager *m = data;

	r = sd_bus_message_read(msg, "ii", &link_lable, &user_choice);
	if (r < 0)
	{
		log_error("%s read dbus msg for castop failed", __func__);
		goto exit;
	}

	log_info("agent notified user choice %d", user_choice);
	
	l = manager_find_link(m, link_lable);

	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		r = -1;
		goto exit;
	}

	supplicant_got_user_approve_result(l->s, user_choice);

exit:
	sd_bus_reply_method_return(msg, NULL);
	return r;
}


static int agent_user_dbus_cast_op(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
#define IS_SUPPLICANT_CFG_STR_VALID(s) (s && strlen(s))

	bool bStart;
	int link_lable;
	char* config_methods;
	char* friendly_name;
	int r;
	struct link *l = NULL;
	struct manager *m = data;

	r = sd_bus_message_read(msg, "biss", &bStart, &link_lable, &friendly_name, &config_methods);
	if (r < 0)
	{
		log_error("%s read dbus msg for castop failed", __func__);
		return r;
	}

	log_info("Projection Operation from UI: op %s, link %d, device_name %s, config_methods %s", 
		bStart ? "start":"stop", link_lable, 
		IS_SUPPLICANT_CFG_STR_VALID(friendly_name) ? friendly_name : "default",
		IS_SUPPLICANT_CFG_STR_VALID(config_methods) ? config_methods : "default");
	
	l = manager_find_link(m, link_lable);

	if(!l)
	{
		log_error("%s : manager find link %d failed", __func__, link_lable);
		return -1;
	}

	if(bStart && IS_SUPPLICANT_CFG_STR_VALID(friendly_name))
		link_set_friendly_name(l, friendly_name);

	if(bStart && IS_SUPPLICANT_CFG_STR_VALID(config_methods))
		link_set_config_methods(l, config_methods);

	l->running = bStart;
	manager_set_user_enabled(l->m, bStart);//bUserEnabled is a global switch to control all wpas-objects scanning
	link_supplicant_running_changed(l, bStart);//notify sinkctl to run and manage this link

	return sd_bus_reply_method_return(msg, NULL);
}

static void report_friendlyname(struct manager *m)
{
	int r = sd_bus_emit_signal(m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"NameReport",
					"s", m->friendly_name);
	if (r < 0)
		log_error("wifid emit hostname %s signal failed", m->friendly_name);
	else
		log_debug("HANDSHAKEING... report friendly name %s to agent", m->friendly_name);
}

static void visit_link(unsigned int *elem, void *data)
{
	struct manager *m = data;
	int r;

	if(elem)
	{
		struct link *l = link_from_htable(elem);

		r = sd_bus_emit_signal(m->bus,
						"/org/freedesktop/miracle/wifi/ui",
						"org.freedesktop.miracle.wifi.ui",
						"LinkReport",
						"is", l->ifindex, l->ifname);
		if (r < 0)
			log_error("wifid emit ifidx %d LinkReport signal failed", l->ifindex);
		else
			log_debug("HANDSHAKEING... report link %d-%s to agent", l->ifindex, l->ifname);
	}
}

static int agent_user_dbus_handshake_recv(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	struct manager *m = data;

	sd_bus_reply_method_return(msg, NULL);

	//Notify agent to give the management of this running LINK to NetworkManager, thus wifi will continue to work....
	//TODO: change the whole architecture of miracle-cast, using libnm api or use a session-bus based agent to talk with the system wpa_supplicant.service directly
	//As this will be a huge change and need a lot of coding and testing work, do it later , maybe June....	
	shl_htable_visit_uint(&m->links, visit_link, data);

	report_friendlyname(m);

	return 0;
}

static int agent_user_dbus_stop_wpas(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	char command[128] = {0};
	struct manager *m = data;
	int r = 0;
	bool state;

	sd_bus_reply_method_return(msg, NULL);

	snprintf(command, sizeof(command)-1, "systemctl mask wpa_supplicant");

	do{
		r = system(command);
		if(r)
		{
			log_error("Error: mask system wpa_supplicant failed!!!!");
			usleep(500000);
		}
	}while(r);

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command)-1, "systemctl stop wpa_supplicant");

	do{
		r = system(command);
		if(r)
		{
			log_error("Error: Stop system wpa_supplicant failed!!!!");
			usleep(500000);
		}
	}while(r);

	log_info("Stop system wpa_supplicant success!!!!\n");
	
	manager_set_wpasStopped_flag(m, true);
	
	return r;
}

void recover_wpa_supplicant(int rsvd, void *data)
{
	char command[128] = {0};
	struct manager *m = data;
	int r = 0;

	snprintf(command, sizeof(command)-1, "systemctl unmask wpa_supplicant");

	do{
		r = system(command);
		if(r)
		{
			log_error("Error: unmask system wpa_supplicant failed!!!!");
			usleep(500000);
		}
	}while(r);

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command)-1, "systemctl start wpa_supplicant");

	do{
		r = system(command);
		if(r)
		{
			log_error("Error: Start system wpa_supplicant failed!!!!");
			usleep(500000);
		}
	}while(r);

	log_info("Start system wpa_supplicant success!!!!");
	
	manager_set_wpasStopped_flag(m, false);
}

static int agent_user_dbus_start_wpas(sd_bus_message *msg,
				void *data, sd_bus_error *err)
{
	sd_bus_reply_method_return(msg, NULL);
	
	recover_wpa_supplicant(0, data);

	return 0;
}


static int ui_dbus_find(sd_bus *bus,
			  const char *path,
			  const char *interface,
			  void *data,
			  void **found,
			  sd_bus_error *err)
{
	*found = data;
	return 1;
}

static const sd_bus_vtable ui_dbus_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("UsrDisconnect",
		      "s",
		      NULL,
		      agent_user_dbus_disconnect,
		      SD_BUS_VTABLE_UNPRIVILEGED),	//this is a key factor,otherwise meth_call from sess-bus to sys-bus will not permitted
	SD_BUS_METHOD("ProjectionOp",
		      "biss",
		      NULL,
		      agent_user_dbus_cast_op,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("HandShake",
		      "",
		      NULL,
		      agent_user_dbus_handshake_recv,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("StopWpas",
		      "",
		      NULL,
		      agent_user_dbus_stop_wpas,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("StartWpas",
		      "",
		      NULL,
		      agent_user_dbus_start_wpas,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("SetSinkName",
		      "is",
		      NULL,
		      agent_user_dbus_set_name,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("SetSinkConfigMethods",
		      "is",
		      NULL,
		      agent_user_dbus_set_config_methods,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("QueryConnectedDevice",
		      "",
		      "a{ss}",
		      agent_user_dbus_query_connected_devices,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("DeleteConnectedDevice",
		      "s",
		      "i",
		      agent_user_dbus_delete_connected_devices,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("ResolutionUpdate",
		      "iii",
		      NULL,
		      agent_user_dbus_update_sink_resolution,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("AgtStartR2Sess",
		      "sii",
		      NULL,
		      agent_r2_session_start,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("QueryGroupInfo",
		      "",
		      "ssu",
		      query_established_group_info,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("QueryWpaStopped",
		      "",
		      "i",
		      query_wpas_stopped_status,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("SetManufacturer",
		      "ii",
		      NULL,
		      set_manufacturer,
		      SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("userChosen",
		      "ii",
		      NULL,
		      agent_sent_user_choice,
		      SD_BUS_VTABLE_UNPRIVILEGED),

	SD_BUS_SIGNAL("ProvDisc", "sssu", 0),
	SD_BUS_SIGNAL("Invitation", "s", 0),
	SD_BUS_SIGNAL("GoNegReq", "su", 0),
	SD_BUS_SIGNAL("GoNegUsrApprove", "s", 0),
	SD_BUS_SIGNAL("ssidInvalid", "s", 0),
	SD_BUS_SIGNAL("LinkReport", "is", 0),
	SD_BUS_SIGNAL("NameReport", "s", 0),
	SD_BUS_SIGNAL("LinkIsReady", "is", 0),
	SD_BUS_SIGNAL("LinkAdded", "is", 0),
	SD_BUS_SIGNAL("LinkRemoved", "is", 0),
	SD_BUS_SIGNAL("FormationFailure", "s", 0),
	SD_BUS_SIGNAL("FormationOk", "", 0),
	SD_BUS_SIGNAL("KillAgent", "", 0),
	SD_BUS_SIGNAL("PeerQuit", "", 0),
	SD_BUS_SIGNAL("StopPlayerFast", "", 0),
	SD_BUS_SIGNAL("SpawnPlayer", "iiiiiisssu", 0),
	SD_BUS_SIGNAL("RtspFailed", "", 0),
	SD_BUS_SIGNAL("AppNotified", "iss", 0),
	SD_BUS_VTABLE_END
};

static int peer_dbus_find(sd_bus *bus,
			  const char *path,
			  const char *interface,
			  void *data,
			  void **found,
			  sd_bus_error *err)
{
	_shl_free_ char *label = NULL;
	struct manager *m = data;
	struct link *l;
	struct peer *p;
	char *sep;
	int r;

	r = sd_bus_path_decode(path,
			       "/org/freedesktop/miracle/wifi/peer",
			       &label);
	if (r <= 0)
		return r;

	sep = strchr(label, '@');
	if (sep) {
		*sep = 0;
		l = manager_find_link_by_label(m, sep + 1);
		if (!l || !l->public)
			return 0;

		p = link_find_peer_by_label(l, label);
		if (!p || !p->public)
			return 0;
	} else {
		p = NULL;

		MANAGER_FOREACH_LINK(l, m) {
			if (!l->public)
				continue;

			p = link_find_peer_by_label(l, label);
			if (p)
				break;
		}

		if (!p || !p->public)
			return 0;
	}

	*found = p;
	return 1;
}

void peer_dbus_properties_changed(struct peer *p, const char *prop, ...)
{
	_shl_free_ char *node = NULL;
	char **strv;
	int r;

	if (!p->public)
		return;

	node = peer_dbus_get_path(p);
	if (!node)
		return;

	strv = strv_from_stdarg_alloca(prop);
	r = sd_bus_emit_properties_changed_strv(p->l->m->bus,
						node,
						"org.freedesktop.miracle.wifi.Peer",
						strv);
	if (r < 0)
		log_vERR(r);
}

void peer_dbus_provision_discovery(struct peer *p,
				   const char *peerName,
				   const char *type,
				   const char *pin)
{
	_shl_free_ char *node = NULL;
	uint32_t self_uid = INVALID_UID;
	int r;

	if (!type)
		return;
	if (!pin)
		pin = "";

	node = peer_dbus_get_path(p);
	if (!node)
	{
		log_error("PinCode get dbus peer path failed");
		return;
	}

	if(!strcmp(type, "display"))
	{
		r = sd_bus_emit_signal(p->l->m->bus,
					node,
					"org.freedesktop.miracle.wifi.Peer",
					"ProvisionDiscovery",
					"ss", type, pin);
		if (r < 0)
			log_vERR(r);
	}

	//Get current user UID
	if(!p->l)
	{
		log_error("%s nullptr", __func__);
		return;
	}
	get_current_login_user_info(p->l, &self_uid, NULL);
	supplicant_set_uid(p->l->s, self_uid);

	// emit signal : pin generated / or pbc
	r = sd_bus_emit_signal(p->l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"ProvDisc",
					"sssu", type, pin, peerName, self_uid);
	if (r < 0)
		log_vERR(r);
	log_info("************ ProvDisc msg %s - %s emitted", type, pin);
}

void peer_dbus_go_neg_request(struct peer *p,
				   const char *type,
				   const char *pin,
				   const char *friendly_name,
				   int local_sink_go_intent)
{
	_shl_free_ char *node = NULL;	
	uint32_t self_uid = INVALID_UID;	
	int r;

	if (!type)
		return;
	if (!pin)
		pin = "";

	node = peer_dbus_get_path(p);
	if (!node)
		return;

	r = sd_bus_emit_signal(p->l->m->bus,
			       node,
			       "org.freedesktop.miracle.wifi.Peer",
			       "GoNegRequest",
			       "ssi", type, pin, local_sink_go_intent);
	if (r < 0)
		log_vERR(r);

	//Get current user UID
	get_current_login_user_info(p->l, &self_uid, NULL);
	supplicant_set_uid(p->l->s, self_uid);

	r = sd_bus_emit_signal(p->l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"GoNegReq",
					"su", friendly_name,self_uid);
	if (r < 0)
		log_vERR(r);
}

void peer_dbus_invitation_recv(struct peer *p, const char *friendly_name)
{
	uint32_t self_uid = INVALID_UID;

	//Get current user UID
	if(!p->l)
	{
		log_error("%s nullptr", __func__);
		return;
	}
	get_current_login_user_info(p->l, &self_uid, NULL);
	supplicant_set_uid(p->l->s, self_uid);

	int r = sd_bus_emit_signal(p->l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"Invitation",
					"su", friendly_name, self_uid);
	if (r < 0)
		log_vERR(r);
}

void peer_dbus_formation_failure(struct peer *p, const char *reason)
{
	_shl_free_ char *node = NULL;
	int r;

	node = peer_dbus_get_path(p);
	if (!node)
		return;

	r = sd_bus_emit_signal(p->l->m->bus,
			       node,
			       "org.freedesktop.miracle.wifi.Peer",
			       "PeerFormationFailure",
			       "s", reason);
	if (r < 0)
		log_vERR(r);

	r = sd_bus_emit_signal(p->l->m->bus,
			       "/org/freedesktop/miracle/wifi/ui",
			       "org.freedesktop.miracle.wifi.ui",
			       "FormationFailure",
			       "s", reason);
	if (r < 0)
		log_vERR(r);
}

void peer_dbus_added(struct peer *p)
{
	_shl_free_ char *node = NULL;
	int r;

	node = peer_dbus_get_path(p);
	if (!node)
		return;

	r = sd_bus_emit_interfaces_added(p->l->m->bus,
					 node,
					 "org.freedesktop.miracle.wifi.Peer",
					 NULL);
	if (r < 0)
		log_vERR(r);
}

void peer_dbus_removed(struct peer *p)
{
	_shl_free_ char *node = NULL;
	int r;

	node = peer_dbus_get_path(p);
	if (!node)
		return;

	r = sd_bus_emit_interfaces_removed(p->l->m->bus,
					   node,
					   /*
					   "org.freedesktop.DBus.Properties",
					   "org.freedesktop.DBus.Introspectable",
					   */
					   "org.freedesktop.miracle.wifi.Peer",
					   NULL);
	if (r < 0)
		log_vERR(r);
}

/*
 * Link DBus
 */

static int link_dbus_get_interface_index(sd_bus *bus,
					 const char *path,
					 const char *interface,
					 const char *property,
					 sd_bus_message *reply,
					 void *data,
					 sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append_basic(reply, 'u', &l->ifindex);
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_get_interface_name(sd_bus *bus,
					const char *path,
					const char *interface,
					const char *property,
					sd_bus_message *reply,
					void *data,
					sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append_basic(reply, 's', l->ifname);
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_get_friendly_name(sd_bus *bus,
				       const char *path,
				       const char *interface,
				       const char *property,
				       sd_bus_message *reply,
				       void *data,
				       sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append(reply, "s", link_get_friendly_name(l));
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_set_friendly_name(sd_bus *bus,
				       const char *path,
				       const char *interface,
				       const char *property,
				       sd_bus_message *value,
				       void *data,
				       sd_bus_error *err)
{
	struct link *l = data;
	const char *name;
	int r;

	r = sd_bus_message_read(value, "s", &name);
	if (r < 0)
		return r;

	if (!name || !*name)
		return -EINVAL;

	return link_set_friendly_name(l, name);
}

static int link_dbus_get_managed(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *reply,
				      void *data,
				      sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append(reply, "b", link_get_managed(l));
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_get_running(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *reply,
				      void *data,
				      sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append(reply, "b", link_get_running(l));
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_set_managed(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *value,
				      void *data,
				      sd_bus_error *err)
{
	struct link *l = data;
	int val, r;

	r = sd_bus_message_read(value, "b", &val);
	if (r < 0)
		return r;

	return link_set_managed(l, val);
}

int link_dbus_close_player_immediately(struct link *l)
{
	if (!l)
		return -1;

	int r = sd_bus_emit_signal(l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"StopPlayerFast",
					"");
	if (r < 0)
	{
		log_vERR(r);
		return r;
	}

	log_info("notify agent to stop player immediately");
	return 0;
}

int link_dbus_group_removed_by_peer_notification(struct peer *p)
{
	int r = 0;
	_shl_free_ char *node = NULL;

	node = peer_dbus_get_path(p);
	if (!node)
		return -1;

	r = sd_bus_emit_signal(p->l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"PeerQuit",
					"");
	if (r < 0)
	{
		log_vERR(r);
		return r;
	}

	log_debug("Send notification : disconnected from miracast");
	return 0;
}

static int link_dbus_get_p2p_scanning(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *reply,
				      void *data,
				      sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append(reply, "b", link_get_p2p_scanning(l));
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_set_p2p_scanning(sd_bus *bus,
				      const char *path,
				      const char *interface,
				      const char *property,
				      sd_bus_message *value,
				      void *data,
				      sd_bus_error *err)
{
	struct link *l = data;
	int val, r;

	r = sd_bus_message_read(value, "b", &val);
	if (r < 0)
		return r;

	if(manager_get_user_enabled(l->m))
		return link_set_p2p_scanning(l, val);
	else//if global disabled by user, no need to talk to wpas as it is killed already
		return 0;
}

static int link_dbus_get_wfd_subelements(sd_bus *bus,
					 const char *path,
					 const char *interface,
					 const char *property,
					 sd_bus_message *reply,
					 void *data,
					 sd_bus_error *err)
{
	struct link *l = data;
	int r;

	r = sd_bus_message_append(reply, "s", link_get_wfd_subelements(l));
	if (r < 0)
		return r;

	return 1;
}

static int link_dbus_set_wfd_subelements(sd_bus *bus,
					 const char *path,
					 const char *interface,
					 const char *property,
					 sd_bus_message *value,
					 void *data,
					 sd_bus_error *err)
{
	struct link *l = data;
	const char *val;
	int r;

	r = sd_bus_message_read(value, "s", &val);
	if (r < 0)
		return r;

	return link_set_wfd_subelements(l, "0", val);//not reached now
}

static const sd_bus_vtable link_dbus_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("StartPlayer",
		      "iiiiiiss",
		      NULL,
		      link_spawn_player,
		      0),
	SD_BUS_METHOD("StopPlayer",
		      "",
		      NULL,
		      link_kill_player,
		      0),
	SD_BUS_METHOD("SinkStopR2Sess",
		      "",
		      NULL,
		      link_r2_session_stopped,
		      0),
	SD_BUS_PROPERTY("InterfaceIndex",
			"u",
			link_dbus_get_interface_index,
			0,
			SD_BUS_VTABLE_PROPERTY_CONST),
	SD_BUS_PROPERTY("InterfaceName",
			"s",
			link_dbus_get_interface_name,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_PROPERTY("Running",
			"b",
			link_dbus_get_running,
			0,
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_WRITABLE_PROPERTY("FriendlyName",
				 "s",
				 link_dbus_get_friendly_name,
				 link_dbus_set_friendly_name,
				 0,
				 SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_WRITABLE_PROPERTY("Managed",
				 "b",
				 link_dbus_get_managed,
				 link_dbus_set_managed,
				 0,
				 SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_WRITABLE_PROPERTY("P2PScanning",
				 "b",
				 link_dbus_get_p2p_scanning,
				 link_dbus_set_p2p_scanning,
				 0,
				 SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_WRITABLE_PROPERTY("WfdSubelements",
				 "s",
				 link_dbus_get_wfd_subelements,
				 link_dbus_set_wfd_subelements,
				 0,
				 SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_SIGNAL("SinkResolution", "ii", 0),
	SD_BUS_SIGNAL("SinkR2SessStart", "sis", 0),
	SD_BUS_VTABLE_END
};

static int link_dbus_find(sd_bus *bus,
			  const char *path,
			  const char *interface,
			  void *data,
			  void **found,
			  sd_bus_error *err)
{
	_shl_free_ char *label = NULL;
	struct manager *m = data;
	struct link *l;
	int r;

	r = sd_bus_path_decode(path,
			       "/org/freedesktop/miracle/wifi/link",
			       &label);
	if (r <= 0)
		return r;

	l = manager_find_link_by_label(m, label);
	if (!l || !l->public)
		return 0;

	*found = l;
	return 1;
}

void link_dbus_properties_changed(struct link *l, const char *prop, ...)
{
	_shl_free_ char *node = NULL;
	char **strv;
	int r;

	if (!l->public)
		return;

	node = link_dbus_get_path(l);
	if (!node)
		return;
	
	strv = strv_from_stdarg_alloca(prop);
	r = sd_bus_emit_properties_changed_strv(l->m->bus,
						node,
						"org.freedesktop.miracle.wifi.Link",
						strv);
	if (r < 0)
		log_vERR(r);
}

void link_dbus_added(struct link *l)
{
	_shl_free_ char *node = NULL;
	int r;

	node = link_dbus_get_path(l);
	if (!node)
		return;

	r = sd_bus_emit_interfaces_added(l->m->bus,
					 node,
					 /*
					 "org.freedesktop.DBus.Properties",
					 "org.freedesktop.DBus.Introspectable",
					 */
					 "org.freedesktop.miracle.wifi.Link",
					 NULL);
	if (r < 0)
		log_vERR(r);
}

void link_dbus_removed(struct link *l)
{
	_shl_free_ char *node = NULL;
	int r;

	node = link_dbus_get_path(l);
	if (!node)
		return;

	r = sd_bus_emit_interfaces_removed(l->m->bus,
					   node,
					   /*
					   "org.freedesktop.DBus.Properties",
					   "org.freedesktop.DBus.Introspectable",
					   */
					   "org.freedesktop.miracle.wifi.Link",
					   NULL);
	if (r < 0)
		log_vERR(r);
}

/*
 * Manager DBus
 */

static const sd_bus_vtable manager_dbus_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_VTABLE_END
};

static int manager_dbus_enumerate(sd_bus *bus,
				  const char *path,
				  void *data,
				  char ***out,
				  sd_bus_error *err)
{
	struct manager *m = data;
	struct link *l;
	struct peer *p;
	size_t i, peer_cnt;
	char **nodes, *node;
	int r;

	peer_cnt = 0;
	MANAGER_FOREACH_LINK(l, m)
		if (l->public)
			peer_cnt += l->peer_cnt;

	nodes = malloc(sizeof(*nodes) * (m->link_cnt + peer_cnt + 2));
	if (!nodes)
		return log_ENOMEM();

	i = 0;
	MANAGER_FOREACH_LINK(l, m) {
		if (i >= m->link_cnt + peer_cnt) {
			log_warning("overflow: skipping link %s",
				    l->ifname);
			continue;
		}

		if (!l->public)
			continue;

		node = link_dbus_get_path(l);
		if (!node)
			goto error;

		nodes[i++] = node;

		LINK_FOREACH_PEER(p, l) {
			if (i >= m->link_cnt + peer_cnt) {
				log_warning("overflow: skipping peer %s",
					    p->p2p_mac);
				continue;
			}

			if (!p->public)
				continue;

			node = peer_dbus_get_path(p);
			if (!node)
				goto error;

			nodes[i++] = node;
		}
	}

	node = strdup("/org/freedesktop/miracle/wifi");
	if (!node) {
		r = log_ENOMEM();
		goto error;
	}

	nodes[i++] = node;
	nodes[i] = NULL;
	*out = nodes;

	return 0;

error:
	while (i--)
		free(nodes[i]);
	free(nodes);
	return r;
}

static int load_miracast_ini_file(struct link *l){
#define IS_SUPPLICANT_CFG_STR_VALID(s) (s && strlen(s))

	FILE *fp;
	char filepath[512] = {0};
	char device_info[256] = {0};
	char *line;
	char friendly_name[256] = {0};
	char config_methods[64] = {0};
	char *current_uname = NULL;
	int r = 0;
	
	if(!l)
	{
		log_error("%s : nullptr", __func__);
		return -1;
	}
	get_current_login_user_info(l, NULL, &current_uname);

	if(!current_uname)
	{
		log_error("failed to get current username");
		return -1;
	}
	snprintf(filepath, sizeof(filepath), "/home/%s/.config/miracast.ini", current_uname);						

	fp = fopen(filepath, "r");			
	if(fp)
	{
		while (!feof(fp) && !ferror(fp)){
			fgets(device_info, sizeof(device_info)-1, fp);
			line = strstr(device_info, "host=");
			if (line){
				line += strlen("host=");
				memcpy(friendly_name, line, strlen(line) - 1);	
			}
			
			line = strstr(device_info, "config_method=");
			if (line){
				line += strlen("config_method=");
				memcpy(config_methods, line, strlen(line) - 1);	
			}
		}
		fclose(fp);
	}
	else
	{
		log_error("open %s failed", filepath);
		return -1;
	}
	
	if(IS_SUPPLICANT_CFG_STR_VALID(friendly_name))
	{
		r = link_set_friendly_name(l, friendly_name);
		if (r < 0)
			log_error("failed to set friendly_name=%s", friendly_name);
	}
	if(IS_SUPPLICANT_CFG_STR_VALID(config_methods))
	{
		link_set_config_methods(l, config_methods);
		if (r < 0)
			log_error("failed to set config_methods=%s", config_methods);
	}
	
	return r ? -1 : 0;
}

int ctl_login1_Manager_fn(sd_bus_message *m,
				  void *data,
				  sd_bus_error *err)
{
	if (sd_bus_message_is_signal(m,"org.freedesktop.login1.Manager","UserRemoved"))
	{
		const char *objpath;
		uint32_t current_uid;
		uint32_t initial_uid = INVALID_UID;
		int r = 0;
		struct link *l = NULL;
		struct manager *manager = data;
		
		r = sd_bus_message_read(m, "uo", &current_uid, &objpath);
		if (r < 0)
			return log_EINVAL();
		
		l = MANAGER_FIRST_LINK(manager);
		if(!l)
		{
			log_error("%s : manager find link failed", __func__);
			return -1;
		}

		r = supplicant_get_uid(l->s, &initial_uid);
		if(r < 0)
			return log_EINVAL();

		if(current_uid == initial_uid)
		{
			user_logout_handler(m, data);
			supplicant_set_uid(l->s, INVALID_UID);
			log_info("UserRemoved....user %d is logout", current_uid);
			return 0;
		}

		log_info("UserRemoved....Switch user or user %d is logout", current_uid);
		r = load_miracast_ini_file(l);
		if(r < 0)
			log_error("failed to load miracast.ini");
	}

	return 0;
}

int manager_dbus_connect(struct manager *m)
{
	int r;

	r = sd_bus_add_object_vtable(m->bus, NULL,
				     "/org/freedesktop/miracle/wifi",
				     "org.freedesktop.miracle.wifi.Manager",
				     manager_dbus_vtable,
				     m);
	if (r < 0)
		goto error;

	r = sd_bus_add_node_enumerator(m->bus, NULL,
				       "/org/freedesktop/miracle/wifi",
				       manager_dbus_enumerate,
				       m);
	if (r < 0)
		goto error;

	r = sd_bus_add_fallback_vtable(m->bus, NULL,
				       "/org/freedesktop/miracle/wifi/link",
				       "org.freedesktop.miracle.wifi.Link",
				       link_dbus_vtable,
				       link_dbus_find,
				       m);
	if (r < 0)
		goto error;

	r = sd_bus_add_fallback_vtable(m->bus, NULL,
				       "/org/freedesktop/miracle/wifi/peer",
				       "org.freedesktop.miracle.wifi.Peer",
				       peer_dbus_vtable,
				       peer_dbus_find,
				       m);
	if (r < 0)
		goto error;

	r = sd_bus_add_fallback_vtable(m->bus, NULL,
				       "/org/freedesktop/miracle/wifi/ui",
				       "org.freedesktop.miracle.wifi.ui",
				       ui_dbus_vtable,
				       ui_dbus_find,
				       m);

	if (r < 0)
		goto error;

	r = sd_bus_add_match(m->bus, NULL,
			     "type='signal',"
			     "sender='org.freedesktop.login1',"
			     "interface='org.freedesktop.login1.Manager'",
			     ctl_login1_Manager_fn,
			     m);

	if (r < 0)
	{
		log_error("Subscribe signal login failed");
		goto error;
	}

	r = sd_bus_add_object_manager(m->bus, NULL, "/org/freedesktop/miracle/wifi");
	if (r < 0)
		goto error;

	r = sd_bus_request_name(m->bus, "org.freedesktop.miracle.wifi", 0);
	if (r < 0) {
		if (r == -EEXIST) {
			log_info("cannot claim org.freedesktop.miracle.wifi bus-name: it is already being adquired");
		} else {
			log_error("cannot claim org.freedesktop.miracle.wifi bus-name: %d",
			  r);
		}
		goto error_silent;
	}

	return 0;

error:
	log_vERR(r);
error_silent:
	manager_dbus_disconnect(m);
	return r;
}

void manager_dbus_disconnect(struct manager *m)
{
	if (!m || !m->bus)
		return;

	sd_bus_release_name(m->bus, "org.freedesktop.miracle.wifi");
}

void dbus_signal_for_android_app_agent(struct link *l, bool bConnected, const char* localAddr, const char* remoteAddr)
{
	int r;

	r = sd_bus_emit_signal(l->m->bus,
					"/org/freedesktop/miracle/wifi/ui",
					"org.freedesktop.miracle.wifi.ui",
					"AppNotified",
					"iss", bConnected ? 1 : 0, localAddr, remoteAddr);
	if (r < 0)
		log_error("%s signal emit failed", __func__);
	else
		log_info("Link-DBus-MethodCall: Notify App: local %s, remote %s", localAddr, remoteAddr);
}