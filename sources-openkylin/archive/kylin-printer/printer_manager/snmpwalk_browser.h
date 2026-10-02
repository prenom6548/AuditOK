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

#ifndef SNMPWALK_BROWSER_H
#define SNMPWALK_BROWSER_H

#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/*
 * a list of hosts to query
 */
struct HostNode
{
	const char *name;
	const char *community;
};
// HostNode hosts[] = {
// 	{ "192.168.17.90", "public" },
// 	{ NULL }
// };

/*
 * a list of variables to query for
 */
struct OidNode
{
    const char *Name;
    oid Oid[MAX_OID_LEN];
    int OidLen;
};
// struct OidNode oids[] = {
// 	{ "1.3.6.1.2.1.4.20.1.1" },
// 	{ "1.3.6.1.2.1.4.20.1.3" },
// 	{ "1.3.6.1.2.1.1.5" },
// 	{ "1.3.6.1.2.1.43.9.2.1.8" },
// 	{ "1.3.6.1.2.1.25.3.2.1.3" },
// 	{ "1.3.6.1.2.1.43.5.1.1.17" },
// 	{ NULL }
// };



int snmpwalkBrowser_initialize(std::vector<OidNode>  &oids);

int snmpwalkBrowser_synchronous(std::vector<std::string>    &buf,
                            const std::vector<HostNode> &hosts,
                            std::vector<OidNode>        &oids);

#endif // SNMPWALK_BROWSER_H