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

#include "search-view.h"
#include "file-utils.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <glib.h>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <file-info.h>
#include <file-info-job.h>
#include "search-file-item.h"
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QWindow>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QStyleHints>
#include <QScrollBar>
#define LISTVIEW_ITEM_BORDER_RADIUS 6

SearchView::SearchView(QWidget *parent): QTreeView(parent)
{
    m_renameTimer = new QTimer(this);
    m_renameTimer->setInterval(3000);
    m_renameTimer->setSingleShot(true);

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setObjectName("_searchview_layout");
    setProperty("highlightMode", true);
    //setFrameShape(QFrame::NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setAutoScroll(true);
    setAutoScrollMargin(100);
    auto cornerWidget = new QWidget;
    cornerWidget->setObjectName("_listview_corner");
    cornerWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    cornerWidget->setBackgroundRole(QPalette::Base);
    cornerWidget->setAutoFillBackground(true);
    setCornerWidget(cornerWidget);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSelectionBehavior(QTreeView::SelectRows);
    setAlternatingRowColors(true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    header()->setSectionResizeMode(QHeaderView::Interactive);
    header()->setSectionsMovable(true);
    header()->setStretchLastSection(false);
    header()->setMinimumSectionSize(130);
    header()->setTextElideMode(Qt::ElideRight);

    connect(header(), &QHeaderView::sectionResized, this, [=]{

    });

    setExpandsOnDoubleClick(false);
    setSortingEnabled(true);

    setEditTriggers(QTreeView::NoEditTriggers);
    setDragEnabled(true);
    setDragDropMode(QTreeView::DragDrop);
    setSelectionMode(QTreeView::ExtendedSelection);

    setUniformRowHeights(true);
    setIconSize(QSize(40, 40));
    setMouseTracking(true);//追踪鼠标
    setViewportMargins(0, 0, 0, 0);
    m_labelAlignment = GlobalSettings::getInstance()->getValue(LABLE_ALIGNMENT).toInt();
}

SearchView::~SearchView()
{
    m_selectedIndexes.clear();
    m_selectionInfos.clear();
}

QModelIndexList SearchView::getSelectedIndexes()
{
    return this->selectionModel()->selectedRows();
}

void SearchView::startDrag(Qt::DropActions flags)
{
    auto indexes = selectedIndexes();
    m_isDraging = true;
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
        QMimeData* data = model()->mimeData(indexes);
        QVariant isSearchData = QVariant(true);
        data->setData("peony-qt/is-search", isSearchData.toByteArray());
        //临时添加属性解决无法拖拽侧边栏问题，后续model层完善后，触发flags可去除下面代码
        QByteArray encodeData;
        QDataStream stream(&encodeData, QIODevice::WriteOnly);
        data->setData("application/x-qabstractitemmodeldatalist", encodeData);

        drag->setMimeData(data);
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
            drag->setDragCursor(QPixmap(), Qt::MoveAction);
            drag->exec(Qt::MoveAction);
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
                auto viewItemDelegate = static_cast<TreeviewDelegate *>(itemDelegate());
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
            drag->setDragCursor(QPixmap(), Qt::MoveAction);
            drag->exec(Qt::MoveAction);
        }
    }
}

void SearchView::invertSelections()
{
    auto selectionModel = this->selectionModel();
    if (!selectionModel) return;
    // 获取当前选中行
    auto selectedRows = selectionModel->selectedRows();  
    // 创建新选中区域
    QItemSelection newSelection;  
    // 遍历所有行进行反选
    auto proxmodel = qobject_cast<SearchFilterProxyModel *>(model());
    auto searchmodel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());

    for (int row = 0; row < model()->rowCount(); ++row) {
        QModelIndex catIndex = model()->index(row, 0, QModelIndex());
        for (int j = 0; j < model()->rowCount(catIndex); ++j) {
            QModelIndex childIndex = model()->index(j, 0, catIndex);
            if (!selectedRows.contains(childIndex)) {
                if(row == 1 && searchmodel->isDefaultContentItem(proxmodel->mapToSource(childIndex))){
                    continue;
                }
                int columnCount = model()->columnCount(childIndex.parent());
                QModelIndex lastColumnIndex = model()->index(childIndex.row(), columnCount - 1, childIndex.parent());
                newSelection.select(childIndex, lastColumnIndex);
            }
        }
    }
    // 应用反选
    selectionModel->select(newSelection, QItemSelectionModel::ClearAndSelect);
}

void SearchView::bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel)
{
    return;
}

void SearchView::setProxy(DirectoryViewProxyIface *proxy)
{
    return;
}

const QString SearchView::getDirectoryUri()
{
    return "";
}

const QStringList SearchView::getSelections()
{
    return QStringList();
}

const QStringList SearchView::getAllFileUris()
{
    return QStringList();
}

void SearchView::open(const QStringList &uris, bool newWindow)
{
    return;
}

void SearchView::setDirectoryUri(const QString &uri)
{
    return;
}

void SearchView::beginLocationChange()
{
    return;
}

void SearchView::stopLocationChange()
{
    return;
}

void SearchView::closeView()
{
    return;
}

void SearchView::setSelections(const QStringList &uris)
{
    clearSelection();
    QItemSelection selection;
    SearchFilterProxyModel *proxmodel = static_cast<SearchFilterProxyModel*>(model());
    auto FileItemModel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());
    auto nameItem =  FileItemModel->getFileNameItem();
    auto contentItem  = FileItemModel->getFileContentItem();
    for (auto uri: uris) {
        const QModelIndex index = FileItemModel->indexFromUri(uri,nameItem);
        if (index.isValid()) {
            QItemSelection selectionToBeMerged(index, index);
            selection.merge(selectionToBeMerged, QItemSelectionModel::Select);
        }
        const QModelIndex contentIndex = FileItemModel->indexFromUri(uri,contentItem);
        if (contentIndex.isValid()) {
            QItemSelection selectionToBeMerged(contentIndex, contentIndex);
            selection.merge(selectionToBeMerged, QItemSelectionModel::Select);
        }
    }
    auto flags = QItemSelectionModel::Select|QItemSelectionModel::Rows;
    selectionModel()->select(selection, flags);
    return;
}

void SearchView::scrollToSelection(const QString &uri)
{
    return;
}

void SearchView::setCutFiles(const QStringList &uris)
{
    return;
}

DirectoryViewProxyIface *SearchView::getProxy()
{
    return nullptr;
}

int SearchView::getSortType()
{
    return 0;
}

void SearchView::setSortType(int sortType)
{
    return;
}

int SearchView::getSortOrder()
{
    return 0;
}

void SearchView::setSortOrder(int sortOrder)
{
    return;
}

void SearchView::editUri(const QString &uri)
{
    setState(QTreeView::NoState);

    auto selected = selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        qWarning() << "No selection";
        return;
    }
    auto proxmodel = qobject_cast<SearchFilterProxyModel *>(model());
    auto searchmodel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());
    QModelIndex index = selected.first();
    auto id = proxmodel->mapToSource(index).internalId();
    if(searchmodel->isContentItem(index) && searchmodel->hasDefaultContent()){
        return;
    }
    auto fileUri = index.data(Qt::UserRole).toString();
    queryFileinfo(fileUri);
    edit(index);
}

void SearchView::editUris(const QStringList uris)
{
    this->editUri(uris.first());
}

void SearchView::doMultiSelect(bool)
{
    return;
}

bool SearchView::isEnableMultiSelect()
{
    return true;
}

void SearchView::setItemsVisible(bool visible)
{
    return;
}

const int SearchView::getAllDisplayFileCount()
{
    auto proxyModel = qobject_cast<SearchFilterProxyModel*>(this->model());
    int count = 0;
    if (!proxyModel) {
        return count;
    }
    QModelIndex root = QModelIndex();

    for(int i = 0; i < proxyModel->rowCount(root); ++i) {
        QModelIndex catIndex = proxyModel->index(i, 0, root);
        int childcount = 0;
        if (catIndex.isValid()) {
            childcount = proxyModel->rowCount(catIndex);
        }
        for(int j=0;j<childcount;j++){
            QModelIndex childIndex = proxyModel->index(j,0, catIndex);
            auto realindex = proxyModel->mapToSource(childIndex);
            SearchFileItem *item = static_cast<SearchFileItem*>(realindex.internalPointer());
            if(item!=nullptr && item->m_info != nullptr){
                count++;
            }
        }
    }
    qDebug() << __func__ << __LINE__ << count;
    return count;
}

void SearchView::mousePressEvent(QMouseEvent *e)
{
    m_editValid = true;

    // 普通点击时不过滤，让默认逻辑处理
    // 只在框选时才过滤不可选项
    auto proxmodel = qobject_cast<SearchFilterProxyModel *>(model());
    auto searchmodel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());
    if (e->button() == Qt::LeftButton) {
        m_isSelecting = false;
        searchmodel->setSelecting(m_isSelecting);
    }

    QTreeView::mousePressEvent(e);
    m_selectedIndexes = this->selectionModel()->selectedRows();
    if (e->button() == Qt::RightButton) {
        //m_selectionInfos.clear();
        // for (auto index : m_selectedIndexes) {
        //     auto uri = index.data(Qt::UserRole).toString();
        //     queryFileinfo(uri);
        //  }
        QPoint mousePressPos = e->pos();
        if (!indexAt(mousePressPos).isValid()) {
            clearSelection();
        }

        QTimer::singleShot(50, this, [=]() {
            Q_EMIT menuRequest(viewport()->mapToGlobal(mousePressPos));
        });
        return;
    }

    if(!m_renameTimer->isActive()) {
        m_renameTimer->start();
        m_editValid = false;
    } else {
        //if remain time is between[0.75, 3000],then trigger rename event;
        //to make sure only click one row

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
        //qDebug()<<m_renameTimer->remainingTime()<<m_editValid<<all_index_in_same_row<<qApp->styleHints()->mouseDoubleClickInterval();
        //qDebug()<<"m_last_index:"<<m_last_index<<"indexAt(e->pos()):"<<indexAt(e->pos());
        //优化文件点击策略，提升用户体验，关联bug#125368
        //在双击时间间隔内，如果未触发双击事件，但是点击的是同一个有效图标，触发双击事件
        //系统默认双击间隔为400ms, 策略为[0,400]，触发双击，(400,3000)触发重命名      
        if(m_renameTimer->remainingTime()> 0 && m_renameTimer->remainingTime() < 3000 - qApp->styleHints()->mouseDoubleClickInterval()
            && indexAt(e->pos()) == m_last_index && m_last_index.isValid() && m_editValid == true && all_index_in_same_row) {
            auto index = proxmodel->mapToSource(indexAt(e->pos()));
            if(searchmodel->isContentItem(index) && searchmodel->hasDefaultContent()){
                return;
            }
            slotRename();
        } else {
            m_editValid = false;
        }
    }
}

void SearchView::mouseMoveEvent(QMouseEvent *e)
{
    QTreeView::mouseMoveEvent(e);
    //处理框选逻辑
    if ((e->buttons() & Qt::LeftButton) && !m_isSelecting) {
        // 检查是否开始拖拽选择（移动距离超过阈值）
        if ((e->pos() - m_dragStartPosition).manhattanLength() >= QApplication::startDragDistance()) {
            m_isSelecting = true;
            auto proxmodel = qobject_cast<SearchFilterProxyModel *>(model());
            auto searchmodel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());
            searchmodel->setSelecting(m_isSelecting);
        }
    }
}

void SearchView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_isSelecting) {
        m_isSelecting = false;
        auto proxmodel = qobject_cast<SearchFilterProxyModel *>(model());
        auto searchmodel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());
        //m_selectionRect = QRect();
        //viewport()->update();
    }

    QTreeView::mouseReleaseEvent(e);
}

void SearchView::updateGeometries()
{
    setUpdatesEnabled(false);
    QTreeView::updateGeometries();
    reUpdateScrollBar();
    setUpdatesEnabled(true);
}

void SearchView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if (!model()) {
      painter->restore();
      return QTreeView::drawRow(painter, option, index);
    }
    auto clipedUris = Peony::ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()) {
      QString clipedFilesParentUri = ClipboardUtils::getClipedFilesParentUri();
      if (clipedUris.contains(index.data(Qt::UserRole).toString())) {
          painter->setOpacity(0.5);
      } else {
          painter->setOpacity(1.0);
      }
    }
    QTreeView::drawRow(painter, option, index);
    painter->restore();
}

void SearchView::reUpdateScrollBar()
{
    if (!model())
        return;

    if (model()->rowCount() == 0) {
        return;
    }

    int rowCount = model()->rowCount();
    auto index = model()->index(0, 0);
    int fileNameCount = model()->rowCount(model()->index(0, 0));
    int fileContentCount = model()->rowCount(model()->index(1, 0));
    rowCount += (fileNameCount + fileContentCount);
    int totalHeight = sizeHintForIndex(index).height()*rowCount;

    int currentScrollBarValue = verticalScrollBar()->value();
    verticalScrollBar()->setSingleStep(iconSize().height());
    verticalScrollBar()->setPageStep(viewport()->height() - header()->height());
    verticalScrollBar()->setRange(0, totalHeight + header()->height()+100 - viewport()->height());
    verticalScrollBar()->setValue(currentScrollBarValue);
}

void SearchView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_UNUSED(index)
    Q_UNUSED(hint)
    QTreeView::scrollTo(index,hint);
    reUpdateScrollBar();
}

void SearchView::slotRename()
{
    //standardPaths not allow rename
    auto currentSelections = getSelections();
    bool hasStandardPath = FileUtils::containsStandardPath(currentSelections);
    if (hasStandardPath)
        return;

    //delay edit action to avoid doubleClick or dragEvent
    qDebug()<<"slotRename"<<m_editValid;
    QTimer::singleShot(300, this, [&]() {
        if(m_isDraging){
            m_isDraging = false;
            return;
        }
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

void SearchView::queryFileinfo(QString uri)
{
    m_selectionInfos.clear();
    auto info = Peony::FileInfo::fromUri(uri);
    if (info->isEmptyInfo()) {
        Peony::FileInfoJob j(info);
        j.querySync();
    }
    m_selectionInfos.append(info);
}

void SearchView::getSelectedIndex(const QItemSelection &selected, const QItemSelection &deselected)
{
    //qDebug()<<"list view selection changed"<<m_delegate_editing;
    //continue to fix bug#89540，98951
    auto currentSelections = selected.indexes();
    if (m_delegate_editing){
        //fix bug#194642, list view can not trigger rename issue
        if (! currentSelections.isEmpty()) {
            m_last_index = currentSelections.first();
        }
        return;
    }
    for (auto index : deselected.indexes()) {
        this->setIndexWidget(index, nullptr);
    }

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
            if(m_last_index.row() != currentSelections.first().row()) {
                m_editValid = false;
            }
            m_last_index = currentSelections.first();
        }
    } else {
        m_last_index = QModelIndex();
        m_editValid = false;
    }
}

void SearchView::keyPressEvent(QKeyEvent *e)
{
    QTreeView::keyPressEvent(e);
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return){
        auto selected = selectionModel()->selectedRows();
        if (selected.isEmpty()) {
          return;
        }
        Q_EMIT QTreeView::doubleClicked(selected.first());/* 与按下enter键效果一样 */
    }
}

bool SearchView::isDragging()
{
    return state() == QAbstractItemView::DraggingState;
}

void SearchView::setLabelAlignment(int alignment)
{
    m_labelAlignment = alignment;
}

int SearchView::getLabelAlignment() const
{
    return m_labelAlignment;
}
