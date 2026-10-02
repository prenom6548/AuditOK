/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KCOMBOBOXWIDGET_H
#define KCOMBOBOXWIDGET_H

#include "gui_g.h"
#include <QLabel>
#include <QToolButton>
#include <QWidget>

namespace kdk
{

/**
 * @brief 可选择多个或者单个选项
 * @since 2.5.2.0
 */
class KComboBoxWidgetPrivate;
class KComboBoxWidget : public QWidget
{
    Q_OBJECT
public:
    KComboBoxWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置combobox的选项列表
     * @param list
     */
    void setStringList(QStringList list);

    /**
     * @brief 获取选中的列表
     * @return
     */
    QStringList getChosenList();

    /**
     * @brief 设置主文本
     * @param text
     */
    void setMainText(QString text);

    /**
     * @brief 获取弹出菜单
     * @return
     */
    QMenu *menu();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *e);
    void enterEvent(QEvent *e);
    void paintEvent(QPaintEvent *event);
signals:
    void comBoxWidgetClicked();

private:
    KComboBoxWidgetPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(KComboBoxWidget)
};
}

#endif // KCOMBOBOXWIDGET_H
