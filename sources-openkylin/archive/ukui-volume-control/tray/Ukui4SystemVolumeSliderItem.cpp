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

#include "Ukui4SystemVolumeSliderItem.h"

#include <QHBoxLayout>
#include <QVariant>

Ukui4SystemVolumeSlider::Ukui4SystemVolumeSlider(QSlider *s) : m_pSlider(s)
{
    m_pSlider->setOrientation(Qt::Horizontal);
    m_pSlider->setOrientation(Qt::Horizontal);
    m_pSlider->setFocusPolicy(Qt::StrongFocus);
    m_pSlider->setProperty("needTranslucent", true); // Increase translucent effect
    m_pSlider->setFixedSize(286, 48);
    m_pSlider->setRange(0, 100);
}

void Ukui4SystemVolumeSlider::setRang(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui4SystemVolumeSlider::setValue(int value)
{
    if (m_pSlider) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(value);
        m_pSlider->blockSignals(false);
    }
}

QWidget* Ukui4SystemVolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui4SystemVolumeSliderItem::Ukui4SystemVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider> slider, QLabel* percLabel, QPushButton* iconBtn, const VolumeSliderType& type, QWidget* parent) :
    ITrayVolumeSliderItem(slider, iconBtn), m_pPercentLabel(percLabel), m_type(type), QWidget(parent)
{
    initUi();
    initSlots();
}

void Ukui4SystemVolumeSliderItem::initUi()
{
    m_pIconBtn->setFixedSize(36, 36);
    m_pIconBtn->setCheckable(true);
    m_pIconBtn->setProperty("isRoundButton", true);    // 圆形按钮
    m_pIconBtn->setProperty("useButtonPalette", true); // 灰色按钮
    m_pIconBtn->setProperty("needTranslucent", true);  // 灰色半透明按钮
    m_pPercentLabel->setText("0%");
//    m_pPercentLabel->setFixedSize(52, 48);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacing(3);
    hLayout->addWidget(m_pVolumeSlider->getWidget());
    hLayout->addWidget(m_pPercentLabel);
    hLayout->setSpacing(0);
    setLayout(hLayout);
    hLayout->setContentsMargins(11, 0, 24, 0);
}

void Ukui4SystemVolumeSliderItem::initSlots()
{
    connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));
}

QWidget* Ukui4SystemVolumeSliderItem::getWidget()
{
    return this;
}

void Ukui4SystemVolumeSliderItem::setIcon(const QIcon& icon)
{
    m_pIconBtn->setIcon(icon);
}

void Ukui4SystemVolumeSliderItem::setValue(int value)
{
    updateMuteBtnIcon(value);
    m_pVolumeSlider->setValue(value);
    setPercent(value);
}

QPushButton* Ukui4SystemVolumeSliderItem::getMuteButton()
{
    return m_pIconBtn;
}

void Ukui4SystemVolumeSliderItem::setPercent(int percent)
{
    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void Ukui4SystemVolumeSliderItem::valueChangedSlots(int value)
{
    updateMuteBtnIcon(value);
    setPercent(value);
}

void Ukui4SystemVolumeSliderItem::updateMuteBtnIcon(int value, bool muted)
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

Ukui4AppVolumeSlider::Ukui4AppVolumeSlider(QSlider *s) : m_pSlider(s)
{
    m_pSlider->setOrientation(Qt::Horizontal);
}

void Ukui4AppVolumeSlider::setRang(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui4AppVolumeSlider::setValue(int value)
{
    if (m_pSlider) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(value);
        m_pSlider->blockSignals(false);
    }
}

QWidget* Ukui4AppVolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui4AppVolumeSliderItem::Ukui4AppVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider> slider, QPushButton* btn, QLabel* percentLabel, QWidget* parent) :
    ITrayVolumeSliderItem(slider, btn), m_pPercentLabel(percentLabel), QWidget(parent)
{
    initUi();
    initSlots();
}

void Ukui4AppVolumeSliderItem::initUi()
{
    m_pIconBtn->setFixedSize(32,32);
    m_pIconBtn->setFlat(true);
    m_pIconBtn->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacing(3);
    hLayout->addWidget(m_pVolumeSlider->getWidget());
    hLayout->addWidget(m_pPercentLabel);
    hLayout->setSpacing(0);
    setLayout(hLayout);
    hLayout->setContentsMargins(11, 0, 24, 0);

    m_pIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                              "padding-left:0px;}"
                              "QPushButton:hover {"
                              "background-color: #00000000;"
                              "color: white;}");
}

void Ukui4AppVolumeSliderItem::initSlots()
{
    connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));
}

void Ukui4AppVolumeSliderItem::setIcon(const QIcon &icon)
{
    m_pIconBtn->setIcon(icon);
}

QWidget* Ukui4AppVolumeSliderItem::getWidget()
{
    return this;
}

void Ukui4AppVolumeSliderItem::setValue(int value)
{
    m_pVolumeSlider->setValue(value);
    setPercent(value);
}

QPushButton* Ukui4AppVolumeSliderItem::getMuteButton()
{
    // 暂时未提供禁音操作
    return nullptr;
}

void Ukui4AppVolumeSliderItem::setPercent(int percent)
{
    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void Ukui4AppVolumeSliderItem::valueChangedSlots(int value)
{
    setPercent(value);
}
