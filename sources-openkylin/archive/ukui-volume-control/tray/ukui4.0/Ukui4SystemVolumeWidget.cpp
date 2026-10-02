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

#include "Ukui4SystemVolumeWidget.h"

#include <QHBoxLayout>
#include <QVariant>
#include <QDebug>

#include "IMainWindow.h"
#include "Ukui4SystemVolumeSliderItem.h"
#include "Ukui4DevicePortSelectItem.h"

using namespace UkuiAudioFramwork;

Ukui4SystemVolumeWidget::Ukui4SystemVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& map,
                                         std::unordered_map<int, std::shared_ptr<IDevicePortItem>>& portMap,
                                         IMainWindow* window, QWidget* parent) :
    m_sysVolumeSliderItemMap(map), m_devicePortItemMap(portMap), m_pMainWid(std::move(window)), QWidget(parent)
{
    initUi();
    initSlots();
}

Ukui4SystemVolumeWidget::~Ukui4SystemVolumeWidget()
{
//    delete m_pMainWid;
}

void Ukui4SystemVolumeWidget::initUi()
{
    m_pSystemVolumeLabel = new QLabel();
    m_pSystemVolumeLabel->setText(tr("Volume"));
    m_pOutputLabel = new QLabel(tr("Output"));
    m_pListItem = std::make_shared<Ukui4DevicePortSelectItem>(new QListWidget, this);

    QVBoxLayout* mainLay = new QVBoxLayout();
    mainLay->addWidget(m_pSystemVolumeLabel);
//    mainLay->addLayout(sysVolumeLay);
    mainLay->addWidget(m_sysVolumeSliderItemMap[EnumToInt(VolumeSliderType::VOLUME_SLIDER_SYSTEM_OUTPUT)]->getWidget());
    mainLay->addWidget(m_pOutputLabel);
    mainLay->addSpacing(10);
//    mainLay->addWidget(m_pListView);
    mainLay->addWidget(m_pListItem->getWidget());
    this->setLayout(mainLay);

    qDebug() << "m_pListItem->size: " << m_pListItem->getWidget()->size();
}

void Ukui4SystemVolumeWidget::initSlots()
{
    for (const auto& it : m_sysVolumeSliderItemMap) {
        connect(static_cast<QSlider*>(it.second->getSlider()->getWidget()), &QSlider::valueChanged, this, [this](int value) {
            m_pMainWid->setVolume(1, value);
        });

        connect(static_cast<QPushButton*>(it.second->getIconButton()), &QPushButton::clicked, this, [it, this]() {
            if (0 == it.first || 1 == it.first)
                m_pMainWid->setMute(1, "");
            else if (2 == it.first)
                m_pMainWid->setMute(0, "");
        });
    }

    connect(std::dynamic_pointer_cast<Ukui4DevicePortSelectItem>(m_pListItem)->getListWidget(), &QListWidget::currentRowChanged, this, [this](int row) {
        auto item = std::dynamic_pointer_cast<Ukui4DevicePortSelectItem>(m_pListItem)->getListWidget()->item(row);
        auto w = (Ukui4DevicePortItem*) std::dynamic_pointer_cast<Ukui4DevicePortSelectItem>(m_pListItem)->getListWidget()->itemWidget(item);

        m_pMainWid->setDefaultDevice(1, w->getPortName(), w->getCardName());
    });
}

void Ukui4SystemVolumeWidget::showWidget()
{
    this->show();
}

QWidget* Ukui4SystemVolumeWidget::getWidget()
{
    return this;
}

void Ukui4SystemVolumeWidget::setValue(int vaule)
{
    m_pSysVolumeSlider->blockSignals(true);
    m_pSysVolumeSlider->setValue(vaule);
    m_pSysVolumeSlider->blockSignals(false);
}

void Ukui4SystemVolumeWidget::addDevice(const QString& iconName, const QVariantList& list)
{
    m_pModel->addDevice(iconName, list);
}

void Ukui4SystemVolumeWidget::removeDevice(const QString& portName, const QString& devName)
{

    // 暂时先全删除
    for (auto it = m_devicePortItemMap.begin();  it != m_devicePortItemMap.end();) {
        m_pListItem->removeItem(it->second->getPortName(), it->second->getCardName());
        it = m_devicePortItemMap.erase(it);
    }

    qDebug() << "not found port: " << portName << " devName: " << devName << " device."
             << " current list:" << m_pListItem->count();
}

void Ukui4SystemVolumeWidget::addDevicePortItem(std::shared_ptr<IDevicePortItem> item)
{
    m_pListItem->insertItem(item);
}

void Ukui4SystemVolumeWidget::removeDevicePortItem(const QString& name, const QVariantList& v)
{

}

void Ukui4SystemVolumeWidget::setCurrentIndex(int idx)
{
    if (idx >= m_pListItem->count() || idx == -1) {
        qDebug() << idx << " outof range, invaild index...";
        return;
    }

    m_pListItem->setCurrentIndex(idx);
}

void Ukui4SystemVolumeWidget::vauleChangedSlots(int vaule)
{
    Q_EMIT vauleChanged(vaule);
}
