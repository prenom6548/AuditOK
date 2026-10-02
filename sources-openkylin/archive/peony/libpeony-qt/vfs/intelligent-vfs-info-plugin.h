/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef INTELLIGENTVFSINFOPLUGIN_H
#define INTELLIGENTVFSINFOPLUGIN_H

#include <QObject>
#include "vfs-info-plugin-iface.h"
#include "idm-dbus-manager.h"

using namespace Peony;

class IntelligentVFSInfoPlugin : public VFSInfoPluginIface {
public:
    IntelligentVFSInfoPlugin();

    virtual PluginType pluginType() override {return VFSINFOPlugin;}

    virtual const QString name() override {return "Intelligent vfs info";}
    virtual const QString description() override {return QObject::tr("Default custom vfs info of peony");}
    virtual const QIcon icon() override {return QIcon();}
    virtual void setEnable(bool enable) {}
    virtual bool isEnable() {return true;}

    QString pathScheme() override;

    bool holdInSideBar() override {return true;}

    QString displayName() override;

    QString displayIcon() override;

    CustomErrorHandler *customErrorHandler() override {return nullptr;}

    QStringList fileEnumerator(const QString &path) override;

    std::shared_ptr<FileVFSInfo> queryFile(const QString &path) override;

    VFSError* handerMakeDirectory(const QString &uri, bool &ret) override;

    VFSError* handerFileCreate(const QString &uri, bool &ret) override;

    VFSError* handerFileDelete(const QString &uri, bool &ret) override;

    VFSError* handerFileCopy(const QString &sourceUri, const QString &destUri, bool &ret) override;

    VFSError* handerFileMove(const QString &sourceUri, const QString &destUri, bool &ret) override;

    VFSError* handerFileRename(const QString &sourceUri, const QString &newName) override;

    void handerMonitorDirectory(HanderTransfer *transfer, const QString &uri) override;

    VFSError* handleSetAttribute(const QString &uri, const QString attribute, AttributeType type, QueryFlags flags, void *value, bool &ret) override;

private:
    IDMDBusManager *m_idmDBusManager = nullptr;
};

#endif // INTELLIGENTVFSINFOPLUGIN_H
