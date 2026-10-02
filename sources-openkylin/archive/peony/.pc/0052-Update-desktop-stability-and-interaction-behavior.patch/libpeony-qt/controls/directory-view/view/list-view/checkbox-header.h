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

#ifndef CHECKBOXHEADER_H
#define CHECKBOXHEADER_H

#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include "peony-core_global.h"

class QCheckBox;
class QResizeEvent;

namespace Peony {

/**
 * @brief 复选框表头类
 * 用于列表视图表头，点击可全选/取消全选
 * 只有第一列显示复选框
 */
class PEONYCORESHARED_EXPORT CheckBoxHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit CheckBoxHeader(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~CheckBoxHeader() override;

    // 设置复选框状态
    void setCheckState(Qt::CheckState state);
    Qt::CheckState checkState() const { return m_checkState; }

    // 根据选中数量自动更新状态
    void updateCheckState(int selectedCount, int totalCount);

    // 是否启用复选框
    void setCheckBoxEnabled(bool enabled);
    bool isCheckBoxEnabled() const { return m_checkboxEnabled; }

Q_SIGNALS:
    void checkStateChanged(Qt::CheckState state);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *e) override;
    void updateGeometries() override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QRect getHeaderCheckBoxRect(const QRect &sectionRect) const;
    QRect getHeaderTextRect(const QRect &sectionRect) const;
    int getDataRowTextLeft() const;  // 获取数据行文本的左边缘（视口坐标系）
    void updateCheckBoxPosition();
    bool shouldPaintCheckBox(int logicalIndex) const;

private:
    QCheckBox *m_checkBox = nullptr;
    Qt::CheckState m_checkState = Qt::Unchecked;
    bool m_checkboxEnabled = false;
};

}

#endif // CHECKBOXHEADER_H
