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
#ifndef WALLPAPER_H
#define WALLPAPER_H

#include "shell/interface.h"
#include "wallpaperui.h"
#include "ukcccommon.h"
using namespace ukcc;
#include <QObject>
#include <QtPlugin>
#include <QFileDialog>
#include <QFutureWatcher>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

class Wallpaper : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Wallpaper();
    ~Wallpaper();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

public:
    void initContent();
    void initModes();
    void initTypes();
    void initMode();
    void initType();
    void initPictures();
    void connectUiSignals();
    void showLocalWpDialog();
    void loadPictureInfo();
    bool convertPixmapSlot(const QString &filename);
    void picUnitClickSlot(PictureUnit* picUnit, QString fn);
    void loadPictureInfoSlot();

private:
    void onlineButtonClickedSlot();
    bool getDefaultAppId(const char *contentType);

private:
    bool            mFirstLoad;
    QString         pluginName;
    int             pluginType;
    QString         toChangeKey        = "";
    WallpaperUi    *wallpaperUi        = nullptr;
    QDBusInterface *wallpaperInterface = nullptr;
    QList<PictureInfo*> m_picInfoList;
    QFutureWatcher<void> m_watcher;
    QStringList m_previewPictures;
    QStringList m_sourcePictures;

public Q_SLOTS:
    void dataChanged(const QString &key);
    void setVisibleBySecurity();
};

#endif // WALLPAPER_H
