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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef LISTVIEW_H
#define LISTVIEW_H
#include <QObject>
#include <QTreeView>
#include "directory-view-plugin-iface.h"
#include "peony-core_global.h"

#include "directory-view-widget.h"
#include "global-settings.h"
#include "directoryviewhelper.h"
#include "checkbox-header.h"
#include <QApplication>
#include <QTimer>
#include <QPropertyAnimation>

namespace Peony {

class FileItemModel;
class FileItemProxyFilterSortModel;

namespace DirectoryView {

/*!
 * \brief The ListView class
 * \todo
 * improve extend selection actions.
 */
class PEONYCORESHARED_EXPORT ListView : public QTreeView, public DirectoryViewIface, public DirectoryViewIface2
{
    friend class ListView2;
    friend class ListViewDelegate;
    Q_OBJECT
public:
    enum LabelAlignment{
        AlignVertical=0,
        AlignHorizontal
    };
    explicit ListView(QWidget *parent = nullptr);
    ~ListView() override;

    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

    const QString viewId() override {
        return tr("List View");
    }

    bool isDragging();

    void bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel) override;
    void setProxy(DirectoryViewProxyIface *proxy) override;

    /*!
     * \brief getProxy
     * \return
     * \deprecated
     */
    DirectoryViewProxyIface *getProxy() override;

    //location
    const QString getDirectoryUri() override;

    //selections
    const QStringList getSelections() override;

    //children
    const QStringList getAllFileUris() override;

    int getCurrentCheckboxColumn();

    /**
     * @brief 定制版本需要显示多选框
     * @return
     */
    bool isEnableMultiSelect();
    bool getDelegateEditFlag();

    /*!
     * \brief checkboxColumn 返回当前模型中复选框列的逻辑索引。
     * 复选框列由 FileItemModel 在 ListView 绑定时追加，并经由 header()->moveSection()
     * 移动到视觉最左侧。多选未启用时该列被隐藏。
     * \return 复选框列的逻辑索引，未启用时返回 -1。
     */
    int checkboxColumn() const;

    void setItemsVisible(bool visible) override;

    int getLabelAlignment() const;

Q_SIGNALS:
    void zoomLevelChangedRequest(bool zoomIn);
    void updateSelectStatus(bool status);
    void quickLookRequested(const QModelIndex &index);

public Q_SLOTS:
    //location
    void open(const QStringList &uris, bool newWindow) override;
    void setDirectoryUri(const QString &uri) override;
    void beginLocationChange() override;
    void stopLocationChange() override;
    void closeView() override;

    //selections
    void setSelections(const QStringList &uris) override;
    void invertSelections() override;
    void scrollToSelection(const QString &uri) override;

    //clipboard
    void setCutFiles(const QStringList &uris) override;

    int getSortType() override;
    void setSortType(int sortType) override;

    int getSortOrder() override;
    void setSortOrder(int sortOrder) override;

    void editUri(const QString &uri) override;
    void editUris(const QStringList uris) override;

    void keyboardSearch(const QString &key) override;

    void resort();
    void reportViewDirectoryChanged();
    void adjustColumnsSize();
    void multiSelect();
    void disableMultiSelect();

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void setSearchKey(const QString &key);

    void saveColumnWidths();
    void restoreColumnWidths();
    void reapplyCheckboxColumnLayout();

    void doMultiSelect(bool isMultiSlelect);

    const int getAllDisplayFileCount();

    void setLabelAlignment(int alignment);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void resizeEvent(QResizeEvent *e) override;

    void updateGeometries() override;
    void reUpdateScrollBar();

    void wheelEvent (QWheelEvent *e) override;

    void scrollContentsBy(int dx, int dy) override;

    void focusInEvent(QFocusEvent *e) override;
    void changeEvent(QEvent *e) override;

    void startDrag(Qt::DropActions flags) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;

    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event) const override;

    bool isRenameSecondClick(const QModelIndex &index, const QPoint &pos) const;
    bool isClickOnCheckBox(const QModelIndex &index) const;

private:
    void updateHeaderCheckBox();

private Q_SLOTS:
    void slotRename();
    void stretchLastColumn();
    void onHeaderCheckBoxStateChanged(Qt::CheckState state);
    void onSelectionChangedForHeader();

private:
    FileItemModel *m_model = nullptr;
    FileItemProxyFilterSortModel *m_proxy_model = nullptr;

    QTimer* m_renameTimer;
    bool  m_editValid;
    bool  m_ctrl_key_pressed = false;
    bool  m_delegate_editing = false;
    bool m_flag = false;

    QRubberBand *m_rubberBand;
    QPoint m_lastPressedLogicPoint;
    bool m_isLeftButtonPressed = false;
    QPoint m_dragStartPos;           // 记录拖拽开始时的鼠标位置（相对于视口），用于数位板拖拽热点计算

    QModelIndex m_last_index;

    DirectoryViewProxyIface *m_proxy = nullptr;

    QPropertyAnimation *scrollAnimation = nullptr;

    QString m_current_uri;
    QString m_version;
    QSize m_last_size;

    const int BOTTOM_STATUS_MARGIN = 200;

    QTimer *m_touch_active_timer = nullptr;
    bool m_multi_select = false;
    bool m_mouse_release_unselect = false;
    bool m_header_section_resized_manually = false;
    int m_labelAlignment = 0;
    bool m_noSelectOnPress = false;
    bool m_skipClickDeselect = false;
    bool m_pressedOnBlank = false;       // 按下处在"非有效区域"（行内空白或完全空白），用于决定框选而非拖拽
    bool m_pressedOnEmptyArea = false;   // 按下处不在任何行上（真正的空白），release 时才清空选中/退出多选
    bool m_boxSelectActive = false;      // 已进入框选拖动；与单击区分，仅此时 ClearAndSelect
    QModelIndex m_toggleClickRowIndex;
    QModelIndex m_multiSelectAnchor;  // Shift 区间选择锚点（proxy model 行索引）
    bool m_shiftRangeHandled = false;  // 标记本次 Shift 区间选择已手动处理

    int m_accumulatedScroll = 0;

    CheckBoxHeader *m_checkbox_header = nullptr;

    // 拖拽优先检测相关
    bool m_pressedOnSelected = false;       // 是否在已选中项上按下
    QPoint m_pressedGlobalPos;              // 按下时的全局位置，用于拖拽距离检测
    bool m_dragStarted = false;             // 标记是否已开始拖拽，避免重复触发
};

//ListView2
class ListView2 : public DirectoryViewWidget
{
    Q_OBJECT
    //internal plugin
public:
    explicit ListView2(QWidget *parent = nullptr);
    ~ListView2();

    const QString viewId() {
        return "List View";
    }

    //location
    const QString getDirectoryUri() {
        return m_view->getDirectoryUri();
    }

    //selections
    const QStringList getSelections() {
        return m_view->getSelections();
    }

    //children
    const QStringList getAllFileUris() {
        return m_view->getAllFileUris();
    }

    int getSortType() {
        return m_view->getSortType();
    }
    Qt::SortOrder getSortOrder() {
        return Qt::SortOrder(m_view->getSortOrder());
    }

    // int currentZoomLevel() {
    //     return m_zoom_level;
    // }
    int currentZoomLevel();


    int minimumZoomLevel() {
        return 0;
    }
    int maximumZoomLevel() {
        QString version = qApp->property("version").toString();
        if (version == "ukui4.0") {
            return 24;
        } else if (version == "ukui3.0") {
             return 20;
        }
        return 24;
    }

    bool supportZoom() {
        return Peony::GlobalSettings::getInstance()->getValue(ZOOM_SLIDER_VISIBLE).toBool();
    }

public Q_SLOTS:
    void bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel);

    //location
    //void open(const QStringList &uris, bool newWindow);
    void setDirectoryUri(const QString &uri) {
        m_need_resize_header = true;
        m_view->setDirectoryUri(uri);
    }
    void beginLocationChange() {
        m_view->beginLocationChange();
    }
    void stopLocationChange() {
        m_view->stopLocationChange();
    }

    void closeDirectoryView() {
        m_view->closeView();
    }

    //selections
    void setSelections(const QStringList &uris) {
        m_view->setSelections(uris);
    }
    void invertSelections() {
        m_view->invertSelections();
    }
    void selectAll() {
        m_view->selectAll();
    }
    void scrollToSelection(const QString &uri) {
        m_view->scrollToSelection(uri);
    }

    //clipboard
    //cut items should be drawn differently.
    void setCutFiles(const QStringList &uris) {
        m_view->setCutFiles(uris);
    }

    void setSortType(int sortType) {
        m_view->setSortType(sortType);
    }
    void setSortOrder(int sortOrder) {
        m_view->setSortOrder(sortOrder);
    }

    void editUri(const QString &uri) {
        m_view->editUri(uri);
    }
    void editUris(const QStringList uris) {
        m_view->editUris(uris);
    }

    void setCurrentZoomLevel(int zoomLevel);

    void clearIndexWidget();

    void multiSelect(){
        m_view->multiSelect();
    }
    void disableMultiSelect(){
        m_view->disableMultiSelect();
    }

    void repaintView();

private:
    ListView *m_view = nullptr;
    FileItemModel *m_model = nullptr;
    FileItemProxyFilterSortModel *m_proxy_model = nullptr;

    int m_zoom_level = 24;
    bool m_need_resize_header;
};

}

}

#endif // LISTVIEW_H
