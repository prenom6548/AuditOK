/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: huheng <huheng@kylinos.cn>
 *
 */

#ifndef PEONYVFSMULT_H
#define PEONYVFSMULT_H

#include <QObject>
#include <syslog.h>

#include <peony-qt/vfs-plugin-iface.h>
#include "multiterminal-vfs-dbus-interface.h"

#define DISTFILES "peony-vfs-kylin-multiterminal.json"

class PeonyVFSMult
    : public QObject
    , public Peony::VFSPluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID VFSPluginInterface_iid FILE DISTFILES);
    Q_INTERFACES(Peony::VFSPluginIface);

public:
    explicit PeonyVFSMult(QObject *parent = nullptr);

    virtual QString uriScheme()
    {
        return "mult://";
    }
    virtual bool holdInSideBar()
    {
        return true;
    }
    virtual void initVFS();
    virtual void *parseUriToVFSFile(const QString &uri);
    virtual Peony::CustomErrorHandler *customErrorHandler()
    {
        return nullptr;
    }

    virtual PluginType pluginType()
    {
        return PluginType::VFSPlugin;
    }

    virtual const QString name()
    {
        return QObject::tr("Multiterminal");
    }
    virtual const QString description()
    {
        return QObject::tr("Multiterminal");
    }
    virtual const QIcon icon()
    {
        QIcon icn = QIcon::fromTheme("folder-phone-symbolic");
        if (icn.isNull()) {
            icn = QIcon::fromTheme("folder-phone-symbolic");
        }
        return icn;
    }
    virtual void setEnable(bool enable)
    {
    }
    virtual bool isEnable()
    {
        return true;
    }
};

#endif  // PEONYVFSMULT_H
