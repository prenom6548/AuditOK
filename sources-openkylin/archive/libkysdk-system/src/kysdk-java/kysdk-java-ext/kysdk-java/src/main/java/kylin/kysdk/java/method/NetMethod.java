/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

/**
 * @file NetMethod.java
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief Java获取网络信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup libkysdk-system-java
 * @{
 * 
 */

package kylin.kysdk.java.method;

import kylin.kysdk.java.client.DBusClient;
import com.kylin.kysdk.net;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;
import java.util.List;

/**
 * @brief 获取网络信息类
 * 主要用来获取指定网络端口的状态、区间端口状态、网关信息、获取防火墙信息
 */
@Slf4j
public class NetMethod implements net {
    /**
     * @brief 获取指定网络端口的状态
     * 
     * @param port 端口号
     * @return int 端口状态 TCP_ESTABLISHED:0x1   TCP_SYN_SENT:0x2    TCP_SYN_RECV:0x3    TCP_FIN_WAIT1:0x4    TCP_FIN_WAIT2:0x5
     * TCP_TIME_WAIT:0x6    TCP_CLOSE:0x7    TCP_CLOSE_WAIT:0x8    TCP_LAST_ACL:0x9    TCP_LISTEN:0xa    TCP_CLOSING:0xb
     */
    @Override
    public int getPortState(int port){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            net obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/net", net.class);
            return obj.getPortState(port);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getPortState:", e);
        }
        return -1;
    }

    /**
     * @brief 获取区间端口状态
     * 
     * @param begin 开始端口
     * @param end 结束端口
     * @return List<String> 区间端口状态列表
     */
    @Override
    public List<String>  getMultiplePortStat(int begin,int end){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            net obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/net", net.class);
            return obj.getMultiplePortStat(begin,end);
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getMultiplePortStat:", e);
        }
        return null;
    }

    /**
     * @brief 获取网关信息
     *
     * @return List<String> 网关信息网卡名和地址列表
     */
    @Override
    public List<String> getGatewayInfo(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            net obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/net", net.class);
            return obj.getGatewayInfo();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getGatewayInfo:", e);
        }
        return null;
    }

    /**
     * @brief 获取防火墙信息
     * 
     * @return List<String> 防火墙信息列表
     */
    @Override
    public List<String> getFirewallState(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            net obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/net", net.class);
            return obj.getFirewallState();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getFirewallState:", e);
        }
        return null;
    }

    /**
     * @brief 判断是否为远程对象
     *
     * @return boolean Returns true on remote objects. Local objects implementing this interface MUST return false.
     */
    @Override
    public boolean isRemote() {
        return false;
    }

    /**
     * @brief 获取ObjectPath【未实现】
     *
     * @return String ObjectPath
     */
    @Override
    public String getObjectPath() {
        return null;
    }
}

/**
 * \example kysdk-system/src/kysdk-java/kysdk-java-ext/kysdk-java/src/test/java/kylin/kysdk/java/EventTest.java
 * 
 */
