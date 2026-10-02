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

#ifndef APP_SETTINGS_PANEL_H
#define APP_SETTINGS_PANEL_H

#include <QWidget>
#include <QTabWidget>
#include "appwidget.h"

class Preferences;
class QDBusInterface;

class AppSettingsPanel : public QWidget
{
    Q_OBJECT

public:
    AppSettingsPanel(QWidget *parent = 0);
    ~AppSettingsPanel() {}

    void init();

private slots:
    void slotSettingsPageChanged(int index);

private:
    bool mKmreEnvReady;
    QDBusInterface *mKmreDbus;
    QTabWidget *mSettingsPages;
    Preferences* mPref;

    void initSetting(int index);
};

#endif // APP_SETTINGS_PANEL_H
