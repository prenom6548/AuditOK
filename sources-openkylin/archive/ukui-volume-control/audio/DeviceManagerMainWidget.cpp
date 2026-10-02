/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "DeviceManagerMainWidget.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include "../backend/Util.h"

DeviceManagerMainWidget::DeviceManagerMainWidget(QWidget *parent)
    : KWidget(parent)
{
    createItems();
    initUi();
    initSlots();
}

void DeviceManagerMainWidget::createItems()
{
    for (const auto& [k, v] : m_keys) {
        auto item = new QStandardItem(QIcon::fromTheme(v.at(0)), v.at(1));
        auto widget = new DeviceManagerWidget(new QLabel(v.at(1)), this);

        // 设置项的样式
        QString itemStyle = "QStandardItem { min-width: 176px; max-width: 176px; min-height: 36px; max-height: 36px; }";
        item->setData(itemStyle, Qt::UserRole);
        item->setData(UkuiAudioFramwork::EnumToInt(k), Qt::UserRole);

        m_deviceWidgetMap.emplace(k, widget);
        m_itemMap.emplace(k, item);
    }
}

void DeviceManagerMainWidget::initUi()
{
    setIcon(QIcon::fromTheme("ukui-control-center"));
    setWidgetName(tr("Sound Equipment Control"));
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModality::WindowModal);
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(760, 520);

    m_pNavigationBar = new kdk::KNavigationBar(this);

    for (const auto& [k, v] : m_itemMap) {
        m_pNavigationBar->addItem(v);
    }

    m_pNavigationBar->setFixedSize(200, 478);
    m_pNavigationBar->setContentsMargins(0, 0, 0, 0);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(6, 20, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_pNavigationBar);
    sideBar()->setLayout(vLayout);

    // 创建QStackedWidget用于管理不同的界面
    auto stackedWidget = new QStackedWidget(this);
    stackedWidget->setContentsMargins(0, 0, 0, 0);
    stackedWidget->setFixedSize(560, 478);

    for (const auto& [k, v] : m_deviceWidgetMap) {
        stackedWidget->addWidget(v);
    }

    auto model = m_pNavigationBar->model();
    m_pNavigationBar->listview()->setCurrentIndex(model->item(0, 0)->index());

    // 添加右侧设备窗口布局
    auto vLayout2 = new QVBoxLayout(this);
    vLayout2->setContentsMargins(0, 20, 0, 0);
    vLayout2->setSpacing(0);
    vLayout2->addWidget(stackedWidget);
    baseBar()->setLayout(vLayout2);

    setLayoutType(kdk::HorizontalType);

    // 获取被点击的item的数据，跳转到对应的页面
    connect(m_pNavigationBar->listview(), &QListView::clicked, this, [=](const QModelIndex &index){
        auto idx = index.row();

        stackedWidget->setCurrentWidget(m_deviceWidgetMap[idx == 0 ? DeviceManagerType::Output : DeviceManagerType::Input]);
    });

    // 键盘方向键 + enter键支持
    connect(m_pNavigationBar->listview(), &QListView::activated, this, [=](const QModelIndex &index){
        auto idx = index.row();

        stackedWidget->setCurrentWidget(m_deviceWidgetMap[idx == 0 ? DeviceManagerType::Output : DeviceManagerType::Input]);
    });
}

void DeviceManagerMainWidget::initSlots()
{
    for (const auto& item : m_deviceWidgetMap) {
        connect(item.second, &DeviceManagerWidget::enableDeviceSignals, this, [this](const QString& card, const QString& port, bool enable) {
            Q_EMIT enableDeviceSignals(card, port, enable);
        });

        connect(item.second, &DeviceManagerWidget::closeWindowSignals, this, [this]() {
            Q_EMIT closeWindowSignals();
        });
    }
}

void DeviceManagerMainWidget::addItem(const DeviceManagerType& type, const QString& displayStr, const QString& cardName, const QString& portName)
{
    m_deviceWidgetMap[type]->addItem(displayStr, cardName, portName);
}

void DeviceManagerMainWidget::removeItem(const DeviceManagerType& type, int idx)
{
    m_deviceWidgetMap[type]->removeItem(idx);
}

void DeviceManagerMainWidget::setChecked(const DeviceManagerType& type, int idx, bool checked)
{
    m_deviceWidgetMap[type]->setChecked(idx, checked);
}

DeviceManagerWidget* DeviceManagerMainWidget::getManagerWidgetByType(const DeviceManagerType& type)
{
    return m_deviceWidgetMap[type];
}

void DeviceManagerMainWidget::updateDeviceStatus(const DeviceManagerType& type, bool hasDevice)
{
    m_deviceWidgetMap[type]->updateDeviceStatus(hasDevice);
}
