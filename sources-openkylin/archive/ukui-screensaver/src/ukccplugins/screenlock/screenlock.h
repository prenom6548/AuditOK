/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef SCREENLOCK_H
#define SCREENLOCK_H

#include <interface/mthread.h>
#include <interface/interface.h>
#include "screenlockui.h"
#include <QObject>
#include <QtPlugin>
#include <QFutureWatcher>

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
    /**
     * @brief getDefaultAppId
     * @param contentType
     * @return
     */
    bool getDefaultAppId(const char *contentType);

public Q_SLOTS:
    void keyChangedSlot(const QString &key);

public:
    void initContent();
    void initShowOnLogin();
    void initLockDatetimeEnable();
    void initWallpaper();
    void connectUiSignals();
    void showLocalWpDialog();
    void loadPictureInfo();
    void convertPixmapSlot(const QString &filename, int num);
    void picUnitClickSlot(PictureUnit* picUnit, QString fn);
    void loadPictureInfoSlot();

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
    QList<QString> m_picInfoList;
    QFutureWatcher<void> m_watcher;
    QStringList m_previewPictures;
    QStringList m_sourcePictures;
};

#endif // SCREENLOCK_H
