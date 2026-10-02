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
#ifndef DEFAULTAPP_H
#define DEFAULTAPP_H

#include <QObject>
#include <QtPlugin>
#include <QComboBox>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QtDBus>
#include <QtConcurrent>
#include <QTime>
#include <QIcon>
#include <QFileSystemWatcher>
#include <QDBusInterface>
#include <QMap>
#include <KService>


#include "shell/interface.h"
#include "mthread.h"
#include "comboxwidget.h"
#include "settinggroup.h"
#include "pushbuttonwidget.h"

#include "defaultappui.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include "service.h"

namespace Ui {
class DefaultAppWindow;
}

class DefaultApp : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DefaultApp();
    ~DefaultApp();
    void preInitialize();
    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

    void initDefaultUI();
    void setupCompenent();
    void initSlots();
    void initSearchText();

    void load(ComboxWidget *widget, AppInfoService *appInfoService,
              const char *mimeType, const QString &type = NULL);
    void connectToServer();
    void findSelectItem(QComboBox *combox);

private:
    void loadData(AppInfoService *appInfoService, const char *mimeType);
    void kSycocaChangedSlot();

private:
    DefaultAppUi *mDefaultWidget = nullptr;

    QString pluginName;
    int pluginType;
    QDBusInterface *m_cloudInterface;

    QString mDefaultString = NULL;
    QString mSelectString = NULL;
    QString mLocalMimefile = NULL;

    QMap <const char *, QString> mSysDefaultList;
    QDBusInterface  *mDefaultUkccDbus   = nullptr;
    bool mFirstLoad;
    AppInfoService browserService;
    AppInfoService imageService;
    AppInfoService mailService;
    AppInfoService audioService;
    AppInfoService videoService;
    AppInfoService textService;

public slots:
    void browserComBoBox_changed_cb(int index);
    void mailComBoBox_changed_cb(int index);
    void imageComBoBox_changed_cb(int index);
    void audioComBoBox_changed_cb(int index);
    void videoComBoBox_changed_cb(int index);
    void textComBoBox_changed_cb(int index);
    void keyChangedSlot(const QString &key);
    void reset();
    void defaultChangedSlot();
    void defaultMimeTypeChangedSlot(const QString& mimeType, const QString& appid);
};

#endif // DEFAULTAPP_H
