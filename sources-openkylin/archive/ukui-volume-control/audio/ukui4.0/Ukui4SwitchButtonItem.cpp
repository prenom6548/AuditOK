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

#include "Ukui4SwitchButtonItem.h"
#include <QHBoxLayout>
#include <QDBusMessage>
#include <QDebug>
#include "ClientMethod.h"
#include <kysdk/applications/accessinfohelper.h>

using namespace UkuiAudioFramwork;
using namespace kdk;

Ukui4SwitchButton::Ukui4SwitchButton(QPushButton *btn, const QString& objName)
    : m_pSwitchBtn(btn)
{
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSwitchBtn, "Audio", objName, "");
}

bool Ukui4SwitchButton::isChecked() const
{
    return m_pSwitchBtn->isChecked();
}

void Ukui4SwitchButton::setChecked(bool checked)
{
    m_pSwitchBtn->blockSignals(true);
    m_pSwitchBtn->setChecked(checked);
    m_pSwitchBtn->blockSignals(false);
}

QWidget* Ukui4SwitchButton::getWidget()
{
    return m_pSwitchBtn;
}

Ukui4SwitchButtonItem::Ukui4SwitchButtonItem(std::shared_ptr<ISwitchButton> btn, QLabel* label, SwitchButtonType type, QWidget* parent)
    : ISwitchButtonItem(btn, label, type), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui4SwitchButtonItem::initUi()
{
    setFixedHeight(60);

    m_pDisplayLabel->setFixedHeight(28);
    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pDisplayLabel);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hlayout->addWidget(m_pSwitchBtn->getWidget());
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hlayout);
}

void Ukui4SwitchButtonItem::initSlots()
{
//    connect(m_pSwitchBtn->getWidget(), SIGNAL(stateChanged(bool)), this, SLOT(stateChangedSlots(bool)));
}

QWidget* Ukui4SwitchButtonItem::getWidget()
{
    return this;
}

void Ukui4SwitchButtonItem::stateChangedSlots(bool state)
{
    QString method = "";

    switch (m_type) {
    case SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST:
        method = "setVolumeBoostStatus";
        break;
    case SwitchButtonType::SWITCH_BUTTON_MONO:
        method = "setMonoStatus";
        break;
    case SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL:
        method = "setEchoCancelStatus";
        break;
    case SwitchButtonType::SWITCH_BUTTON_LOOPBACK:
        method = "setLoopbackStatus";
        break;
    case SwitchButtonType::SWITCH_BUTTON_STARTUP:

        break;
    case SwitchButtonType::SWITCH_BUTTON_POWEROFF:

        break;
    case SwitchButtonType::SWITCH_BUTTON_LOGOUT:

        break;
    case SwitchButtonType::SWITCH_BUTTON_WAKEUP:

        break;
    case SwitchButtonType::SWITCH_BUTTON_ALERT:
        method = "setAlertStatus";
        break;
    default:
        break;
    }

    qDebug() << "Ukui4SwitchButtonItem::stateChangedSlots, type: " << EnumToInt(m_type) << ", state: " << state;
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             SETTINGS_INTERFACE,
                                             method,
                                             state);
}

Ukui4SwitchButtonWithDescItem::Ukui4SwitchButtonWithDescItem(std::shared_ptr<ISwitchButton> btn, QLabel* disLabel, QLabel* descLabel, SwitchButtonType type, QWidget* parent)
    : ISwitchButtonItem(btn, disLabel, type), m_pDescLabel(descLabel), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui4SwitchButtonWithDescItem::initUi()
{
    setFixedHeight(90);

    QPalette palette = m_pDescLabel->palette();
    QColor color = qApp->property("kfont-secondary").value<QBrush>().color();
    palette.setColor(QPalette::Text, color);
    m_pDescLabel->setPalette(palette);

    m_pDisplayLabel->setFixedHeight(28);
    m_pDescLabel->setFixedHeight(28);
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addWidget(m_pDisplayLabel);
    vLayout->addWidget(m_pDescLabel);
    vLayout->addStretch();
    vLayout->setSpacing(0);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addLayout(vLayout);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hLayout->addWidget(m_pSwitchBtn->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
//    setFrameShape(QFrame::Shape::Box);
}

void Ukui4SwitchButtonWithDescItem::initSlots()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* themeName = new QGSettings("org.ukui.style");
        QObject::connect(themeName, &QGSettings::changed, this, [this](const QString& key){
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor"
                    || key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                qDebug() << "Ukui4SwitchButtonWithDescItem::initSlots QGSettings::changed key:" << key;

                QPalette palette = m_pDescLabel->palette();
                QColor color = qApp->property("kfont-secondary").value<QBrush>().color();
                palette.setColor(QPalette::Text, color);
                m_pDescLabel->setPalette(palette);
}
        });
    }
}

QWidget* Ukui4SwitchButtonWithDescItem::getWidget()
{
    return this;
}
