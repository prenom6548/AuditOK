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

#include "Ukui2SystemVolumeSliderItem.h"

#include <QHBoxLayout>
#include <QVariant>

Ukui2SystemVolumeSlider::Ukui2SystemVolumeSlider(QSlider *s) : m_pSlider(s)
{
    m_pSlider->setOrientation(Qt::Horizontal);
    m_pSlider->setFocusPolicy(Qt::StrongFocus);
    m_pSlider->setProperty("needTranslucent", true); // Increase translucent effect
    m_pSlider->setFixedSize(200, 22);
    m_pSlider->setRange(0, 100);
}

void Ukui2SystemVolumeSlider::setRang(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui2SystemVolumeSlider::setValue(int value)
{
    if (m_pSlider) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(value);
        m_pSlider->blockSignals(false);
    }
}

QWidget* Ukui2SystemVolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui2SystemVolumeSliderItem::Ukui2SystemVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider> slider, QLabel* percLabel, QPushButton* iconBtn, const VolumeSliderType& type, QWidget* parent) :
    ITrayVolumeSliderItem(slider, iconBtn), m_pPercentLabel(percLabel), m_type(type), QWidget(parent)
{
    initUi();
    initSlots();
}

void Ukui2SystemVolumeSliderItem::initUi()
{
    m_pIconBtn->setFixedSize(36, 36);
    m_pIconBtn->setCheckable(true);
    m_pIconBtn->setProperty("isRoundButton", true);    // 圆形按钮
    m_pIconBtn->setProperty("useButtonPalette", true); // 灰色按钮
    m_pIconBtn->setProperty("needTranslucent", true);  // 灰色半透明按钮
    m_pPercentLabel->setText("0%");

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addSpacing(16);
    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacing(13);
    hLayout->addWidget(m_pVolumeSlider->getWidget());
    hLayout->addSpacing(15);
    hLayout->addWidget(m_pPercentLabel);
    hLayout->addSpacing(19);
    setLayout(hLayout);
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
}

void Ukui2SystemVolumeSliderItem::initSlots()
{
    connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));
}

QWidget* Ukui2SystemVolumeSliderItem::getWidget()
{
    return this;
}

void Ukui2SystemVolumeSliderItem::setIcon(const QIcon& icon)
{
    m_pIconBtn->setIcon(icon);
}

void Ukui2SystemVolumeSliderItem::setValue(int value)
{
    updateMuteBtnIcon(value);
    m_pVolumeSlider->setValue(value);
    setPercent(value);
}

QPushButton* Ukui2SystemVolumeSliderItem::getMuteButton()
{
    return m_pIconBtn;
}

void Ukui2SystemVolumeSliderItem::setPercent(int percent)
{
    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void Ukui2SystemVolumeSliderItem::valueChangedSlots(int value)
{
    updateMuteBtnIcon(value);
    setPercent(value);
}

void Ukui2SystemVolumeSliderItem::updateMuteBtnIcon(int value, bool muted)
{
    QString iconStr;
    if (muted) {
        iconStr = iconNameOutputs[0];
    }
    else if(value <= 0){
        iconStr = iconNameOutputs[0];
    }
    else if (value > 0 && value <= 33) {
        iconStr = iconNameOutputs[1];
    }
    else if (value >33 && value <= 66) {
        iconStr = iconNameOutputs[2];
    }
    else {
        iconStr = iconNameOutputs[3];
    }

    m_pIconBtn->setIcon(QIcon::fromTheme(iconStr));
}

Ukui2DeviceVolumeSliderItem::Ukui2DeviceVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider> slider, QLabel* descLabel, QPushButton* muteBtn, QPushButton* iconBtn, const VolumeSliderType& type, QWidget* parent) :
    ITrayVolumeSliderItem(slider, iconBtn), m_pDescLabel(descLabel), m_pMuteBtn(muteBtn), m_type(type), QWidget(parent)
{
    initUi();
    initSlots();
}

void Ukui2DeviceVolumeSliderItem::initUi()
{
    m_pIconBtn->setFixedSize(36, 36);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addSpacing(16);
    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacing(13);
    hLayout->addWidget(m_pVolumeSlider->getWidget());
    hLayout->addSpacing(15);
    hLayout->addWidget(m_pMuteBtn);
    hLayout->addSpacing(19);
    hLayout->setSpacing(0);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addWidget(m_pDescLabel);
    vLayout->addLayout(hLayout);
    vLayout->setSpacing(10);
    setLayout(vLayout);
    vLayout->setContentsMargins(0, 0, 0, 0);

    m_pIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                              "padding-left:0px;}"
                              "QPushButton:hover {"
                              "background-color: #00000000;"
                              "color: white;}");

    m_pMuteBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                              "padding-left:0px;}"
                              "QPushButton:hover {"
                              "background-color: #00000000;"
                              "color: white;}");
}

void Ukui2DeviceVolumeSliderItem::initSlots()
{
    connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));
}

QWidget* Ukui2DeviceVolumeSliderItem::getWidget()
{
    return this;
}

void Ukui2DeviceVolumeSliderItem::setIcon(const QIcon& icon)
{
    m_pIconBtn->setIcon(icon);
}

void Ukui2DeviceVolumeSliderItem::setValue(int value)
{
    updateMuteBtnIcon(value);
    m_pVolumeSlider->setValue(value);
    setPercent(value);
}

QPushButton* Ukui2DeviceVolumeSliderItem::getMuteButton()
{
    return m_pMuteBtn;
}

void Ukui2DeviceVolumeSliderItem::setPercent(int percent)
{
//    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void Ukui2DeviceVolumeSliderItem::setDescription(const QString& name)
{
    m_pDescLabel->setText(name);
}

void Ukui2DeviceVolumeSliderItem::valueChangedSlots(int value)
{
    updateMuteBtnIcon(value);
    setPercent(value);
}

void Ukui2DeviceVolumeSliderItem::updateMuteBtnIcon(int value, bool muted)
{
    QString iconStr;
    if (muted) {
        iconStr = iconNameOutputs[0];
    }
    else if(value <= 0){
        iconStr = iconNameOutputs[0];
    }
    else if (value > 0 && value <= 33) {
        iconStr = iconNameOutputs[1];
    }
    else if (value >33 && value <= 66) {
        iconStr = iconNameOutputs[2];
    }
    else {
        iconStr = iconNameOutputs[3];
    }

    m_pMuteBtn->setIcon(QIcon::fromTheme(iconStr));
}

Ukui2AppVolumeSlider::Ukui2AppVolumeSlider(QSlider *s) : m_pSlider(s)
{
    m_pSlider->setOrientation(Qt::Horizontal);
}

void Ukui2AppVolumeSlider::setRang(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui2AppVolumeSlider::setValue(int value)
{
    if (m_pSlider) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(value);
        m_pSlider->blockSignals(false);
    }
}

QWidget* Ukui2AppVolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui2AppVolumeSliderItem::Ukui2AppVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider> slider, QPushButton* iconBtn, QPushButton* muteBtn, QLabel* label, QWidget* parent) :
    ITrayVolumeSliderItem(slider, iconBtn), m_pMuteBtn(muteBtn), m_pDisplayLabel(label), QWidget(parent)
{
    initUi();
    initSlots();
}

void Ukui2AppVolumeSliderItem::initUi()
{
    m_pIconBtn->setFixedSize(32,32);
    m_pIconBtn->setFlat(true);
    m_pIconBtn->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacing(18);
    hLayout->addWidget(m_pVolumeSlider->getWidget());
    hLayout->addSpacing(12);
    hLayout->addWidget(m_pMuteBtn);
    hLayout->setSpacing(0);
//    hLayout->setContentsMargins(11, 0, 24, 0);
    hLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addWidget(m_pDisplayLabel);
    vLayout->addLayout(hLayout);
    vLayout->setSpacing(14);
    vLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(vLayout);

    m_pIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                              "padding-left:0px;}"
                              "QPushButton:hover {"
                              "background-color: #00000000;"
                              "color: white;}");
}

void Ukui2AppVolumeSliderItem::initSlots()
{
    connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));
}

void Ukui2AppVolumeSliderItem::setIcon(const QIcon &icon)
{
    m_pIconBtn->setIcon(icon);
}

QWidget* Ukui2AppVolumeSliderItem::getWidget()
{
    return this;
}

void Ukui2AppVolumeSliderItem::setValue(int value)
{
    m_pVolumeSlider->setValue(value);
    setPercent(value);
}

QPushButton* Ukui2AppVolumeSliderItem::getMuteButton()
{
    return m_pMuteBtn;
}

void Ukui2AppVolumeSliderItem::setPercent(int percent)
{
//    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void Ukui2AppVolumeSliderItem::valueChangedSlots(int value)
{
    setPercent(value);
}
