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

#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <QTreeView>
#include "directory-view-plugin-iface.h"
#include <controls/directory-view/directoryviewhelper.h>
#include <file-info.h>
#include <searchresultmodel.h>
using namespace Peony;
class SearchView : public QTreeView,public DirectoryViewIface, public DirectoryViewIface2
{
    Q_OBJECT
public:
    enum LabelAlignment{
        AlignVertical=0,
        AlignHorizontal
    };
    explicit SearchView(QWidget *parent = nullptr);
    ~SearchView();
    QModelIndexList getSelectedIndexes();
    void invertSelections() override;
    void bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel) override;
    void setProxy(DirectoryViewProxyIface *proxy) override;
    const QString getDirectoryUri()  override;
    const QStringList getSelections()  override;
    const QStringList getAllFileUris()  override;
    void open(const QStringList &uris, bool newWindow) override;
    void setDirectoryUri(const QString &uri) override;
    void beginLocationChange() override;
    void stopLocationChange() override;
    void closeView() override;
    void setSelections(const QStringList &uris) override;
    void scrollToSelection(const QString &uri) override;
    void setCutFiles(const QStringList &uris) override;
    DirectoryViewProxyIface *getProxy() override;
    int getSortType()  override;
    void setSortType(int sortType) override;
    int getSortOrder()  override;
    void setSortOrder(int sortOrder) override;
    void editUri(const QString &uri) override;
    void editUris(const QStringList uris) override;
    void startDrag(Qt::DropActions flags) override;
    // void bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel) override;
    // void setProxy(DirectoryViewProxyIface *proxy) override ;
    void reUpdateScrollBar();
    const QString viewId() override {
        return "Search View";
    }
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    // //location
    // const  QString getDirectoryUri() override;

    // //selections
    // const  QStringList getSelections() override;

    // //children
    // const  QStringList getAllFileUris() override;

    // void setDirectoryUri(const QString &uri) override;
    // void beginLocationChange() override;
    // void stopLocationChange() override;
    // void closeView() override;

    // //selections
    // void setSelections(const QStringList &uris) override;
    // // virtual void invertSelections() = 0;
    // void scrollToSelection(const QString &uri) override;

    // //clipboard
    // void setCutFiles(const QStringList &uris) override;

    // DirectoryViewProxyIface *getProxy() override;

    // int getSortType() override;
    // void setSortType(int sortType) override;

    // int getSortOrder() override;
    // void setSortOrder(int sortOrder) override;

    // //edit
    // void editUri(const QString &uri) override;
    /*!
     * \brief editUris
     * \param uris
     * \todo
     * implement batch rename
     */

    void doMultiSelect(bool) override;
    bool isEnableMultiSelect() override;

    void setItemsVisible(bool visible) override;
    const int getAllDisplayFileCount() override;
    void slotRename();
    void queryFileinfo(QString uri);
    bool  m_delegate_editing = false;
    bool isDragging();
    void setLabelAlignment(int alignment);
    int getLabelAlignment() const;
public Q_SLOTS:
    void getSelectedIndex(const QItemSelection &selected, const QItemSelection &deselected);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void updateGeometries() override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const ;
private:
    QModelIndexList m_selectedIndexes;
    QList<std::shared_ptr<Peony::FileInfo>> m_selectionInfos;
    QTimer* m_renameTimer;
    bool  m_editValid;
    QModelIndex m_last_index;
    bool m_isSelecting = false;
    QPoint m_dragStartPosition;
    bool m_isDraging = false;
    int m_labelAlignment = 0;
Q_SIGNALS:
     void menuRequest(const QPoint &pos);
};

#endif // SEARCHVIEW_H
