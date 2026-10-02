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

#include "icon-view.h"
#include "file-item.h"

#include "icon-view-delegate.h"
#include "icon-view-style.h"

#include "directory-view-menu.h"
#include "icon-view-index-widget.h"
#include "file-info.h"
#include "file-utils.h"
#include "file-operation-utils.h"

#include "global-settings.h"
#include "search-vfs-uri-parser.h"
#include "directoryviewhelper.h"

#include <QMouseEvent>

#include <QDragEnterEvent>
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>

#include <QPainter>
#include <QPaintEvent>

#include <QApplication>

#include <QVBoxLayout>

#include <QHoverEvent>

#include <QScrollBar>

#include <QMouseEvent>
#include <QApplication>

#include <QStringList>
#include <QStyleHints>
#include <QPoint>

#include <QDebug>
#include <QToolTip>

#include <QStandardPaths>

#include <QDrag>
#include <QWindow>
#include <QMessageBox>
#include <QFontMetrics>
#include <QStyleOptionRubberBand>
#include <QCursor>

using namespace Peony;
using namespace Peony::DirectoryView;

IconView::IconView(QWidget *parent) : QListView(parent)
{
    setProperty("useIconHighlightEffect", 0x0);

    QColor hlColor = palette().color(QPalette::Highlight);
    hlColor.setAlphaF(0.24);
    QBrush semiTransparentBrush(hlColor);
    setProperty("setSelectBrush", semiTransparentBrush);
    setProperty("setBranchSelectBrush", semiTransparentBrush);

    m_touch_active_timer = new QTimer(this);
    m_touch_active_timer->setSingleShot(true);

    //setFrameShape(QFrame::NoFrame);

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);

    setAutoScroll(true);
    setAutoScrollMargin(100);
    QString version = qApp->property("version").toString();
    if (version == "ukui3.0"){
        setStyle(IconViewStyle::getStyle());
    } else {
        setFrameShape(QFrame::NoFrame);
    }

    //setStyle(IconViewStyle::getStyle());
    //FIXME: do not create proxy in view itself.
    IconViewDelegate *delegate = new IconViewDelegate(this);
    setItemDelegate(delegate);
    connect(delegate, &IconViewDelegate::isEditing, this, [=](const bool &editing)
    {
        m_delegate_editing = editing;
    });

    //fix long file name not fully painted issue when drag sliderbar
    QScrollBar *verticalBar = verticalScrollBar();
    connect(verticalBar, &QScrollBar::sliderPressed, [=](){
        m_slider_bar_draging = true;
    });
    connect(verticalBar, &QScrollBar::sliderReleased, [=](){
        m_slider_bar_draging = false;
    });
    connect(verticalBar, &QScrollBar::valueChanged, [=](){
        if (m_slider_bar_draging)
            viewport()->update(viewport()->rect());
    });

    setSelectionMode(QListView::ExtendedSelection);
    setEditTriggers(QListView::NoEditTriggers);
    setViewMode(QListView::IconMode);
    // 禁用 QListView 内部橡皮筋控件，改为 paintEvent 中自绘
    // 原因：滚动动画期间 QListView 内部橡皮筋状态不可控（被移动/隐藏/Z-order变化），导致偶尔看不到
    setSelectionRectVisible(false);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Snap);
    //setWordWrap(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setIconSize(QSize(64, 64));
    setGridSize(itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex()) + QSize(20, 20));

    m_renameTimer = new QTimer(this);
    m_renameTimer->setInterval(3000);
    m_renameTimer->setSingleShot(true);
    m_editValid = false;

    setMouseTracking(true);//追踪鼠标

    setViewportMargins(0, 0, 0, 0);

    scrollAnimation = new QPropertyAnimation(verticalScrollBar(), "value", this);
    scrollAnimation->setDuration(200); // 动画持续时间(毫秒)
    scrollAnimation->setEasingCurve(QEasingCurve::OutQuad); // 使用缓动曲线
}

IconView::~IconView()
{
    if (scrollAnimation) {
        scrollAnimation->deleteLater();
        scrollAnimation = nullptr;
    }
}

DirectoryViewProxyIface *IconView::getProxy()
{
    return m_proxy;
}

//selection
//FIXME: implement the selection functions.
void IconView::setSelections(const QStringList &uris)
{
    clearSelection();
    QItemSelection selection;
    for (auto uri: uris) {
        const QModelIndex index = m_sort_filter_proxy_model->indexFromUri(uri);
        if (index.isValid()) {
            QItemSelection selectionToBeMerged(index, index);
            selection.merge(selectionToBeMerged, QItemSelectionModel::Select);
        }
    }

    selectionModel()->select(selection, QItemSelectionModel::Select);
}

const QStringList IconView::getSelections()
{
    QStringList uris;
    QModelIndexList selections = selectedIndexes();
    for (auto index : selections) {
        auto item = m_sort_filter_proxy_model->itemFromIndex(index);
        uris<<item->uri();
    }
    uris.removeDuplicates();
    return uris;
}

void IconView::invertSelections()
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

    selectionModel->select(toDeselect, QItemSelectionModel::Deselect);
    selectionModel->select(toSelect, QItemSelectionModel::Select);

    viewport()->update();
}

void IconView::scrollToSelection(const QString &uri)
{
    auto index = m_sort_filter_proxy_model->indexFromUri(uri);
    scrollTo(index);
}

//clipboard
void IconView::setCutFiles(const QStringList &uris)
{
    //let delegate and model know how to deal with cut files.
}

//location
//FIXME: implement location functions.
void IconView::setDirectoryUri(const QString &uri)
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

const QString IconView::getDirectoryUri()
{
    return m_model->getRootUri();
}

void IconView::beginLocationChange()
{
    m_last_index = QModelIndex();
    traverseNode();
    m_editValid = false;
    m_model->setRootUri(m_current_uri);
}

void IconView::stopLocationChange()
{
    m_model->cancelFindChildren();
}

void IconView::traverseNode()
{
    //fix bug 194738, clear model index
    QList<IconViewIndexWidget *> widgets = this->findChildren<IconViewIndexWidget *>("peony_icon_view_index_widget");
    if (!widgets.isEmpty()) {
        for (int i = 0; i < widgets.size(); ++i) {
            auto node = widgets.at(i);
            if (node) {
                node->setUpdatesEnabled(false);
                node->clearModelIndex();
            }
        }
    }
}

//other
void IconView::open(const QStringList &uris, bool newWindow)
{

}

void IconView::closeView()
{
    this->deleteLater();
}

void IconView::dragEnterEvent(QDragEnterEvent *e)
{
    m_editValid = false;
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    bool isdropbyukuiidm = (e->mimeData()->hasFormat("text/ukui_idm") && e->mimeData()->data("text/ukui_idm") == "ukui_idm") ? true : false;
    if(isdropbyukuiidm){
        action = Qt::CopyAction;
    }
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

void IconView::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    //fix can not drag in the second time issue
    if (this->isDraggingState())
    {
        if (m_allow_set_index_widget) {
            m_allow_set_index_widget = false;
            for (auto index : selectedIndexes()) {
                setIndexWidget(index, nullptr);
            }
        }
    }

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    //qDebug()<<"dragMoveEvent()" <<action <<m_ctrl_key_pressed;
    bool isdropbyukuiidm = (e->mimeData()->hasFormat("text/ukui_idm") && e->mimeData()->data("text/ukui_idm") == "ukui_idm") ? true : false;
    if(isdropbyukuiidm){
        action = Qt::CopyAction;
     }
    auto index = indexAt(e->pos());
    if (index.isValid() && index != m_last_index) {
        QHoverEvent he(QHoverEvent::HoverMove, e->posF(), e->posF());
        viewportEvent(&he);
    } else {
        QHoverEvent he(QHoverEvent::HoverLeave, e->posF(), e->posF());
        viewportEvent(&he);
    }
    if (this == e->source() || !model()->flags(QModelIndex()).testFlag(Qt::ItemIsDropEnabled)&& !isdropbyukuiidm) {
        return QListView::dragMoveEvent(e);
    }
    e->setDropAction(action);
    e->accept();
}

void IconView::dropEvent(QDropEvent *e)
{
    m_last_index = QModelIndex();
    //m_edit_trigger_timer.stop();
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;

    bool isdropbyukuiidm = (e->mimeData()->hasFormat("text/ukui_idm") && e->mimeData()->data("text/ukui_idm") == "ukui_idm") ? true : false;
    if(isdropbyukuiidm){
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
    auto index = m_sort_filter_proxy_model->mapToSource(proxy_index);
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
            auto uri = m_sort_filter_proxy_model->itemFromIndex(proxy_index)->uri();
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
    m_model->dropMimeData(e->mimeData(), action, 0, 0, index);
}

void IconView::mouseMoveEvent(QMouseEvent *e)
{
    QModelIndex itemIndex = indexAt(e->pos());
    if (!itemIndex.isValid()) {
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    }

    if (m_ignore_mouse_move_event) {
        return;
    }

    // 手动拖拽检测：
    // 在图标/文本区域按下时（非空白），不应框选而应拖拽。QListView 默认对未选中项会进入
    // DragSelectingState 触发框选，且 IconView 未开启 setDragEnabled，故统一改为手动检测拖拽，
    // 并跳过 QListView::mouseMoveEvent 的框选逻辑。多选模式下 IndexWidget 处理切换后 QListView
    // 拖拽机制失效，同样由此分支接管。
    if (!m_dragStarted && !m_pressedOnBlank && (e->buttons() & Qt::LeftButton)) {
        const int dragDistance = (e->globalPos() - m_pressedGlobalPos).manhattanLength();
        if (dragDistance >= QApplication::startDragDistance()) {
            m_dragStarted = true;
            m_noSelectOnPress = false;  // 拖拽开始，取消选中标志清除
            if (!selectedIndexes().isEmpty()) {
                startDrag(defaultDropAction());
            }
            return;
        }
        return;  // 未达到拖拽阈值，跳过 QListView 的框选/拖拽逻辑，避免在图标/文本处框选
    }

    QListView::mouseMoveEvent(e);

    // 当开始拖拽或框选时，清除 m_noSelectOnPress，避免 mouseRelease 执行取消选中逻辑
    if (state() == DraggingState || m_boxSelectActive) {
        m_noSelectOnPress = false;
    }

    // 框选区域更新（仅在空白区域按下时才框选，在图标/文本区域按下则走拖拽逻辑）
    if (e->buttons() & Qt::LeftButton && m_pressedOnBlank
            && m_lastPressedLogicPoint.x() >= 0 && m_lastPressedLogicPoint.y() >= 0) {
        auto pos = e->pos();
        auto offset = QPoint(horizontalOffset(), verticalOffset());
        auto logicPos = pos + offset;
        QRect logicRect = QRect(logicPos, m_lastPressedLogicPoint);
        logicRect = logicRect.normalized();

        if (logicRect.width() > 2 || logicRect.height() > 2) {
            m_boxSelectActive = true;
        }

        int dx = -horizontalOffset();
        int dy = -verticalOffset();
        QRect visualRect = logicRect.adjusted(dx, dy, dx, dy);

        QRect updateRect = visualRect;
        if (m_lastVisualRect.isValid()) {
            updateRect = updateRect.united(m_lastVisualRect);
        }

        viewport()->update(updateRect.adjusted(-10, -10, 10, 10));
        m_lastVisualRect = visualRect;
    } else {
        m_lastVisualRect = QRect();
    }

    // fix #115124, drag selection can not trigger auto scroll in view.
    if (e->buttons() & Qt::LeftButton && !this->viewport()->rect().adjusted(0, autoScrollMargin(), 0, -autoScrollMargin()).contains(e->pos())) {
        doAutoScroll();
    }

}

void IconView::mousePressEvent(QMouseEvent *e)
{
    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    if (singleClicked) {
        if (!m_touch_active_timer->isActive()) {
            m_touch_active_timer->start(1100);
        }
    }

    // 当 IndexWidget 已处理多选切换时，跳过坐标查找，直接使用 m_toggleClickIndex
    // 因为事件坐标是相对于 IndexWidget 的，不是相对于 IconView 的
    QModelIndex itemIndex = QModelIndex();
    if (!m_multiSelectToggleHandled) {
        for (int i = 0; i < model()->rowCount(); i++) {
            auto index = model()->index(i, 0);
            if (visualRect(index).contains(e->pos())) {
                itemIndex = index;
                break;
            }
        }
    }

    if (e->button() == Qt::RightButton && m_multi_select && !itemIndex.isValid() && !m_multiSelectToggleHandled) {
        clearSelection();
        disableMultiSelect();
        QListView::mousePressEvent(e);
        return;
    }

    m_allow_set_index_widget = true;
    if (e->modifiers() & Qt::ControlModifier) {
        m_ctrl_key_pressed = true;
    } else {
        m_ctrl_key_pressed = false;
    }

    m_boxSelectActive = false;
    // 当 IndexWidget 已处理时，不是点击空白
    m_pressedOnBlank = (e->button() == Qt::LeftButton && !itemIndex.isValid() && !m_multiSelectToggleHandled);

    // 当 IndexWidget 已处理多选切换时，使用正确的索引
    QModelIndex effectiveIndex = itemIndex;
    if (m_multiSelectToggleHandled && m_toggleClickIndex.isValid()) {
        effectiveIndex = m_toggleClickIndex;
    }

    if (effectiveIndex.isValid() && m_multi_select) {
        m_mouse_release_unselect = selectedIndexes().contains(effectiveIndex);
    } else {
        m_mouse_release_unselect = false;
    }

    const bool wasSelectedBeforePress = effectiveIndex.isValid() && selectedIndexes().contains(effectiveIndex);
    // 如果 IndexWidget 已处理多选切换，不再覆盖其设置的 m_toggleClickIndex
    if (!m_multiSelectToggleHandled) {
        if (e->button() == Qt::LeftButton && wasSelectedBeforePress
                && ((e->modifiers() & Qt::ControlModifier) || m_multi_select)) {
            m_noSelectOnPress = true;
            m_toggleClickIndex = effectiveIndex;
        } else {
            m_noSelectOnPress = false;
            m_toggleClickIndex = QModelIndex();
        }
    }

    m_editValid = true;
    // 当 IndexWidget 已处理多选切换时，跳过 QListView::mousePressEvent，避免其默认行为清空其他选中项
    // 但仍然需要设置状态用于框选和拖拽检测
    if (!m_multiSelectToggleHandled) {
        // 多选模式下 Shift+点击：手动做区间选择（无 IndexWidget 的多数项走这里）。
        // 不能依赖 QListView::mousePressEvent，因为 selectionCommand 在多选模式下
        // 返回 Select 而非 SelectCurrent，Qt 不会触发区间选择。
        const bool shiftPressed = e->button() == Qt::LeftButton
                && (e->modifiers() & Qt::ShiftModifier)
                && m_multi_select
                && effectiveIndex.isValid()
                && m_multiSelectAnchor.isValid()
                && m_multiSelectAnchor.model() == effectiveIndex.model()
                && m_multiSelectAnchor.parent() == effectiveIndex.parent();
        if (shiftPressed) {
            QItemSelection range(m_multiSelectAnchor, effectiveIndex);
            const QItemSelectionModel::SelectionFlags flag = (e->modifiers() & Qt::ControlModifier)
                    ? QItemSelectionModel::Select
                    : QItemSelectionModel::ClearAndSelect;
            selectionModel()->select(range, flag);
            // 锚点保持不变，与 Qt ExtendedSelection 行为一致
            setState(QAbstractItemView::NoState);
            selectionModel()->setCurrentIndex(effectiveIndex, QItemSelectionModel::NoUpdate);
            m_pressedGlobalPos = e->globalPos();
            m_dragStarted = false;
            m_noSelectOnPress = false;
            m_toggleClickIndex = QModelIndex();
            m_multiSelectToggleHandled = true;  // 标记已处理，跳过下面的追加选中与 Qt 默认行为
        } else {
            QListView::mousePressEvent(e);
        }
    } else {
        // 手动设置拖拽所需的状态
        setState(QAbstractItemView::NoState);
        if (effectiveIndex.isValid()) {
            // 设置当前索引，用于拖拽时确定拖拽的项目
            selectionModel()->setCurrentIndex(effectiveIndex, QItemSelectionModel::NoUpdate);
        }
        // 记录按下位置用于拖拽检测
        m_pressedGlobalPos = e->globalPos();
        m_dragStarted = false;
    }

    // 非 IndexWidget 处理时，追加选中未选中项
    if (!m_multiSelectToggleHandled && m_multi_select && e->button() == Qt::LeftButton && effectiveIndex.isValid()
            && !wasSelectedBeforePress && !(e->modifiers() & Qt::ControlModifier)) {
        selectionModel()->select(effectiveIndex, QItemSelectionModel::Select);
    }

    // 非 Shift 点击统一刷新锚点（覆盖追加选中、取消选中、Ctrl 切换等情况），
    // 为下一次 Shift 区间选择提供起点。Shift 点击不刷新锚点。
    if (!m_multiSelectToggleHandled && m_multi_select && e->button() == Qt::LeftButton
            && effectiveIndex.isValid() && !(e->modifiers() & Qt::ShiftModifier)) {
        m_multiSelectAnchor = effectiveIndex;
    }

    if (e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton) {
        m_dragStartPos = e->pos();  // 保存拖拽开始时的鼠标位置，用于数位板拖拽热点计算
    }
    if (e->button() == Qt::LeftButton) {
        m_lastPressedLogicPoint = e->pos() + QPoint(horizontalOffset(), verticalOffset());
        m_lastVisualRect = QRect();
        m_pressedGlobalPos = e->globalPos();  // 用于在图标/文本区域按下时手动检测拖拽
        m_dragStarted = false;
    }

    if (e->button() != Qt::LeftButton) {
        return;
    }

    if (true == m_mouse_release_unselect) {
        //selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }

    viewport()->update(viewport()->rect());

    if (!itemIndex.isValid()) {
        // Ctrl/Shift 按下时点击空白不取消已有选中
        if (!m_multi_select && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
            clearSelection();
        }
        return;
    }
    //FIXME: Modify the icon style, only click on the text to respond, click on the icon to not respond
    QRect rect = visualRect(indexAt(e->pos()));
    QSize iconExpectedSize = iconSize();

    QRect testRect(rect.x(), rect.y()+ iconExpectedSize.height(),rect.width(),rect.height()-iconExpectedSize.height());
    if(!testRect.contains(e->pos()))
    {
        m_editValid = false;
        m_renameTimer->start();
        return;
    }
    if(!m_renameTimer->isActive())
    {
        m_renameTimer->start();
        m_editValid = false;
    }
    else
    {
        if(m_renameTimer->remainingTime() >0 && m_renameTimer->remainingTime() < 3000 - qApp->styleHints()->mouseDoubleClickInterval()
                && indexAt(e->pos()) == m_last_index && m_last_index.isValid() && m_editValid == true)
        {
            slotRename();
        } else
        {
            m_editValid = false;
        }
    }
}

void IconView::mouseReleaseEvent(QMouseEvent *e)
{
    const bool wasBoxSelect = m_boxSelectActive;
    const bool wasDragStarted = m_dragStarted;
    // 修复：非多选模式下，Ctrl+点击也应支持取消选中
    const bool deferClickDeselect = m_noSelectOnPress && (isEnableMultiSelect() || m_ctrl_key_pressed) && !wasDragStarted;
    const QModelIndex deselectIndex = deferClickDeselect ? m_toggleClickIndex : QModelIndex();

    QListView::mouseReleaseEvent(e);

    m_noSelectOnPress = false;
    m_toggleClickIndex = QModelIndex();
    m_multiSelectToggleHandled = false;
    m_dragStarted = false;

    if (deferClickDeselect && deselectIndex.isValid()) {
        // 立即取消选中，不使用延迟
        if (!m_skipClickDeselect) {
            if (selectionModel()->isSelected(deselectIndex)) {
                selectionModel()->select(deselectIndex, QItemSelectionModel::Deselect);
            }
        }
    }

    // 在重置之前，先更新最后一次的框选区域，清除残留的蓝色线
    if (m_lastVisualRect.isValid()) {
        viewport()->update(m_lastVisualRect.adjusted(-10, -10, 10, 10));
    }

    m_lastPressedLogicPoint = QPoint(-1, -1);
    m_lastVisualRect = QRect();
    m_dragStartPos = QPoint(-1, -1);  // 重置拖拽开始位置
    m_boxSelectActive = false;

    if (e->button() == Qt::LeftButton && m_pressedOnBlank && !wasBoxSelect && m_multi_select
            && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
        clearSelection();
        disableMultiSelect();
    }

    m_pressedOnBlank = false;

    if (e->button() != Qt::LeftButton) {
        return;
    }
}

void IconView::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_editValid = false;
    // 双击事件在 Qt 内部会代替第二次 mousePressEvent，因此需要与 mousePressEvent
    // 同步刷新 m_dragStartPos，否则双击后若触发拖拽，startDrag 拿到的是 release
    // 中被清空的 (-1,-1)，导致拖拽热点错位（数位板场景尤其明显）。
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_dragStartPos = event->pos();
    }
    m_skipClickDeselect = true;

    if (event->button() == Qt::LeftButton && isEnableMultiSelect()) {
        const QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            const QString uri = m_sort_filter_proxy_model->data(index, FileItemModel::UriRole).toString();
            if (!uri.isEmpty()) {
                setSelections(QStringList() << uri);
                Q_EMIT activated(index);
            }
            const int guardMs = qApp->styleHints()->mouseDoubleClickInterval() + 50;
            QTimer::singleShot(guardMs, this, [this]() {
                m_skipClickDeselect = false;
            });
            event->accept();
            return;
        }
    }

    QListView::mouseDoubleClickEvent(event);
    m_skipClickDeselect = false;
}

void IconView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        if (getSelections().count() == 1 && currentIndex().isValid()) {
            Q_EMIT quickLookRequested(currentIndex());
            e->accept();
            return;
        }
    }

    QListView::keyPressEvent(e);

    if (e->key() == Qt::Key_Control)
        m_ctrl_key_pressed = true;

    if(e->key() == Qt::Key_F10 && e->modifiers() == Qt::ShiftModifier) {
        if (getSelections().count() == 1 ) {
            auto currentIndex = selectionModel()->selection().indexes();
            QPoint menuPos = this->visualRect(currentIndex.first()).center();
            Q_EMIT customContextMenuRequested(menuPos);
            return;
        }
    }
}

void IconView::keyReleaseEvent(QKeyEvent *e)
{
    QListView::keyReleaseEvent(e);
    if (e->key() == Qt::Key_Control)
        m_ctrl_key_pressed = false;
}

void IconView::paintEvent(QPaintEvent *e)
{
    QPainter p(this->viewport());
    p.fillRect(this->geometry(), this->palette().base());
    if (m_repaint_timer.isActive()) {
        m_repaint_timer.stop();
        QTimer::singleShot(100, this, [this]() {
            this->repaint();
        });
    }
    QListView::paintEvent(e);

    // 自绘橡皮筋：不依赖 QListView 内部 QRubberBand 控件
    // 滚动期间内部控件状态不可控，直接用 m_lastVisualRect 绘制更可靠
    if (m_boxSelectActive && m_lastVisualRect.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(viewport());
        opt.rect = m_lastVisualRect;
        opt.shape = QRubberBand::Rectangle;
        style()->drawControl(QStyle::CE_RubberBand, &opt, &p, viewport());
    }
}

void IconView::resizeEvent(QResizeEvent *e)
{
    //FIXME: first resize is disfluency.
    //but I have to reset the index widget in view's resize.
    QListView::resizeEvent(e);
    setIndexWidget(m_last_index, nullptr);
}

void IconView::wheelEvent(QWheelEvent *e)
{
    if ((e->modifiers() & Qt::ControlModifier)) {
        if (e->delta() > 0) {
            zoomLevelChangedRequest(true);
        } else {
            zoomLevelChangedRequest(false);
        }
        e->accept();
        return;
    }

    if (e->buttons() == Qt::LeftButton)
        this->viewport()->update();

    //垂直滚动动画
    bool isVerticalScroll = qAbs(e->angleDelta().y()) > qAbs(e->angleDelta().x());
    if (isVerticalScroll) {
        if (e->source() != Qt::MouseEventNotSynthesized) {
            //手指滑动
            // step = (e->angleDelta().y() / e->DefaultDeltasPerStep ) * gridSize().height();
            QListView::wheelEvent(e);
        } else {
            //鼠标滚轮
            QScrollBar *vBar = verticalScrollBar();
            int step, targetValue;

            // 获取滚轮角度变化，可在控制面板中设置，默认iconview为一行的1.5倍
            step = (e->angleDelta().y() / e->DefaultDeltasPerStep ) * gridSize().height() * 1.5;

            if (scrollAnimation->state() == QPropertyAnimation::Running) {
                scrollAnimation->stop();
                //上次滚动动画未执行完，累加动画执行的终点
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
        QListView::wheelEvent(e);
    }
}

void IconView::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);

    // 框选过程中，滚轮平滑动画会改变 scrollOffset，但不会触发 mouseMoveEvent。
    // 橡皮筋几何停留在旧 viewport 坐标，与"按下锚点"视觉错位，
    // 且选中项无法实时更新。这里用当前鼠标位置重算橡皮筋几何并更新选中。
    if (!m_boxSelectActive) {
        return;
    }
    if (state() == QAbstractItemView::DraggingState) {
        return;
    }
    if (!(QGuiApplication::mouseButtons() & Qt::LeftButton)) {
        return;
    }
    if (m_lastPressedLogicPoint.x() < 0 || m_lastPressedLogicPoint.y() < 0) {
        return;
    }

    const QPoint viewportPos = viewport()->mapFromGlobal(QCursor::pos());
    const QPoint scrollOffset(horizontalOffset(), verticalOffset());
    const QPoint currentLogicPos = viewportPos + scrollOffset;
    const QRect contentBandRect = QRect(currentLogicPos, m_lastPressedLogicPoint).normalized();
    const QRect viewportBandRect = contentBandRect.translated(-scrollOffset);

    // 更新视觉框选矩形并触发重绘（清除滚动 bitblt 产生的残影）
    QRect updateRect = viewportBandRect;
    if (m_lastVisualRect.isValid()) {
        updateRect = updateRect.united(m_lastVisualRect);
    }
    viewport()->update(updateRect.adjusted(-10, -10, 10, 10));
    m_lastVisualRect = viewportBandRect;

    // 同步选中项：滚轮平滑滚动期间不会触发 mouseMoveEvent，正常选中更新链路
    // （mouseMoveEvent -> QListView -> setSelection）被切断，导致橡皮筋经过的新项
    // 不会实时高亮。Ctrl 框选语义由 Qt 在鼠标事件中维护，此处不主动 setSelection 以免破坏其 toggle 逻辑。
    const Qt::KeyboardModifiers mods = QGuiApplication::queryKeyboardModifiers();
    if (!(mods & Qt::ControlModifier)) {
        setSelection(viewportBandRect, QItemSelectionModel::ClearAndSelect);
    }
}

void IconView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel*>(model());
    for (int row = start; row <= end; ++row) {
        QModelIndex sourceIndex = m_model->index(row, 0, parent);
        QModelIndex proxyIndex = proxyModel->mapFromSource(sourceIndex);

        if (proxyIndex.isValid()) {
            if (auto w = indexWidget(proxyIndex))
                w->hide();
            setIndexWidget(proxyIndex, nullptr);
        }
    }

    QListView::rowsAboutToBeRemoved(parent, start, end);
}
void IconView::updateGeometries()
{
#if QT_VERSION_CHECK(5, 15, 0)
    //try fix #55341
    int verticalValue = verticalOffset();

    QListView::updateGeometries();

    if (!model() || model()->columnCount() == 0 || model()->rowCount() == 0) {
        return;
    }

    int itemRowCount = model()->rowCount();
    auto lastIndex = model()->index(itemRowCount - 1, 0);
    QRegion itemRegion = visualRect(lastIndex);
    int lastItemBottom = itemRegion.boundingRect().bottom();

    if ((lastItemBottom + gridSize().height()) < viewport()->height()) {
        verticalScrollBar()->setRange(0, 0);
    } else {
        int vertiacalMax = verticalScrollBar()->maximum();
        verticalScrollBar()->setMaximum(vertiacalMax + gridSize().height());
        verticalScrollBar()->setValue(verticalValue);
    }

#else
    horizontalScrollBar()->setRange(0, 0);
    if (!model()) {
        verticalScrollBar()->setRange(0, 0);
        return;
    }

    if (model()->columnCount() == 0 || model()->rowCount() == 0) {
        verticalScrollBar()->setRange(0, 0);
        return;
    }

    int itemRowCount = model()->rowCount();
    auto lastIndex = model()->index(itemRowCount - 1, 0);
    QRegion itemRegion = visualRect(lastIndex);
    int lastItemBottom = itemRegion.boundingRect().bottom();

    if ((lastItemBottom + gridSize().height()) < viewport()->height()) {
        verticalScrollBar()->setRange(0, 0);
    } else {
        verticalScrollBar()->setSingleStep(gridSize().height()/2);
        verticalScrollBar()->setPageStep(viewport()->height());
        verticalScrollBar()->setRange(0, lastItemBottom - viewport()->height() + gridSize().height());
    }
#endif
}

void IconView::focusInEvent(QFocusEvent *e)
{
    QListView::focusInEvent(e);
    if (e->reason() == Qt::TabFocusReason) {
        if (selectedIndexes().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
        } else {
            scrollTo(selectedIndexes().first(), QListView::PositionAtCenter);
            auto selections = getSelections();
            clearSelection();
            //added for tab key to focus button issue
            //use uri rather than index, to fix crash bug#68788, 96145
            QTimer::singleShot(100, this, [=](){
               setSelections(selections);
            });
        }
    }
}

void IconView::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        QColor hlColor = palette().color(QPalette::Highlight);
        hlColor.setAlphaF(0.24);
        QBrush semiTransparentBrush(hlColor);
        setProperty("setSelectBrush", semiTransparentBrush);
        setProperty("setBranchSelectBrush", semiTransparentBrush);
    }
    QListView::changeEvent(e);
}

void IconView::startDrag(Qt::DropActions flags)
{
    // 拖拽启动时立即隐藏 tooltip，避免残留预览、避免进入 DraggingState
    // 后再次触发同步的 directory size 计算（du 在主线程阻塞最长 1500ms）。
    if (QToolTip::isVisible()) {
        QToolTip::hideText();
    }

    m_dragStarted = true;  // 标记已开始拖拽，避免 Qt 内部拖拽与手动检测重复触发
    auto indexes = selectedIndexes();
    if (indexes.count() > 0) {
        // 优先使用保存的拖拽开始位置（数位板的 QCursor::pos() 可能不准确）
        // 但当 m_dragStartPos 因异常路径未及时刷新而为非法值时，回退到实时光标，
        // 避免拖拽热点错位（如双击后立刻拖拽、外部触发的拖拽等场景）。
        QPoint pressPos = m_dragStartPos;
        if (pressPos.x() < 0 || pressPos.y() < 0) {
            pressPos = mapFromGlobal(QCursor::pos());
        }
        auto pos = pressPos;
        qreal scale = 1.0;
        QWidget *window = this->window();
        if (window) {
            auto windowHandle = window->windowHandle();
            if (windowHandle) {
                scale = windowHandle->devicePixelRatio();
            }
        }
        //拖拽开始时，重新监听ctrl快捷键是否按下，避免缓存干扰
        Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
        if (modifiers & Qt::ControlModifier) {
            m_ctrl_key_pressed = true;
        } else {
            m_ctrl_key_pressed = false;
        }

        auto drag = new QDrag(this);
        if(m_current_uri.startsWith("search://")){
            QMimeData* data = model()->mimeData(indexes);
            QVariant isSearchData = QVariant(true);
            data->setData("peony-qt/is-search", isSearchData.toByteArray());
            drag->setMimeData(data);
        } else if (m_current_uri.startsWith("smb://")) {
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

        int num = indexes.count();
        if (num > 100) {
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
               if (count > 10) {
                   break;
               }
               count++;
               iconRect.moveTo(iconRect.x()+3, iconRect.y()+3);
               QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
               if (!icon.isNull()) {
                   painter.save();
                   painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                   painter.drawPixmap(QPoint(100 + count*3, 100 + count*3), icon.pixmap(139, 139));
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

            for (auto index : indexes) {
                painter.save();
                QStyleOptionViewItem opt = viewOptions();
                auto viewItemDelegate = static_cast<IconViewDelegate *>(itemDelegate());
                viewItemDelegate->initIndexOption(&opt, index);

                opt.state |= QStyle::State_Selected;
                opt.rect.setSize(visualRect(index).size());
                painter.translate(indexRectHash.value(index).topLeft() - rect.boundingRect().topLeft());

                viewItemDelegate->setStartDrag(true);
                itemDelegate()->paint(&painter, opt, index);
                viewItemDelegate->setStartDrag(false);
                painter.restore();
            }
            drag->setPixmap(pixmap);
            drag->setHotSpot(pos - rect.boundingRect().topLeft());
            drag->setDragCursor(QPixmap(), m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
            drag->exec(m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
        }
    }

    // 从 IconViewIndexWidget 启动的拖拽不会触发 mouseReleaseEvent，
    // 必须在此处统一清理，否则多选状态会残留到下一次点击。
    m_multiSelectToggleHandled = false;
    m_toggleClickIndex = QModelIndex();
    m_noSelectOnPress = false;
    m_dragStarted = false;
}

void IconView::slotRename()
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
    QTimer::singleShot(300, m_renameTimer, [&]() {
        qDebug()<<"singleshot"<<m_editValid;
        //fix bug#98951, click edit box boarder will reenter edit issue
        if(m_editValid && ! m_delegate_editing) {
            m_renameTimer->stop();
            setIndexWidget(m_last_index, nullptr);
            edit(m_last_index);
            m_editValid = false;
        }
    });

}

bool IconView::getIgnore_mouse_move_event() const
{
    return m_ignore_mouse_move_event;
}

void IconView::setIgnore_mouse_move_event(bool ignore_mouse_move_event)
{
    m_ignore_mouse_move_event = ignore_mouse_move_event;
}

void IconView::bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel)
{
    // fix: #239734 filedialog has dirty view region
    if (topLevelWidget()->objectName() != "_peony_mainwindow")
        setFrameShape(QFrame::NoFrame);

    m_model = sourceModel;
    m_sort_filter_proxy_model = proxyModel;

    // 图标视图复选框绑定在 column 0。列表视图的 bindModel 会将 checkboxColumn
    // 设为独立列（如 5），由于 model/proxy 在视图间共用，此处需重置为 0。
    proxyModel->setCheckboxColumn(0);

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

    setModel(m_sort_filter_proxy_model);
    m_sort_filter_proxy_model->setSelectionModel(selectionModel());

    //edit trigger
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selection, const QItemSelection &deselection) {
        //qDebug()<<"selection changed";
        // Avoid expanding a Ctrl+A selection into thousands of QModelIndex
        // objects when only the single-item case needs special handling.
        const bool singleItemSelection = selection.count() == 1
                && selection.first().top() == selection.first().bottom()
                && selection.first().left() == selection.first().right();

        if (m_last_index.isValid() && deselection.contains(m_last_index)) {
            /* 解决：旧widget未及时delete还能接收鼠标事件，导致图标状态不对也选不中；link to bug#225660. */
            auto widget = this->indexWidget(m_last_index);
            if (widget) {
                widget->hide();
            }//end
            this->setIndexWidget(m_last_index, nullptr);
        }

        if (state() == QListView::DragSelectingState)
            m_allow_set_index_widget = false;

        //Q_EMIT m_proxy->viewSelectionChanged();
        if (singleItemSelection) {
            const auto currentIndex = selection.first().topLeft();
            qDebug()<<"m_last_index  "<<(m_last_index == currentIndex)<<currentIndex;
            if(m_last_index != currentIndex)
            {
                m_editValid = false;
            }
            m_last_index = currentIndex;
            //qDebug()<<"IconView::bindModel:"<<"selection changed: "<<"resetEditTriggerTimer";
            //this->resetEditTriggerTimer();
        } else {
            m_last_index = QModelIndex();
            m_editValid = false;
        }


        //qDebug()<<"selection changed2"<<m_editValid;
    });
}

void IconView::setProxy(DirectoryViewProxyIface *proxy)
{
    if (!proxy)
        return;

    m_proxy = proxy;
    if (!m_proxy) {
        return;
    }

    //connect(m_model, &FileItemModel::dataChanged, this, &IconView::clearIndexWidget);
    //connect(m_model, &FileItemModel::updated, this, &IconView::resort);

    connect(m_model, &FileItemModel::findChildrenFinished,
            this, &IconView::reportViewDirectoryChanged);

    connect(this, &IconView::activated, [=](const QModelIndex &index) {
        qDebug()<<"double click"<<index.data(FileItemModel::UriRole);
        //when selections is more than 1, let mainwindow to process
        if (getSelections().count() != 1)
            return;
        auto uri = getSelections().first();
        //process open symbolic link lllll
        //auto info = FileInfo::fromUri(uri);
        //if (info->isSymbolLink() && uri.startsWith("file://") && info->isValid())
          //  uri = "file://" + FileUtils::getSymbolicTarget(uri);
        Q_EMIT m_proxy->viewDoubleClicked(uri);
    });



    //menu
//    connect(this, &IconView::customContextMenuRequested, [=](const QPoint &pos){
//        if (!indexAt(pos).isValid())
//            this->clearSelection();

//        //NOTE: we have to ensure that we have cleared the
//        //selection if menu request at blank pos.
//        QTimer::singleShot(1, [=](){
//            Q_EMIT this->getProxy()->menuRequest(QCursor::pos());
//        });
//    });
}

// NOTE: When icon view was resorted,
// index widget would deviated from its normal position by somehow.
// So, do not set any index widget when the resorting.
void IconView::resort()
{
    //fix uncompress selected file will cover file before it issue
    clearIndexWidget();
    if (m_last_index.isValid()) {
        this->setIndexWidget(m_last_index, nullptr);
    }

    if (m_sort_filter_proxy_model)
        m_sort_filter_proxy_model->sort(getSortType(), Qt::SortOrder(getSortOrder()));
}

void IconView::reportViewDirectoryChanged()
{
    if (m_proxy)
        Q_EMIT m_proxy->viewDirectoryChanged();
}

QRect IconView::visualRect(const QModelIndex &index) const
{
    auto rect = QListView::visualRect(index);
    rect.setX(rect.x() + 10);
    rect.setY(rect.y() + 15);
    auto size = itemDelegate()->sizeHint(QStyleOptionViewItem(), index);
    rect.setSize(size);
    return rect;
}

bool IconView::getDelegateEditFlag()
{
    return m_delegate_editing;
}

int IconView::getSortType()
{
    int type = m_sort_filter_proxy_model->expectedSortType();
    return type<0? 0: type;
}

void IconView::setSortType(int sortType)
{
    m_sort_filter_proxy_model->sort(sortType, Qt::SortOrder(getSortOrder()));
}

int IconView::getSortOrder()
{
    return m_sort_filter_proxy_model->expectedSortOrder();
}

void IconView::setSortOrder(int sortOrder)
{
    m_sort_filter_proxy_model->sort(getSortType(), Qt::SortOrder(sortOrder));
}

const QStringList IconView::getAllFileUris()
{
    return m_sort_filter_proxy_model->getAllFileUris();
}

const int IconView::getAllDisplayFileCount()
{
    return m_sort_filter_proxy_model->rowCount();
}

void IconView::editUri(const QString &uri)
{
    setState(QListView::NoState);
    auto origin = FileUtils::getOriginalUri(uri);
    if(uri.startsWith("mtp://") || uri.startsWith("smb://"))/* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
        origin = uri;
    QModelIndex index = m_sort_filter_proxy_model->indexFromUri(origin);
    setIndexWidget(index, nullptr);
    qDebug() <<"editUri:" <<uri <<origin;
    QListView::scrollTo(index);
    edit(index);
//    if (! m_delegate_editing)
//        edit(m_sort_filter_proxy_model->indexFromUri(origin));
}

void IconView::editUris(const QStringList uris)
{
    //FIXME:
    //implement batch rename.
    setState(QListView::NoState);
    auto origin = FileUtils::getOriginalUri(uris.first());
    if(uris.first().startsWith("mtp://"))/* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
        origin = uris.first();
    QModelIndex index = m_sort_filter_proxy_model->indexFromUri(origin);
    setIndexWidget(index, nullptr);
    qDebug() << "editUris:" << uris << origin;
    QListView::scrollTo(index);
    edit(index);
}

void IconView::selectAll()
{
    // fix: #62397
//    for (int i = 0; i < model()->rowCount(); i++) {
//        selectionModel()->select(model()->index(i, 0), QItemSelectionModel::Select);
//    }
    // optimize selectAll(). do not trigger selection changed signal to many times.
    QItemSelection selection(model()->index(0, 0), model()->index(model()->rowCount() - 1, 0));
    selectionModel()->select(selection, QItemSelectionModel::Select);
}

void IconView::clearIndexWidget()
{
    for (int i = 0; i < m_sort_filter_proxy_model->rowCount(); i++) {
        auto index = m_sort_filter_proxy_model->index(i, 0);
        setIndexWidget(index, nullptr);
        closePersistentEditor(index);
    }
}

void IconView::multiSelect()
{
    if (m_multi_select) {
        return;
    }
    if (m_sort_filter_proxy_model->getSelectionModeHint() != NoSelection) {
        return;
    }
    if (!GlobalSettings::getInstance()->getValue(SHOW_MULTISELECT_CHECKBOX).toBool())
        return;

    int savedScroll = verticalScrollBar()->value();
    m_multi_select = true;
    m_sort_filter_proxy_model->setMultiSelectMode(true);
    // 初始化 Shift 区间锚点：优先用 currentIndex，否则用第一个选中项
    m_multiSelectAnchor = currentIndex();
    if (!m_multiSelectAnchor.isValid()) {
        const auto sel = selectedIndexes();
        if (!sel.isEmpty()) {
            m_multiSelectAnchor = sel.first();
        }
    }
    verticalScrollBar()->setValue(qMin(savedScroll, verticalScrollBar()->maximum()));
    viewport()->update(viewport()->rect());
    Q_EMIT updateSelectStatus(true);
}

void IconView::disableMultiSelect()
{
    if (!m_multi_select) {
        return;
    }
    if (m_sort_filter_proxy_model->getSelectionModeHint() != NoSelection) {
        return;
    }
    int savedScroll = verticalScrollBar()->value();
    m_multi_select = false;
    m_sort_filter_proxy_model->setMultiSelectMode(false);
    m_multiSelectAnchor = QModelIndex();
    // 先同步工具栏状态，避免 clearSelection 触发 updateToolBar 时
    // 因 model URI 尚未更新被误判为「切换视图」而重新开启多选
    Q_EMIT updateSelectStatus(false);
    // 取消多选模式时，清空所有已选中项
    clearSelection();
    verticalScrollBar()->setValue(qMin(savedScroll, verticalScrollBar()->maximum()));
    viewport()->update(viewport()->rect());
}

void IconView::setSearchKey(const QString &key)
{
    auto viewItemDelegate = static_cast<IconViewDelegate *>(itemDelegate());
    viewItemDelegate->setSearchKeyword(key);
}

void IconView::edit(const QModelIndex &index)
{
    QListView::edit(index);
}

bool IconView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    return  QListView::edit(index, trigger, event);
}

void IconView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QListView::setSelection(rect, command);
}

QItemSelectionModel::SelectionFlags IconView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    if (!event)
        return QListView::selectionCommand(index, event);

    // 当 IndexWidget 已处理多选切换时，不要让 QListView 执行默认选择行为
    if (m_multiSelectToggleHandled) {
        return QItemSelectionModel::NoUpdate;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<const QMouseEvent *>(event);
        const bool indexSelected = index.isValid() && selectionModel()->isSelected(index);
        if (e->button() == Qt::LeftButton && m_multi_select && !(e->modifiers() & Qt::ControlModifier)) {
            if (index.isValid()) {
                return indexSelected ? QItemSelectionModel::NoUpdate : QItemSelectionModel::Select;
            }
        }
        if (e->button() == Qt::LeftButton && indexSelected && (e->modifiers() & Qt::ControlModifier)) {
            return QItemSelectionModel::NoUpdate;
        }
        // Ctrl/Shift+点击空白处不取消已有选中
        if (e->button() == Qt::LeftButton && !index.isValid() && (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
            return QItemSelectionModel::NoUpdate;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto e = static_cast<const QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && m_multi_select) {
            return QItemSelectionModel::NoUpdate;
        }
        if (e->button() == Qt::LeftButton && m_noSelectOnPress) {
            return QItemSelectionModel::NoUpdate;
        }
    }
    return QListView::selectionCommand(index, event);
}

void IconView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QListView::closeEditor(editor,hint);
}

void IconView::doMultiSelect(bool isMultiSlelect)
{
    if (isMultiSlelect) {
        multiSelect();
    } else {
        disableMultiSelect();
    }
}

void IconView::setItemsVisible(bool visible)
{
    viewport()->setVisible(visible);
}

bool IconView::isEnableMultiSelect()
{
    return m_multi_select;
}

bool IconView::isRenameSecondClick(const QModelIndex &index) const
{
    if (m_multi_select)
        return false;

    if (!index.isValid() || !m_last_index.isValid() || !m_renameTimer->isActive())
        return false;

    if (index.row() != m_last_index.row())
        return false;

    const int remaining = m_renameTimer->remainingTime();
    const int renameWindow = 3000 - qApp->styleHints()->mouseDoubleClickInterval();
    return remaining > 0 && remaining < renameWindow;
}

void IconView::releaseUnselect(bool select)
{
    m_mouse_release_unselect = select;
}

//Icon View 2
IconView2::IconView2(QWidget *parent) : DirectoryViewWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setObjectName("_iconview2_layout");
    layout->setMargin(0);
    layout->setSpacing(0);
    m_view = new IconView(this);

    QString version = qApp->property("version").toString();
    if (version == "ukui3.0") {
        m_zoom_level = 25;
    }

    DirectoryViewHelper * viewHelper = DirectoryViewHelper::globalInstance();
    viewHelper->addIconViewWithDirectoryViewWidget(m_view, this);
    connect(m_view, &IconView::updateSelectStatus, viewHelper, &DirectoryViewHelper::updateSelectStatus);

    // int defaultZoomLevel = GlobalSettings::getInstance()->getValue(DEFAULT_VIEW_ZOOM_LEVEL).toInt();
    // if (defaultZoomLevel >= minimumZoomLevel() && defaultZoomLevel <= maximumZoomLevel())
    //     m_zoom_level = defaultZoomLevel;

    connect(m_view, &IconView::zoomLevelChangedRequest, this, &IconView2::zoomRequest);

    layout->addWidget(m_view);

    setLayout(layout);
}

IconView2::~IconView2()
{
    // 从DirectoryViewHelper中移除，避免悬空指针
    DirectoryViewHelper::globalInstance()->removeView(this);
}

int IconView2::currentZoomLevel()
{
    int defaultZoomLevel = FileUtils::getViewZoomLevel(m_view->m_current_uri);
    if (defaultZoomLevel >= minimumZoomLevel() && defaultZoomLevel <= maximumZoomLevel())
        m_zoom_level = defaultZoomLevel;

    return m_zoom_level;
}

void IconView2::bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel)
{
    auto layout = findChild<QVBoxLayout *>("_iconview2_layout");
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

    m_view->bindModel(model, proxyModel);
    connect(m_model, &FileItemModel::selectRequest, this, &DirectoryViewWidget::updateWindowSelectionRequest);
    connect(m_model,&FileItemModel::signal_itemAdded, this, [=](const QString& uri){
        Q_EMIT this->signal_itemAdded(uri);
    });
    connect(model, &FileItemModel::findChildrenFinished, this, &DirectoryViewWidget::viewDirectoryChanged);
    //connect(m_model, &FileItemModel::dataChanged, m_view, &IconView::clearIndexWidget);
    //connect(m_model, &FileItemModel::updated, m_view, &IconView::resort);
    connect(m_model, &FileItemModel::updated, m_view->viewport(), [=]{
        if (this->cursor().shape() == Qt::BusyCursor || this->cursor().shape() == Qt::WaitCursor) {
            return;
        }
        repaintView();
    });

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]() {
        Q_EMIT viewSelectionChanged();
    });

    connect(m_view, &IconView::activated, this, [=](const QModelIndex &index) {
        //平板模式下，长按打开文件处理
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
        //process open symbolic link
       // auto info = FileInfo::fromUri(uri);
       // if (info->isSymbolLink() && uri.startsWith("file://") && info->isValid())
        //    uri = "file://" +  FileUtils::getSymbolicTarget(uri);
        Q_EMIT this->viewDoubleClicked(uri);

        m_view->m_touch_active_timer->stop();
    });

    connect(m_view, &IconView::quickLookRequested, this, [=](const QModelIndex &) {
        if (getSelections().count() != 1)
            return;
        Q_EMIT this->quickLookRequested(getSelections().first());
    });

    connect(m_view, &IconView::customContextMenuRequested, this, [=](const QPoint &pos) {
        if (m_view->m_delegate_editing) {
            return;
        }

        m_menuRequesting = true;

        // we should clear the dirty rubber band due to call context menu.
        bool isDragSelecting = m_view->isDraggingState();
        if (isDragSelecting) {
            // send a fake mouse release event for clear the rubber band.
            // note that we should pass mouse move event durring delaying requesting
            // directory view menu, otherwize both dnd and menu action will be triggered.
            m_view->setIgnore_mouse_move_event(true);
            QMouseEvent fakeEvent(QMouseEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            qApp->sendEvent(m_view, &fakeEvent);
            m_view->viewport()->repaint();
        }

        bool indexWidgetContainsPos = false;
        auto indexWidget = m_view->indexWidget(m_view->currentIndex());
        if (indexWidget) {
            indexWidgetContainsPos = indexWidget->geometry().contains(pos);
        }
        if (!m_view->indexAt(pos).isValid() && !indexWidgetContainsPos)
            m_view->clearSelection();

        //NOTE: we have to ensure that we have cleared the
        //selection if menu request at blank pos.
        QTimer::singleShot(isDragSelecting? 300: 1, this, [=]() {
            m_view->setIgnore_mouse_move_event(false);
            m_view->m_touch_active_timer->stop();
            Q_EMIT this->menuRequest(mapToGlobal(pos));
            m_menuRequesting = false;
        });
    });

    connect(m_proxy_model, &FileItemProxyFilterSortModel::layoutChanged, this, [=]() {
        Q_EMIT this->sortOrderChanged(Qt::SortOrder(getSortOrder()));
    });
    connect(m_proxy_model, &FileItemProxyFilterSortModel::layoutChanged, this, [=]() {
        Q_EMIT this->sortTypeChanged(getSortType());
    });
}

void IconView2::repaintView()
{
    if (m_menuRequesting)
        return;
    m_view->update();
    m_view->viewport()->update();
}

void IconView2::setCurrentZoomLevel(int zoomLevel)
{
    if (zoomLevel <= maximumZoomLevel() && zoomLevel >= minimumZoomLevel()) {
        m_zoom_level = zoomLevel;
        //FIXME: implement zoom
        int base = 64 - 25; //50
        QString version = qApp->property("version").toString();
        if (version == "ukui3.0") {
             base = 64 - 25;
        }
        int adjusted = base + zoomLevel;
        m_view->setIconSize(QSize(adjusted, adjusted));
        m_view->setGridSize(m_view->itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex()) + QSize(20, 20));
    }
}

void IconView2::clearIndexWidget()
{
    for (auto index : m_proxy_model->getAllFileIndexes()) {
        m_view->closePersistentEditor(index);
        m_view->setIndexWidget(index, nullptr);
    }
}
