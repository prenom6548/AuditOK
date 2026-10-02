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

#ifndef KADDFILEBUTTON_H
#define KADDFILEBUTTON_H

#include "gui_g.h"
#include <QFileDialog>
#include <QPushButton>

namespace kdk
{

/** @defgroup PushbuttonModule
 */

class KAddFileButtonPrivate;

/**
 * @brief 选择文件的按钮
 * @since 2.4.1.0
 */
class GUI_EXPORT KAddFileButton : public QPushButton
{
    Q_OBJECT
public:
    KAddFileButton(QWidget *parent = nullptr);

    /**
     * @brief 设置文本
     * @param str
     */
    void setText(QString str);

    /**
     * @brief 获取filedialog
     * @return QFileDialog
     */
    QFileDialog *fileDialog();

    /**
     * @brief 将文件对话框中使用的过滤器设置为给定的过滤器。
     * @param filter
     */
    void setNameFilter(const QString &filter);

    /**
     * @brief 设置文件对话框中使用的过滤器。
     * @param filters
     */
    void setNameFilters(const QStringList &filters);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    KAddFileButtonPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(KAddFileButton)
};

}

#endif // KADDFILEBUTTON_H
