/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>

class CustomListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit CustomListWidget(QWidget *parent = nullptr);
    friend class TestClass;  // 核心修改：声明测试类为友元

    ~CustomListWidget();
protected:
    virtual void mouseMoveEvent(QMouseEvent* event);
signals:
    void getPosition(QPointF pt);
private:

};

#endif // CUSTOMLISTWIDGET_H
