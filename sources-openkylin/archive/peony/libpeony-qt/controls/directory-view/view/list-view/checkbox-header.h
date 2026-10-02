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

namespace Peony {

/**
 * @brief 复选框表头类
 *
 * 列表视图专用：在指定的逻辑列（复选框列）的表头 section 中绘制一个全选复选框。
 * 点击该 section 会在"全选/取消全选"间切换，并发出 checkStateChanged 信号；
 * 点击其他 section 走 QHeaderView 默认行为（排序等）。
 *
 * 复选框列由 FileItemModel 在 ListView 绑定时追加，并通过 moveSection 移到视觉最左侧。
 * 表头与数据行复选框共享 indicatorRect() 计算位置，保证严格对齐。
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

    /*!
     * \brief setCheckBoxColumn 设置承载全选复选框的逻辑列索引。
     */
    void setCheckBoxColumn(int logicalIndex) { m_check_box_column = logicalIndex; }
    int checkBoxColumn() const { return m_check_box_column; }

    /*!
     * \brief indicatorRect 计算复选框指示器在容器（单元格或表头 section）内的位置。
     *
     * 表头与数据行复用此函数，确保二者复选框在视觉上严格对齐：
     *   - 指示器固定 16x16（\ref indicatorSize）
     *   - 在容器中水平+垂直居中
     * 不使用 QSS，全部直接由 QPainter/style 绘制。
     */
    static QRect indicatorRect(const QRect &containerRect);

    /// 复选框列固定宽度
    static int columnWidth() { return 36; }
    /// 复选框指示器尺寸（固定 16x16）
    static int indicatorSize() { return 16; }

    /*!
     * \brief setCheckBoxColumnWidth 设置复选框列宽度。
     *
     * 全局 minimumSectionSize 已设为 \ref columnWidth()（36px），
     * 因此复选框列可直接 resize，无需额外处理。
     */
    void setCheckBoxColumnWidth(int width);

    /*!
     * \brief isUserResizing 用户是否正通过鼠标/触摸交互操作表头（按下到释放之间）。
     *
     * 供 ListView 的 sectionResized 回调区分“用户拖拽分隔线”与
     * “程序内部 resizeSection/setSectionResizeMode 触发”，避免后者被误判为
     * 用户手动调整而锁定列宽、跳过内容自适应。
     */
    bool isUserResizing() const { return m_user_resizing; }

Q_SIGNALS:
    void checkStateChanged(Qt::CheckState state);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    Qt::CheckState m_checkState = Qt::Unchecked;
    bool m_checkboxEnabled = false;
    int m_check_box_column = -1;
    // 标记用户是否正通过鼠标/触摸交互（按下到释放之间）操作表头
    bool m_user_resizing = false;
};

}

#endif // CHECKBOXHEADER_H
