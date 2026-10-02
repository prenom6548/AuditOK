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
 * @file BiosMethod.java
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief Java获取bios信息
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
import com.kylin.kysdk.bios;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;
import java.util.List;


/**
 * @brief 获取bios信息类
 * 主要用来获取bios的厂商和版本信息
 */
@Slf4j
public class BiosMethod implements bios {

    /**
     * @brief 获取bios厂商
     *
     * @return String 厂商名
     */
    @Override
    public String getBiosVendor(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            bios obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/bios", bios.class);
            return obj.getBiosVendor();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getBiosVendor:", e);
        }
        return null;
    }

    /**
     * @brief 获取bios版本
     *
     * @return String 版本号
     */
    @Override
    public String getBiosVersion(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            bios obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/bios", bios.class);
            return obj.getBiosVersion();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getBiosVersion:", e);
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
