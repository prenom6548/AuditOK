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

#ifndef KTEXTEDIT_H
#define KTEXTEDIT_H

#include "gui_g.h"
#include "klabel.h"
#include "ktag.h"
#include <QLineEdit>
#include <QTextEdit>

namespace kdk
{

/**
 * @defgroup @DialogBoxModule
 * {
 */

class KTextEditPrivate;

/**
 * @brief 继承QTextEdit
 * @since 2.4.1.0
 */
class GUI_EXPORT KTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    KTextEdit(QWidget *parent = nullptr);

    /**
     * @brief 设置label是否可见
     * @param flag
     */
    void setLabelVisible(bool flag);

    /**
     * @brief 设置图标
     * @param pixmap
     */
    void setPixmap(const QPixmap &pixmap);

    /**
     * @brief 获取图标
     * @return
     */
    const QPixmap *pixmap() const;

    /**
     * @brief 设置label文本
     * @param text
     */
    void setLabelText(const QString &text);

    /**
     * @brief 设置是否可编辑状态
     * @param ro
     */
    void setReadOnly(bool ro);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KTextEdit)
    KTextEditPrivate *const d_ptr;
};

}
#endif // KTEXTEDIT_H
