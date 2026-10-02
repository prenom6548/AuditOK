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

#ifndef KLABEL_H
#define KLABEL_H

#include "gui_g.h"
#include <QLabel>

namespace kdk
{

/**
 * @defgroup LabelModule
 * {
 */

enum KLabelType {
    NormalType,
    DataType,
    CircleType
};

class KLabelPrivate;

/**
 * @brief 继承qlabel
 * @since 2.4.1.0
 */
class GUI_EXPORT KLabel : public QLabel
{
    Q_OBJECT
public:
    KLabel(QWidget *parent = nullptr);

    /**
     * @brief 设置label类型，NormalType和IconAndTextType
     * @param type
     */
    void setLabelType(KLabelType type = KLabelType::DataType);

    /**
     * @brief 设置是否启用文本/图标高亮色
     * @param flag
     */
    void setDataHightColor(bool flag);

    /**
     * @brief 设置图标pixmap
     * @param pixmap
     */
    void setPixmap(const QPixmap &pixmap);

    /**
     * @brief 设置图标icon
     * @param icon
     * @since 2.5.2.0-0k0.3
     */
    void setPixmap(QIcon icon, int width, int height);

    /**
     * @brief 获取图标
     * @return
     */
    const QPixmap *pixmap() const;

    /**
     * @brief 设置文本
     * @since 2.5.2.0
     * @param str
     */
    void setText(const QString &str);

    /**
     * @brief 获取文本
     * @since 2.5.2.0
     * @return
     */
    QString text();

    /**
     * @brief 设置背景颜色
     * @param flag
     */
    void setBackgroundColor(bool flag, QColor color = Qt::white);

    /**
     * @brief 设置是否启用背景高亮色
     * @since 2.5.2.0
     * @param flag
     */
    void setHighlightBackground(bool flag);

    /**
     * @brief 设置圆角
     * @param bottomLeft
     * @param topLeft
     * @param topRight
     * @param bottomRight
     */
    void setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight);

    /**
     * @brief 设置圆角
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 设置是否启用换行
     * @since 2.5.2.0
     * @param flag
     */
    void setWordWrap(bool flag);

    /**
     * @brief 获取是否启用换行
     * @since 2.5.2.0
     * @return
     */
    bool wordWrap();

    /**
     * @brief 设置字体颜色
     * @param color
     * @since 2.5.2.0
     */
    void setFontColor(QColor color);

    /**
     * @brief 设置字体颜色colorRole
     * @param role
     * @since 3.0.1.0
     */
    void setFontColorRole(QPalette::ColorRole role); // lightlabel

    /**
     * @brief 设置三态是否开启
     * @param flag
     * @since 3.0.1.0
     */
    void setTristate(bool flag); // tristatelabel

    /**
     * @brief 获取三态是否开启
     * @return
     * @since 3.0.1.0
     */
    bool isTristate();

Q_SIGNALS:
    /**
     * @brief 鼠标进入信号
     * @since 3.0.1.0
     */
    void enterWidget(); // iconlabel

    /**
     * @brief 鼠标离开信号
     * @since 3.0.1.0
     */
    void leaveWidget();

    /**
     * @brief 鼠标点击信号
     * @param clicked
     * @since 3.0.1.0
     */
    void clicked(bool clicked); // eyelabel

    /**
     * @brief 鼠标双击信号
     * @since 3.0.1.0
     */
    void doubleClicked();

protected:
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *event);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    Q_DECLARE_PRIVATE(KLabel)
    KLabelPrivate *const d_ptr;
};

}
#endif // KLABEL_H
