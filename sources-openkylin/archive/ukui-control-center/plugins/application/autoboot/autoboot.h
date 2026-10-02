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
#ifndef AUTOBOOT_H
#define AUTOBOOT_H

#include <QObject>
#include <QtPlugin>

#include "shell/interface.h"
#include "ukcccommon.h"
using namespace ukcc;
#include "hoverwidget.h"
#include "switchwidget.h"
#include "settinggroup.h"
#include "ukccframe.h"
#include "autobootui.h"
#include "selectdesktopfiledialog.h"
#include <QFileDialog>


#include <QtDBus>
#include <QGSettings>
#include <QVBoxLayout>
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#define USR_CONFIG_DIR             "/usr/share/applications/"

enum Pos
{
    LOCALPOS,
    SYSTEMPOS
};


class ukFileDialog : public QFileDialog
{
public:
    explicit ukFileDialog(QWidget *parent = nullptr,
                         const QString &caption = QString(),
                         const QString &directory = QString(),
                         const QString &filter = QString())
        : QFileDialog(parent, caption, directory, filter)
    {
        blacklist.append(QString("%1%2").arg(USR_CONFIG_DIR).arg( "gparted.desktop")); //需要root权限才能打开，session无法打开
        blacklist.append(QString("%1").arg(USR_CONFIG_DIR) + "ukui-mimeapps.list");
        blacklist.append(QString("%1").arg(USR_CONFIG_DIR) + "mimeinfo.cache");
        blacklist.append(QString("%1").arg(USR_CONFIG_DIR) + "defaults.list");
        blacklist.append(QString("%1").arg(USR_CONFIG_DIR) + "bamf-2.index");
        blacklist.append(QString("%1").arg(USR_CONFIG_DIR) + "mimeapps.list");
        mAutobootDBus = new QDBusInterface("org.ukui.ukcc.session",
                                           "/Autoboot",
                                           "org.ukui.ukcc.session.Autoboot",
                                           QDBusConnection::sessionBus(), this);
    }
protected:
   void accept() override;

private:
   QStringList blacklist;
   QDBusInterface *mAutobootDBus;

};

struct AutoApp
{
    QString bname;
    QString icon;
    QString name;
    bool hidden;
    int position;
    QString path;
    bool no_display;
    bool shown;
    QString comment;
    QString exec;
    QString description;
};
const QDBusArgument &operator<<(QDBusArgument &argument, const AutoApp &app);
const QDBusArgument &operator>>(const QDBusArgument &argument, AutoApp &app);
Q_DECLARE_METATYPE(AutoApp)

namespace Ui {
class AutoBoot;
}

class AutoBoot : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    AutoBoot();
    ~AutoBoot();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    void initAutoUI();
    void initItem(const AutoApp& it);
    void connectToServer();
    bool initConfig();
    void addItem(const QString &file);
    void setAutoPixmap(QPixmap &pixmap, const QString &icon);
    void setSortAppList();
    void addOtherDesktopSlot();

private:
    AutoBootUi *mAutoWidget;

    QString pluginName;
    int pluginType;

    QDBusInterface *m_cloudInterface;

    QMap<QString, AutoApp> appMaps;
    QMap<QString, AutoApp> localappMaps;
    QMap<QString, AutoApp> statusMaps;
    QMap<QLabel*, QString> mIconLabelMap;
    QMultiMap<QString, QWidget *> appgroupMultiMaps;

    QSignalMapper *checkSignalMapper;

    bool mFirstLoad;
    QGSettings *mQtSettings;
    QGSettings *mUkccSettings;
    QStringList whitelist;
    QStringList mAutoAppList;

    QDBusInterface *mAutobootDBus;
    QStringList m_addAutoBootList;
    QStringList m_sortAppList;
    QStringList m_writeSortAppList;
    SelectDesktopFileDialog* m_selectDesktopFileDialog = nullptr;
    ukFileDialog* m_ukFileDialog = nullptr;

public slots:
    void checkboxChangedSlot(QString bname);
    void keyChangedSlot(const QString &key);
    void addAppSlot();
};

#endif // AUTOBOOT_H
