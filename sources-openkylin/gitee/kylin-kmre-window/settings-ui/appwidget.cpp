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

#include "appwidget.h"
#include "appview.h"
#include "utils.h"
#include "preferences.h"
#include "getkmreinterface.h"
#include "messagebox.h"
#include "appconfiguration.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#include <QDBusPendingCallWatcher>

AppWidget::AppWidget(SettingsType type, Preferences *pref, QWidget *parent) 
    : SettingScrollContent(parent)
    , mSettingsType(type)
    , mPref(pref)
{
    m_view = new AppView();
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_view->setVisible(false);
    m_view->setFixedSize(520,420);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setStyleSheet("QListWidget{border: 1px solid #E9E9E9;}");

    m_tipLabel = new QLabel();
    m_tipLabel->setStyleSheet("QLabel {color: #ff5a5a;}");
    m_tipLabel->setAlignment(Qt::AlignCenter);

    m_title->setVisible(false);
    QLabel *tiplabel = new QLabel;
    switch (mSettingsType) {
        case eSettings_Tray:
            tiplabel->setText(tr("Set the application displayed in the tray area"));
            break;
        case eSettings_BootFullScreen:
            tiplabel->setText(tr("Set the application booted with full screen"));
            break;
        default:
            break;
    }
    
    QHBoxLayout *titlelayout = new QHBoxLayout;
    titlelayout->addWidget(tiplabel);
    titlelayout->addStretch();

    m_layout = new QVBoxLayout();
    m_layout->setMargin(0);
    m_layout->addSpacing(8);
    m_layout->addWidget(tiplabel);
    m_layout->addWidget(m_view, 0, Qt::AlignLeft);
    m_layout->addStretch();
    m_layout->addWidget(m_tipLabel, 0, Qt::AlignCenter);

    QFrame *centralWidget = new QFrame;
    centralWidget->setLayout(m_layout);
    setContent(centralWidget);

    connect(m_view, &AppView::sigClear, this, [=] {
        updateTips(tr("No items!"));
    });
    connect(m_view, SIGNAL(sigCheckedChanged(QString, bool)), this, SLOT(slotCheckedChanged(QString, bool)));
}

void AppWidget::init(bool ready)
{
    if (ready) {
        m_tipLabel->setVisible(false);
        m_view->setVisible(true);

        switch (mSettingsType) {
            case eSettings_Tray:
                m_view->reloadItems(kmre::utils::getInstalledAppList(), 
                                    AppConfiguration::getInstance()->getAppListByConfig("Tray"));
                break;
            case eSettings_BootFullScreen:
                m_view->reloadItems(kmre::utils::getInstalledAppList(), 
                                    AppConfiguration::getInstance()->getAppListByConfig("BootFullScreen"));
                break;
            default:
            break;
        }
    }
    else {
        updateTips(tr("KMRE is not running!"));
    }
}

void AppWidget::updateTips(const QString &tip)
{
    m_view->setVisible(false);
    m_tipLabel->setVisible(true);
    m_tipLabel->setText(tip);
}

void AppWidget::selectItemWithPkgName(QString pkgName)
{
    m_view->selectItemWithPkgName(pkgName);
}

void AppWidget::slotCheckedChanged(QString pkgName, bool checked)
{
    switch (mSettingsType) {
        case eSettings_Tray:
            AppConfiguration::getInstance()->setAppTray(pkgName, checked);
            break;
        case eSettings_BootFullScreen:
            AppConfiguration::getInstance()->setAppBootFullScreen(pkgName, checked);
            break;
        default:
            break;
    }
}