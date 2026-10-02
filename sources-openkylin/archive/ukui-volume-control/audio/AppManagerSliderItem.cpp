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

#include "AppManagerSliderItem.h"

#include <QApplication>
#include <QPen>
#include <QDebug>
#include <QVariant>
#include <QHBoxLayout>
#include <QGSettings>

AppManagerSliderItem::AppManagerSliderItem(std::shared_ptr<IVolumeSlider> slider, QLabel* disLabel, QLabel* percLabel, QPushButton* muteBtn, VolumeSliderType type, QWidget* parent)
    : IVolumeSliderItem(slider, disLabel), m_pPercentLabel(percLabel), m_pMuteBtn(muteBtn), m_type(type), QFrame(parent)
{
    initUi();
    initSlots();
}

void AppManagerSliderItem::initUi()
{
    setFixedSize(512, 60);
    setFrameShape(QFrame::Shape::Box);
    m_pVolumeSlider->setRange(0, 100);

    m_pMuteBtn->setFocusPolicy(Qt::NoFocus);
    m_pMuteBtn->setFixedSize(16, 16);
    m_pMuteBtn->setFlat(true);
    m_pMuteBtn->setProperty("setClickPen", QPen(Qt::NoPen));
    m_pMuteBtn->setProperty("setHoverPen", QPen(Qt::NoPen));
    m_pMuteBtn->setProperty("setClickBrush", QBrush(Qt::transparent));
    m_pMuteBtn->setProperty("setHoverBrush", QBrush(Qt::transparent));

    m_pDisplayLabel->setFixedSize(120, 36);
    m_pDisplayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // m_pVolumeSlider->getWidget()->setFixedSize(264, 55);
    m_pPercentLabel->setFixedSize(52, 28);
    m_pPercentLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pMuteBtn);
    // hLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pVolumeSlider->getWidget());
    hLayout->addWidget(m_pPercentLabel);
    hLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed));

    setLayout(hLayout);

    QString theme = "ukui-default";
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* gst = new QGSettings("org.ukui.style");
        if (gst->keys().contains("styleName")) {
            theme = gst->get("style-name").toString();
        }
    }

    auto pal = palette();
    QColor cl = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
    pal.setBrush(QPalette::Base, cl);
    setPalette(pal);
}

void AppManagerSliderItem::initSlots()
{
//    connect(m_pVolumeSlider->getWidget(), SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlots(int)));

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* themeName = new QGSettings("org.ukui.style");
        QObject::connect(themeName, &QGSettings::changed, this, [this](const QString& key){
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor"
                    || key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                auto pal = palette();
                QColor cl = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
                pal.setBrush(QPalette::Base, cl);
                setPalette(pal);
            }
        });
    }
}

QWidget* AppManagerSliderItem::getWidget()
{
    return this;
}

void AppManagerSliderItem::setValue(int value)
{
    if (m_pVolumeSlider->isEnabled()) {
        m_pVolumeSlider->setValue(value);
        setPercent(value);
        updateIcon(value);
    }
}

void AppManagerSliderItem::setEnabled(bool enable)
{
    m_pMuteBtn->setEnabled(enable);
    m_pVolumeSlider->setEnabled(enable);
}

QPushButton* AppManagerSliderItem::getMuteBtn() const
{
    return m_pMuteBtn;
}

void AppManagerSliderItem::setMute(bool muted)
{
    m_muted = muted;
    updateIcon(m_pVolumeSlider->getValue());
}

void AppManagerSliderItem::setPercent(int percent)
{
    m_pPercentLabel->setText(QString("%1").arg(percent) + "%");
}

void AppManagerSliderItem::setIcon(const QIcon& icon)
{
    m_pMuteBtn->setIcon(icon);
}

void AppManagerSliderItem::updateIcon(int value)
{
    auto iconStr = "application-x-desktop";

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

    setIcon(QIcon::fromTheme(QString(iconStr)));
}

void AppManagerSliderItem::valueChangedSlots(int value)
{
    // TODO
}
