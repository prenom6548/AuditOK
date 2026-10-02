/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#include "peony-space-view-plugin.h"
#include "space-view-container.h"
#include <QDebug>
#include <QTranslator>
#include <QApplication>
#include<file-utils.h>
using namespace Peony;

PeonyIntelligentSpaceViewPlugin::PeonyIntelligentSpaceViewPlugin(QObject *parent)
{
    //加载翻译
    QTranslator *t = new QTranslator(this);
    t->load(":/peony-space-view_"+QLocale::system().name());
    QApplication::installTranslator(t);
}

int PeonyIntelligentSpaceViewPlugin::priority(const QString &directoryUri)
{
    qDebug() << "=======" << __func__ << directoryUri;
    if(directoryUri.startsWith("idm:///")&&m_currentUri!="idm:///"){
        QString prestr = "idm:///";
        QString uri = FileUtils::urlDecode(directoryUri);
        if (uri.startsWith(prestr)) {
            m_spacename = uri.mid(prestr.length());
        }
        QDBusInterface* m_signalTransInterface = new QDBusInterface("com.peony.idm.service",
                                                         "/org/peony/idm/space/service",
                                                         "org.peony.idm.modelcontroller");
        QDBusReply<QStringList> reply = m_signalTransInterface->call("getFilesInSpaceByName",m_spacename,false);
        QDBusReply<QStringList> names = m_signalTransInterface->call("getAllSpacesName");
        if(reply.value().isEmpty()&& names.value().contains(m_spacename)){
            return 1;
        }
    }
    return -1;
}

DirectoryViewWidget *PeonyIntelligentSpaceViewPlugin::create()
{
    return new SpaceViewContainer(nullptr,m_currentUri,m_spacename);
}
