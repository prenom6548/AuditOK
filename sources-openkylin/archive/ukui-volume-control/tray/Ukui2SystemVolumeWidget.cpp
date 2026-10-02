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

#include "Ukui2SystemVolumeWidget.h"

#include <QHBoxLayout>
#include <QVariant>
#include <QDebug>

#include "IMainWindow.h"

Ukui2SystemVolumeWidget::Ukui2SystemVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& map,
                                                 QPushButton* switchBtn, QPushButton* devVolumeBtn, QPushButton* appVolumeBtn,
                                                 IMainWindow* window, QWidget* parent) :
    m_sysVolumeSliderItemMap(map),
    m_pSwitchModuleBtn(switchBtn),
    m_pDeviceVolumeBtn(devVolumeBtn),
    m_pAppVolumeBtn(appVolumeBtn),
    m_pMainWid(std::move(window)),
    QWidget(parent)
{
    initUi();
    initSlots();
}

Ukui2SystemVolumeWidget::~Ukui2SystemVolumeWidget()
{
//    delete m_pMainWid;
}

void Ukui2SystemVolumeWidget::initUi()
{
    m_pOutputLabel = new QLabel(tr("Output Device"));
    m_pInputLabel = new QLabel(tr("Input Device"));

    m_pBtnWidget = new QWidget(this);
    m_pBtnWidget->setFixedSize(38, 320);

    QVBoxLayout* vLayout1 = new QVBoxLayout();
    vLayout1->addWidget(m_pDeviceVolumeBtn);
    vLayout1->addWidget(m_pAppVolumeBtn);
    vLayout1->setSpacing(3);
    vLayout1->setContentsMargins(0, 0, 0, 200);
    m_pBtnWidget->setLayout(vLayout1);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addWidget(m_pOutputLabel);
    vLayout->addWidget(m_sysVolumeSliderItemMap[1]->getWidget());
    vLayout->addWidget(m_pInputLabel);
    vLayout->addWidget(m_sysVolumeSliderItemMap[2]->getWidget());
    vLayout->setSpacing(14);
    vLayout->setContentsMargins(0, 0, 0, 0);

    m_sysVolumeSliderItemMap[1]->setIcon(QIcon::fromTheme("audio-card"));
    m_sysVolumeSliderItemMap[2]->setIcon(QIcon("/usr/share/ukui-media/img/audio-input-microphone.svg"));

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pBtnWidget);
    hLayout->addLayout(vLayout);
    hLayout->setContentsMargins(0, 0, 30, 30);
    setLayout(hLayout);
}

void Ukui2SystemVolumeWidget::initSlots()
{
    for (const auto& it : m_sysVolumeSliderItemMap) {
         connect(static_cast<QSlider*>(it.second->getSlider()->getWidget()), &QSlider::valueChanged, this, [this, it](int value) {

            if (0 == it.first || 1 == it.first)
                m_pMainWid->setVolume(1, value);
            else if (2 == it.first)
                m_pMainWid->setVolume(0, value);
        });

        connect(static_cast<QPushButton*>(it.second->getMuteButton()), &QPushButton::clicked, this, [this, it]() {
            if (0 == it.first || 1 == it.first)
                m_pMainWid->setMute(1, "");
            else if (2 == it.first)
                m_pMainWid->setMute(0, "");
        });
    }
}

void Ukui2SystemVolumeWidget::showWidget()
{
    this->show();
}

QWidget* Ukui2SystemVolumeWidget::getWidget()
{
    return this;
}

void Ukui2SystemVolumeWidget::setValue(int vaule)
{

}

void Ukui2SystemVolumeWidget::addDevice(const QString& iconName, const QVariantList& list)
{
}

void Ukui2SystemVolumeWidget::removeDevice(const QString& portName, const QString& devName)
{
}

void Ukui2SystemVolumeWidget::addDevicePortItem(std::shared_ptr<IDevicePortItem> item)
{
}

void Ukui2SystemVolumeWidget::removeDevicePortItem(const QString& name, const QVariantList& v)
{
}

void Ukui2SystemVolumeWidget::setCurrentIndex(int idx)
{
}

void Ukui2SystemVolumeWidget::vauleChangedSlots(int vaule)
{
    Q_EMIT vauleChanged(vaule);
}
