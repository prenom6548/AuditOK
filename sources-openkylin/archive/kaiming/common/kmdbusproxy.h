/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMDBUSPROXY_H__
#define __KMDBUSPROXY_H__

#include "KMDbusProxy.h"

using namespace org::freedesktop::Kaiming;

/**
 * @brief dbus proxy
 */
class KMDBusProxy : public SessionHelper_proxy, public DBus::ObjectProxy
{
public:
    KMDBusProxy(DBus::Connection &conn, const std::string &path, const std::string &name)
        : SessionHelper_proxy()
        , DBus::ObjectProxy(conn, path, name.data())
    {
    }
};

#endif // __KMDBUSPROXY_H__
