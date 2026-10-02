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
#ifndef MYNETWORKWIDGET_H
#define MYNETWORKWIDGET_H

#include <QWidget>
#include <QPluginLoader>

class Interface;
class MyTabWidget;
class MyNetworkWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyNetworkWidget(QWidget *parent = nullptr);

    void loadNetPlugin();
    void unloadNetPlugin();
    void setDefaultPage(int pageType);

private Q_SLOTS:
    /**
     * @brief 网络窗口有线和无线弹窗切换事件，用于通知网络插件窗口切换
     * @param index   当前页面索引
     * @return
     */
    void onNetTabWidgetChanged(int index);
    void onUpdatePluginHidden();
    void onNetInPutStatus();
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void paintEvent(QPaintEvent *p1);

private:
    void sendNetPluginVisible(bool visible);

Q_SIGNALS:
    void showVirtualKeyboard();

private:
    QPluginLoader netloader;
    QPluginLoader wlanloader;

    Interface *netInterface = nullptr;
    Interface *wlanInterface = nullptr;

    MyTabWidget *tabWidget = nullptr;

    QWidget *netWidget = nullptr;
    QWidget *wlanWidget = nullptr;
    bool m_netTabShow = true;
    bool m_wlanTabShow = true;
    int m_defaultPage = 0;
};

#endif // WIDGET_H
