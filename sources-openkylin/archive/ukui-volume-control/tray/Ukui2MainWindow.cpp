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

#include "Ukui2MainWindow.h"

#include <QDebug>

#include "Ukui2AppVolumeWidget.h"
#include "Ukui2SystemVolumeWidget.h"
#include "DBusClient.h"
#include "Ukui4CustomControl.h"

using namespace UkuiAudioFramwork;

Ukui2MainWindow::Ukui2MainWindow(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& systemVolumeSliderMap,
                                 std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& appVolumeSliderMap,
                                 QWidget* parent) :
    m_sysVolumeSliderItemMap(systemVolumeSliderMap), m_appSliderItemMap(appVolumeSliderMap), QWidget(parent)
{
    for (const auto [k, v] : m_sysVolumeKeys) {
        std::shared_ptr<ITrayVolumeSlider> s = nullptr;
        std::shared_ptr<ITrayVolumeSliderItem> item = nullptr;

        switch (k) {
        case VolumeSliderType::VOLUME_SLIDER_MINI_OUTPUT:
        case VolumeSliderType::VOLUME_SLIDER_APP_OUTPUT: {
            s = std::make_shared<Ukui2SystemVolumeSlider>(new UkmediaVolumeSlider);
            auto* percentLabel = new QLabel();
            percentLabel->setAccessibleName(v);
            auto* muteBtn = new QPushButton();
            muteBtn->setAccessibleName(tr("Mute"));
            item = std::make_shared<Ukui2SystemVolumeSliderItem>(s, percentLabel, muteBtn, k, this);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_DEVICE_OUTPUT:
        case VolumeSliderType::VOLUME_SLIDER_DEVICE_INPUT: {
            s = std::make_shared<Ukui2SystemVolumeSlider>(new UkmediaVolumeSlider);
            auto* descLabel = new QLabel();
            descLabel->setAccessibleName(v);
            auto* iconBtn = new QPushButton();
            iconBtn->setAccessibleName(tr("Device icon"));
            auto* muteBtn = new QPushButton();
            muteBtn->setAccessibleName(tr("Mute"));
            item = std::make_shared<Ukui2DeviceVolumeSliderItem>(s, descLabel, iconBtn, muteBtn, k, this);
            break;
        }
        default:
            break;
        }

        if (item)
            m_sysVolumeSliderItemMap.emplace(EnumToInt(k), item);
    }
}

void Ukui2MainWindow::initUi()
{
    m_pSwitchModuleBtn = new QPushButton();
    m_pSwitchModuleBtn->setAccessibleName(tr("Switch volume panel"));
    m_pSwitchModuleBtn->setFixedSize(36, 36);
    m_pSwitchModuleBtn->setIconSize(QSize(16, 16));
    m_pSwitchModuleBtn->setIcon(QIcon("/usr/share/ukui-media/img/mini-module.svg"));

    m_pDeviceVolumeBtn = new QPushButton();
    m_pDeviceVolumeBtn->setAccessibleName(tr("Device volume"));
    m_pDeviceVolumeBtn->setFlat(true);
    m_pDeviceVolumeBtn->setFocusPolicy(Qt::NoFocus);
    m_pDeviceVolumeBtn->setFixedSize(36, 36);
    m_pDeviceVolumeBtn->setIcon(QIcon("/usr/share/ukui-media/img/device.svg"));

    m_pAppVolumeBtn = new QPushButton();
    m_pAppVolumeBtn->setAccessibleName(tr("Application volume"));
    m_pAppVolumeBtn->setFlat(true);
    m_pAppVolumeBtn->setFocusPolicy(Qt::NoFocus);
    m_pAppVolumeBtn->setFixedSize(36, 36);
    m_pAppVolumeBtn->setIcon(QIcon("/usr/share/ukui-media/img/application.svg"));

    m_pStackedWidget = new QStackedWidget(this);
    m_pAppVolumeWidget = std::make_shared<Ukui2AppVolumeWidget>(m_sysVolumeSliderItemMap, m_appSliderItemMap, m_pSwitchModuleBtn,
                                                                m_pDeviceVolumeBtn, m_pAppVolumeBtn, this, m_pStackedWidget);
    m_pSysVolumeWidget = std::make_shared<Ukui2SystemVolumeWidget>(m_sysVolumeSliderItemMap, m_pSwitchModuleBtn, m_pDeviceVolumeBtn,
                                                                   m_pAppVolumeBtn, this, m_pStackedWidget);

    m_pMiniWidget = new Ukui2MiniWidget(m_sysVolumeSliderItemMap, m_pSwitchModuleBtn, m_pStackedWidget);
    m_pSwitchModuleBtn->setParent(m_pMiniWidget);
    m_pSwitchModuleBtn->move(305, 6);

    m_pStackedWidget->addWidget(m_pMiniWidget);
    m_pStackedWidget->addWidget(m_pSysVolumeWidget->getWidget());
    m_pStackedWidget->addWidget(m_pAppVolumeWidget->getWidget());
    m_pStackedWidget->setFixedSize(420, 320);
    qDebug() << "m_pStackedWidget.size: " << m_pStackedWidget->size();
}

void Ukui2MainWindow::initSlots()
{
    connect(m_pSwitchModuleBtn, SIGNAL(pressed()), this, SLOT(siwtchModuleBtnPressedSlots()));

    connect(m_pDeviceVolumeBtn, SIGNAL(pressed()), this, SLOT(deviceVolumeBtnPressedSlots()));

    connect(m_pAppVolumeBtn, SIGNAL(pressed()), this, SLOT(appVolumeBtnPressedSlots()));
}

void Ukui2MainWindow::addAppItem(int idx, const QString& iconName, const QString& descName)
{
    std::shared_ptr<ITrayVolumeSlider> s = std::make_shared<Ukui2AppVolumeSlider>(new QSlider);
    auto* iconBtn = new QPushButton();
    iconBtn->setAccessibleName(descName);
    auto* muteBtn = new QPushButton();
    muteBtn->setAccessibleName(tr("Mute %1").arg(descName));
    auto* displayLabel = new QLabel(descName);
    std::shared_ptr<ITrayVolumeSliderItem> item = std::make_shared<Ukui2AppVolumeSliderItem>(s, iconBtn, muteBtn, displayLabel);
    item->setIcon(QIcon::fromTheme(iconName));

    m_appSliderItemMap.emplace(idx, item);
    m_pAppVolumeWidget->addAppWidget(idx);
}

void Ukui2MainWindow::removeAppItem(int idx)
{
    m_pAppVolumeWidget->removeAppWidget(idx);
}

void Ukui2MainWindow::setParentWidget(QWidget* parent)
{
    setParent(parent);
}

void Ukui2MainWindow::addAppToWidget(const QString& appName, const QString& iconName)
{
    m_pAppVolumeWidget->addAppToWidget(appName, iconName);
}

void Ukui2MainWindow::removeAppFromWidget(const QString& appName)
{
    m_pAppVolumeWidget->removeAppfromWidget(appName);
}

void Ukui2MainWindow::addDevToWidget([[maybe_unused]] const QString& iconName, [[maybe_unused]] const ClientDeviceInfo& info)
{

}

void Ukui2MainWindow::removeDevFromWidget(const QString& portName, const QString& devName)
{
    if (m_pSysVolumeWidget)
        m_pSysVolumeWidget->removeDevice(portName, devName);
}

void Ukui2MainWindow::setCurrentIndex(int idx)
{
    if (m_pSysVolumeWidget)
        m_pSysVolumeWidget->setCurrentIndex(idx);
}

void Ukui2MainWindow::siwtchModuleBtnPressedSlots()
{
    if (0 == m_pStackedWidget->currentIndex()) {
        m_pSwitchModuleBtn->move(361, 6);

        switch (m_prevType) {
        case StackedWidgetType::ADVANCE_SYS_WIDGET: {
            m_pSwitchModuleBtn->setParent(m_pSysVolumeWidget->getWidget());
            break;
        }
        case StackedWidgetType::ADVANCE_APP_WIDGET: {
            m_pSwitchModuleBtn->setParent(m_pAppVolumeWidget->getWidget());
            break;
        }
        default:
            break;
        }

        setStackWidgetCurrentIndex(EnumToInt(m_prevType));
    }
    else {
        m_pSwitchModuleBtn->move(305, 6);
        m_pSwitchModuleBtn->setParent(m_pMiniWidget);
        setStackWidgetCurrentIndex(0);
    }
}

void Ukui2MainWindow::deviceVolumeBtnPressedSlots()
{
    if (1 == m_pStackedWidget->currentIndex())
        return;

    m_prevType = StackedWidgetType::ADVANCE_SYS_WIDGET;
    m_pDeviceVolumeBtn->setParent(m_pSysVolumeWidget->getWidget());
    m_pAppVolumeBtn->setParent(m_pSysVolumeWidget->getWidget());
    m_pSwitchModuleBtn->setParent(m_pSysVolumeWidget->getWidget());
    setStackWidgetCurrentIndex(1);
}

void Ukui2MainWindow::appVolumeBtnPressedSlots()
{
    if (2 == m_pStackedWidget->currentIndex())
        return;

    m_prevType = StackedWidgetType::ADVANCE_APP_WIDGET;
    m_pDeviceVolumeBtn->setParent(m_pAppVolumeWidget->getWidget());
    m_pAppVolumeBtn->setParent(m_pAppVolumeWidget->getWidget());
    m_pSwitchModuleBtn->setParent(m_pAppVolumeWidget->getWidget());
    setStackWidgetCurrentIndex(2);
}

void Ukui2MainWindow::setValue(int type, int idx, int value)
{
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::INPUT: {
        m_sysVolumeSliderItemMap[2]->setValue(value);
        break;
    }
    case NodeType::OUTPUT: {
        m_sysVolumeSliderItemMap[0]->setValue(value);
        m_sysVolumeSliderItemMap[1]->setValue(value);
        m_sysVolumeSliderItemMap[3]->setValue(value);
        break;
    }

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

void Ukui2MainWindow::updateDescription(const QString& input, const QString& output)
{
    m_pMiniWidget->setDescription(output);
    std::dynamic_pointer_cast<Ukui2DeviceVolumeSliderItem>(m_sysVolumeSliderItemMap[1])->setDescription(output);
    std::dynamic_pointer_cast<Ukui2DeviceVolumeSliderItem>(m_sysVolumeSliderItemMap[2])->setDescription(input);
}

void Ukui2MainWindow::setStackWidgetCurrentIndex(int idx)
{
    if (0 == idx)
        setSize(QSize(345, 100));
    else
        setSize(QSize(420, 320));

    m_pStackedWidget->setCurrentIndex(idx);
}

