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

#ifndef SPACEVIEWCONTAINER_H
#define SPACEVIEWCONTAINER_H
#include <QWidget>
#include <peony-qt/directory-view-plugin-iface2.h>
#include <peony-qt/controls/directory-view/directory-view-widget.h>
#include "space-view.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QContextMenuEvent>
namespace Peony {

class SpaceViewContainer : public DirectoryViewWidget
{
public:
    explicit SpaceViewContainer(QWidget *parent = nullptr,QString uri="",QString Spacename="");
    ~SpaceViewContainer();

    const virtual QString viewId() {
        return "Space View";
    }

    const virtual QString getDirectoryUri();

    //selections
    const virtual QStringList getSelections() {
        return QStringList();
    }

    //children
    const virtual QStringList getAllFileUris(){
        return QStringList();
    }

    virtual int getSortType() {
        return 0;
    }
    virtual Qt::SortOrder getSortOrder() {
        return Qt::AscendingOrder;
    }

    //zoom
    virtual int currentZoomLevel() {
        return -1;
    }
    virtual int minimumZoomLevel() {
        return -1;
    }
    virtual int maximumZoomLevel() {
        return -1;
    }
    virtual bool supportZoom() {
        return false;
    }
public Q_SLOTS:
    virtual void bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel);
    //location
    //virtual void open(const QStringList &uris, bool newWindow) {}
    virtual void setDirectoryUri(const QString &uri);
    virtual void beginLocationChange();
    virtual void stopLocationChange();

    virtual void closeDirectoryView() {}

    //selections
    virtual void setSelections(const QStringList &uris) {}
    virtual void invertSelections() {}
    virtual void scrollToSelection(const QString &uri) {}

    //clipboard
    //cut items should be drawn differently.
    virtual void setCutFiles(const QStringList &uris) {}

    virtual void setSortType(int sortType) {}
    virtual void setSortOrder(int sortOrder) {}

    virtual void editUri(const QString &uri) {}
    virtual void editUris(const QStringList uris) {}

    virtual void repaintView() {}

    virtual void clearIndexWidget() {}

    //zoom
    virtual void setCurrentZoomLevel(int zoomLevel) {}
protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
private:
    QString m_currentUri;
    QString m_Spacename;
    Spaceview *m_view = nullptr;
};

}

#endif // SPACEVIEWCONTAINER_H
