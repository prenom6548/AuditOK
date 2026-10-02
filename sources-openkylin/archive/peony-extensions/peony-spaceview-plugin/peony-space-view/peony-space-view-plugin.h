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

#ifndef PEONYINTELLIGENTSEARCHVIEWPLUGIN_H
#define PEONYINTELLIGENTSEARCHVIEWPLUGIN_H

#include "peony-space-view-plugin_global.h"
#include <peony-qt/directory-view-plugin-iface2.h>
#include <QObject>

namespace Peony {

class PEONYINTELLIGENTSEARCHVIEW_EXPORT PeonyIntelligentSpaceViewPlugin : public QObject, public DirectoryViewPluginIface2
{
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID DirectoryViewPluginIface2_iid FILE "common.json")
    Q_INTERFACES(Peony::DirectoryViewPluginIface2)
public:
    explicit PeonyIntelligentSpaceViewPlugin(QObject *parent = nullptr);

    PluginType pluginType() {return PluginType::DirectoryViewPlugin2;}

    const QString name() {return QObject::tr("Intelligent Space View");}
    const QString description() {return QObject::tr("Use customized Intelligent Data Manager space views when the spaces file content is empty");}
    const QIcon icon() {return QIcon::fromTheme("kylin-data-manager");}
    void setEnable(bool enable) {}
    bool isEnable() {return true;}

    //view
    QString viewIdentity() {
        return  "Space View";
    }
    QString viewName() {return name();}
    QIcon viewIcon() {return icon();}
    bool supportUri(const QString &uri) {return uri.startsWith("idm:///");}

    int zoom_level_hint() {return -1;}
    int minimumSupportedZoomLevel() {return -1;}
    int maximumSupportedZoomLevel() {return -1;}

    int priority(const QString &directoryUri);
    bool supportZoom() {return false;}

    DirectoryViewWidget *create();

Q_SIGNALS:
private:
    QString m_currentUri;
    QString m_spacename;
};

}

#endif // PEONYINTELLIGENTSEARCHVIEWPLUGIN_H
