/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef NETCHECKWIDGET_H
#define NETCHECKWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "netcheckhomepage.h"
#include "mainwindow.h"

class NetCheckWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetCheckWidget(QWidget *parent = nullptr);

private:
    QStackedWidget *stacked_widget;
    NetCheckHomePage *mHomeWidget = nullptr;
    MainWindow *mMainWidget = nullptr;
    int themeColor = 1; //默认1是深色，0是浅色
    int mFontSize = 0;
    void initThemeGetting();
    void changeThemeColor(int color);
    void changeSystemFontSize(int size);

signals:

public slots:
    void slotChangeCurrentStackWidget(int page);
};

#endif // NETCHECKWIDGET_H
