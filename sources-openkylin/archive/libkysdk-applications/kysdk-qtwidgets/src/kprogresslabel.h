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

#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QSize>
#include <QWidget>

namespace kdk
{

/**
 * @defgroup ProgressModule
 * @since 2.5.2.0
 * {
 */

class KProgressLabelPrivate;

class KProgressLabel : public QWidget
{
    Q_OBJECT
public:
    KProgressLabel(QWidget *parent = nullptr);
    ~KProgressLabel();

public:
    /**
     * @brief 设置第二行文本和第一行文本
     * @param list1
     * @param list2
     */
    void setStringList(QStringList list1, QStringList list2);

    /**
     * @brief setStringList
     * @param list1
     */
    void setStringList(QStringList list1);

    /**
     * @brief 设置当前进度
     * @param index
     */
    void setCurrentIndex(int index);

    /**
     * @brief 获取当前进度
     * @return
     */
    int currentIndex();

    /**
     * @brief 设置圆形label的大小
     * @param size
     */
    void setCircleSize(QSize size);

private:
    Q_DECLARE_PRIVATE(KProgressLabel)
    KProgressLabelPrivate *const d_ptr;
};

}
#endif // MYWIDGET_H
