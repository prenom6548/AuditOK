/*
 * libkysdk-thememanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

#include "kwatermark.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <libkysysinfo.h>

namespace kdk
{

#define KWIN_SERVER "org.ukui.KWin"
#define KWIN_PATH "/Watermark"
#define KWIN_INTERFACE "org.ukui.kwin.Watermark"

#define WLCOM_SERVER "com.kylin.Wlcom"
#define WLCOM_PATH "/com/kylin/Wlcom/Watermark"
#define WLCOM_INTERFACE "com.kylin.Wlcom.Watermark"

    WatermarkData::WatermarkData()
        : opticity(0),
          x(0),
          y(0),
          expand(0),
          topmost(false)
    {
    }

    KWatermark::KWatermark()
    {
    }

    KWatermark::~KWatermark()
    {
    }

    WatermarkData KWatermark::createWatermark(QString path, double opticity, int x, int y, int expand, bool topmost)
    {
        const char *major = kdk_system_get_major_version();
        if (0 == strncmp(major, "V10-SP1", strlen("V10-SP1")))
        {
            return createWatermarkSP1(path, opticity, x, y, expand, topmost);
        }
        else if (0 == strncmp(major, "V11", strlen("V11")))
        {
            return createWatermarkV11(path, opticity, x, y, expand, topmost);
        }

        return WatermarkData();
    }

    bool KWatermark::updateWatermark(WatermarkData &handle, QString path, double opticity, int x, int y, int expand, bool topmost)
    {
        const char *major = kdk_system_get_major_version();
        if (0 == strncmp(major, "V10-SP1", strlen("V10-SP1")))
        {
            return updateWatermarkSP1(handle, path, opticity, x, y, expand, topmost);
        }
        else if (0 == strncmp(major, "V11", strlen("V11")))
        {
            return updateWatermarkV11(handle, path, opticity, x, y, expand, topmost);
        }

        return false;
    }

    void KWatermark::destroyWatermark(WatermarkData &handle)
    {
        const char *major = kdk_system_get_major_version();
        if (0 == strncmp(major, "V10-SP1", strlen("V10-SP1")))
        {
            destroyWatermarkSP1(handle);
        }
        else if (0 == strncmp(major, "V11", strlen("V11")))
        {
            destroyWatermarkV11(handle);
        }

        return;
    }

    WatermarkData KWatermark::createWatermarkSP1(QString &path, double opticity, int x, int y, int expand, bool topmost)
    {
        WatermarkData data;
        QDBusInterface interface(KWIN_SERVER, KWIN_PATH, KWIN_INTERFACE);
        if (interface.isValid())
        {
            QDBusReply<void> reply = interface.call("updateWatermarkEx", path, opticity, x, y, expand, topmost);
            if (reply.isValid())
            {
                data.path = path;
                data.opticity = opticity;
                data.x = x;
                data.y = y;
                data.expand = expand;
                data.topmost = topmost;
            }
            else
            {
                qDebug() << reply.error().message();
            }
        }
        else
            qDebug() << "Connect dbus fail";

        return data;
    }

    WatermarkData KWatermark::createWatermarkV11(QString &path, double opticity, int x, int y, int expand, bool topmost)
    {
        WatermarkData data;

        QDBusInterface interface(WLCOM_SERVER, WLCOM_PATH, WLCOM_INTERFACE);
        if (interface.isValid())
        {
            QDBusReply<QString> reply = interface.call("createWatermark", path, opticity, x, y, expand, topmost);
            if (reply.isValid())
            {
                data.handle = reply.value();
                qDebug() << data.handle;
                data.path = path;
                data.opticity = opticity;
                data.x = x;
                data.y = y;
                data.expand = expand;
                data.topmost = topmost;
            }
            else
            {
                qDebug() << reply.error().message();
            }
        }
        else
            qDebug() << "Connect dbus fail";

        return data;
    }

    bool KWatermark::updateWatermarkSP1(WatermarkData &handle, QString &path, double opticity, int x, int y, int expand, bool topmost)
    {
        QDBusInterface interface(KWIN_SERVER, KWIN_PATH, KWIN_INTERFACE);
        if (interface.isValid())
        {
            QDBusReply<void> reply = interface.call("updateWatermarkEx", "", handle.opticity, handle.x, handle.y, handle.expand, handle.topmost);
            if (reply.isValid())
            {
                reply = interface.call("updateWatermarkEx", path, opticity, x, y, expand, topmost);
                if (reply.isValid())
                {
                    handle.path = path;
                    handle.opticity = opticity;
                    handle.x = x;
                    handle.y = y;
                    handle.expand = expand;
                    handle.topmost = topmost;
                    return true;
                }
                else
                {
                    qDebug() << reply.error().message();
                }
            }
        }

        return false;
    }

    bool KWatermark::updateWatermarkV11(WatermarkData &handle, QString &path, double opticity, int x, int y, int expand, bool topmost)
    {
        QDBusInterface interface(WLCOM_SERVER, WLCOM_PATH, WLCOM_INTERFACE);
        if (interface.isValid())
        {
            QDBusReply<bool> reply = interface.call("updateWatermark", handle.handle, path, opticity, x, y, expand, topmost);
            if (reply.isValid() && reply.value())
            {
                handle.path = path;
                handle.opticity = opticity;
                handle.x = x;
                handle.y = y;
                handle.expand = expand;
                handle.topmost = topmost;
                return true;
            }
            else
            {
                qDebug() << reply.error().message();
            }
        }
        else
            qDebug() << "Connect dbus fail";

        return false;
    }

    void KWatermark::destroyWatermarkSP1(WatermarkData &handle)
    {
        QDBusInterface interface(KWIN_SERVER, KWIN_PATH, KWIN_INTERFACE);
        if (interface.isValid())
        {
            QDBusReply<void> reply = interface.call("updateWatermarkEx", "", handle.opticity, handle.x, handle.y, handle.expand, handle.topmost);
            if (!reply.isValid())
            {
                qDebug() << reply.error().message();
            }
        }
        else
        {
            qDebug() << "Connect dbus fail";
        }
    }

    void KWatermark::destroyWatermarkV11(WatermarkData &handle)
    {
        QDBusInterface interface(WLCOM_SERVER, WLCOM_PATH, WLCOM_INTERFACE);
        if (interface.isValid())
        {
            QDBusReply<void> reply = interface.call("destroyWatermark", handle.handle);
            if (!reply.isValid())
            {
                qDebug() << reply.error().message();
            }
        }
        else
        {
            qDebug() << "Connect dbus fail";
        }
    }

}
