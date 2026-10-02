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
 * @file MainBoardMethod.java
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief Java获取主板信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup libkysdk-system-java
 * @{
 * 
 */

package kylin.kysdk.java.method;
import com.kylin.kysdk.mainboard;
import kylin.kysdk.java.client.DBusClient;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.exceptions.DBusException;
import lombok.extern.slf4j.Slf4j;;

/**
 * @brief 获取主板信息类
 * 主要用来获取主板型号、生产日期、序列号、厂商信息
 */
@Slf4j
public class MainBoardMethod implements mainboard {
    /**
     * @brief 获取主板型号
     *
     * @return String 主板型号
     */
    @Override
    public String getMainboardName(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            mainboard obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/mainboard", mainboard.class);
            return obj.getMainboardName();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getMainboardName:", e);
        }
        return null;
    }

    /**
     * @brief 获取主板生产日期
     *
     * @return String 主板生产日期
     */
    @Override
    public String getMainboardDate(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            mainboard obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/mainboard", mainboard.class);
            return obj.getMainboardDate();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getMainboardDate:", e);
        }
        return null;
    }

    /**
     * @brief 获取主板序列号
     *
     * @return String 主板序列号
     */
    @Override
    public String getMainboardSerial(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            mainboard obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/mainboard", mainboard.class);
            return obj.getMainboardSerial();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getMainboardSerial:", e);
        }
        return null;
    }

    /**
     * @brief 获取主板厂商
     *
     * @return String 主板厂商名
     */
    @Override
    public String getMainboardVendor(){
        DBusConnection conn = null;
        try {
            conn = DBusClient.getConn(DBusConnection.DBusBusType.SYSTEM);
            mainboard obj = conn.getRemoteObject("com.kylin.kysdk.service",
                    "/com/kylin/kysdk/mainboard", mainboard.class);
            return obj.getMainboardVendor();
        } catch (DBusException e) {
            if (conn != null) conn.disconnect();
            log.error("DBusException getMainboardVendor:", e);
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
