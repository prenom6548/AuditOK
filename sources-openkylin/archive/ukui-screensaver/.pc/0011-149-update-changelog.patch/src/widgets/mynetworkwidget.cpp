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
#include "mynetworkwidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QBitmap>
#include <QLayout>
#include <QApplication>
#include <QLabel>
#include <QTabBar>

#include <kylin-nm/kynetworkicon.h>
#include <kylin-nm-interface.h>

#include "mytabwidget.h"

MyNetworkWidget::MyNetworkWidget(QWidget *parent) : QWidget(parent)
{
    setObjectName("m_networkWidget");
    setStyleSheet("#m_networkWidget{background-color: white; border-radius:12px;}");
}

void MyNetworkWidget::loadNetPlugin()
{
    if (netloader.isLoaded() || wlanloader.isLoaded())
        return;

    if (!tabWidget) {
        tabWidget = new MyTabWidget(this);
        tabWidget->setObjectName("tabwidget");
        tabWidget->setFixedSize(420, 528);
        tabWidget->tabBar()->setFixedHeight(40);
        tabWidget->installEventFilter(this);

        QHBoxLayout *m_tabBarLayout = new QHBoxLayout(tabWidget);
        m_tabBarLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *m_lanLabel = new QLabel(tr("LAN"));
        m_lanLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        QLabel *m_wlanLabel = new QLabel(tr("WLAN"));
        m_wlanLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        m_tabBarLayout->addWidget(m_lanLabel);
        m_tabBarLayout->addWidget(m_wlanLabel);
        tabWidget->tabBar()->setLayout(m_tabBarLayout);
        tabWidget->tabBar()->setFixedWidth(420);
    }

    netloader.setFileName("/usr/lib/kylin-nm/libnetconnect.so");   //有线
    wlanloader.setFileName("/usr/lib/kylin-nm/libwlanconnect.so"); //无线

    //如果要修改插件加载顺序，涉及mkylinNM控件index的都需要修改
    if (netloader.load()) {
        QObject *plugin = netloader.instance();

        if (plugin) {
            netInterface = qobject_cast<Interface *>(plugin);
            // 插件是否启用
            if (!netInterface) {
                return;
            }

            netInterface->setPluginType(SIMPLE);
            QWidget *widget = netInterface->pluginUi();
            tabWidget->addTab(widget, "");
            // connect(plugin, SIGNAL(updatePluginHidden(bool)), this, SLOT(onUpdatePluginHidden(bool)));
        } else {
            qDebug() << "Load Failed: " << netloader.errorString() << "\n";
            return;
        }
    } else {
        qDebug() << "Load Failed: " << netloader.errorString() << "\n";
        return;
    }

    if (wlanloader.load()) {
        QObject *plugin = wlanloader.instance();

        if (plugin) {
            wlanInterface = qobject_cast<Interface *>(plugin);
            // 插件是否启用
            if (!wlanInterface) {
                return;
            }

            wlanInterface->setPluginType(SIMPLE);

            /*这里需要先调用setParentWidget，否则会出现网络连接弹窗无法弹出来的问题*/
            wlanInterface->setParentWidget(this);
            QWidget *widget = wlanInterface->pluginUi();
            tabWidget->addTab(widget, "");
            // connect(plugin, SIGNAL(updatePluginHidden(bool)), this, SLOT(onUpdatePluginHidden(bool)));

            //  平板模式输入状态下自动调出虚拟键盘
            connect(wlanInterface, SIGNAL(needShowVirtualKeyboard()), this, SLOT(onNetInPutStatus()));
        } else {
            qDebug() << "Load Failed: " << wlanloader.errorString() << "\n";
            return;
        }
    } else {
        qDebug() << "Load Failed: " << wlanloader.errorString() << "\n";
        return;
    }

    QPalette pal = qApp->palette();
    pal.setBrush(QPalette::Background, Qt::white);
    //    m_kylinNM->setPalette(pal);
    tabWidget->widget(0)->setPalette(pal);
    tabWidget->widget(1)->setPalette(pal);

    setGeometry(
        this->width() - tabWidget->width() - 20,
        this->height() - tabWidget->height() - 72 - 8,
        tabWidget->width(),
        tabWidget->height());

    connect(tabWidget, &MyTabWidget::currentChanged, this, &MyNetworkWidget::onNetTabWidgetChanged);

    onUpdatePluginHidden();
}

void MyNetworkWidget::unloadNetPlugin()
{
    if (wlanloader.isLoaded()) {
        tabWidget->removeTab(1);
        wlanloader.unload();
    }

    if (netloader.isLoaded()) {
        tabWidget->removeTab(0);
        netloader.unload();
    }

    if (tabWidget) {
        tabWidget->hide();
        tabWidget->deleteLater();
        tabWidget = nullptr;
    }

    if (tabWidget) {
        tabWidget->hide();
        tabWidget->deleteLater();
        tabWidget = nullptr;
    }
}

void MyNetworkWidget::onNetTabWidgetChanged(int index)
{
    /*
        if(index == 0){
            if(netInterface)
                netInterface->setWidgetVisable(true);
            if(wlanInterface){
                wlanInterface->setWidgetVisable(false);
            }
        }else if(index == 1){
            if(netInterface)
                netInterface->setWidgetVisable(false);
            if(wlanInterface)
                wlanInterface->setWidgetVisable(true);
        }
    */
}

void MyNetworkWidget::onUpdatePluginHidden()
{
    /*
        if (netInterface &&
                m_netTabShow != netInterface->checkPluginIsHidden()) {
            if (m_netTabShow) {
                tabWidget->removeTab(0);
            } else {
                tabWidget->insertTab(0, netWidget, "");
            }
            m_netTabShow = !m_netTabShow;
        }
        if (wlanInterface &&
                m_wlanTabShow != wlanInterface->checkPluginIsHidden()) {
            if (m_wlanTabShow) {
                if (m_netTabShow)
                    tabWidget->removeTab(1);
                else
                    tabWidget->removeTab(0);
            } else {
                if (m_netTabShow)
                    tabWidget->insertTab(1, wlanWidget, "");
                else
                    tabWidget->insertTab(0, wlanWidget, "");

            }
            m_wlanTabShow = !m_wlanTabShow;
        }
        if (m_netTabShow && m_wlanTabShow) {
            tabWidget->tabBar()->show();
        } else {
            tabWidget->tabBar()->hide();
        }
    */
}

void MyNetworkWidget::sendNetPluginVisible(bool visible)
{
    /*
        if(!tabWidget)
            return;
        if(tabWidget->currentIndex() == 0 && netInterface){
            netInterface->setWidgetVisable(visible);
        }else if(tabWidget->currentIndex() == 1 && wlanInterface){
            wlanInterface->setWidgetVisable(visible);
        }
    */
}

void MyNetworkWidget::onNetInPutStatus()
{
    Q_EMIT showVirtualKeyboard();
}

void MyNetworkWidget::showEvent(QShowEvent *event)
{
    sendNetPluginVisible(true);
}

void MyNetworkWidget::hideEvent(QHideEvent *event)
{
    sendNetPluginVisible(false);
}

void MyNetworkWidget::paintEvent(QPaintEvent *p1)
{
    //绘制样式
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this); //绘制样式

    QBitmap bmp(this->size());
    bmp.fill();
    QPainter painter(&bmp);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundedRect(bmp.rect(), 12, 12);
    setMask(bmp);
}
