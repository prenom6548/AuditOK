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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "list-view.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"

#include "list-view-delegate.h"

#include "file-item.h"
#include "file-utils.h"
#include "file-info.h"
#include "list-view-style.h"

#include "global-settings.h"

#include "file-meta-info.h"
#include "search-vfs-uri-parser.h"
#include "clipboard-utils.h"
#include "file-operation-utils.h"
#include <QHeaderView>
#include <QSignalBlocker>

#include <QVBoxLayout>
#include <QMouseEvent>

#include <QScrollBar>

#include <QWheelEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDrag>
#include <QPainter>
#include <QWindow>

#include <QApplication>
#include <QStyleHints>

#include <QToolTip>
#include <QDebug>
#include <QToolTip>

#include <QStyleOptionViewItem>

#include <QStandardPaths>
#include <QMessageBox>

#include <QPainterPath>

#define LISTVIEW_ITEM_BORDER_RADIUS 6

using namespace Peony;
using namespace Peony::DirectoryView;

ListView::ListView(QWidget *parent) : QTreeView(parent)
{ 
    m_touch_active_timer = new QTimer(this);
    m_touch_active_timer->setInterval(2000);
    m_touch_active_timer->setSingleShot(true);

    setProperty("useIconHighlightEffect", 0x0);

    QColor hlColor = palette().color(QPalette::Highlight);
    hlColor.setAlphaF(0.24);
    QBrush semiTransparentBrush(hlColor);
    setProperty("setSelectBrush", semiTransparentBrush);
    setProperty("setBranchSelectBrush", semiTransparentBrush);
    //setFrameShape(QFrame::NoFrame);

    // use scroll per pixel mode for calculate vertical scroll bar range.
    // see reUpdateScrollBar()
    setVerticalScrollMode(ScrollPerPixel);
    m_version = qApp->property("version").toString();
    if (m_version == "ukui3.0") {
        setStyle(Peony::DirectoryView::ListViewStyle::getStyle());
    } else {
        setFrameShape(QFrame::NoFrame);
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } 

    setAutoScroll(true);
    setAutoScrollMargin(100);
    auto cornerWidget = new QWidget;
    cornerWidget->setObjectName("_listview_corner");
    cornerWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    cornerWidget->setBackgroundRole(QPalette::Base);
    cornerWidget->setAutoFillBackground(true);
    setCornerWidget(cornerWidget);

    installEventFilter(horizontalScrollBar());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setSelectionBehavior(QTreeView::SelectRows);

    setAlternatingRowColors(true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    // 创建自定义表头复选框
    m_checkbox_header = new CheckBoxHeader(Qt::Horizontal, this);
    setHeader(m_checkbox_header);
    connect(m_checkbox_header, &CheckBoxHeader::checkStateChanged,
            this, &ListView::onHeaderCheckBoxStateChanged);
    //setItemDelegate(new ListViewDelegate(this));

    header()->setSectionResizeMode(QHeaderView::Interactive);
    header()->setSectionsMovable(true);
    header()->setStretchLastSection(false);
    header()->setMinimumSectionSize(130);
    header()->setTextElideMode(Qt::ElideRight);

    connect(header(), &QHeaderView::sectionResized, this, [=]{
        m_header_section_resized_manually = true;
        // 保存列宽设置
        QTimer::singleShot(200, this, [=](){
            saveColumnWidths();
        });
        stretchLastColumn();
    });

    setExpandsOnDoubleClick(false);
    // setItemsExpandable(false);      // 禁用展开箭头
    // setRootIsDecorated(false);      // 禁用根节点装饰
    setSortingEnabled(true);

    setEditTriggers(QTreeView::NoEditTriggers);
    setDragEnabled(true);
    setDragDropMode(QTreeView::DragDrop);
    setSelectionMode(QTreeView::ExtendedSelection);

    m_renameTimer = new QTimer(this);
    m_renameTimer->setInterval(3000);
    m_renameTimer->setSingleShot(true);
    m_editValid = false;

    //use this property to fix bug 44314 and 33558
    //bug#42244 need to find and fix update fail issue
    setUniformRowHeights(true);
    setIconSize(QSize(40, 40));
    setMouseTracking(true);//追踪鼠标

    m_rubberBand = new QRubberBand(QRubberBand::Shape::Rectangle, this->viewport());

    m_labelAlignment = GlobalSettings::getInstance()->getValue(LABLE_ALIGNMENT).toInt();

    //FIXME: do not create proxy in view itself.
    ListViewDelegate *delegate = new ListViewDelegate(this);
    setItemDelegate(delegate);
    connect(delegate, &ListViewDelegate::isEditing, this, [=](const bool &editing)
    {
        m_delegate_editing = editing;
    });

    //fix head indication sort type and order not change in preference file issue, releated to bug#92525,
    connect(header(), &QHeaderView::sortIndicatorChanged, this, [=](int logicalIndex, Qt::SortOrder order)
    {
        m_proxy_model->manualUpdateExpectedSortInfo(logicalIndex, order);
        //qDebug() << "sortIndicatorChanged:" <<logicalIndex<<order;
        if (this->topLevelWidget()->objectName() == "_peony_mainwindow") {
            if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
                Peony::GlobalSettings::getInstance()->setValue(SORT_COLUMN, logicalIndex);
                Peony::GlobalSettings::getInstance()->setValue(SORT_ORDER, order);
            } else {
                auto metaInfo = FileMetaInfo::fromUri(m_current_uri);
                if (!metaInfo) {
                    qWarning()<<"no meta info"<<m_current_uri;
                } else {
                    metaInfo->setMetaInfoInt(SORT_COLUMN, logicalIndex);
                    metaInfo->setMetaInfoInt(SORT_ORDER, order);
                }
            }
        }
    });

    setViewportMargins(0, 0, 0, 0);

    scrollAnimation = new QPropertyAnimation(verticalScrollBar(), "value", this);
    scrollAnimation->setDuration(200); // 动画持续时间(毫秒)
    scrollAnimation->setEasingCurve(QEasingCurve::OutQuad); // 使用缓动曲线
}

ListView::~ListView()
{
    if (scrollAnimation) {
        scrollAnimation->deleteLater();
        scrollAnimation = nullptr;
    }
}

void ListView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    // Note: due to we rewrite the calculation of view scroll area based on current process,
    // we could not use QTreeView::scrollTo in some cases because it still based on old process
    // and will conflict with new calculation.
    Q_UNUSED(index)
    Q_UNUSED(hint)
    reUpdateScrollBar();
}

bool ListView::isDragging()
{
    return state() == QAbstractItemView::DraggingState;
}

void ListView::bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel)
{
    if (topLevelWidget()->objectName() != "_peony_mainwindow") {
        setFrameShape(QFrame::NoFrame);
    } else {
        // note: 如果在构造函数中判断topLevelWidget的objectName，不会找到mainwindow，因为此时还没有设置parent为directoryViewContainer
        connect(header(), &QHeaderView::sectionClicked, this, [=](){
            //update sort policy
            auto settings = GlobalSettings::getInstance();
            if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
                settings->setValue(SORT_COLUMN, getSortType());
                settings->setValue(SORT_ORDER, getSortOrder());
            } else {
                auto metaInfo = FileMetaInfo::fromUri(getDirectoryUri());
                if (metaInfo) {
                    metaInfo->setMetaInfoVariant(SORT_COLUMN, getSortType());
                    metaInfo->setMetaInfoVariant(SORT_ORDER, getSortOrder());
                } else {
                    qCritical()<<"failed to set meta info"<<getDirectoryUri();
                }
            }
        });
    }

    if (!sourceModel || !proxyModel)
        return;
    m_model = sourceModel;
    m_proxy_model = proxyModel;

    auto proxyModelSelectionModelHint = proxyModel->getSelectionModeHint();
    if (proxyModelSelectionModelHint != NoSelection) {
        setSelectionMode(proxyModelSelectionModelHint);
    }

    connect(proxyModel, &FileItemProxyFilterSortModel::setSelectionModeChanged, this, [=]{
        auto proxyModelSelectionModelHint = proxyModel->getSelectionModeHint();
        if (proxyModelSelectionModelHint != NoSelection) {
            setSelectionMode(proxyModelSelectionModelHint);
        }
    });

    m_proxy_model->setSourceModel(m_model);
    setModel(proxyModel);
    m_proxy_model->setSelectionModel(selectionModel());

    auto scheduleHeaderCheckBoxUpdate = [this]() {
        QTimer::singleShot(0, this, [this]() {
            updateHeaderCheckBox();
        });
    };
    connect(proxyModel, &QAbstractItemModel::rowsInserted, this, scheduleHeaderCheckBoxUpdate);
    connect(proxyModel, &QAbstractItemModel::rowsRemoved, this, scheduleHeaderCheckBoxUpdate);
    connect(proxyModel, &QAbstractItemModel::modelReset, this, scheduleHeaderCheckBoxUpdate);
    connect(proxyModel, &QAbstractItemModel::layoutChanged, this, scheduleHeaderCheckBoxUpdate);

    restoreColumnWidths();
    //adjust columns layout.
    adjustColumnsSize();

    //edit trigger
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selection, const QItemSelection &deselection) {
        //qDebug()<<"list view selection changed"<<m_delegate_editing;
        //continue to fix bug#89540，98951
        auto currentSelections = selection.indexes();
        if (m_delegate_editing){
            //fix bug#194642, list view can not trigger rename issue
            if (! currentSelections.isEmpty()) {
               m_last_index = currentSelections.first();
            }
            return;
        }

        for (auto index : deselection.indexes()) {
            this->setIndexWidget(index, nullptr);
        }

        //rename trigger
        if (!currentSelections.isEmpty()) {
            int first_index_row = currentSelections.first().row();
            bool all_index_in_same_row = true;
            for (auto index : currentSelections) {
                if (first_index_row != index.row()) {
                    all_index_in_same_row = false;
                    break;
                }
            }
            if (all_index_in_same_row) {
                if(m_last_index.row() != currentSelections.first().row())
                {
                    m_editValid = false;
                }
                m_last_index = currentSelections.first();
            }
        } else {
            m_last_index = QModelIndex();
            m_editValid = false;
        }

        // 更新表头复选框状态
        updateHeaderCheckBox();
    });
}

void ListView::keyPressEvent(QKeyEvent *e)
{
    QTreeView::keyPressEvent(e);

    if(e->key() == Qt::Key_Space){
        Q_EMIT QTreeView::activated(currentIndex());/* 与按下enter键效果一样 */
    }

    if(e->key() == Qt::Key_F10 && e->modifiers() == Qt::ShiftModifier) {
        if (getSelections().count() == 1 ) {
            auto currentIndex = selectionModel()->selection().indexes();
            QPoint menuPos = this->visualRect(currentIndex.first()).center();
            Q_EMIT customContextMenuRequested(menuPos);
            return;
        }
    }
   //if(e->key() == Qt::Key_Down||e->key() == Qt::Key_Up)
    //{
     //   QStringList selections = getSelections();
      //  if(selections.size() == 1)
       //     this->scrollToSelection(selections.at(0));
  //  }
    switch (e->key()) {
    case Qt::Key_Control:
        m_ctrl_key_pressed = true;
        break;
    case Qt::Key_Up: {
        if (!selectedIndexes().isEmpty()) {
            QTreeView::scrollTo(currentIndex());
        }
        break;
    }
    case Qt::Key_Down: {
        if (!selectedIndexes().isEmpty()) {
            auto index = selectedIndexes().first();
            if (index.row() + 1 == model()->rowCount()) {
                verticalScrollBar()->setValue(qMin(verticalScrollBar()->value() + iconSize().height(), verticalScrollBar()->maximum()));
            } else {
                QTreeView::scrollTo(currentIndex());
            }
        }
        break;
    }
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        //fix bug#160799, can not update scrollBar to show selected file issue
        if (!selectedIndexes().isEmpty()) {
            QTreeView::scrollTo(currentIndex());
        }
        break;
    }
    default:
        break;
    }
}

void ListView::keyReleaseEvent(QKeyEvent *e)
{
    QTreeView::keyReleaseEvent(e);
    if (e->key() == Qt::Key_Control)
        m_ctrl_key_pressed = false;
}

void ListView::mousePressEvent(QMouseEvent *e)
{ 
    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    if (singleClicked) {
        if (!m_touch_active_timer->isActive()) {
            m_touch_active_timer->start(1100);
        }
    }

    if (e->button() == Qt::RightButton) {
        if (m_delegate_editing) {
            return;
        }
        if (this->state() == QTreeView::EditingState) {
            if (indexWidget(indexAt(e->pos())))
                return;
        }
        if (isEnableMultiSelect() && !indexAt(e->pos()).isValid()) {
            clearSelection();
            disableMultiSelect();
            updateHeaderCheckBox();
        }
        Q_EMIT customContextMenuRequested(e->pos());
        m_rubberBand->hide();
        m_lastPressedLogicPoint = QPoint(-1, -1);
        return;
    }

    m_rubberBand->hide();
    if (e->button() == Qt::LeftButton) {
        m_lastPressedLogicPoint = e->pos();
    } else {
        m_lastPressedLogicPoint = QPoint(-1, -1);
    }

    auto index = indexAt(e->pos());
    const QModelIndex rowIndex = index.isValid() ? index.sibling(index.row(), 0) : QModelIndex();
    bool isIndexSelected = rowIndex.isValid()
            && (selectionModel()->isSelected(rowIndex) || selectionModel()->isSelected(index));

    // 判断点击是否在有效区域内（非边缘/非空白）
    bool pressedInValidArea = false;
    if (rowIndex.isValid()) {
        auto visualRect = this->visualRect(index);
        auto sizeHint = itemDelegate()->sizeHint(viewOptions(), index);
        auto validRect = QRect(visualRect.topLeft(), sizeHint);
        pressedInValidArea = validRect.contains(e->pos());
    }

    m_boxSelectActive = false;
    m_pressedOnBlank = (e->button() == Qt::LeftButton && !index.isValid());

    // 记录是否在已选中项的有效区域内按下，用于拖拽优先检测
    // 只有在有效区域内按下才触发拖拽优先，边缘/空白区域仍允许框选
    m_pressedOnSelected = (e->button() == Qt::LeftButton && isIndexSelected && pressedInValidArea);
    if (e->button() == Qt::LeftButton) {
        m_pressedGlobalPos = e->globalPos();
    }

    if (!isEnableMultiSelect() && e->button() == Qt::LeftButton && isRenameSecondClick(index, e->pos())) {
        m_editValid = true;
        slotRename();
        e->accept();
        return;
    }

    if (isEnableMultiSelect() && rowIndex.isValid()) {
        m_mouse_release_unselect = isIndexSelected;
    } else {
        m_mouse_release_unselect = false;
    }

    if (e->button() == Qt::LeftButton && isIndexSelected
            && ((e->modifiers() & Qt::ControlModifier) || isEnableMultiSelect())) {
        m_noSelectOnPress = true;
        m_toggleClickRowIndex = rowIndex;
    } else {
        m_noSelectOnPress = false;
        m_toggleClickRowIndex = QModelIndex();
    }

    const bool wasSelectedBeforePress = isIndexSelected;
    m_editValid = true;
    QTreeView::mousePressEvent(e);

    // 多选单击未选中项：追加选中，避免 QTreeView 清空其它已选项
    // 但点击复选框区域时，交给editorEvent在mouseRelease中处理
    if (isEnableMultiSelect() && e->button() == Qt::LeftButton && rowIndex.isValid()
            && !wasSelectedBeforePress && !(e->modifiers() & Qt::ControlModifier)
            && !isClickOnCheckBox(rowIndex, e->pos())) {
        selectionModel()->select(rowIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    if (m_mouse_release_unselect) {
        //this->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }

    // 多选模式下不执行clearSelection逻辑，保持已有选中项
    if (!isEnableMultiSelect()) {
        auto visualRect = this->visualRect(index);
        auto sizeHint = itemDelegate()->sizeHint(viewOptions(), index);
        auto validRect = QRect(visualRect.topLeft(), sizeHint);
        if (!validRect.contains(e->pos()) && e->modifiers() == Qt::NoModifier && e->button() == Qt::LeftButton) {
            if (isIndexSelected) {
                clearSelection();
                setCurrentIndex(index);
            }
            setState(QAbstractItemView::DragSelectingState);
        }
    }
    if (isEnableMultiSelect()) {
        updateHeaderCheckBox();
    }

    if (!isEnableMultiSelect()) {
        if(!m_renameTimer->isActive())
        {
            m_renameTimer->start();
            m_editValid = false;
        }
        else
        {
            bool all_index_in_same_row = true;
            if (!this->selectedIndexes().isEmpty()) {
                int first_index_row = this->selectedIndexes().first().row();
                for (auto index : this->selectedIndexes()) {
                    if (first_index_row != index.row()) {
                        all_index_in_same_row = false;
                        break;
                    }
                }
            }
            if (m_editValid && all_index_in_same_row && isRenameSecondClick(indexAt(e->pos()), e->pos()))
            {
                slotRename();
            } else
            {
                m_editValid = false;
            }
        }
    }
}

void ListView::mouseReleaseEvent(QMouseEvent *e)
{
    const bool wasBoxSelect = m_boxSelectActive;
    const bool deferClickDeselect = m_noSelectOnPress && isEnableMultiSelect();
    const QModelIndex deselectRowIndex = deferClickDeselect ? m_toggleClickRowIndex : QModelIndex();

    QTreeView::mouseReleaseEvent(e);
    m_noSelectOnPress = false;
    m_toggleClickRowIndex = QModelIndex();

    if (deferClickDeselect && deselectRowIndex.isValid()) {
        // 立即取消选中，不使用延迟，避免时序问题
        if (!m_skipClickDeselect) {
            if (selectionModel()->isSelected(deselectRowIndex)) {
                selectionModel()->select(deselectRowIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                updateHeaderCheckBox();
            }
        }
    }
    m_rubberBand->hide();
    m_lastPressedLogicPoint = QPoint(-1, -1);
    m_isLeftButtonPressed = false;

    if (m_pressedOnBlank && !wasBoxSelect && e->button() == Qt::LeftButton) {
        if (isEnableMultiSelect()) {
            clearSelection();
            disableMultiSelect();
            updateHeaderCheckBox();
        } else {
            clearSelection();
        }
    }
    m_pressedOnBlank = false;
    m_boxSelectActive = false;
    m_pressedOnSelected = false;  // 重置拖拽优先检测状态
}

void ListView::mouseMoveEvent(QMouseEvent *e)
{
    QModelIndex itemIndex = indexAt(e->pos());
    if (!itemIndex.isValid()) {
        if (QToolTip::isVisible()) {
             QToolTip::hideText();
         }
    }

    QTreeView::mouseMoveEvent(e);

    // 当开始拖拽或框选时，清除 m_noSelectOnPress，避免 mouseRelease 执行取消选中逻辑
    if (state() == DraggingState || m_boxSelectActive) {
        m_noSelectOnPress = false;
    }

    // fix #115124, drag selection can not trigger auto scroll in view.
    if (e->buttons() & Qt::LeftButton && !this->viewport()->rect().adjusted(0, autoScrollMargin(), 0, -autoScrollMargin()).contains(e->pos())) {
        doAutoScroll();
    }

    if (e->buttons() & Qt::LeftButton && m_lastPressedLogicPoint.x() >= 0 && m_lastPressedLogicPoint.y() >= 0) {
        const QRect bandRect = QRect(e->pos(), m_lastPressedLogicPoint).normalized();

        // 拖拽优先检测：如果在已选中项上按下，先检测拖拽意图
        if (m_pressedOnSelected) {
            const int dragDistance = (e->globalPos() - m_pressedGlobalPos).manhattanLength();
            // 移动距离达到拖拽阈值，禁用框选，让 Qt 触发拖拽
            if (dragDistance >= QApplication::startDragDistance()) {
                m_rubberBand->hide();
                m_lastPressedLogicPoint = QPoint(-1, -1);
                m_boxSelectActive = false;
                return;
            }
            // 移动距离不够，暂不触发框选（等待拖拽或释放）
            return;
        }

        m_logicRect = bandRect;
        if (bandRect.width() > 2 || bandRect.height() > 2) {
            m_boxSelectActive = true;
            if (state() != QAbstractItemView::DragSelectingState) {
                setState(QAbstractItemView::DragSelectingState);
            }
        }
        if (!m_rubberBand->isVisible()) {
            m_rubberBand->show();
        }
        m_rubberBand->setGeometry(bandRect);
    } else {
        m_rubberBand->hide();
        m_lastPressedLogicPoint = QPoint(-1, -1);
    }

    // fix #115124, drag selection can not trigger auto scroll in view.
    if (e->buttons() & Qt::LeftButton && !this->viewport()->rect().adjusted(0, autoScrollMargin(), 0, -autoScrollMargin()).contains(e->pos())) {
        doAutoScroll();
    }

}

void ListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_editValid = false;
    m_skipClickDeselect = true;
    /**
     * @bug #250731: [File Manager] Right clicking on the same folder several times in the file manager will take you to the folder
     *
     * Prevent double-click events from triggering on right-click
     * Only double left clicks will be processed
     *
     * @author Renyg
     * @date 2024-08-12
     */
    if (event->button() == Qt::RightButton) {
        m_skipClickDeselect = false;
        return;
    }

    if (event->button() == Qt::LeftButton && isEnableMultiSelect()) {
        const QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            const QModelIndex rowIndex = index.sibling(index.row(), 0);
            const QString uri = m_proxy_model->data(rowIndex, FileItemModel::UriRole).toString();
            if (!uri.isEmpty()) {
                setSelections(QStringList() << uri);
                Q_EMIT activated(rowIndex);
            }
            const int guardMs = qApp->styleHints()->mouseDoubleClickInterval() + 50;
            QTimer::singleShot(guardMs, this, [this]() {
                m_skipClickDeselect = false;
            });
            event->accept();
            return;
        }
    }

    QTreeView::mouseDoubleClickEvent(event);
    m_skipClickDeselect = false;
}

void ListView::dragEnterEvent(QDragEnterEvent *e)
{
    m_editValid = false;
    qDebug()<<"dragEnterEvent()";
    //QTreeView::dragEnterEvent(e);
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    bool isdropbyukuiidm = (e->mimeData()->hasFormat("text/ukui_idm") && e->mimeData()->data("text/ukui_idm") == "ukui_idm") ? true : false;
    if(isdropbyukuiidm){
        action = Qt::CopyAction;
     }

    qDebug()<<"dragEnterEvent()" <<action <<m_ctrl_key_pressed;
    // 菜单应用拖拽适配
    if (e->mimeData()->hasUrls() || e->mimeData()->hasFormat("url")) {
        if (FileUtils::containsStandardPath(e->mimeData()->urls())) {
            e->ignore();
            if (this == e->source()) {
                clearSelection();
            }
            return;
        }
        e->setDropAction(action);
        e->accept();
    }
}

void ListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;

    bool isdropbyukuiidm = (e->mimeData()->hasFormat("text/ukui_idm") && e->mimeData()->data("text/ukui_idm") == "ukui_idm") ? true : false;
     if(isdropbyukuiidm)
     {
         action = Qt::CopyAction;
     }

    bool dropDesktopItem = e->mimeData()->hasFormat("text/ukui_desktop") ? true : false;

    //qDebug()<<"list view dragMoveEvent()" <<action <<m_ctrl_key_pressed;
    auto index = indexAt(e->pos());
    if (index.isValid() && index != m_last_index) {
        QHoverEvent he(QHoverEvent::HoverMove, e->posF(), e->posF());
        viewportEvent(&he);
    } else {
        QHoverEvent he(QHoverEvent::HoverLeave, e->posF(), e->posF());
        viewportEvent(&he);
    }

    if ((this == e->source() || !model()->flags(QModelIndex()).testFlag(Qt::ItemIsDropEnabled)) && !isdropbyukuiidm && (!dropDesktopItem)) {
        return QTreeView::dragMoveEvent(e);
    }
    e->setDropAction(action);
    e->accept();
}

void ListView::dropEvent(QDropEvent *e)
{
    // do not comment this code.
    if (e->source() == this) {
        // only handle the drop event on item.
        switch (dropIndicatorPosition()) {
        case QAbstractItemView::DropIndicatorPosition::OnItem: {
            break;
        }
        case QAbstractItemView::DropIndicatorPosition::OnViewport: {
            if (e->keyboardModifiers() & Qt::ControlModifier) {
                break;
            } else {
                return;
            }
        }
        default:
            return;
        }
    }
//    QTreeView::dropEvent(e);

    m_last_index = QModelIndex();
    //m_edit_trigger_timer.stop();
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;

    bool isdropbyukuiidm = (e->mimeData()->hasFormat("text/ukui_idm") && e->mimeData()->data("text/ukui_idm") == "ukui_idm") ? true : false;
    if(isdropbyukuiidm)
    {
        action = Qt::CopyAction;
    }

    e->setDropAction(action);
    if (e->keyboardModifiers() & Qt::ShiftModifier) {
        action = Qt::TargetMoveAction;
    }

    //Do not allow dragging files to file manager when searching
    //related to bug#107063,118004
    if (m_current_uri.startsWith("search://") || m_current_uri.startsWith("favorite://")) {
        QMessageBox::warning(this, tr("warn"), tr("This operation is not supported."));
        return;
    }

    auto proxy_index = indexAt(e->pos());
    auto index = m_proxy_model->mapToSource(proxy_index);
    qDebug()<<"dropEvent" <<action <<m_ctrl_key_pressed <<indexAt(e->pos()).isValid();

    QString username = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).split("/").last();
    QString boxpath = "file://"+QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.box";
    QString oldboxpath = "file://box/"+username;

    if(m_current_uri == boxpath || m_current_uri == oldboxpath || m_current_uri == "filesafe:///"){
        return;
    }
    //move in current path, do nothing
    if (e->source() == this)
    {

        if (indexAt(e->pos()).isValid())
        {
            auto uri = m_proxy_model->itemFromIndex(proxy_index)->uri();
            if(!e->mimeData()->urls().contains(uri))
                m_model->dropMimeData(e->mimeData(), action, 0, 0, index);
        } else {
            if (m_ctrl_key_pressed) {
                m_model->dropMimeData(e->mimeData(), Qt::CopyAction, 0, 0, QModelIndex());
            }
        }
        return;
    }

    if (e->mimeData()->hasFormat("url")) {
        QString tmp = e->mimeData()->data("url");
        tmp.replace("app://", "file://");
        FileOperationUtils::copyDesktopFile(QStringList() << tmp, m_current_uri, true);
        return;
    }

    // Use the full visual rect of the target index instead of the smaller delegate size rect.
    // This avoids false negatives when dropping from external processes onto existing rows
    // (e.g., when the pointer is within the row but outside the delegate's size area).
    auto validRect = visualRect(proxy_index);
    if (!validRect.contains(e->pos())) {
        //拖拽到在空白处，就移动到当前目录下
        m_model->dropMimeData(e->mimeData(), action, 0, 0, QModelIndex());
        return;
    }
    m_model->dropMimeData(e->mimeData(), action, 0, 0, index);
}

void ListView::resizeEvent(QResizeEvent *e)
{
    QTreeView::resizeEvent(e);

    if (m_last_size != size()) {
        m_last_size = size();
        adjustColumnsSize();
    }
    if (state() == QTreeView::EditingState && qApp->property("tabletMode").toBool()) {
        scrollTo(currentIndex());
    }
}

/*!
 * \brief ListView::reUpdateScrollBar
 * \details
 * tree view use QTreeViewPrivate::updateScrollBars() for reset scrollbar range.
 * there are 3 parts for that method called.
 *
 * 1. QTreeView::scrollTo()
 * 2. QTreeView::dataChanged()
 * 3. QTreeView::updateGeometries()
 *
 * we have to override all of them to make sure that our custom scrollbar range
 * set correctly.
 */
void ListView::reUpdateScrollBar()
{
    if (!model())
        return;

    if (model()->rowCount() == 0) {
        return;
    }

    int rowCount = model()->rowCount();
    auto index = model()->index(0, 0);
    int totalHeight = sizeHintForIndex(index).height()*rowCount;

    int currentScrollBarValue = verticalScrollBar()->value();
    verticalScrollBar()->setSingleStep(iconSize().height());
    verticalScrollBar()->setPageStep(viewport()->height() - header()->height());
    verticalScrollBar()->setRange(0, totalHeight + header()->height() + 100 - viewport()->height());
    verticalScrollBar()->setValue(currentScrollBarValue);
}

void ListView::updateGeometries()
{
    if (m_flag)
        return;
    setUpdatesEnabled(false);
    QTreeView::updateGeometries();
    reUpdateScrollBar();
    setUpdatesEnabled(true);
}

void ListView::wheelEvent(QWheelEvent *e)
{
    if ((e->modifiers() & Qt::ControlModifier)) {
        zoomLevelChangedRequest(e->delta() > 0);
        e->accept();
        return;
    }

    //垂直滚动动画
    bool isVerticalScroll = qAbs(e->angleDelta().y()) > qAbs(e->angleDelta().x());
    if (isVerticalScroll) {
        if (e->source() != Qt::MouseEventNotSynthesized) {
            //手指滑动
            // step = (e->angleDelta().y() / e->DefaultDeltasPerStep ) * this->visualRect(index).height() * 1.5;
            QTreeView::wheelEvent(e);
        } else {
            //鼠标滚轮
            QScrollBar *vBar = verticalScrollBar();
            QModelIndex index = this->model()->index(0, 0, this->rootIndex());
            int step, targetValue;

            // 获取滚轮角度变化，可在控制面板中设置，默认listview为3行
            step = (e->angleDelta().y() / e->DefaultDeltasPerStep ) * this->visualRect(index).height() * 3;

            if (scrollAnimation->state() == QPropertyAnimation::Running) {
                //上次滚动动画未执行完，累加动画执行的终点
                scrollAnimation->stop();
                targetValue = m_accumulatedScroll - step;
                targetValue = qMax(vBar->minimum(), qMin(targetValue, vBar->maximum()));
            } else {
                // 计算目标值，确保在滚动条范围内
                targetValue = vBar->value() - step;
                targetValue = qMax(vBar->minimum(), qMin(targetValue, vBar->maximum()));
            }
            m_accumulatedScroll = targetValue; //记录当前需滑动的终点位置

            // 配置动画
            scrollAnimation->setStartValue(vBar->value());
            scrollAnimation->setEndValue(targetValue);

            // 启动动画
            scrollAnimation->start();
            e->accept();
        }
    } else {
        QTreeView::wheelEvent(e);
    }
}

void ListView::focusInEvent(QFocusEvent *e)
{
    QTreeView::focusInEvent(e);
    if (e->reason() == Qt::TabFocus) {
        if (selectedIndexes().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
        } else {
            QTreeView::scrollTo(selectedIndexes().first(), QTreeView::EnsureVisible);
            reUpdateScrollBar();
            auto selections = getSelections();
            clearSelection();
            //use uri rather than index, to fix crash bug#68788, 96145
            QTimer::singleShot(100, this, [=](){
                setSelections(selections);
            });
        }
    }
}

void ListView::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        QColor hlColor = palette().color(QPalette::Highlight);
        hlColor.setAlphaF(0.24);
        QBrush semiTransparentBrush(hlColor);
        setProperty("setSelectBrush", semiTransparentBrush);
        setProperty("setBranchHoverBrush", semiTransparentBrush);
        setProperty("setBranchSelectBrush", semiTransparentBrush);
    }
    QTreeView::changeEvent(e);
}

void ListView::startDrag(Qt::DropActions flags)
{
    m_rubberBand->hide();
    m_lastPressedLogicPoint = QPoint(-1, -1);
    auto indexes = selectedIndexes();
    if (indexes.count() > 0) {
        auto pos = mapFromGlobal(QCursor::pos());
        qreal scale = 1.0;
        QWidget *window = this->window();
        if (window) {
            auto windowHandle = window->windowHandle();
            if (windowHandle) {
                scale = windowHandle->devicePixelRatio();
            }
        }

        auto drag = new QDrag(this);
        if(m_current_uri.startsWith("search://")){
            QMimeData* data = model()->mimeData(indexes);
            QVariant isSearchData = QVariant(true);
            data->setData("peony-qt/is-search", isSearchData.toByteArray());
            drag->setMimeData(data);
        }else if(m_current_uri.startsWith("smb://")){
            QMimeData* data = new QMimeData();
            QList<QUrl> urls;
            QStringList encodedUris;
            QByteArray target;
            for (auto index : indexes) {
                if (index.column() == 0)
                {

                    QString uri = index.data(FileItemModel::UriRole).toString();
                    if(uri.startsWith("smb://"))
                    {
                        uri = "file://" + FileUtils::getTargetPath(uri);
                    }
                    auto encodeUrl = Peony::FileUtils::urlEncode(uri);
                    urls << QString(encodeUrl);
                    target.append("\n");
                    target.append(encodeUrl);
                    encodedUris << QString(encodeUrl);
                }
            }
            data->setUrls(urls);
            QString string = encodedUris.join(" ");
            data->setData("peony-qt/encoded-uris", string.toUtf8());
            data->setText(string);
            data->setData("x-special/gnome-copied-files", target);
            drag->setMimeData(data);
        }
        else{
            drag->setMimeData(model()->mimeData(indexes));
        }

        int num = indexes.count() / 4;
        if (num > 50) {
            QRect pixmapRect = QRect(100, 100, 400, 400);
            QPixmap pixmap(pixmapRect.size() * scale);
            pixmap.fill(Qt::transparent);
            pixmap.setDevicePixelRatio(scale);
            QPainter painter(&pixmap);
            quint64 count = 0;

            painter.save();
            QRect iconRect = pixmapRect;
            iconRect.setSize(QSize(139, 139));
            for (auto index : indexes) {
               if (count > 50) {
                   break;
               }
               count++;
               iconRect.moveTo(iconRect.x()+1, iconRect.y()+1);
               QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
               if (!icon.isNull()) {
                   painter.save();
                   painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                   painter.drawPixmap(QPoint(100 + count, 100 + count), icon.pixmap(139, 139));
                   painter.restore();
              }
            }
            QFont font = qApp->font();
            font.setPointSize(10);
            QFontMetrics metrics(font);
            QString text = num > 999 ? "..." : QString::number(num);
            int height = metrics.width(text);
            int width = metrics.height();

            int diameter = std::max(height, width);
            int radius = diameter / 2;
            QRectF textRect = QRectF(iconRect.topRight().x() - diameter - 10, iconRect.topRight().y(), diameter + 10, diameter + 10);
            painter.setBrush(Qt::red);
            painter.setPen(Qt::red);
            painter.drawEllipse(textRect);
            painter.setPen(Qt::white);
            painter.drawText(textRect, Qt::AlignCenter, text);
            painter.restore();

            drag->setPixmap(pixmap);
            drag->setHotSpot(QPoint(200,200));
            drag->setDragCursor(QPixmap(), m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
            drag->exec(m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
        } else {
            QRegion rect;
            QHash<QModelIndex, QRect> indexRectHash;
            for (auto index : indexes) {
                rect += (visualRect(index));
                indexRectHash.insert(index, visualRect(index));
            }

            QRect realRect = rect.boundingRect();
            QPixmap pixmap(realRect.size() * scale);
            pixmap.fill(Qt::transparent);
            pixmap.setDevicePixelRatio(scale);
            QPainter painter(&pixmap);
            quint64 count = 0;

            for (auto index : indexes) {
                painter.save();
                painter.translate(indexRectHash.value(index).topLeft() - rect.boundingRect().topLeft());
                //painter.translate(-rect.boundingRect().topLeft());
                painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                QStyleOptionViewItem opt = viewOptions();
                auto viewItemDelegate = static_cast<ListViewDelegate *>(itemDelegate());
                viewItemDelegate->initIndexOption(&opt, index);
                opt.displayAlignment = Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter);
                opt.rect.setSize(indexRectHash.value(index).size());
                opt.rect.moveTo(0, 0);
                opt.state |= QStyle::State_Selected;
                painter.setOpacity(0.8);

                count++;
                if(count == 1){
                    QPainterPath leftRoundedRegion;
                    leftRoundedRegion.setFillRule(Qt::WindingFill);
                    leftRoundedRegion.addRoundedRect(opt.rect, LISTVIEW_ITEM_BORDER_RADIUS, LISTVIEW_ITEM_BORDER_RADIUS);
                    leftRoundedRegion.addRect(opt.rect.adjusted(LISTVIEW_ITEM_BORDER_RADIUS, 0, 0, 0));
                    painter.setClipPath(leftRoundedRegion);
                }else if(count == 4){
                    QPainterPath rightRoundedRegion;
                    rightRoundedRegion.setFillRule(Qt::WindingFill);
                    rightRoundedRegion.addRoundedRect(opt.rect, LISTVIEW_ITEM_BORDER_RADIUS, LISTVIEW_ITEM_BORDER_RADIUS);
                    rightRoundedRegion.addRect(opt.rect.adjusted(0, 0, -LISTVIEW_ITEM_BORDER_RADIUS, 0));
                    painter.setClipPath(rightRoundedRegion);
                    count = 0;
                }

                QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, &painter, this);
                painter.restore();
            }
            drag->setPixmap(pixmap);
            drag->setHotSpot(pos - rect.boundingRect().topLeft() - QPoint(0, header()->height()));
            drag->setDragCursor(QPixmap(), m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
            drag->exec(m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
        }
    }
}

void ListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    // 多选模式下框选（空白/非文本区域）使用 ClearAndSelect，以最新框选结果为准
    // fix #I7GWAX: 按整行与框选区域求交，避免 QTreeView 默认按列/单元格误判导致多选
    if (!model() || !selectionModel()) {
        return;
    }

    const QRect selRect = rect.normalized();
    if (selRect.isEmpty()) {
        return;
    }

    QItemSelection selection;
    const QModelIndex root = rootIndex();
    const int rowCount = model()->rowCount(root);
    const int colCount = model()->columnCount(root);
    if (rowCount <= 0 || colCount <= 0) {
        return;
    }

    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex firstIdx = model()->index(row, 0, root);
        if (!firstIdx.isValid()) {
            continue;
        }

        QRect rowRect = visualRect(firstIdx);
        for (int col = 1; col < colCount; ++col) {
            const QModelIndex colIdx = model()->index(row, col, root);
            if (colIdx.isValid()) {
                rowRect = rowRect.united(visualRect(colIdx));
            }
        }

        if (!rowRect.isValid() || rowRect.height() <= 0) {
            continue;
        }
        if (!selRect.intersects(rowRect)) {
            continue;
        }

        const QModelIndex lastIdx = model()->index(row, colCount - 1, root);
        selection.select(firstIdx, lastIdx);
    }

    selectionModel()->select(selection, command);
}

QItemSelectionModel::SelectionFlags ListView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    if (!event)
        return QTreeView::selectionCommand(index, event);

    if (event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<const QMouseEvent *>(event);
        const QModelIndex rowIndex = index.isValid() ? index.sibling(index.row(), 0) : QModelIndex();
        const bool rowSelected = rowIndex.isValid()
                && (selectionModel()->isSelected(rowIndex) || selectionModel()->isSelected(index));
        if (e->button() == Qt::LeftButton && m_multi_select && !(e->modifiers() & Qt::ControlModifier)) {
            if (rowIndex.isValid()) {
                // 多选模式下点击复选框区域，交给editorEvent处理，不在此处改变选中状态
                if (isClickOnCheckBox(rowIndex, e->pos())) {
                    return QItemSelectionModel::NoUpdate;
                }
                // 多选模式单击：未选中→选中（追加），已选中→暂不处理（松开时取消）
                if (rowSelected) {
                    return QItemSelectionModel::NoUpdate;
                }
                return QItemSelectionModel::Select | QItemSelectionModel::Rows;
            }
        }
        if (e->button() == Qt::LeftButton && rowSelected && (e->modifiers() & Qt::ControlModifier)) {
            return QItemSelectionModel::NoUpdate;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto e = static_cast<const QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && m_multi_select) {
            return QItemSelectionModel::NoUpdate;
        }
        if (e->button() == Qt::LeftButton && m_noSelectOnPress && m_multi_select) {
            if (isRenameSecondClick(index, e->pos())) {
                return QItemSelectionModel::NoUpdate;
            }
            return QItemSelectionModel::NoUpdate;
        }
        if (e->button() == Qt::LeftButton && m_noSelectOnPress && (e->modifiers() & Qt::ControlModifier)) {
            return QItemSelectionModel::NoUpdate;
        }
    }
    return QTreeView::selectionCommand(index, event);
}

void ListView::stretchLastColumn()
{
    if (!model() || model()->columnCount() < 1)
        return;

    int leftPartsSize = 0;
    int lastColumnIndex = model()->columnCount() - 1;

    header()->blockSignals(true);
    // 计算所有非最后一列的总宽度
    for (int column = 0; column < lastColumnIndex; column++) {
        leftPartsSize += header()->sectionSize(column);
    }

    resizeColumnToContents(lastColumnIndex);

    if (this->width() - leftPartsSize >  header()->sectionSize(lastColumnIndex)) {
        setColumnWidth(lastColumnIndex, this->width() - leftPartsSize);
        header()->blockSignals(false);
        updateGeometries();
        return;
    }

    // 确保视口宽度减去非最后一列的宽度，并将其分配给最后一列
    int availableWidth = this->viewport()->width();
    int lastColumnSize = availableWidth - leftPartsSize;
    lastColumnSize = lastColumnSize > header()->sectionSize(lastColumnIndex)? lastColumnSize : header()->sectionSize(lastColumnIndex);

    header()->resizeSection(lastColumnIndex, lastColumnSize);
    header()->blockSignals(false);
    updateGeometries();
}

bool ListView::isRenameSecondClick(const QModelIndex &index, const QPoint &pos) const
{
    if (m_multi_select)
        return false;

    if (!index.isValid() || !m_last_index.isValid() || !m_renameTimer->isActive())
        return false;

    if (index.row() != m_last_index.row() || index.parent() != m_last_index.parent())
        return false;

    const int remaining = m_renameTimer->remainingTime();
    const int renameWindow = 3000 - qApp->styleHints()->mouseDoubleClickInterval();
    if (remaining <= 0 || remaining >= renameWindow)
        return false;

    if (m_multi_select) {
        const QModelIndex col0 = index.sibling(index.row(), 0);
        QStyleOptionViewItem option;
        option.initFrom(this);
        option.rect = visualRect(col0);
        option.features |= QStyleOptionViewItem::HasCheckIndicator;
        option.checkState = selectionModel()->isSelected(col0) ? Qt::Checked : Qt::Unchecked;
        const QRect checkRect = style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, this);
        if (checkRect.isValid() && checkRect.contains(pos))
            return false;
    }

    bool all_index_in_same_row = true;
    if (!selectedIndexes().isEmpty()) {
        const int first_index_row = selectedIndexes().first().row();
        for (const auto &selectedIndex : selectedIndexes()) {
            if (first_index_row != selectedIndex.row()) {
                all_index_in_same_row = false;
                break;
            }
        }
    }
    return all_index_in_same_row;
}

bool ListView::isClickOnCheckBox(const QModelIndex &index, const QPoint &pos) const
{
    if (!m_multi_select || !index.isValid())
        return false;

    const QModelIndex col0 = index.sibling(index.row(), 0);
    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = visualRect(col0);
    option.features |= QStyleOptionViewItem::HasCheckIndicator;
    option.checkState = selectionModel()->isSelected(col0) ? Qt::Checked : Qt::Unchecked;
    const QRect checkRect = style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, this);
    return checkRect.isValid() && checkRect.contains(pos);
}

void ListView::slotRename()
{
    //special path like trash path not allow rename
    if (getDirectoryUri().startsWith("trash://")
        || getDirectoryUri().startsWith("recent://")
        || getDirectoryUri().startsWith("favorite://")
//        || getDirectoryUri().startsWith("search://")   //comment fix in search result can not click to rename issue
        || getDirectoryUri().startsWith("network://")
        || getDirectoryUri().startsWith("label://"))

        return;

    //standardPaths not allow rename
    auto currentSelections = getSelections();
    bool hasStandardPath = FileUtils::containsStandardPath(currentSelections);
    if (hasStandardPath)
        return;

    //delay edit action to avoid doubleClick or dragEvent
    qDebug()<<"slotRename"<<m_editValid;
    QTimer::singleShot(300, this, [&]() {
        qDebug()<<"singleshot"<<m_editValid;
        //fix bug#98951, click edit box boarder will reenter edit issue
        if(m_editValid &&  ! m_delegate_editing) {
            m_renameTimer->stop();
            setIndexWidget(m_last_index, nullptr);
            edit(m_last_index);
            m_editValid = false;
        }
    });

}

void ListView::setProxy(DirectoryViewProxyIface *proxy)
{

}

void ListView::resort()
{
    m_proxy_model->sort(getSortType(), Qt::SortOrder(getSortOrder()));
}

void ListView::reportViewDirectoryChanged()
{
    Q_EMIT m_proxy->viewDirectoryChanged();
}

void ListView::adjustColumnsSize()
{
    int columnSize = 0;

    if (!model())
        return;

    if (model()->columnCount() == 0)
        return;

    // try fixing #155969, list view can not save columns' state while resizing.
    if (m_header_section_resized_manually) {
        stretchLastColumn(); /* linkto bug#220914 */
        return;
    }

    // do not trigger header's sectionResized() signal. related to #155969.
    header()->blockSignals(true);
    header()->resizeSections(QHeaderView::ResizeToContents);

    int rightPartsSize = 0;
    for (int column = 1; column < model()->columnCount(); column++) {
        columnSize = header()->sectionSize(column);
        rightPartsSize += columnSize;
    }

    //set column 0 minimum width, fix header icon overlap with name issue
    if(columnWidth(0) < columnWidth(1))
        setColumnWidth(0, columnWidth(1));

    if (this->width() - rightPartsSize < BOTTOM_STATUS_MARGIN) {
        int size = width() - BOTTOM_STATUS_MARGIN;
        size /= header()->count() - 1;
        setColumnWidth(0, BOTTOM_STATUS_MARGIN);
        for (int column = 1; column < model()->columnCount(); column++) {
            setColumnWidth(column, size);
        }
        header()->blockSignals(false);
        return;
    }

    header()->resizeSection(0, this->viewport()->width() - rightPartsSize);
    header()->blockSignals(false);
}

// 保存列宽设置
void ListView::saveColumnWidths()
{
    if (!model())
        return;

    int columnCount = model()->columnCount();
    if (columnCount <= 0)
        return;

    // 保存每列的宽度
    QStringList widths;
    for (int i = 0; i < columnCount; i++) {
        int width = header()->sectionSize(i);
        widths.append(QString::number(width));
    }

    // 保存到全局设置
    GlobalSettings::getInstance()->setValue(LISTVIEW_COLUMN_WIDTHS, widths.join(","));
}

// 恢复列宽设置
void ListView::restoreColumnWidths()
{
    if (!model())
        return;

    int columnCount = model()->columnCount();
    if (columnCount <= 0)
        return;

    // 获取保存的列宽
    QString widthsStr = GlobalSettings::getInstance()->getValue(LISTVIEW_COLUMN_WIDTHS).toString();
    if (widthsStr.isEmpty())
        return;

    QStringList colsWidth = widthsStr.split(",");
    if (colsWidth.count() >= 4) {
        // 恢复列宽
        header()->blockSignals(true);
        for (int column = 0; column < model()->columnCount() && column < colsWidth.count(); column++) {
            int width = colsWidth[column].toInt();
            if (width > 0) {
                setColumnWidth(column, width);
            }
        }
        header()->blockSignals(false);
        m_header_section_resized_manually = true;
    }
}

void ListView::multiSelect()
{
    if (m_multi_select) {
        return;
    }
    if (m_proxy_model->getSelectionModeHint() != NoSelection) {
        return;
    }
    if (!GlobalSettings::getInstance()->getValue(SHOW_MULTISELECT_CHECKBOX).toBool())
        return;

    const int savedScroll = verticalScrollBar()->value();
    m_multi_select = true;
    m_proxy_model->setMultiSelectMode(true);
    // 保持 ExtendedSelection，框选/拖拽与普通模式一致，仅显示复选框
    auto restoreScroll = [this, savedScroll]() {
        verticalScrollBar()->setValue(qMin(savedScroll, verticalScrollBar()->maximum()));
        reUpdateScrollBar();
    };
    restoreScroll();
    QTimer::singleShot(0, this, restoreScroll);
    viewport()->update(viewport()->rect());
    updateHeaderCheckBox();
    QTimer::singleShot(0, this, [this]() {
        updateHeaderCheckBox();
    });
    Q_EMIT updateSelectStatus(true);
}

void ListView::disableMultiSelect()
{
    if (!m_multi_select) {
        return;
    }
    if (m_proxy_model->getSelectionModeHint() != NoSelection) {
        return;
    }
    const int savedScroll = verticalScrollBar()->value();
    m_multi_select = false;
    m_proxy_model->setMultiSelectMode(false);
    auto restoreScroll = [this, savedScroll]() {
        verticalScrollBar()->setValue(qMin(savedScroll, verticalScrollBar()->maximum()));
        reUpdateScrollBar();
    };
    restoreScroll();
    QTimer::singleShot(0, this, restoreScroll);
    viewport()->update(viewport()->rect());
    updateHeaderCheckBox();
    Q_EMIT updateSelectStatus(false);
}

void ListView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QTreeView::dataChanged(topLeft, bottomRight, roles);
    reUpdateScrollBar();
}

DirectoryViewProxyIface *ListView::getProxy()
{
    return m_proxy;
}

const QString ListView::getDirectoryUri()
{
    if (!m_model)
        return nullptr;
    return m_model->getRootUri();
}

void ListView::setDirectoryUri(const QString &uri)
{
    if (m_current_uri != uri) {
        disableMultiSelect();
    }
    m_current_uri = uri;
    if (m_current_uri.startsWith("search://")) {
        QString nameRegexp = SearchVFSUriParser::getSearchUriNameRegexp(uri);
        setSearchKey(nameRegexp);
    } else {
        setSearchKey("");
    }

}

const QStringList ListView::getSelections()
{
    QStringList uris;
    QString uri;
    QModelIndexList selections = selectedIndexes();
    for (auto index : selections) {
        if (index.column() == 0)
            uris<<index.data(FileItemModel::UriRole).toString();
    }
    uris.removeDuplicates();
    return uris;
}

void ListView::setSelections(const QStringList &uris)
{
    clearSelection();
    QItemSelection selection;
    for (auto uri: uris) {
        const QModelIndex index = m_proxy_model->indexFromUri(uri);
        if (index.isValid()) {
            QItemSelection selectionToBeMerged(index, index);
            selection.merge(selectionToBeMerged, QItemSelectionModel::Select);
        }
    }
    auto flags = QItemSelectionModel::Select|QItemSelectionModel::Rows;
    selectionModel()->select(selection, flags);
}

const QStringList ListView::getAllFileUris()
{
    return m_proxy_model->getAllFileUris();
}

const int ListView::getAllDisplayFileCount()
{
    return m_proxy_model->rowCount();
}

int ListView::getCurrentCheckboxColumn()
{
    // 复选框始终显示在第一列（表头位置）
    return 0;
}

void ListView::open(const QStringList &uris, bool newWindow)
{
    return;
}

void ListView::beginLocationChange()
{
    m_editValid = false;
    m_last_index = QModelIndex();
    //setModel(nullptr);
    m_model->setRootUri(m_current_uri);
}

void ListView::stopLocationChange()
{
    m_model->cancelFindChildren();
}

void ListView::closeView()
{
    this->deleteLater();
}

void ListView::invertSelections()
{
    QItemSelectionModel *selectionModel = this->selectionModel();
    if (!selectionModel || !model()) {
        return;
    }

    QItemSelection toSelect;
    QItemSelection toDeselect;
    const int rowCount = model()->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model()->index(row, 0);
        if (!index.isValid()) {
            continue;
        }
        if (selectionModel->isSelected(index)) {
            toDeselect.select(index, index);
        } else {
            toSelect.select(index, index);
        }
    }

    selectionModel->select(toDeselect, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    selectionModel->select(toSelect, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    viewport()->update();
}

void ListView::scrollToSelection(const QString &uri)
{
    auto index = m_proxy_model->indexFromUri(uri);
    QTreeView::scrollTo(index);
    reUpdateScrollBar();
}

void ListView::setCutFiles(const QStringList &uris)
{
    return;
}

bool ListView::getDelegateEditFlag()
{
    return m_delegate_editing;
}

void ListView::setItemsVisible(bool visible)
{
    viewport()->setVisible(visible);
}

int ListView::getSortType()
{
    int type = m_proxy_model->expectedSortType();
    return type<0? 0: type;
}

void ListView::setSortType(int sortType)
{
    //fix indicator not agree with actual sort order issue, link to bug#71475
    header()->blockSignals(true);
    header()->setSortIndicator(sortType, Qt::SortOrder(getSortOrder()));
    header()->blockSignals(false);
    m_proxy_model->sort(sortType, Qt::SortOrder(getSortOrder()));
}

int ListView::getSortOrder()
{
    return m_proxy_model->expectedSortOrder();
}

void ListView::setSortOrder(int sortOrder)
{
    //fix indicator not agree with actual sort order issue, link to bug#71475
    header()->blockSignals(true);
    header()->setSortIndicator(getSortType(), Qt::SortOrder(sortOrder));
    header()->blockSignals(false);
    m_proxy_model->sort(getSortType(), Qt::SortOrder(sortOrder));
}

void ListView::editUri(const QString &uri)
{
    setState(QTreeView::NoState);
    auto origin = FileUtils::getOriginalUri(uri);
    if(uri.startsWith("mtp://") || uri.startsWith("smb://"))/* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
        origin = uri;
    QModelIndex index = m_proxy_model->indexFromUri(origin);
    setIndexWidget(index, nullptr);
    //fix bug#70769, edit box overlapped with status bar issue
    //qDebug() <<"editUri row"<<m_proxy_model->rowCount()<<index.row();
    if(index.row() >= m_proxy_model->rowCount()-1) {
        reUpdateScrollBar();
        QTreeView::scrollToBottom();
        QTimer::singleShot(300, this, [=]{
            if (qApp->property("tabletMode").toBool()) {
                reUpdateScrollBar();
                QTreeView::scrollToBottom();
            }
        });
    }
    //注释该行以修复bug:#60474
//    QTreeView::scrollTo(m_proxy_model->indexFromUri(origin));
    edit(index);
}

void ListView::editUris(const QStringList uris)
{
    //FIXME:
    //implement batch rename.
    setState(QTreeView::NoState);
    auto origin = FileUtils::getOriginalUri(uris.first());
    if(uris.first().startsWith("mtp://"))/* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
        origin = uris.first();
    QModelIndex index = m_proxy_model->indexFromUri(origin);
    setIndexWidget(index, nullptr);
    //fix bug#70769, edit box overlapped with status bar issue
    //qDebug() <<"editUri row"<<m_proxy_model->rowCount()<<index.row();
    if(index.row() >= m_proxy_model->rowCount()-1)
       QTreeView::scrollToBottom();
    //注释该行以修复bug:#60474
    // (bug-60474)不会受批量重命名的影响
    QTreeView::scrollTo(m_proxy_model->indexFromUri(origin));
    edit(index);
}

bool ListView::isEnableMultiSelect()
{
    return m_multi_select;
}

void ListView::keyboardSearch(const QString &key)
{
    // ensure current index is index in display name column
    if (currentIndex().column() != 0) {
        selectionModel()->setCurrentIndex(m_model->index(currentIndex().row(), 0, currentIndex().parent()), QItemSelectionModel::SelectCurrent);
    }

    // note: checking qtreeview.cpp we can find that the keyboard search only select rows
    // while selection mode is single selection. so we have a trick here for trigger that
    // action.
    setSelectionMode(QTreeView::SingleSelection);
    QAbstractItemView::keyboardSearch(key);
    setSelectionMode(QTreeView::ExtendedSelection);

    auto indexes = selectedIndexes();
    if (!indexes.isEmpty()) {
        QTreeView::scrollTo(indexes.first(), QTreeView::PositionAtCenter);
        reUpdateScrollBar();
        if (verticalScrollBar()->value() < viewport()->height()) {
            return;
        }
        verticalScrollBar()->setValue(qMin(verticalScrollBar()->value() + iconSize().height(), verticalScrollBar()->maximum()));
    }
}

void ListView::setSearchKey(const QString &key)
{
    auto viewItemDelegate = static_cast<ListViewDelegate *>(itemDelegate());
    viewItemDelegate->setSearchKeyword(key);
}

void ListView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    auto proxyModel = static_cast<FileItemProxyFilterSortModel*>(model());
    auto sourceIndex = proxyModel->mapToSource(index);

    FileItem *item = sourceIndex.isValid()? static_cast<FileItem*>(sourceIndex.internalPointer()): nullptr;
#ifdef KY_UDF_BURN
    if (item) {
        /* R类型光盘，所有用于刻录的文件（夹）展示在挂载点时都应该半透明显示，区别于普通文件 ,linkto task#122470 */
        if(item->property("isFileForBurning").toBool()){
            painter->setOpacity(0.5);
        }else{
            painter->setOpacity(1.0);
        }
    }
#endif

    if (!m_model) {
        painter->restore();
        return QTreeView::drawRow(painter, option, index);
    }

    auto clipedUris = ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()) {
        bool bSearchTab = false;
        QString actualDirUri = m_model->getRootUri();
        if(actualDirUri.startsWith("search:///search_uris")){
            actualDirUri = FileUtils::getActualDirFromSearchUri(actualDirUri);
            bSearchTab = true;
        }

        QString clipedFilesParentUri = ClipboardUtils::getClipedFilesParentUri();
        if (FileUtils::isSamePath(clipedFilesParentUri, actualDirUri) || (bSearchTab && clipedFilesParentUri.startsWith(actualDirUri))) {
            if (clipedUris.contains(index.data(Qt::UserRole).toString())) {
                painter->setOpacity(0.5);
            }
            else {
                painter->setOpacity(1.0);
            }
        }
    }
    QTreeView::drawRow(painter, option, index);
    painter->restore();
}

void ListView::doMultiSelect(bool isMultiSlelect)
{
    if (isMultiSlelect) {
        multiSelect();
    } else {
        disableMultiSelect();
    }

    viewport()->update();
    updateHeaderCheckBox();
    if (isMultiSlelect) {
        QTimer::singleShot(0, this, [this]() {
            updateHeaderCheckBox();
        });
    }
}

void ListView::setLabelAlignment(int alignment)
{
    m_labelAlignment = alignment;
}

int ListView::getLabelAlignment() const
{
    return m_labelAlignment;
}

void ListView::updateHeaderCheckBox()
{
    if (!m_checkbox_header || !model()) {
        return;
    }

    const bool showHeaderCheckbox = m_multi_select
            && GlobalSettings::getInstance()->getValue(SHOW_MULTISELECT_CHECKBOX).toBool();
    m_checkbox_header->setCheckBoxEnabled(showHeaderCheckbox);

    if (!showHeaderCheckbox) {
        QSignalBlocker blocker(m_checkbox_header);
        m_checkbox_header->setCheckState(Qt::Unchecked);
        return;
    }

    const int columnCount = qMax(1, model()->columnCount());
    const int selectedCount = selectedIndexes().count() / columnCount;
    const int totalCount = model()->rowCount();
    m_checkbox_header->updateCheckState(selectedCount, totalCount);
}

void ListView::onHeaderCheckBoxStateChanged(Qt::CheckState state)
{
    if (!m_multi_select) {
        return;
    }

    if (state == Qt::Checked) {
        // 全选所有行
        selectAll();
    } else if (state == Qt::Unchecked) {
        // 取消全选
        clearSelection();
    }
    // PartiallyChecked 状态表示部分选中，不做操作
}

void ListView::onSelectionChangedForHeader()
{
    updateHeaderCheckBox();
}

//List View 2
ListView2::ListView2(QWidget *parent) : DirectoryViewWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setObjectName("_listview2_layout");
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_view = new ListView(this);

    DirectoryViewHelper * viewHelper = DirectoryViewHelper::globalInstance();
    viewHelper->addListViewWithDirectoryViewWidget(m_view, this);
    connect(m_view, &ListView::updateSelectStatus, viewHelper, &DirectoryViewHelper::updateSelectStatus);

    // int defaultZoomLevel = GlobalSettings::getInstance()->getValue(DEFAULT_VIEW_ZOOM_LEVEL).toInt();
    // if (defaultZoomLevel >= minimumZoomLevel() && defaultZoomLevel <= maximumZoomLevel())
    //     m_zoom_level = defaultZoomLevel;

    connect(m_view, &ListView::zoomLevelChangedRequest, this, &ListView2::zoomRequest);
    layout->addWidget(m_view);

    setLayout(layout);
}

ListView2::~ListView2()
{
    m_model->setPositiveResponse(true);
    // 从DirectoryViewHelper中移除，避免悬空指针
    DirectoryViewHelper::globalInstance()->removeView(this);
}

int ListView2::currentZoomLevel()
{
    int defaultZoomLevel = FileUtils::getViewZoomLevel(m_view->m_current_uri);
    if (defaultZoomLevel >= minimumZoomLevel() && defaultZoomLevel <= maximumZoomLevel())
        m_zoom_level = defaultZoomLevel;

    return m_zoom_level;
}

void ListView2::bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel)
{
    auto layout = findChild<QVBoxLayout *>("_listview2_layout");
    bool ok = false;
    if (parentWidget()) {
        int statusBarHeight = parentWidget()->property("statusBarHeight").toInt(&ok);
        if (ok) {
            layout->setContentsMargins(0, 0, 0, statusBarHeight);
        }
    }

    disconnect(m_model);
    disconnect(m_proxy_model);
    m_model = model;
    m_proxy_model = proxyModel;

    //m_model->setPositiveResponse(false);

    m_view->bindModel(model, proxyModel);
    connect(m_model, &FileItemModel::selectRequest, this, &DirectoryViewWidget::updateWindowSelectionRequest);
    connect(m_model,&FileItemModel::signal_itemAdded, this, [=](const QString& uri){
        Q_EMIT this->signal_itemAdded(uri);
    });
    connect(model, &FileItemModel::findChildrenFinished, this, &DirectoryViewWidget::viewDirectoryChanged);
    //connect(m_model, &FileItemModel::updated, m_view, &ListView::resort);
    connect(m_model, &FileItemModel::updated, m_view->viewport(), QOverload<>::of(&QWidget::update));

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]() {
        m_view->m_flag = true;
        m_view->doItemsLayout();
        m_view->m_flag = false;
        Q_EMIT viewSelectionChanged();
    });

    connect(m_view, &ListView::activated, this, [=](const QModelIndex &index) {
        if (m_view->m_touch_active_timer->isActive()) {
            auto costTime = m_view->m_touch_active_timer->interval() - m_view->m_touch_active_timer->remainingTime();
            if (costTime > qApp->doubleClickInterval()) {
                m_view->m_touch_active_timer->stop();
                return;
            }
        }

        //when selections is more than 1, let mainwindow to process
        if (getSelections().count() != 1)
            return;
        auto uri = getSelections().first();
        Q_EMIT this->viewDoubleClicked(uri);

        m_view->m_touch_active_timer->stop();
    });

    //FIXME: how about multi-selection?
    //menu
    connect(m_view, &ListView::customContextMenuRequested, this, [=](const QPoint &pos) {
        qDebug()<<"menu request";
        if (!m_view->indexAt(pos).isValid())
        {
            m_view->clearSelection();
            //m_view->clearFocus();
        }

        auto index = m_view->indexAt(pos);
        auto selectedIndexes = m_view->selectionModel()->selection().indexes();
        auto visualRect = m_view->visualRect(index);
        auto sizeHint = m_view->itemDelegate()->sizeHint(m_view->viewOptions(), index);
        auto validRect = QRect(visualRect.topLeft(), sizeHint);
        if (!selectedIndexes.contains(index)) {
            if (!validRect.contains(pos)) {
                m_view->clearSelection();
                //m_view->clearFocus();
            } else {
                auto flags = QItemSelectionModel::Select|QItemSelectionModel::Rows;
                m_view->clearSelection();
                //m_view->clearFocus();
                m_view->selectionModel()->select(m_view->indexAt(pos), flags);
            }
        }

        //NOTE: we have to ensure that we have cleared the
        //selection if menu request at blank pos.
        QTimer::singleShot(1, this, [=]() {
            m_view->m_touch_active_timer->stop();
            Q_EMIT this->menuRequest(mapToGlobal(pos + m_view->header()->geometry().bottomLeft()));
        });
    });

    connect(m_proxy_model, &FileItemProxyFilterSortModel::layoutChanged, this, [=]() {
        Q_EMIT this->sortOrderChanged(Qt::SortOrder(getSortOrder()));
    });
    connect(m_proxy_model, &FileItemProxyFilterSortModel::layoutChanged, this, [=]() {
        Q_EMIT this->sortTypeChanged(getSortType());
    });

    connect(m_model, &FileItemModel::findChildrenFinished, this, [=]() {
        if (m_need_resize_header) {
            QTimer::singleShot(0, this, [=]{
                m_view->adjustColumnsSize();
            });
        }
        m_need_resize_header = false;
    });
}

void ListView2::repaintView()
{
    m_view->update();
    m_view->viewport()->update();
}

void ListView2::setCurrentZoomLevel(int zoomLevel)
{
    int base = 16;
    int adjusted = base + zoomLevel;

    m_view->setIconSize(QSize(adjusted, adjusted));
    m_zoom_level = zoomLevel;
}

void ListView2::clearIndexWidget()
{
    for (auto index : m_proxy_model->getAllFileIndexes()) {
        m_view->setIndexWidget(index, nullptr);
        m_view->closePersistentEditor(index);
    }
}
