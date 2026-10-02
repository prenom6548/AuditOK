/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QDebug>

#ifdef USE_SELF_CONTAINED_DEP
    #include "securec.h"
#else
    #include <boundscheck/securec.h>
#endif // USE_SELF_CONTAINED_DEP

#include "snmpwalk_browser.h"

int find_first_oid(netsnmp_session *ss, oid *base, int base_length)
{
	netsnmp_pdu *response;
	netsnmp_pdu *pdu;
	int running = 1;
	int status;
	int length = 0;
	pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
	snmp_add_null_var(pdu, base, base_length);
	while (running)
	{
		// 这一句在一些情况下会报 “corrupted size vs. prev_size“
		// 导致程序段错误，但是概率较低，之前会在输出错误的ip地址偶尔复现，
		// 暂时没有可以稳定复现的方法，也没有定位到问题的原因，怀疑和网络环境有关
		status = snmp_synch_response(ss, pdu, &response);
		if (status != STAT_SUCCESS || !response)
		{
			snmp_sess_perror("snmp_synch_response", ss);
			return -1;
		}
		if (response->errstat != SNMP_ERR_NOERROR)
		{
			fprintf(stderr, "snmp: Error in packet: %s\n", snmp_errstring(response->errstat));
			return -1;
		}
		if (response && snmp_oid_compare(response->variables->name, SNMP_MIN(base_length, response->variables->name_length), base, base_length) != 0)
			running = 0;
		else
		{
			memcpy(base, response->variables->name, response->variables->name_length * sizeof(oid));
			length = response->variables->name_length;
			pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
			snmp_add_null_var(pdu, response->variables->name, response->variables->name_length);
			running = 0;
		}
		snmp_free_pdu(response);
	}
	return length;
}

/*
 * initialize
 */
int initialize(std::vector<OidNode> &oids)
{
	/* Win32: init winsock */
	SOCK_STARTUP;

	/* initialize library */
	init_snmp("asynchapp");

	/* parse the oids */
	for (size_t i = 0; i < oids.size(); i++) {
		OidNode *op = &oids[i];
		op->OidLen = sizeof(op->Oid) / sizeof(op->Oid[0]);
		if (!read_objid(op->Name, op->Oid, (size_t *)&(op->OidLen))) {
			snmp_perror("read_objid");
			return 1;
		}
	}
	return 0;
}

/*
 * simple printing of returned data
 */
int print_result(std::vector<std::string> &output,
                 int status,
				 struct snmp_session *sp,
				 struct snmp_pdu *pdu,
				 int &res)
{
	char buf[1024];
	struct variable_list *vp;

	res = status;
	switch (status)
	{
	case STAT_SUCCESS:
		res = int(pdu->errstat);
		vp = pdu->variables;
		if (pdu->errstat == SNMP_ERR_NOERROR) {
			while (vp) {
				snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);
				// snprintf(buf, sizeof(buf), "%s", buf);
				// fprintf(stdout, "%s: %s\n", sp->peername, buf);
				output.push_back(buf);
				vp = vp->next_variable;
			}
		}
		else if (pdu->errstat == SNMP_ERR_NOSUCHNAME) {
			qDebug("%s: %s: %s\n",
				   sp->peername, buf, snmp_errstring(pdu->errstat));
			return 0;
		}
		else {
			for (int ix = 1; vp && ix != pdu->errindex; vp = vp->next_variable, ix++)
				;
			if (vp)
				snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
			else
				strcpy_s(buf, sizeof(buf), "(none)");
			qDebug("%s: %s: %s\n",
					sp->peername, buf, snmp_errstring(pdu->errstat));
		}
		return 1;
	case STAT_TIMEOUT:
		qDebug("%s: Timeout\n", sp->peername);
		return 0;
	case STAT_ERROR:
		snmp_perror(sp->peername);
		return 0;
	}
	return 0;
}

/*****************************************************************************/

/*
 * simple synchronous loop
 */
void synchronous(std::vector<std::string>    &buf,
				 const std::vector<HostNode> &hosts,
				 std::vector<OidNode>  &oids,
				 int &res)
{
	for (size_t i = 0; i < hosts.size(); i++)
	{
		const HostNode *hp = &hosts[i];
		struct snmp_session ss, *sp;

		snmp_sess_init(&ss); /* initialize session */
        ss.version = SNMP_VERSION_1;
		ss.peername = strdup(hp->name);
		ss.community = (u_char *)strdup(hp->community);
		ss.community_len = strlen((char *)(ss.community));
		if (!(sp = snmp_open(&ss)))
		{
			snmp_perror("snmp_open");
			continue;
		}
		for (size_t j = 0; j < oids.size(); j++)
		{
			OidNode *op = &oids[j];
			struct snmp_pdu *req, *resp;
			int status;
			// printf("%s %d\n",op->Name, op->OidLen);
			int new_length = find_first_oid(sp, op->Oid, op->OidLen);
			if (new_length == -1) {
				res = STAT_ERROR;
				return;
			}
			req = snmp_pdu_create(SNMP_MSG_GET);
			snmp_add_null_var(req, op->Oid, new_length);
			status = snmp_synch_response(sp, req, &resp);
			if (!print_result(buf, status, sp, resp, res))
				break;
			snmp_free_pdu(resp);
		}
		snmp_close(sp);
	}
}

/*****************************************************************************/

int snmpwalkBrowser_initialize(std::vector<OidNode>  &oids)
{
	return initialize(oids);
}

int snmpwalkBrowser_synchronous(std::vector<std::string>    &buf,
                                const std::vector<HostNode> &hosts,
                                std::vector<OidNode>        &oids)
{
	if (hosts.size() != 1) {
		return -1;
	}
	int res = 0;
	// std::cout << hosts[0].name << ' ' <<oids[0].Name << std::endl;
	synchronous(buf, hosts, oids, res);
	// std::cout << hosts[0].name << ' ' <<oids[0].Name << std::endl;
	// std::cout << buf[0] << std::endl;
	return res;
}
