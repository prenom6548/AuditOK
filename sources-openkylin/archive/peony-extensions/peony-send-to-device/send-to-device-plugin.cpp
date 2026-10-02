/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include "send-to-device-plugin.h"
#include "file-info.h"

#include <QUrl>
#include <QFile>
#include <QMenu>
#include <QDebug>
#include <QtGlobal>
#include <QProcess>
#include <QTranslator>
#include <QtConcurrent>
#include <QApplication>
#include <file-copy-operation.h>
#include <file-operation-manager.h>
#include <file-info-job.h>
#include <gio/gunixmounts.h>

#include <QRegularExpression>

#ifdef KYLIN_COMMON
#include <ukuisdk/kylin-com4cxx.h>
#endif

#define V10_SP1_EDU "V10SP1-edu"

using namespace Peony;

SendToPlugin::SendToPlugin(QObject *parent) : QObject(parent), mEnable(true)
{
    // translator
    QTranslator *t = new QTranslator(this);
    t->load(":/translations/peony-send-to-device_"+QLocale::system().name());
    QFile file(":/translations/peony-send-to-device_"+QLocale::system().name()+".ts");
    QApplication::installTranslator(t);
}

QList<QAction *> SendToPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
#ifdef KYLIN_COMMON
    if (QString::fromStdString(KDKGetPrjCodeName()) == V10_SP1_EDU) {
        return QList<QAction*>();
    }
#endif

    QList<QAction *> l;

    if (selectionUris.count() <= 0) {
        return l;
    }

    auto info = FileInfo::fromUri(selectionUris.first());

    if (selectionUris.first().startsWith("computer:///")
            || selectionUris.first().startsWith("trash:///")
            || selectionUris.first().startsWith("filesafe:///")
            || info->isVirtual()) {
        return l;
    }

    QStringList selectionTargerUris = selectionUris;
    if (selectionUris.first().startsWith("recent:///")) {
        selectionTargerUris.clear();
        for (auto u : selectionUris){
            selectionTargerUris << FileUtils::getTargetUri(u);
        }
    }

    QAction* action = new DriverAction(selectionTargerUris);
    if (action) {
        action->setIcon(QIcon::fromTheme("phone-symbolic"));
        l << action;
    }

    Q_UNUSED(uri)
    Q_UNUSED(types)

    return l;
}

DriverItem::DriverItem(QString uri, QIcon icon, QString name, QObject* parent) : QAction(parent), mName(name), mIcon(icon), mUri(uri)
{
    setIcon(mIcon);
    setText(name);
}

const QIcon DriverItem::icon()
{
    return mIcon;
}

const QString DriverItem::name()
{
    return mName;
}

const QString DriverItem::uri()
{
    return mUri;
}

DriverAction::DriverAction(const QStringList& uris, QObject *parent) : QAction(parent)
{
    mMenu = new QMenu();

    connect(this, &DriverAction::driverAdded, this, [=] (QString uri, QString name, QString icon) {
        if (!mDrivers.contains(uri)) {
            QString curUri = uri;
            if (curUri.endsWith("/")) {
                curUri.chop(1);
            }

            QString usrName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).section("/", -1, -1);

            if ("file:///data" == curUri
                    || "file:///backup" == curUri
                    || "file:///boot" == curUri
                    || "file:///tmp" == curUri
                    || "file:///var" == curUri
                    || "file:///media/" + usrName + "/SYSBOOT" == curUri
                    || "file:///media/" + usrName + "/sysboot" == curUri
                    || "file:///media/" + usrName + "/data" == curUri
                    || "file:///media/" + usrName + "/DATA" == curUri
                    || curUri.startsWith("burn://")
                    || curUri.startsWith("ftp://")
                    || curUri.startsWith("smb://")
                    || curUri.startsWith("file:///media/" + usrName + "/KYLIN-")
                    || curUri.startsWith("file:///media/" + usrName + "/kylin-")
                    ) {
                return ;
            }

            auto it = new DriverItem (uri, QIcon::fromTheme(icon), name);
            it->connect(it, &QAction::triggered, it, [=] () {
                qDebug() << "======" << __func__ << "uris:" << uris << "it->uri" << it->uri();
                FileCopyOperation* op = new FileCopyOperation(uris, it->uri(), nullptr);
                op->setAutoDelete(true);
                FileOperationManager::getInstance()->startOperation(op);
            });
            mMenu->addAction(it);
            mDrivers[uri] = it;
        }
        showAction();
    });

    GList* unixMounts = g_unix_mounts_get(nullptr);
    for (GList* l = unixMounts; l != nullptr; l = l->next) {
        GUnixMountEntry* entry = (GUnixMountEntry*)l->data;
        const char* mountPath = g_unix_mount_get_mount_path(entry);
        if (mountPath && (g_str_has_prefix(mountPath, "/media/") || g_str_has_prefix(mountPath, "/run/media/"))) {
            const char* fsType = g_unix_mount_get_fs_type(entry);
            if (fsType && strstr(fsType, "fuse.kyfs")) {
                continue;
            }
            
            qDebug() << "unix_mounts found:" << mountPath;
            QString uriStr = QString("file://") + QString::fromUtf8(mountPath);
            const char* devFile = g_unix_mount_get_device_path(entry);
            QString devPath = devFile ? QString::fromUtf8(devFile) : QString();
            QString mountName = QString::fromUtf8(g_path_get_basename(mountPath));


            // 非目标挂载
            if (devPath.isEmpty() || !devPath.contains("/dev/sd")) {
                continue ;
            }
            
            FileInfoJob* fileInfo = new FileInfoJob(uriStr);
            fileInfo->setAutoDelete();
            fileInfo->queryAsync();

            QString devKey = devPath;
            devKey.remove(QRegularExpression("\\d+$"));
            
            DevAggr& aggr = devAggrMap[devKey];
            aggr.pending++; //当前设备待处理的分区+1


            //异步触发执行
            connect(fileInfo, &FileInfoJob::queryAsyncFinished, this, [=](bool success) mutable {

                DevAggr& aggr = devAggrMap[devKey];

                if (!success)   return ;
                std::shared_ptr<FileInfo> info = fileInfo->getInfo();
                if (!info->canExecute() || !info->canWrite())     return ;

                // qWarning() << "[msg]: 可读可写 devPath:" << devPath << ",uriStr:" << uriStr << ",mountName:" << mountName;
                double size = FileUtils::getDeviceSize(devPath.toUtf8().constData());//单位 GB

                aggr.totalGB += size;
                aggr.parts.append(qMakePair(uriStr, mountName));//存储分区信息

                // qWarning() << "[msg] 对于设备：(" << devKey << ")，当前累计分区名：("<< mountName << ") 累加了分区大小：[" << size << "], 当前共：[" << aggr.totalGB << "]";

                if ((--aggr.pending) == 0)//是处理的某设备的最后一个分区
                {
                    // qWarning() << "[msg] 对于设备: (" << devKey << "),所有分区处理完毕，分区累加大小最终：(" << aggr.totalGB << ")";
                    QString icon = (aggr.totalGB > 128) ? "drive-harddisk-usb" : "drive-removable-media-usb";
                    for(const auto& vol : aggr.parts) { //对每个分区单独设置图标(菜单选项)
                        Q_EMIT driverAdded(vol.first, vol.second, icon);
                    }
                    devAggrMap.remove(devKey);
                }

            });

#if 0
            connect(fileInfo, &FileInfoJob::queryAsyncFinished, this, [=](bool success) {
                if (success) {
                    std::shared_ptr<FileInfo> info = fileInfo->getInfo();
                    if (info->canExecute() && info->canWrite()) {
                        QString icon = "drive-removable-media-usb";
                        if (!devPath.isEmpty() && devPath.contains("/dev/sd")) {
                            double size = FileUtils::getDeviceSize(devPath.toUtf8().constData());
                            icon = (size > 128) ? "drive-harddisk-usb" : "drive-removable-media-usb";
                        }
                        Q_EMIT driverAdded(uriStr, mountName, icon);
                    }
                }
            });
#endif
        }
    }
    g_list_free_full(unixMounts, (GDestroyNotify)g_unix_mount_free);

    setMenu(mMenu);
    setText(tr("Send to a removable device"));
    showAction();
}

DriverAction::~DriverAction()
{
    for (auto it = mDrivers.begin(); it != mDrivers.end(); ++it) {
        it.value()->deleteLater();
    }
    if (!mDrivers.isEmpty()) {
        mDrivers.clear();
    }
}

void DriverAction::showAction()
{
    setVisible(mDrivers.size() > 0 ? true : false);
}
