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

#ifndef _APP_WIDGET_H
#define _APP_WIDGET_H

#include <QWidget>

#include "settingscrollcontent.h"

class QVBoxLayout;
class QLabel;
class AppView;
class QDBusInterface;
class Preferences;

class AppWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    typedef enum {
        eSettings_Tray,
        eSettings_BootFullScreen,
    }SettingsType;

    AppWidget(SettingsType type, Preferences *pref, QWidget *parent = 0);
    ~AppWidget() {}

    void init(bool ready);

public slots:
    void selectItemWithPkgName(QString pkgName);
    void slotCheckedChanged(QString pkgName, bool checked);

private:
    Preferences *mPref;
    SettingsType mSettingsType;
    QVBoxLayout *m_layout = nullptr;
    AppView *m_view = nullptr;
    QLabel *m_tipLabel = nullptr;

    void updateTips(const QString &tip);
};

#endif // _APP_WIDGET_H
