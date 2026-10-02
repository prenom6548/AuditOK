/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef SCREENLOCK_H
#define SCREENLOCK_H

#include <interface/mthread.h>
#include <interface/interface.h>
#include "screenlockui.h"
#include <QObject>
#include <QtPlugin>

#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

class Screenlock : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Screenlock();
    ~Screenlock();

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    virtual QString translationPath() const {
        return QStringLiteral("/usr/share/ukui-screensaver/ukccplugins/i18n/%1.ts");
    }

private:
    void connectToServer();

public Q_SLOTS:
    void keyChangedSlot(const QString &key);

public:
    void initContent();
    void initShowOnLogin();
    void initWallpaper();
    void connectUiSignals();
    void showLocalWpDialog();

public Q_SLOTS:
    void dataChanged(const QString &key);

private:
    QString        toChangeKey;
    QString        pluginName;
    int            pluginType;
    bool           mFirstLoad;
    ScreenlockUi   *screenlockui        = nullptr;
    QDBusInterface *screenlockInterface = nullptr;
    bool           bIsCloudService      = false;
};

#endif // SCREENLOCK_H
