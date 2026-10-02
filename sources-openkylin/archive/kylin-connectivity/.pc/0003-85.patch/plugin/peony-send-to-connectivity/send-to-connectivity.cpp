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
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */


#include "send-to-connectivity.h"
#include <file-info.h>
#include <gio/gio.h>

#include <QAction>
#include <QFileInfo>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QUrl>

using namespace Peony;

SendToKConnectivity::SendToKConnectivity(QObject *parent) : QObject(parent)
{
    QTranslator *t = new QTranslator(this);
    t->load("/usr/share/kylin-connectivity/translations/peony-send-to-connectivity_" + QLocale::system().name());
    QApplication::installTranslator(t);

    m_interface = new QDBusInterface("com.kylin.connectivity.send", "/com/kylin/connectivity",
                                     "com.kylin.connectivity.send", QDBusConnection::sessionBus());
}

QList<QAction *> SendToKConnectivity::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
    Q_UNUSED(uri);
    QList<QAction *> actions;
    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow) {
        if (!selectionUris.isEmpty()) {
            auto info = FileInfo::fromUri(selectionUris.first());
            // special type mountable, return
            if (selectionUris.first().startsWith("computer:///") || selectionUris.first().startsWith("trash:///")
                || selectionUris.first().startsWith("recent:///") || selectionUris.first().startsWith("filesafe:///")
                || info->isVirtual())
                return actions;

            QStringList pathList;
            for (auto path : selectionUris) {
                path.remove("file://");
                QString fileName = QByteArray::fromPercentEncoding(path.toUtf8());
                pathList << fileName;
            }
            if (pathList.size() > 0) {
                if (m_interface->isValid()) {
                    QAction *SendToKConnectivity = new QAction(tr("send to connectivity"));
                    actions << SendToKConnectivity;
                    connect(SendToKConnectivity, &QAction::triggered, [=]() {
                        m_interface->call("setLocalPathList", pathList);
                    });
                }
            }
        }
    }

    return actions;
}
