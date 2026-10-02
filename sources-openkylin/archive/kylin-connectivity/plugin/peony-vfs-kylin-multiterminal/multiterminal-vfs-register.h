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

#ifndef MULTVFSREGISTER_H
#define MULTVFSREGISTER_H

#include "peony-core_global.h"
#include "vfs-plugin-iface.h"

#include "multiterminal-vfs-dbus-interface.h"

#define VFS_NAME "mult vfs"
namespace Peony
{

class MultVFSInternalPlugin : public VFSPluginIface
{
public:
    MultVFSInternalPlugin() {}

    virtual PluginType pluginType() override
    {
        return VFSPlugin;
    }

    virtual const QString name() override
    {
        return VFS_NAME;
    }
    virtual const QString description() override
    {
        return QObject::tr("Default multiterminal vfs of peony");
    }
    virtual const QIcon icon() override
    {
        return QIcon();
    }
    virtual void setEnable(bool enable) {}
    virtual bool isEnable()
    {
        return true;
    }

    void initVFS() override;
    QString uriScheme() override
    {
        return "mult://";
    }
    bool holdInSideBar() override
    {
        return false;
    }
    void *parseUriToVFSFile(const QString &uri) override;
    Peony::CustomErrorHandler *customErrorHandler() override
    {
        return nullptr;
    }
};

class PEONYCORESHARED_EXPORT MultVFSRegister
{
public:
    static void registMultVFS();

private:
    MultVFSRegister();
};

} // namespace Peony

#endif // MULTVFSREGISTER_H
