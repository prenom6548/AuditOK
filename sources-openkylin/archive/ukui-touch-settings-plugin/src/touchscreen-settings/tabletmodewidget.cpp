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

#include "tabletmodewidget.h"

#include <ukcc/interface/ukcccommon.h>
#include <kysdk/applications/kswitchbutton.h>
#include <QLabel>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

static const char* ICON_NAME = "dialog-warning";

TabletModeWidget::TabletModeWidget(QWidget* parent)
    : QFrame(parent)
    , m_tabletModeButton(nullptr)
    , m_tabletTextWidegt(nullptr)
{
    setContentsMargins(16, 10, 16, 10);
    setFrameShape(QFrame::Shape::Box);

    initLayout();

    initTabletModeWidget();
    initConnect();
}

void TabletModeWidget::initLayout()
{
    QVBoxLayout* tabletModeVLayout = new QVBoxLayout;
    tabletModeVLayout->setMargin(0);
    setLayout(tabletModeVLayout);
}

void TabletModeWidget::initTabletModeWidget()
{
    QWidget* tabletModeWidget = createTabletModeWidget();
    tabletModeWidget->setMinimumHeight(40);
    layout()->addWidget(tabletModeWidget);

    if (getCurrentTabletMode()) {
        m_tabletTextWidegt = createTabletTextWidget();
        layout()->setSpacing(0);
        layout()->addWidget(m_tabletTextWidegt);
    }
}

void TabletModeWidget::initConnect()
{
    QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                          "/",
                                          "com.kylin.statusmanager.interface",
                                          "modeChangeSignal",
                                          this,
                                          SLOT(onTabletModeChangedState(bool)));
}

QWidget* TabletModeWidget::createTabletModeWidget()
{
    QWidget* tabletModeWidget = new QWidget;
    //~ contents_path /TouchScreen/Show tablet mode in the sidebar
    QLabel* tabletModeText = new QLabel(tr("Show tablet mode in the sidebar"));
    m_tabletModeButton = new kdk::KSwitchButton(tabletModeWidget);
    m_tabletModeButton->setChecked(getTabletModeEnabled());
    m_tabletModeButton->setEnabled(!getCurrentTabletMode());
    connect(m_tabletModeButton, &kdk::KSwitchButton::clicked, this, [this](bool enabled) {
        setTabletModeEnabled(enabled);
        ukcc::UkccCommon::buriedSettings("TouchScreen", "setTabletModeEnabled", "settings", enabled ? "true":"false");
    });

    QHBoxLayout* tabletModeLayout = new QHBoxLayout(tabletModeWidget);
    tabletModeLayout->setContentsMargins(0, 0, 0, 0);
    tabletModeLayout->addWidget(tabletModeText);
    tabletModeLayout->addStretch();
    tabletModeLayout->addWidget(m_tabletModeButton);
    tabletModeWidget->setLayout(tabletModeLayout);

    return tabletModeWidget;
}

QWidget* TabletModeWidget::createTabletTextWidget() const
{
    QWidget* tabletTextWidget = new QWidget;
    QLabel* tabletModeText = new QLabel(tr(" Currently in tablet mode, please turn off tablet mode in the sidebar to activate this feature"));
    QLabel* tabletModeIcon = new QLabel;
    QPixmap iconPixmap = QIcon::fromTheme(ICON_NAME).pixmap(16,16);
    tabletModeIcon->setPixmap(iconPixmap);

    QHBoxLayout* tabletTextLayout = new QHBoxLayout(tabletTextWidget);
    tabletTextLayout->setContentsMargins(0, 0, 0, 0);
    tabletTextLayout->addWidget(tabletModeIcon);
    tabletTextLayout->addWidget(tabletModeText);
    tabletTextLayout->addStretch();
    tabletTextWidget->setLayout(tabletTextLayout);

    if(QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* styleSettings = new QGSettings("org.ukui.style", QByteArray(), tabletTextWidget);
        connect(styleSettings, &QGSettings::changed, tabletModeIcon, [tabletModeIcon](const QString& key){
            if(key == "iconThemeName") {
                QPixmap iconPixmap = QIcon::fromTheme(ICON_NAME).pixmap(16,16);
                tabletModeIcon->setPixmap(iconPixmap);
            }
        });
    }

    return tabletTextWidget;
}

bool TabletModeWidget::getTabletModeEnabled() const
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");
    QDBusReply<bool> reply = statusManager.call("isTabletModeEnabled");
    if (!reply.isValid()) {
        qWarning() << reply.error();
        return false;
    }

    return reply.value();
}

bool TabletModeWidget::getCurrentTabletMode() const
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");
    QDBusReply<bool> reply = statusManager.call("getCurrentTabletMode");
    if (!reply.isValid()) {
        qWarning() << reply.error();
        return false;
    }

    return reply.value();
}

void TabletModeWidget::setTabletModeEnabled(bool enabled)
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");
    QDBusMessage res = statusManager.call("setTabletModeEnabled",
                                          QVariant(enabled));

    if (res.type() == QDBusMessage::ErrorMessage) {
        qWarning() << res.errorName() << res.errorMessage();
    }
}

void TabletModeWidget::onTabletModeChangedState(bool state)
{
    if (state && nullptr == m_tabletTextWidegt) {
        m_tabletTextWidegt = createTabletTextWidget();
        this->layout()->setSpacing(0);
        this->layout()->addWidget(m_tabletTextWidegt);
    } else if (m_tabletTextWidegt != nullptr) {
         delete m_tabletTextWidegt;
         m_tabletTextWidegt = nullptr;
    }

    m_tabletModeButton->setEnabled(!state);
}
