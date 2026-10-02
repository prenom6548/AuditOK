/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "autotabletmodewidget.h"

#include <ukcc/interface/ukcccommon.h>
#include <kysdk/applications/kswitchbutton.h>
#include <QLabel>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

AutoTabletModeWidget::AutoTabletModeWidget(const QString& autoSwitchTabletText, QWidget* parent)
    : QFrame(parent)
{
    setContentsMargins(16, 10, 16, 10);
    setFrameShape(QFrame::Shape::Box);

    initLayout();
    initAutoTabletModeWidget(autoSwitchTabletText);
}

void AutoTabletModeWidget::initLayout()
{
    QVBoxLayout* tabletModeVLayout = new QVBoxLayout;
    tabletModeVLayout->setMargin(0);
    setLayout(tabletModeVLayout);
}

void AutoTabletModeWidget::initAutoTabletModeWidget(const QString& autoSwitchTabletText)
{
    QWidget* autoSwitchTabletWidget = createAutoTabletModeWidget(autoSwitchTabletText);
    autoSwitchTabletWidget->setMinimumHeight(40);
    layout()->addWidget(autoSwitchTabletWidget);
}

QWidget* AutoTabletModeWidget::createAutoTabletModeWidget(const QString& autoSwitchTabletText)
{
    QWidget* autoTabletModeWidget = new QWidget;
    QLabel* tabletModeText = new QLabel(autoSwitchTabletText);
    initTabletModeButton(autoTabletModeWidget);

    QHBoxLayout* autoTabletModeLayout = new QHBoxLayout(autoTabletModeWidget);
    autoTabletModeLayout->setContentsMargins(0, 0, 0, 0);
    autoTabletModeLayout->addWidget(tabletModeText);
    autoTabletModeLayout->addStretch();
    autoTabletModeLayout->addWidget(m_autoTabletModeButton);
    autoTabletModeWidget->setLayout(autoTabletModeLayout);

    return autoTabletModeWidget;
}

void AutoTabletModeWidget::initTabletModeButton(QWidget* widget)
{
    m_autoTabletModeButton = new kdk::KSwitchButton(widget);
    m_autoTabletModeButton->setChecked(getAutoSwitchTablet());
    connect(m_autoTabletModeButton, &kdk::KSwitchButton::clicked, this, [this](bool state) {
        setAutoSwitchTablet(state);
        ukcc::UkccCommon::buriedSettings("AutoSwitchTabletModeButton", "settings", state ? "true":"false");
    });

    QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                          "/",
                                          "com.kylin.statusmanager.interface",
                                          "modemonitor_change_signal",
                                          this,
                                          SLOT(setAutoSwitchTabletBtn(bool)));
}

bool AutoTabletModeWidget::getAutoSwitchTablet() const
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");

    if(!statusManager.isValid()) {
        qWarning() << "com.kylin.statusmanager.interface is invalid";
        return false;
    }

    QDBusReply<bool> reply = statusManager.call("get_modemonitor");
    if (!reply.isValid()) {
        qWarning() << reply.error();
        return false;
    }

    return reply.value();
}

void AutoTabletModeWidget::setAutoSwitchTablet(bool state)
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");

    if(!statusManager.isValid()) {
        qWarning() << "com.kylin.statusmanager.interface is invalid";
        return;
    }

    QDBusMessage res = statusManager.call("set_modemonitor",
                                          QVariant(state),
                                          QVariant("Touch Screen Plugin"),
                                          QVariant("Auto Switch Tablet Mode Button"));

    if (res.type() == QDBusMessage::ErrorMessage) {
        qWarning() << res.errorName() << res.errorMessage();
    }
}

void AutoTabletModeWidget::setAutoSwitchTabletBtn(bool checked)
{
    if (!m_autoTabletModeButton)
        return;
    if (m_autoTabletModeButton->isChecked() == checked)
        return;
    m_autoTabletModeButton->setChecked(checked);
}
