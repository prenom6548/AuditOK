/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef RETURNCODE_H
#define RETURNCODE_H
#include <QString>
//检测状态
enum class CheckStatus : int {
    INIT = 0,
    CHECKING,
    EVERTHING_IS_OK,
    ERROR,
    WARNING,
    UNKNOWN,
    EXTRANET_OK,
    EXTRANET_ERR,
    INTRANET_OK,
    INTRANET_ERR,
    NET_WARNING
};
//检测项
enum class CheckType : int {
    // 1. 网络硬件配置检测
    // 1) 检测是否有硬件设备（networkmanager的getDevices接口返回所有）
    // 2) 检测是否有活动链接（networkmanager的activeconnection）
    // 3) 检测活动链接状态（networkmanager的checkconnectivity）
    NW_HW_DEVICE_CONF = 0,

    // 2. 网络连接配置检测
    // 1） 检测系统首选连接（networkmanager的primaryconnectionj接口返回首选连接名，此项仅会有一项）
    // 2）
    // 检测是否打开了DHCP（检测1）返回的connection的对应settings，其中的对应ipv4或ipv6项的method中是否是auto/manual）
    // 3）若是auto，则不检测此项。若是manual，则检测对应的IP，掩码，网关是否正确。
    NW_CON_CONF,

    // 3. DHCP检测
    // 1） 检测DHCP服务是否正常
    //根据活动链接（networkmanager的activeconnection）获取其DHCP配置（Dhcp4Config），
    //检测其配置中是否有内容，ip地址，掩码，网关是否正确。
    DHCP_CONF,

    // 4. DNS配置
    // 1）检测DNS服务是否正常
    //根据活动链接（networkmanager的activeconnection）获取其IP配置（IP4Config），
    //检测其(NameserverData）项是否有内容。
    //①若有，格式是否正确，检测是否可以ping通
    //②若无，是否检测第二项？还是警示内网不可达的原因有可能是未配置DNS？
    // 2） 检测DNS服务器是否可正常解析网络
    // nslookup + 网站地址（内外网根据用户设置），判断是否可用。
    DNS_CONF,

    // 5. hosts文件检测
    // 1） 内容解析（读取/etc/hosts文件）
    //格式应为：IP格式 +  （空格 + 网站格式）*N
    // 2） 本地回环是否正确（localhost与hostname是否正确）
    HOST_CONF,

    // 6. 系统代理检测
    // 1)系统是否配置代理（根据gsettings取得是否手动配置代理,若未配置，则不检查下一项）
    // 2) 代理是否正常（若配置代理，则检测默认的网站或用户配置的检测网址是否可以ping通，
    //不可ping通则提示用户检查是否是代理配置原因）
    //这里因为功能原因，暂时需要先屏蔽代理项,后期完善了再放开
    //    PROXY_CONF,

    // 7&8电脑可否上网（内网检测与外网检测，两项互斥）
    // 1） 是否配置了内网检测
    // 2） 若配置了内网检测，根据提供的api，检测返回码。
    // 3） 若未配置，则ping百度首页。
    NET_CHECK,
    UNKNOWN = 10000,
};

//错误类型
enum class CHECKRESULT : int {
    SUCC = 0,
    FAIL = 1,
    WARN = 2,
    UNKNOWN = 3,

    // HARDWARE
    HAS_VALID_NETCARD_CONN,
    HAS_VALID_NETCARD_NO_CONN,
    NO_VALID_NETCARD,

    // IP
    DHCP_ON,
    DHCP_OFF,
    SAME_VLAN,
    DIFF_VLAN,

    // DHCP
    DHCP_OK,
    DHCP_ERR,

    // DNS
    NO_DNS,
    DNS_OK,
    DNS_ERR, // 15

    // HOST
    HOST_OK,
    HOST_ERR,
    HOST_NO_FILE,

    // Proxy
    PROXY_OFF,
    PROXY_AUTO_VALID,
    PROXY_AUTO_INVALID,
    PROXY_MANUAL_VALID,
    PROXY_MANUAL_INVALID,

    // net
    NET_IN_CHECK_SUCC,
    NET_IN_CHECK_PERFECT,
    NET_IN_CHECK_ERR,
    NET_OUT_CHECK_SUCC,
    NET_OUT_CHECK_PERFECT,
    NET_OUT_CHECK_ERR,
    NET_ONLY_IN_SUCC, //对应外网检测失败，内网检测成功
    NET_ONLY_OUT_ERR, //对应外网检测失败，但是内网检测又没有开
    NET_BOTH_ERR,     //对应都失败
};

struct statusStruct
{
	int m_index = -1;
	bool m_detailMode = false;

	QString m_projectName = nullptr;
	QString m_projectDetail = nullptr;
	QString m_projectRes = nullptr;

	CheckStatus m_curStutus = CheckStatus::UNKNOWN;
	CHECKRESULT m_curRes = CHECKRESULT::UNKNOWN;

	void operator=(const statusStruct &other)
	{
		m_index = other.m_index;
		m_curStutus = other.m_curStutus;
		m_projectDetail = other.m_projectDetail;
		m_projectRes = other.m_projectRes;
	}

	void setStatusCheck(const CheckStatus &status)
	{
		m_curStutus = status;
	}

	void setCurInfo(const QString &projectDetail, const QString &projectRes)
	{
		m_projectDetail = projectDetail;
		m_projectRes = projectRes;
	}

	void setDetailMode(bool trigger)
	{
		m_detailMode = trigger;
	}
};

#define IPV4_LOCALHOST_REGEX ("(127.0.0.1)( |\t){1,}(localhost)")
#define IPV4_LOCALPCHOST_REGEX ("(127.0.1.1)( |\t){1,}")
#define IPV6_LOCALALLROUTERS_REGEX ("((ff02::2)( |\t){1,}(ip6-allrouters))")
#define IPV6_LOCALALLNODES_REGEX ("((ff02::1)( |\t){1,}(ip6-allnodes))")
#define IPV6_LOCALCAST_REGEX ("((ff00::0)( |\t){1,}(ip6-mcastprefix))")
#define IPV6_LOCALNET_REGEX ("((fe00::0)( |\t){1,}(ip6-localnet))")
#define IPV6_LOCALHOST_REGEX ("((::1)( |\t){1,}(ip6-localhost|ip6-loopback)( |\t){1,}(ip6-localhost|ip6-loopback))")
#define IPV6_ADDRESS_REGEX                                                                                             \
    ("(( "                                                                                                             \
     "|\t){0,}([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]" \
     "{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-" \
     "fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:" \
     "[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{" \
     "1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-" \
     "9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])(" \
     "( |\t){1,}([0-9a-zA-Z]{1,16}\.){1,}([0-9a-zA-Z]{1,16})){1,}) ")
#define IPV4_ADDRESS_REGEX                                                                                             \
    ("(( |\t){0,}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}(( "               \
     "|\t){1,}([0-9a-zA-Z]{1,16}\.){1,}([0-9a-zA-Z]{1,16})){1,})")
#endif // RETURNCODE_H
