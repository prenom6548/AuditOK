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

#include "checkbox-header.h"
#include "global-settings.h"
#include <QCheckBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QTreeView>

using namespace Peony;

CheckBoxHeader::CheckBoxHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);

    m_checkBox = new QCheckBox(this);
    m_checkBox->setVisible(false);
    connect(m_checkBox, &QCheckBox::stateChanged, this, [this](int state) {
        QSignalBlocker blocker(m_checkBox);
        setCheckState(static_cast<Qt::CheckState>(state));
    });
}

CheckBoxHeader::~CheckBoxHeader()
{
}

void CheckBoxHeader::setCheckState(Qt::CheckState state)
{
    //视图列表部分选中 checkbox为不选中
    Qt::CheckState visualState = (state == Qt::PartiallyChecked) ? Qt::Unchecked : state;
    if (m_checkState != visualState) {
        m_checkState = visualState;
        if (m_checkBox) {
            QSignalBlocker blocker(m_checkBox);
            m_checkBox->setCheckState(visualState);
            m_checkBox->update();
        }
        updateCheckBoxPosition();
        updateSection(0);
        Q_EMIT checkStateChanged(visualState);
    } else if (m_checkBox && m_checkBox->checkState() != visualState) {
        QSignalBlocker blocker(m_checkBox);
        m_checkBox->setCheckState(visualState);
        m_checkBox->update();
        updateCheckBoxPosition();
        updateSection(0);
    }
}

void CheckBoxHeader::updateCheckState(int selectedCount, int totalCount)
{
    if (!m_checkboxEnabled) {
        return;
    }

    Qt::CheckState newState;
    if (selectedCount == 0) {
        newState = Qt::Unchecked;
    } else if (selectedCount == totalCount && totalCount > 0) {
        newState = Qt::Checked;
    } else {
        newState = Qt::PartiallyChecked;
    }

    setCheckState(newState);
}

void CheckBoxHeader::setCheckBoxEnabled(bool enabled)
{
    m_checkboxEnabled = enabled;
    updateCheckBoxPosition();
    // QHeaderView 的 section 绘制在 viewport 上，QWidget::update() 不会触发
    // viewport 重绘，必须用 updateSection 才能正确刷新 section 内容
    if (model() && model()->columnCount() > 0) {
        updateSection(0);
    }
}

void CheckBoxHeader::updateCheckBoxPosition()
{
    if (!m_checkBox) {
        return;
    }

    if (!m_checkboxEnabled || !model() || model()->columnCount() == 0) {
        m_checkBox->setVisible(false);
        return;
    }

    const QRect sectionRect(sectionViewportPosition(0), 0, sectionSize(0), height());
    const QRect checkRect = getHeaderCheckBoxRect(sectionRect);

    if (checkRect.isValid()) {
        m_checkBox->setGeometry(checkRect);
        m_checkBox->setVisible(true);
        QSignalBlocker blocker(m_checkBox);
        m_checkBox->setCheckState(m_checkState);
    } else {
        m_checkBox->setVisible(false);
    }
}

void CheckBoxHeader::updateGeometries()
{
    QHeaderView::updateGeometries();
    updateCheckBoxPosition();
}

void CheckBoxHeader::resizeEvent(QResizeEvent *event)
{
    QHeaderView::resizeEvent(event);
    updateCheckBoxPosition();
}

bool CheckBoxHeader::shouldPaintCheckBox(int logicalIndex) const
{
    if (!m_checkboxEnabled) {
        return false;
    }

    // 只有第一列显示复选框
    return logicalIndex == 0;
}

int CheckBoxHeader::getDataRowTextLeft() const
{
    QTreeView *treeView = qobject_cast<QTreeView*>(parentWidget());
    if (!treeView || !treeView->model() || treeView->model()->rowCount() == 0)
        return -1;

    QModelIndex firstIndex = treeView->model()->index(0, 0);
    QRect itemRect = treeView->visualRect(firstIndex);
    if (!itemRect.isValid())
        return -1;

    // 尝试通过样式获取文本矩形
    QStyleOptionViewItem viewOpt;
    viewOpt.initFrom(treeView);
    viewOpt.rect = itemRect;
    viewOpt.widget = treeView;
    viewOpt.state |= QStyle::State_Enabled | QStyle::State_Active;
    viewOpt.features = QStyleOptionViewItem::HasCheckIndicator | QStyleOptionViewItem::HasDecoration;
    viewOpt.checkState = Qt::Checked;
    viewOpt.viewItemPosition = QStyleOptionViewItem::OnlyOne;
    viewOpt.decorationSize = treeView->iconSize();

    QRect textRect = treeView->style()->subElementRect(QStyle::SE_ItemViewItemText, &viewOpt, treeView);
    if (textRect.isValid() && textRect.x() > itemRect.x()) {
        return textRect.x();
    }

    // 备用手动计算
    int checkWidth = treeView->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, treeView);
    if (checkWidth <= 0) checkWidth = 16;
    QSize iconSize = treeView->iconSize();
    int iconWidth = iconSize.isValid() ? iconSize.width() : 16;
    int spacing = 4; // 常见的间距
    int textLeft = itemRect.x() + checkWidth + spacing + iconWidth + spacing;
    return textLeft;
}

QRect CheckBoxHeader::getHeaderCheckBoxRect(const QRect &sectionRect) const
{
    QTreeView *treeView = qobject_cast<QTreeView*>(parentWidget());
    if (treeView && treeView->model() && treeView->model()->rowCount() > 0) {
        QModelIndex firstIndex = treeView->model()->index(0, 0);
        QRect itemRect = treeView->visualRect(firstIndex);
        if (itemRect.isValid()) {
            QStyleOptionViewItem viewOpt;
            viewOpt.initFrom(treeView);
            viewOpt.rect = itemRect;
            viewOpt.widget = treeView;
            viewOpt.state |= QStyle::State_Enabled | QStyle::State_Active;
            viewOpt.features = QStyleOptionViewItem::HasCheckIndicator;
            viewOpt.checkState = Qt::Checked;
            viewOpt.viewItemPosition = QStyleOptionViewItem::OnlyOne;

            QRect checkRect = treeView->style()->subElementRect(
                        QStyle::SE_ItemViewItemCheckIndicator, &viewOpt, treeView);

            if (checkRect.isValid() && checkRect.width() > 0) {
                // 坐标转换到表头控件
                QPoint globalTopLeft = treeView->viewport()->mapToGlobal(checkRect.topLeft());
                QPoint topLeftInHeader = this->mapFromGlobal(globalTopLeft);
                QRect headerCheckRect(topLeftInHeader, checkRect.size());
                headerCheckRect.moveTop(sectionRect.top() + (sectionRect.height() - headerCheckRect.height()) / 2);
                return headerCheckRect;
            }

            // 备用方案
            int checkWidth = treeView->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, treeView);
            int checkHeight = treeView->style()->pixelMetric(QStyle::PM_IndicatorHeight, nullptr, treeView);
            if (checkWidth <= 0) checkWidth = 16;
            if (checkHeight <= 0) checkHeight = 16;
            const QPoint globalTopLeft = treeView->viewport()->mapToGlobal(
                QPoint(itemRect.x() + 4, itemRect.y()));
            const QPoint topLeftInHeader = mapFromGlobal(globalTopLeft);
            return QRect(topLeftInHeader.x(),
                         sectionRect.top() + (sectionRect.height() - checkHeight) / 2,
                         checkWidth,
                         checkHeight);
        }
    }

    // 无数据时的最终备用方案
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.rect = sectionRect;
    QRect checkRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, this);
    checkRect.moveTopLeft(sectionRect.topLeft() + QPoint(4, (sectionRect.height() - checkRect.height()) / 2));
    return checkRect;
}

QRect CheckBoxHeader::getHeaderTextRect(const QRect &sectionRect) const
{
    // 获取文本内容
    QString text = model()->headerData(0, orientation(), Qt::DisplayRole).toString();
    QFontMetrics fm(font());
    int textHeight = fm.height();

    // 计算水平起始位置
    int textLeft;
    int dataTextLeft = getDataRowTextLeft();
    if (dataTextLeft != -1) {
        QTreeView *treeView = qobject_cast<QTreeView*>(parentWidget());
        if (treeView) {
            QPoint globalPos = treeView->viewport()->mapToGlobal(QPoint(dataTextLeft, 0));
            QPoint headerPos = this->mapFromGlobal(globalPos);
            textLeft = headerPos.x();
        } else {
            textLeft = sectionRect.left();
        }
    } else {
        QRect checkRect = getHeaderCheckBoxRect(sectionRect);
        textLeft = checkRect.right() + 4;
    }

    // 垂直居中；宽度需为可用像素宽度（含右端点）
    int textWidth = qMax(0, sectionRect.right() - textLeft + 1);
    if (isSortIndicatorShown() && sortIndicatorSection() == 0) {
        QStyleOptionHeader sortOpt;
        initStyleOption(&sortOpt);
        sortOpt.rect = sectionRect;
        sortOpt.section = 0;
        if (sortIndicatorOrder() == Qt::AscendingOrder) {
            sortOpt.sortIndicator = QStyleOptionHeader::SortDown;
        } else {
            sortOpt.sortIndicator = QStyleOptionHeader::SortUp;
        }
        const QRect arrowRect = style()->subElementRect(QStyle::SE_HeaderArrow, &sortOpt, this);
        if (arrowRect.isValid()) {
            textWidth = qMax(0, arrowRect.left() - textLeft - 2);
        }
    }
    const int y = sectionRect.top() + (sectionRect.height() - textHeight) / 2;
    return QRect(textLeft, y, textWidth, textHeight);
}

void CheckBoxHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (logicalIndex == 0 && !shouldPaintCheckBox(logicalIndex)) {
        QHeaderView::paintSection(painter, rect, logicalIndex);
        return;
    }

    if (logicalIndex == 0) {
        painter->save();

        // 1. 绘制表头背景（排序指示器等）
        QStyleOptionHeader opt;
        opt.section = logicalIndex;
        opt.rect = rect;
        opt.orientation = orientation();
        opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
        if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex) {
            opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
                ? QStyleOptionHeader::SortDown : QStyleOptionHeader::SortUp;
        }
        style()->drawControl(QStyle::CE_Header, &opt, painter, this);

        // 2. 绘制文本（复选框由 QCheckBox 子控件绘制）
        QRect textRect = getHeaderTextRect(rect);
        QString text = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        if (!text.isEmpty() && textRect.width() > 0) {
            const QFontMetrics fm = painter->fontMetrics();
            const QString elidedText = fm.elidedText(text, textElideMode(), textRect.width());
            painter->setPen(palette().text().color());
            painter->save();
            painter->setClipRect(textRect);
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
            painter->restore();
        }

        painter->restore();
    } else {
        QHeaderView::paintSection(painter, rect, logicalIndex);
    }
}
//void CheckBoxHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
//{
//    if (logicalIndex == 0 && !shouldPaintCheckBox(logicalIndex)) {
//        QHeaderView::paintSection(painter, rect, logicalIndex);
//        return;
//    }

//    if (logicalIndex == 0) {
//        painter->save();
//        QStyleOptionHeader opt;
//        opt.section = logicalIndex;
//        opt.rect = rect;
//        opt.orientation = orientation();
//        opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
//        if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex) {
//            opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
//                ? QStyleOptionHeader::SortDown : QStyleOptionHeader::SortUp;
//        }
//        style()->drawControl(QStyle::CE_Header, &opt, painter, this);

////        QRect checkboxRect = getHeaderCheckBoxRect(rect);
//        paintCheckBox(painter, rect, m_checkState);
//        // 绘制文本（垂直居中）
//        QRect textRect = getHeaderTextRect(rect);
////        opt.rect = textRect;
////        opt.textAlignment = Qt::AlignLeft | Qt::AlignVCenter;
////        style()->drawControl(QStyle::CE_HeaderLabel, &opt, painter);

//        QString text = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
//        if (!text.isEmpty()) {
//            painter->setClipping(false);
//            int textX = textRect.right();
//            int textY = textRect.center().y() + painter->fontMetrics().ascent() / 2;
//            painter->setPen(Qt::black);
//            painter->drawText(textX, textY, text);
//        }

//        painter->restore();
//    } else {
//        QHeaderView::paintSection(painter, rect, logicalIndex);
//    }
//}

void CheckBoxHeader::mousePressEvent(QMouseEvent *e)
{
    if (m_checkBox && m_checkBox->isVisible() && m_checkBox->geometry().contains(e->pos())) {
        e->ignore();
        return;
    }
    QHeaderView::mousePressEvent(e);
}
