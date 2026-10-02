/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "appsettingspanel.h"
#include "appwidget.h"
#include "preferences.h"
#include "utils.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#include <QDBusPendingCallWatcher>

AppSettingsPanel::AppSettingsPanel(QWidget *parent) 
    : QWidget(parent)
    , mSettingsPages(new QTabWidget())
    , mPref(new Preferences())
    , mKmreEnvReady(false)
    , mKmreDbus(new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus(), this))
{
    mSettingsPages->setTabShape(QTabWidget::Rounded);
    mSettingsPages->setTabPosition(QTabWidget::North);
    //Attention: Wayland下设置hide()后再立即设置show()会导致Qt窗口异常退出,故wayland下不设置托盘
#ifndef UKUI_WAYLAND
    AppWidget *trayWidget = new AppWidget(AppWidget::eSettings_Tray, mPref);
    trayWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //mSettingsPages->addTab(trayWidget, tr("Tray"));
    const QString origString1 = tr("Tray");
    auto [elided1, elideText1] = kmre::utils::getElideText(origString1, 60);
    mSettingsPages->addTab(trayWidget, elideText1);
    if (elided1) {
        mSettingsPages->setTabToolTip(0, origString1);
    }
#endif
    AppWidget *bootFullscreenWidget = new AppWidget(AppWidget::eSettings_BootFullScreen, mPref);
    bootFullscreenWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //mSettingsPages->addTab(bootFullscreenWidget, tr("FullScreen"));
    const QString origString2 = tr("FullScreen");
    auto [elided2, elideText2] = kmre::utils::getElideText(origString2, 60);
    mSettingsPages->addTab(bootFullscreenWidget, elideText2);
    if (elided2) {
        mSettingsPages->setTabToolTip(1, origString2);
    }

    mSettingsPages->setStyleSheet("QTabBar::tab{font-size:18pt; border-radius:4px; min-width:130px; min-height:35px;}"
                                  "QTabBar::tab:selected{background:#3790FA; color:#FFFFFF;}"
                                  "QTabWidget::tab-bar{left:30px; alignment: left;}");
    connect(mSettingsPages, SIGNAL(currentChanged(int)), this, SLOT(slotSettingsPageChanged(int)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(mSettingsPages);

    setLayout(layout);
}

void AppSettingsPanel::init()
{
    initSetting(0);
}

void AppSettingsPanel::initSetting(int index)
{
    if (index >= mSettingsPages->count()) {
        return;
    }

    //判断kmre环境是否已经运行
    if (!mKmreEnvReady) {
        QDBusPendingReply<QString> reply = mKmreDbus->asyncCall("GetPropOfContainer", kmre::utils::getUserName(), kmre::utils::getUid().toInt(), "sys.kmre.boot_completed");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [&, index] (QDBusPendingCallWatcher *w) {
            QString value;
            if (!w->isError()) {
                QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
                if (reply.isValid()) {
                    value = reply.value();
                }
            }
            w->deleteLater();

            mKmreEnvReady = (value == "1");
            AppWidget *appWidget = dynamic_cast<AppWidget*>(mSettingsPages->widget(index));
            if (appWidget) {
                appWidget->init(mKmreEnvReady);
            }
        });
    }
    else {
        AppWidget *appWidget = dynamic_cast<AppWidget*>(mSettingsPages->widget(index));
        if (appWidget) {
            appWidget->init(true);
        }
    }
}

void AppSettingsPanel::slotSettingsPageChanged(int index)
{
    initSetting(index);
}