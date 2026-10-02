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
#include <QPainter>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionHeader>
#include <QSignalBlocker>

using namespace Peony;

CheckBoxHeader::CheckBoxHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
}

CheckBoxHeader::~CheckBoxHeader()
{
}

void CheckBoxHeader::setCheckBoxColumnWidth(int width)
{
    if (m_check_box_column < 0)
        return;
    resizeSection(m_check_box_column, width);
}

void CheckBoxHeader::setCheckState(Qt::CheckState state)
{
    // 部分选中：UI 上表现为未选中，但保留逻辑标记，不发信号
    // 这样单击行为总是确定地"全选/取消全选"
    if (state == Qt::PartiallyChecked) {
        if (m_checkState != Qt::Unchecked) {
            m_checkState = Qt::Unchecked;
            if (m_checkboxEnabled && m_check_box_column >= 0
                    && model() && m_check_box_column < model()->columnCount()) {
                updateSection(m_check_box_column);
            }
        }
        return;
    }

    if (m_checkState != state) {
        m_checkState = state;
        if (m_checkboxEnabled && m_check_box_column >= 0
                && model() && m_check_box_column < model()->columnCount()) {
            updateSection(m_check_box_column);
        }
        Q_EMIT checkStateChanged(state);
    }
}

void CheckBoxHeader::updateCheckState(int selectedCount, int totalCount)
{
    if (!m_checkboxEnabled)
        return;

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
    if (m_checkboxEnabled == enabled)
        return;
    m_checkboxEnabled = enabled;
    if (m_check_box_column >= 0 && model()
            && m_check_box_column < model()->columnCount()) {
        updateSection(m_check_box_column);
    }
}

void CheckBoxHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    // 仅在复选框列上绘制复选框；其它列走 QHeaderView 默认绘制
    if (logicalIndex != m_check_box_column || !m_checkboxEnabled || m_check_box_column < 0) {
        QHeaderView::paintSection(painter, rect, logicalIndex);
        return;
    }

    painter->save();

    // 自绘表头底框：将 position 设为 OnlyOne，让 style 跳过右分隔线，
    // 从而消除复选框列与第二列之间的分割线
    QStyleOptionHeader headerOpt;
    initStyleOption(&headerOpt);
    headerOpt.rect = rect;
    headerOpt.section = logicalIndex;
    headerOpt.position = QStyleOptionHeader::OnlyOneSection;
    headerOpt.sortIndicator = QStyleOptionHeader::None;
    // 复选框列不应被绘制为"被按下/悬停"等表头交互态，统一以正常 + 启用态绘制
    headerOpt.state |= QStyle::State_Enabled | QStyle::State_Raised;
    headerOpt.state &= ~QStyle::State_Sunken;
    style()->drawControl(QStyle::CE_Header, &headerOpt, painter, this);

    painter->restore();

    // 绘制 16x16 复选框指示器（与数据行复用 indicatorRect，保证对齐）
    const QRect indicator = indicatorRect(rect);

    QStyleOptionButton cbOpt;
    cbOpt.initFrom(this);
    cbOpt.rect = indicator;
    cbOpt.state |= QStyle::State_Enabled;
    // m_checkState 经 setCheckState() 处理后只可能是 Checked/Unchecked
    // （PartiallyChecked 会被归一化为 Unchecked）
    if (m_checkState == Qt::Checked) {
        cbOpt.state |= QStyle::State_On;
    } else {
        cbOpt.state |= QStyle::State_Off;
    }
    style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &cbOpt, painter, this);
}

QRect CheckBoxHeader::indicatorRect(const QRect &containerRect)
{
    // 复选框指示器固定 16x16，在容器中水平+垂直居中
    const QSize sz(indicatorSize(), indicatorSize());

    QRect r(QPoint(0, 0), sz);
    r.moveCenter(containerRect.center());
    return r;
}

void CheckBoxHeader::mousePressEvent(QMouseEvent *e)
{
    m_user_resizing = true;
    const int logical = logicalIndexAt(e->pos());
    if (logical == m_check_box_column && m_check_box_column >= 0 && m_checkboxEnabled) {
        if (e->button() == Qt::LeftButton) {
            // 全选/取消全选切换
            const Qt::CheckState next = (m_checkState == Qt::Checked)
                    ? Qt::Unchecked : Qt::Checked;
            setCheckState(next);
        }
        e->accept();
        return;  // 不触发排序
    }
    QHeaderView::mousePressEvent(e);
}

void CheckBoxHeader::mouseReleaseEvent(QMouseEvent *e)
{
    QHeaderView::mouseReleaseEvent(e);
    m_user_resizing = false;
}
