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
#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QObject>
#include <QtPlugin>
#include <QGSettings>
#include <QtDBus>
#include "screensaverpluginso.h"
#include <interface/interface.h>
#include "screensaverui.h"
#include <interface/mthread.h>
#include <interface/ukcccommon.h>
using namespace ukcc;

using namespace kdk;

class Screensaver : public QWidget, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Screensaver();
    ~Screensaver();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    void plugin_leave()Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    virtual QString translationPath() const {
        return QStringLiteral("/usr/share/ukui-screensaver/ukccplugins/i18n/%1.ts");
    }

public:
    void initOptions();
    void initContent();
    void initPreview();
    void initIdleTime();
    void initScreensaver();
    void initCustomSource();
    void initSwicthMode();
    void initSwitchTime();
    void initCustomText();
    void initTextPosition();
    void initShowBreakTimeUkui();
    void initShowBreakTimeCustom();
    void initLockscreen();
    void connectUiSignals();

private:
    void preInitialize();
    void connectToServer();

private Q_SLOTS:
    void keyChangedSlot(const QString &key);
    void dataChanged(const QString &key);

private:
    QString pluginName;
    bool mFirstLoad;
    int pluginType;
    std::unique_ptr<ScreensaverPlugin> screensaverPtr;

private:
    ScreensaverUi  *screensaverUi        = nullptr;
    QDBusInterface *screensaverInterface = nullptr;
    QString toChangeKey;
};

#endif // SCREENSAVER_H
