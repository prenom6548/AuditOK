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

#ifndef APP_MULTIPLIER_WIDGET_H
#define APP_MULTIPLIER_WIDGET_H

#include <QWidget>
#include <QMap>
#include <QListWidget>

#include "settingscrollcontent.h"
#include "settingsitem.h"

class QDBusInterface;
class QStackedLayout;
class SwitchButton;
class SettingsGroup;

class AppMultiplierWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit AppMultiplierWidget(QWidget *parent = 0);
    ~AppMultiplierWidget();

    void initAppMultiplier();
    void parseAppMultipliers(const QString &apps);
    SettingsItem *createSwitchWidget(const QString &packageName, const QString &appName, int multiplier);
    void clearAllItems();
    void checkInstalledApps();
    void getMainMultiwindowStatus();
    void getSubsMultiwindowStatus();

public slots:
    void onAppMultipliers(const QString &apps);
    void onPostResponseInfo(int id, const QString &pkgName, const QString &category, int ret, const QString &info);

private:
    QLabel *m_tipLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_noResultLabel = nullptr;
    QLabel *m_mainLabel = nullptr;
    QLabel *m_multiplier = nullptr;
    SwitchButton *m_mainAppMultiplierBtn = nullptr;
    QDBusInterface *m_systemBusInterface = nullptr;
    QDBusInterface *m_managerBusInterface = nullptr;
    QDBusInterface *m_windowBusInterface = nullptr;
    QMap<QString, SettingsItem*> m_switchWidgets;
    SettingsGroup *m_group = nullptr;
    QFrame *m_tipFrame = nullptr;
    QFrame *m_mainFrame = nullptr;
    QFrame *m_switchframe = nullptr;
    QStackedLayout *m_stackedLayout = nullptr;
    QMap<QString,QString> m_installedAppsMap;
    QString m_mainMultiWindowStatus;
    QListWidget *m_list = nullptr;
};

#endif // APP_MULTIPLIER_WIDGET_H
