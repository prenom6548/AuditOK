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

#include "Ukui4MainWindow.h"

#include <QDebug>

#include "DBusClient.h"
#include "Ukui4CustomControl.h"
#include "Ukui4DevicePortSelectItem.h"

using namespace UkuiAudioFramwork;

Ukui4MainWindow::Ukui4MainWindow(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& systemVolumeSliderMap,
                                 std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& appVolumeSliderMap,
                                 std::unordered_map<int, std::shared_ptr<IDevicePortItem>>& devicePortItemMap,
                                 QWidget* parent) :
    m_sysVolumeSliderItemMap(systemVolumeSliderMap), m_appSliderItemMap(appVolumeSliderMap), m_devicePortItemMap(devicePortItemMap), QWidget(parent)
{
    for (const auto [k, v] : m_sysVolumeKeys) {
        std::shared_ptr<ITrayVolumeSlider> s = std::make_shared<Ukui4SystemVolumeSlider>(new UkmediaVolumeSlider);
        auto* percentLabel = new QLabel();
        percentLabel->setAccessibleName(v);
        auto* muteBtn = new QPushButton();
        muteBtn->setAccessibleName(tr("Mute"));
        std::shared_ptr<ITrayVolumeSliderItem> item = std::make_shared<Ukui4SystemVolumeSliderItem>(s, percentLabel, muteBtn, k, this);
        m_sysVolumeSliderItemMap.emplace(EnumToInt(k), item);
    }
}

void Ukui4MainWindow::initUi()
{
    m_pAppVolumeWidget = std::make_shared<Ukui4AppVolumeWidget>(m_sysVolumeSliderItemMap, m_appSliderItemMap, this, m_pTabWidget);
    m_pSysVolumeWidget = std::make_shared<Ukui4SystemVolumeWidget>(m_sysVolumeSliderItemMap, m_devicePortItemMap, this, m_pTabWidget);

    m_pAppVolumeWidget->initUi();
//    m_pSysVolumeWidget->initUi();

    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->setAccessibleName(tr("Volume control tabs"));
    m_pTabWidget->setFocusPolicy(Qt::StrongFocus);

    m_pTabWidget->addTab(m_pSysVolumeWidget->getWidget(), tr("System Volume"));
    m_pTabWidget->addTab(m_pAppVolumeWidget->getWidget(), tr("Application Volume"));

    m_pTabWidget->tabBar()->setProperty("setRadius", 12);    // tabbar界面内圆角
    m_pTabWidget->tabBar()->setFixedWidth(this->width()+1);
    m_pTabWidget->tabBar()->setProperty("useTabbarSeparateLine", false); // 去掉标签页中间的分隔线
    m_pTabWidget->setAttribute(Qt::WA_TranslucentBackground, true);      // 背景透明 解决切换黑屏问题


    setFixedSize(420, 476);
}

void Ukui4MainWindow::initSlots()
{

}

void Ukui4MainWindow::addAppItem(int idx, const QString& iconName, const QString& descName)
{
    std::shared_ptr<ITrayVolumeSlider> s = std::make_shared<Ukui4AppVolumeSlider>(new QSlider);
    auto* muteBtn = new QPushButton();
    muteBtn->setAccessibleName(tr("Mute %1").arg(descName));
    auto* displayLabel = new QLabel(descName);
    std::shared_ptr<ITrayVolumeSliderItem> item = std::make_shared<Ukui4AppVolumeSliderItem>(s, muteBtn, displayLabel);
    item->setIcon(QIcon::fromTheme(iconName));
    qDebug() << "Ukui4MainWindow::addAppItem;" << iconName;
    m_appSliderItemMap.emplace(idx, item);
    m_pAppVolumeWidget->addAppWidget(idx);
}

void Ukui4MainWindow::removeAppItem(int idx)
{
    m_pAppVolumeWidget->removeAppWidget(idx);
}

void Ukui4MainWindow::setParentWidget(QWidget* parent)
{
    setParent(parent);
}

void Ukui4MainWindow::addAppToWidget(const QString& appName, const QString& iconName)
{
    m_pAppVolumeWidget->addAppToWidget(appName, iconName);
}

void Ukui4MainWindow::removeAppFromWidget(const QString& appName)
{
    m_pAppVolumeWidget->removeAppfromWidget(appName);
}

void Ukui4MainWindow::addDevToWidget(const QString& iconName, const ClientDeviceInfo& info)
{
    QString displayStr = info.portLabel + " (" + info.cardDesc + ")";
    auto* selectBtn = new QPushButton();
    selectBtn->setAccessibleName(tr("Select %1").arg(displayStr));
    std::shared_ptr<IDevicePortItem> item = std::make_shared<Ukui4DevicePortItem>(new QLabel(displayStr), selectBtn, info.cardName, info.portName, this);
    item->setIcon(QIcon::fromTheme(iconName));
    m_devicePortItemMap.emplace(m_devicePortItemMap.size(), item);    
    m_pSysVolumeWidget->addDevicePortItem(item);
}

void Ukui4MainWindow::removeDevFromWidget(const QString& portName, const QString& devName)
{
    m_pSysVolumeWidget->removeDevice(portName, devName);
}

void Ukui4MainWindow::setCurrentIndex(int idx)
{
    m_pSysVolumeWidget->setCurrentIndex(idx);
}

void Ukui4MainWindow::setValue(int type, int idx, int value)
{
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::OUTPUT:
        m_sysVolumeSliderItemMap[EnumToInt(VolumeSliderType::VOLUME_SLIDER_SYSTEM_OUTPUT)]->setValue(value);
        m_sysVolumeSliderItemMap[EnumToInt(VolumeSliderType::VOLUME_SLIDER_APP_OUTPUT)]->setValue(value);
        break;
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT: {
        auto it = m_appSliderItemMap.find(idx);

        if (it != m_appSliderItemMap.end()) {
            it->second->setValue(value);
        }
        else {
            qDebug() << "m_appVolumeSliderItemMap not found such idx: " << idx;
        }

        break;
    }
    default:
        break;
    }

}

void Ukui4MainWindow::updateDescription(const QString&, const QString&)
{
    // TODO
}
