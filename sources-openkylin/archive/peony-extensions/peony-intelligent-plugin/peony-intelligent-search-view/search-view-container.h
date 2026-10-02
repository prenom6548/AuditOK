/*
 * Peony Intelligent Spaces Service
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
 * Authors: He JinQin <hejinqin@kylinos.cn>
 *
 */

#ifndef SEARCHVIEWCONTAINER_H
#define SEARCHVIEWCONTAINER_H

#include <QWidget>
#include <peony-qt/directory-view-plugin-iface2.h>
#include <peony-qt/controls/directory-view/directory-view-widget.h>
#include "search-view.h"
#include"searchresultmodel.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
enum extractType{
    keyWord=0,
    spaceName,
    searchPath
};
namespace Peony {

class SearchViewContainer : public DirectoryViewWidget
{
public:
    explicit SearchViewContainer(QWidget *parent = nullptr,QString uri="");
    ~SearchViewContainer();

    const virtual QString viewId() {
        return "Search View";
    }

    const virtual QString getDirectoryUri();

    //selections
    const virtual QStringList getSelections();
    //children
    const virtual QStringList getAllFileUris();

    virtual int getSortType();
    virtual Qt::SortOrder getSortOrder();
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
    QString extractWithRegex(extractType type);
    void applySorting();
    void performSearch();
public Q_SLOTS:
    virtual void bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel);

    //location
    //virtual void open(const QStringList &uris, bool newWindow) {}
    virtual void setDirectoryUri(const QString &uri);
    virtual void beginLocationChange();
    virtual void stopLocationChange();

    virtual void closeDirectoryView() {}

    //selections
    virtual void setSelections(const QStringList &uris);
    virtual void invertSelections();
    virtual void scrollToSelection(const QString &uri) {}

    //clipboard
    //cut items should be drawn differently.
    virtual void setCutFiles(const QStringList &uris) {}

    virtual void setSortType(int sortType);
    virtual void setSortOrder(int sortOrder);

    virtual void editUri(const QString &uri);
    virtual void editUris(const QStringList uris);

    virtual void repaintView();

    virtual void clearIndexWidget() {}

    //zoom
    virtual void setCurrentZoomLevel(int zoomLevel) {}
    void onItemDoubleClicked(const QModelIndex &index);
    void requestShowAISubsystemDialog();
private:
    QString m_currentUri;
    SearchView *m_view = nullptr;
    SearchFileItemModel *m_model;
    SearchFilterProxyModel* m_proxyModel;
    QDBusInterface* m_signalTransInterface;
    bool isAiConditionAvialable;
    bool m_isFileContentOn;
    TreeviewDelegate* m_delegate ;
    QString m_clientId;
    AIConditionType aiConditionType;
    int m_sortType;
    int m_sortOrder;
    QString m_defaultUri;
    std::shared_ptr<Peony::FileWatcher> m_watcher;
    QTimer* m_searchTrigger = nullptr;
    QString m_keyword;
    bool isSeachInFileArea = true;
    bool m_searching = false;
    QString m_spaceName;
    QString m_currentSearchPath;
};

}
#endif // SEARCHVIEWCONTAINER_H
