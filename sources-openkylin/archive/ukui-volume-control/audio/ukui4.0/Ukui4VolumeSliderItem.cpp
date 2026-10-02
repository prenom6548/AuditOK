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

#include "Ukui4VolumeSliderItem.h"

#include <QDebug>
#include <QPen>
#include <QHBoxLayout>
#include <kysdk/applications/kslider.h>
#include <kysdk/applications/accessinfohelper.h>

#include "customstyle.h"

using namespace kdk;

Ukui4VolumeSlider::Ukui4VolumeSlider(QSlider *s, const QString& objName)
    : m_pSlider(s)
{
    m_pSlider->setOrientation(Qt::Horizontal);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSlider, "Audio", objName, "");
}

void Ukui4VolumeSlider::setRange(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui4VolumeSlider::setValue(int value)
{
    if (m_pSlider) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(value);
        m_pSlider->blockSignals(false);
    }
}

void Ukui4VolumeSlider::setEnabled(bool enable)
{
    if (m_pSlider)
        m_pSlider->setEnabled(enable);
}

bool Ukui4VolumeSlider::isEnabled() const
{
    return m_pSlider->isEnabled();
}

uint32_t Ukui4VolumeSlider::getValue() const
{
    return m_pSlider->value();
}

QWidget* Ukui4VolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui4VolumeSliderItem::Ukui4VolumeSliderItem(std::shared_ptr<IVolumeSlider> slider, QLabel* disLabel, QLabel* percLabel, QPushButton* muteBtn, VolumeSliderType type, QWidget* parent)
    : IVolumeSliderItem(slider, disLabel), m_pPercentLabel(percLabel), m_pMuteBtn(muteBtn), m_type(type), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui4VolumeSliderItem::initUi()
{
    m_pMuteBtn->setFocusPolicy(Qt::NoFocus);
    m_pMuteBtn->setFixedSize(24, 24);
    m_pMuteBtn->setFlat(true);
    m_pMuteBtn->setProperty("setClickPen", QPen(Qt::NoPen));
    m_pMuteBtn->setProperty("setHoverPen", QPen(Qt::NoPen));
    m_pMuteBtn->setProperty("setClickBrush", QBrush(Qt::transparent));
    m_pMuteBtn->setProperty("setHoverBrush", QBrush(Qt::transparent));

    switch (m_type) {
    case VolumeSliderType::VOLUME_SLIDER_OUTPUT: {
        KDK_EXTEND_ALL_INFO_FORMAT(m_pMuteBtn, "Audio", "volumeSliderOutPut", "");
        break;
    }
    case VolumeSliderType::VOLUME_SLIDER_INPUT: {
        KDK_EXTEND_ALL_INFO_FORMAT(m_pMuteBtn, "Audio", "volumeSliderInPut", "");
        break;
    }
    default:
        break;
    }

    setMinimumSize(550, 60);
    setMaximumSize(16777215, 60);
    m_pDisplayLabel->setFixedWidth(300);
    m_pDisplayLabel->setFixedHeight(28);
//    m_pPercentLabel->setFixedHeight(28);
    updateLabelWidth(); // 动态刷新标签宽度

    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pDisplayLabel);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pMuteBtn);
    // hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pVolumeSlider->getWidget());
    // hlayout->addItem(new QSpacerItem(13, 20, QSizePolicy::Maximum));
    hlayout->addWidget(m_pPercentLabel);
    hlayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Maximum));
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hlayout);
//    setFrameShape(QFrame::Shape::Box);
    qApp->installEventFilter(this);
}

void Ukui4VolumeSliderItem::initSlots()
{
    //connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));
}

QWidget* Ukui4VolumeSliderItem::getWidget()
{
    return this;
}

void Ukui4VolumeSliderItem::setValue(int value)
{
    if (m_pVolumeSlider->isEnabled()) {

        m_pVolumeSlider->setValue(value);
        setPercent(value);
        updateIcon(value);
    }
}

void Ukui4VolumeSliderItem::setEnabled(bool enable)
{
    m_pMuteBtn->setEnabled(enable);
    m_pVolumeSlider->setEnabled(enable);
}

QPushButton* Ukui4VolumeSliderItem::getMuteBtn() const
{
    return m_pMuteBtn;
}

void Ukui4VolumeSliderItem::setMute(bool muted)
{
    m_muted = muted;
    updateIcon(m_pVolumeSlider->getValue());
}

void Ukui4VolumeSliderItem::setPercent(int percent)
{
    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void Ukui4VolumeSliderItem::setIcon(const QIcon& icon)
{
    m_pMuteBtn->setIcon(icon);
}

void Ukui4VolumeSliderItem::updateIcon(int value)
{
    auto iconStr = "application-x-desktop";

    switch (m_type) {
    case VolumeSliderType::VOLUME_SLIDER_INPUT: {
        if (m_muted ||  value <= 0) {
            iconStr = iconNameInputs[0];
        }
        else if (value > 0 && value <= 33) {
            iconStr = iconNameInputs[1];
        }
        else if (value >33 && value <= 66) {
            iconStr = iconNameInputs[2];
        }
        else {
            iconStr = iconNameInputs[3];
        }
        break;
    }
    case VolumeSliderType::VOLUME_SLIDER_OUTPUT: {
        if (m_muted ||  value <= 0) {
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
        break;
    }
    default:
        break;
    }

    setIcon(QIcon::fromTheme(QString(iconStr)));
}

void Ukui4VolumeSliderItem::updateLabelWidth()
{
    QFontMetrics metrics(qApp->font());
    int textWidth = metrics.horizontalAdvance("100%") + 1; // 增加一些边距
    m_pPercentLabel->setFixedWidth(textWidth);
}

bool Ukui4VolumeSliderItem::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange) {
        // 系统字体发生变化，更新标签宽度
        updateLabelWidth();
        // 刷新布局
        layout()->invalidate();
        layout()->activate();
    }
    return QFrame::eventFilter(watched, event);
}

void Ukui4VolumeSliderItem::valueChangedSlots(int value)
{
    setPercent(value);
    updateIcon(value);
}

Ukui4BalanceVolumeSlider::Ukui4BalanceVolumeSlider(QSlider *s, const QString& objName)
    : m_pSlider(s)
{
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSlider, "Audio", objName, "");
//    m_pSlider->setRange(-100, 100);
//    m_pSlider->setSingleStep(100);
//    m_pSlider->setTickInterval(100);
//    static_cast<kdk::KSlider*>(m_pSlider)->setSliderType(kdk::KSliderType::SmoothSlider);
//    m_pSlider->setOrientation(Qt::Horizontal);
//    m_pSlider->setFocusPolicy(Qt::StrongFocus);
//    m_pSlider->setFixedHeight(55);
}

void Ukui4BalanceVolumeSlider::setRange(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui4BalanceVolumeSlider::setValue(int value)
{
    if (m_pSlider) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(value);
        m_pSlider->blockSignals(false);
    }
}

void Ukui4BalanceVolumeSlider::setEnabled(bool enable)
{
    qDebug() << __func__ << "enable:" << enable;
    if (m_pSlider)
        m_pSlider->setEnabled(enable);
}

bool Ukui4BalanceVolumeSlider::isEnabled() const
{
    return m_pSlider->isEnabled();
}

uint32_t Ukui4BalanceVolumeSlider::getValue() const
{
    return m_pSlider->value();
}

QWidget* Ukui4BalanceVolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui4BalanceVolumeSliderItem::Ukui4BalanceVolumeSliderItem(std::shared_ptr<IVolumeSlider> slider, QLabel* disLabel, QLabel* leftLabel, QLabel* rightLabel, QWidget* parent)
    : IVolumeSliderItem(slider, disLabel), m_pLeftLabel(leftLabel), m_pRightLabel(rightLabel), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui4BalanceVolumeSliderItem::initUi()
{
    setMinimumSize(550, 60);
    setMaximumSize(16777215, 60);
    m_pDisplayLabel->setFixedWidth(300);
    m_pDisplayLabel->setFixedHeight(28);
    m_pLeftLabel->setFixedHeight(28);
    m_pRightLabel->setFixedHeight(28);

    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pDisplayLabel);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pLeftLabel);
    // hlayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pVolumeSlider->getWidget());
    // hlayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pRightLabel);
    hlayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Maximum));
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hlayout);
//    setFrameShape(QFrame::Shape::Box);
}

void Ukui4BalanceVolumeSliderItem::initSlots()
{

}

void Ukui4BalanceVolumeSliderItem::setMute(bool mute)
{
    // TODO
}

void Ukui4BalanceVolumeSliderItem::setEnabled(bool enable)
{
    qDebug() << __func__ << "enable:" << enable;
    if (m_pVolumeSlider)
        m_pVolumeSlider->setEnabled(enable);
}

void Ukui4BalanceVolumeSliderItem::setValue(int value)
{
    if (m_pVolumeSlider->isEnabled())
        m_pVolumeSlider->setValue(value);
}

QPushButton* Ukui4BalanceVolumeSliderItem::getMuteBtn() const
{
    return nullptr;
}

QWidget* Ukui4BalanceVolumeSliderItem::getWidget()
{
    return this;
}

Ukui4InputLevelVolumeSlider::Ukui4InputLevelVolumeSlider(QProgressBar *bar)
    : m_pProgressBar(bar)
{
    m_pProgressBar->setOrientation(Qt::Horizontal);
    m_pProgressBar->setStyle(new CustomStyle);
    m_pProgressBar->setTextVisible(false);
}

void Ukui4InputLevelVolumeSlider::setRange(int min, int max)
{
    m_pProgressBar->setRange(min, max);
}

void Ukui4InputLevelVolumeSlider::setValue(int value)
{
    if (m_pProgressBar)
        m_pProgressBar->setValue(value);
}

void Ukui4InputLevelVolumeSlider::setEnabled(bool enable)
{
    if (m_pProgressBar)
        m_pProgressBar->setEnabled(enable);
}

bool Ukui4InputLevelVolumeSlider::isEnabled() const
{
    return m_pProgressBar->isEnabled();
}

uint32_t Ukui4InputLevelVolumeSlider::getValue() const
{
    return m_pProgressBar->value();
}

QWidget* Ukui4InputLevelVolumeSlider::getWidget()
{
    return m_pProgressBar;
}

Ukui4InputLevelVolumeSliderItem::Ukui4InputLevelVolumeSliderItem(std::shared_ptr<IVolumeSlider> slider, QLabel* disLabel, QWidget* parent)
    : IVolumeSliderItem(slider, disLabel), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui4InputLevelVolumeSliderItem::initUi()
{
    setMinimumSize(550, 60);
    setMaximumSize(16777215, 60);
    m_pDisplayLabel->setFixedWidth(300);
    m_pDisplayLabel->setFixedHeight(28);

    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pDisplayLabel);
    hlayout->addItem(new QSpacerItem(25, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pVolumeSlider->getWidget());
    // hlayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Maximum));
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hlayout);
//    setFrameShape(QFrame::Shape::Box);
}

void Ukui4InputLevelVolumeSliderItem::initSlots()
{

}

QWidget* Ukui4InputLevelVolumeSliderItem::getWidget()
{
    return this;
}

void Ukui4InputLevelVolumeSliderItem::setValue(int value)
{
    if (m_pVolumeSlider->isEnabled())
        m_pVolumeSlider->setValue(value);
}

void Ukui4InputLevelVolumeSliderItem::setMute(bool)
{
    // TODO
}

void Ukui4InputLevelVolumeSliderItem::setEnabled(bool enable)
{
    if (m_pVolumeSlider)
        m_pVolumeSlider->setEnabled(enable);
}

QPushButton* Ukui4InputLevelVolumeSliderItem::getMuteBtn() const
{
    return nullptr;
}
