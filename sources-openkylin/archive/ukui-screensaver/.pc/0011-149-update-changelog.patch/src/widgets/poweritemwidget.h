/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef POWERITEMWIDGET_H
#define POWERITEMWIDGET_H

#include <QWidget>
#include "customlabel.h"

class StatusButton;

class PowerItemWidget : public QFrame
{
    Q_OBJECT

public:
    explicit PowerItemWidget(QWidget *parent = 0);
    ~PowerItemWidget();
    void setText(const QString text);
    void setToolTip(const QString tip);
    void setIcon(const QIcon &icon);
    /**
     * @brief 设置按钮当前状态
     * @param selected 是否选中
     * @return
     */
    void setSelected(int status);

    void setFontFamily(QString fontFamily);

    void setFontSize(double fontSize);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

Q_SIGNALS:
    /**
     * @brief 响应点击事件
     * @return
     */
    void clicked();

private:
    StatusButton *itemFace = nullptr;
    CustomLabel *textLabel = nullptr;
};

#endif // POWERITEMWIDGET_H
