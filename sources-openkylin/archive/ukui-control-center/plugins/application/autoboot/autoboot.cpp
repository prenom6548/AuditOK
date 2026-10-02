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
#include "autoboot.h"

#include <QThread>
#include <QSignalMapper>
#include <QDebug>
#include <QFont>
#include <QMouseEvent>
#include <QPushButton>
#include <QGSettings>
#include <QToolButton>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include "rmenu.h"
#include "mthread.h"
#include "selectdesktopfiledialog.h"

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


#include "kswitchbutton.h"
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#define ITEMWIDTH 522
#define ITEMHEIGHT 62
#define HEADHEIGHT 38

#define THEME_QT_SCHEMA  "org.ukui.style"
#define THEME_GTK_SCHEMA "org.mate.interface"

#define UKCC_SCHEMA        "org.ukui.control-center"
#define UKCC_AUTOAPP_EKY   "autoapp-list"

#define ICON_QT_KEY      "icon-theme-name"
#define ICON_GTK_KEY     "icon-theme"

#define LOCAL_CONFIG_DIR           "/.config/autostart/"
#define SYSTEM_CONFIG_DIR          "/etc/xdg/autostart/"

AutoBoot::AutoBoot() : mFirstLoad(true)
{
    pluginName = tr("Auto Start");
    pluginType = APPLICATION;
    m_addAutoBootList.clear();
}

AutoBoot::~AutoBoot()
{
    m_addAutoBootList.clear();
}

QString AutoBoot::plugini18nName()
{
    return pluginName;
}

int AutoBoot::pluginTypes()
{
    return pluginType;
}

QWidget *AutoBoot::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        mAutoWidget = new AutoBootUi;
        mAutobootDBus = new QDBusInterface("org.ukui.ukcc.session",
                                           "/Autoboot",
                                           "org.ukui.ukcc.session.Autoboot",
                                           QDBusConnection::sessionBus(), this);
        if (!mAutobootDBus->isValid()) {
            qCritical() << "org.ukui.ukcc.session.Autoboot DBus error:" << mAutobootDBus->lastError();
        } else {
            QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                                  "/Autoboot",
                                                  "org.ukui.ukcc.session.Autoboot",
                                                  "changed",
                                                  this,
                                                  SLOT(keyChangedSlot(QString)));
            initConfig();
            initAutoUI();
            connectToServer();
        }
    }
    return mAutoWidget;
}

const QString AutoBoot::name() const
{
    return QStringLiteral("Autostart");
}

bool AutoBoot::isShowOnHomePage() const
{
    return true;
}

QIcon AutoBoot::icon() const
{
    return QIcon::fromTheme("ukui-poweron-symbolic");
}

bool AutoBoot::isEnable() const
{
    return true;
}

void AutoBoot::initAutoUI()
{
    appgroupMultiMaps.clear();
    checkSignalMapper = new QSignalMapper(this);
    QMap <QString, QVariant> map = mAutobootDBus->property("statusMap").toMap();

    for (QMap<QString, QVariant>::iterator it = map.begin(); it != map.end(); it++) {
        AutoApp info;
        const QDBusArgument &dbusArgs = it.value().value<QDBusArgument>();
        dbusArgs >> info;
        statusMaps.insert(it.key(), info);
    }

    QList<QPair<QString, AutoApp>> addStatusList;
    m_writeSortAppList.clear();
    QStringList visitedAppList;
    for (const auto& app : m_sortAppList) {
        QMap<QString, AutoApp>::iterator iter = statusMaps.find(app);
        if (iter == statusMaps.end()) {
            continue;
        }
        QString appName = iter.value().name;
        QString name = iter.value().bname;
        if (m_addAutoBootList.contains(name)) {
            addStatusList.append(QPair<QString, AutoApp>(iter.key(), iter.value()));
            qDebug() << Q_FUNC_INFO << "addStatus app" << appName << name;
            continue;
        }
        qDebug() << Q_FUNC_INFO << "add item autoboot statusMaps" << appName << name << m_addAutoBootList;
        initItem(iter.value());
        visitedAppList.append(name);
    }

    for (QMap<QString, AutoApp>::iterator it = statusMaps.begin();
         it != statusMaps.end(); it++) {
        QString appName = it.value().name;
        QString name = it.value().bname;
        if (visitedAppList.contains(name)) {
            continue;
        }
        if (m_addAutoBootList.contains(name)) {
            addStatusList.append(QPair<QString, AutoApp>(it.key(), it.value()));
            qDebug() << Q_FUNC_INFO << "addStatus app" << appName << name;
            continue;
        }
        visitedAppList.append(name);
        initItem(it.value());
        qDebug() << Q_FUNC_INFO << "add item autoboot statusMaps" << appName << name << "not in visitedApp";
    }

    for (const auto& appPath : m_addAutoBootList) {
        for (int i = 0; i < addStatusList.size(); ++i) {
            QString path = addStatusList.at(i).second.bname;
            if (appPath == path) {
                initItem(addStatusList.at(i).second);
                qDebug() << Q_FUNC_INFO << "add item addStatus" << addStatusList.at(i).second.bname << m_addAutoBootList.size() << i;
                break;
            }
        }
    }
    setSortAppList();
    // 列表项全部创建后再建「添加」，避免其默认 Tab 序排在开关前面
    mAutoWidget->ensureAddButton();
    mAutoWidget->getAutobootWidget()->addWidget(mAutoWidget->getAddWidget());
    connect(checkSignalMapper, SIGNAL(mapped(QString)), this, SLOT(checkboxChangedSlot(QString)));
    connect(mAutoWidget->getAddBtn(), &AddButton::clicked, this, &AutoBoot::addAppSlot);
}

void AutoBoot::setSortAppList() {
    QGSettings ukcc("org.ukui.control-center");
    if (!ukcc.keys().contains("sortAppList")) {
        qDebug() << Q_FUNC_INFO << "org.ukui.control-center has no key sortAppList";
        return;
    }
    ukcc.set("sortAppList", m_writeSortAppList);
}

void AutoBoot::initItem(const AutoApp& it)
{
    QString appName = it.name;
    QPixmap pixmap;
    setAutoPixmap(pixmap, it.icon);
    if (pixmap.isNull() || appName.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "pixmap is null or appName is empty, skip" << appName;
        return;
    }
    QString bname = it.bname;
    if (!m_writeSortAppList.contains(bname)) {
        m_writeSortAppList.append(bname);
    }
    qDebug() << Q_FUNC_INFO << "add auto item" << appName << it.path << bname;

    SwitchWidget *baseWidget = new SwitchWidget(appName);
    KDK_EXTEND_ALL_INFO_FORMAT(baseWidget->switchButton(), "Autoboot", "", "switch widget of " + bname);
    baseWidget->switchButton()->setAccessibleName(appName);

    QLabel *iconLabel = new QLabel(baseWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(iconLabel, "Autoboot", "", "icon label of " + bname);
    iconLabel->setFixedSize(32, 32);

    iconLabel->setPixmap(pixmap);
    mIconLabelMap.insert(iconLabel, it.icon);
    baseWidget->insertWidget(0, iconLabel);

    baseWidget->setChecked(!it.hidden);
    checkSignalMapper->setMapping(baseWidget, bname);
    connect(baseWidget, SIGNAL(stateChanged(bool)), checkSignalMapper, SLOT(map()));
    appgroupMultiMaps.insert(bname, baseWidget);

    if (it.position == Pos::LOCALPOS) {
        qDebug() << Q_FUNC_INFO << "add auto item local" << appName << it.path;
        QToolButton *deBtn = new QToolButton(baseWidget);
        KDK_EXTEND_ALL_INFO_FORMAT(deBtn, "Autoboot", "", "toolbutton of" + bname);
        deBtn->setAutoRaise(true);
        deBtn->setProperty("useButtonPalette", true);
        deBtn->setPopupMode(QToolButton::InstantPopup);
        deBtn->setFixedSize(QSize(36, 36));
        deBtn->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
        deBtn->setAccessibleName(appName + "菜单");

        RMenu *pMenu = new RMenu(deBtn);

        deBtn->setMenu(pMenu);
        QAction* mDel = new QAction(tr("Delete"),this);
        KDK_OBJ_INFO_FORMAT(mDel);
        pMenu->addAction(mDel);
        connect(mDel, &QAction::triggered, this, [=](){
            UkccCommon::buriedSettings(name(), "autoboot item " + bname, QString("settings"), "delete from list");
            QMap<QString, AutoApp>::iterator iter = statusMaps.find(bname);
            if (iter == statusMaps.end()) {
                qDebug() << "AutoBoot Data Error";
                return;
            }
            mAutobootDBus->call("deleteLocalFile", bname);
            appgroupMultiMaps.erase(appgroupMultiMaps.find(bname));
            statusMaps.erase(iter);
            mIconLabelMap.erase(mIconLabelMap.find(iconLabel));
            baseWidget->close();
            if (m_addAutoBootList.contains(bname)) {
                m_addAutoBootList.removeAll(bname);
                qDebug() << Q_FUNC_INFO << "remove addAutoBootList name:" << bname;
            }
            if (m_writeSortAppList.contains(bname)) {
                m_writeSortAppList.removeAll(bname);
                qDebug() << Q_FUNC_INFO << "remove writeSortApp name:" << bname;
                setSortAppList();
            }
        });
        baseWidget->insertWidget(2, deBtn, 1, Qt::AlignRight);
    }
    mAutoWidget->getAutobootWidget()->addWidget(baseWidget);
}

void AutoBoot::addItem(const QString &file)
{
    qDebug() << Q_FUNC_INFO << "add file" << file;
    if (file.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "file is empty, skip addItem" << file;
        return;
    }
    QDBusReply<bool> reply = mAutobootDBus->call("addAutobootApp", file);
    if (!reply.value()) {
        qDebug() << Q_FUNC_INFO << "addAutobootApp return false" << file;
        return;
    }
    statusMaps.clear();
    QMap <QString, QVariant> map = mAutobootDBus->property("statusMap").toMap();

    for (QMap<QString, QVariant>::iterator it = map.begin(); it != map.end(); it++) {
        AutoApp info;
        const QDBusArgument &dbusArgs = it.value().value<QDBusArgument>();
        dbusArgs >> info;
        statusMaps.insert(it.key(), info);
    }
    QFileInfo fileinfo(file);
    AutoApp app = statusMaps[fileinfo.fileName()];
    if (app.bname.isEmpty())
        return;
    mAutoWidget->getAutobootWidget()->removeWidget(mAutoWidget->getAddWidget());
    QStringList pathList = file.split("/");
    QString newPath = pathList.at(pathList.size() - 1);
    m_addAutoBootList.append(newPath);
    qDebug() << Q_FUNC_INFO << "add autoboot" << app.name << file << newPath;
    initItem(app);
    setSortAppList();
    mAutoWidget->getAutobootWidget()->addWidget(mAutoWidget->getAddWidget());
}

void AutoBoot::setAutoPixmap(QPixmap &pixmap, const QString &icon)
{
    QFileInfo iconfile(QString("/usr/share/pixmaps/"+ icon + ".png"));
    QIcon currenticon = QIcon::fromTheme(icon);
    if (!currenticon.isNull()) {
        pixmap = currenticon.pixmap(QSize(32, 32));
    } else if (iconfile.exists()) {
        pixmap = QPixmap(iconfile.filePath()).scaled(32, 32);
    }
}


void AutoBoot::addOtherDesktopSlot() {
    QString filters = tr("Desktop files(*)");
    if (!m_ukFileDialog) {
        qDebug() << Q_FUNC_INFO << "ukc new dialog";
        m_ukFileDialog = new ukFileDialog(mAutoWidget);
        m_ukFileDialog->setDirectory(USR_CONFIG_DIR);
        m_ukFileDialog->setModal(true);
        m_ukFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        m_ukFileDialog->setViewMode(QFileDialog::List);
        m_ukFileDialog->setNameFilter(filters);
        m_ukFileDialog->setFileMode(QFileDialog::ExistingFile);
        m_ukFileDialog->setWindowTitle(tr("Select Autoboot Desktop"));
        m_ukFileDialog->setLabelText(QFileDialog::Accept, tr("Select"));
        m_ukFileDialog->setLabelText(QFileDialog::Reject, tr("Cancel"));
    }
    if (m_ukFileDialog->exec() != QDialog::Accepted) {
        return;
    }

    QString selectedfile;
    selectedfile = m_ukFileDialog->selectedFiles().first();
    addItem(selectedfile);
    m_selectDesktopFileDialog->close();
    UkccCommon::buriedSettings(QStringLiteral("Autoboot"), "add other to autoboot list", QString("settings"), selectedfile);
}

void AutoBoot::addAppSlot()
{
    if (m_selectDesktopFileDialog) {
        delete m_selectDesktopFileDialog;
        m_selectDesktopFileDialog = nullptr;
    }
    if (!m_selectDesktopFileDialog) {
        m_selectDesktopFileDialog = new SelectDesktopFileDialog(mAutoWidget);
    }

    m_selectDesktopFileDialog->show();
    connect(m_selectDesktopFileDialog, &SelectDesktopFileDialog::confirmed, this, [=](QString selectDesktopPath) {
        addItem(selectDesktopPath);
        UkccCommon::buriedSettings(QStringLiteral("Autoboot"), "add to autoboot list", QString("settings"), selectDesktopPath);
    });
    connect(m_selectDesktopFileDialog, &SelectDesktopFileDialog::addOtherApp, this, [=]() {
        addOtherDesktopSlot();
    });
}

void AutoBoot::checkboxChangedSlot(QString bname)
{
    foreach (QString key, appgroupMultiMaps.keys()) {
        if (key == bname) {
            UkccCommon::buriedSettings(name(), "whether " + bname + " auto startup", QString("settings"), ((KSwitchButton *)appgroupMultiMaps.value(key))->isChecked() ? "true" : "false");
            mAutobootDBus->call("saveAppStatus", bname, ((SwitchWidget *)appgroupMultiMaps.value(key))->isChecked());
        }
    }
}

void AutoBoot::keyChangedSlot(const QString &key)
{
    if (key == "boot") {
        mAutoWidget->resetUi();
        initAutoUI();
    } else if (key == "iconThemeName") {
        for (QMap<QLabel*, QString>::iterator it = mIconLabelMap.begin(); it != mIconLabelMap.end(); it++) {
            QPixmap pixmap;
            setAutoPixmap(pixmap, it.value());
            it.key()->setPixmap(pixmap);
        }
    }
}

void AutoBoot::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&AutoBoot::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

bool AutoBoot::initConfig()
{
    m_sortAppList.clear();
    QGSettings ukcc("org.ukui.control-center");
    if (ukcc.keys().contains("sortAppList")) {
        m_sortAppList = ukcc.get("sortAppList").toStringList();
        qDebug() << Q_FUNC_INFO << "load sort applist.size:" << m_sortAppList.size();
    }
    QDir localdir(QString(QDir::homePath()+LOCAL_CONFIG_DIR).toUtf8());
    if(localdir.exists()) {
      return true;
    } else {
       return localdir.mkdir(QDir::homePath()+LOCAL_CONFIG_DIR);
    }
}

void ukFileDialog::accept()
{
    QString selectedfile;
    selectedfile = this->selectedFiles().first();
    if (!mAutobootDBus->isValid()) {
        QFileDialog::accept();
        return;
    }

    QByteArray ba;
    ba = selectedfile.toUtf8();

    // 解析desktop文件
    GKeyFile *keyfile;
    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);
        qDebug() << Q_FUNC_INFO << "not desktop file, skip" << selectedfile;
        return;
    }
    bool no_display = g_key_file_get_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, FALSE);

    g_key_file_free(keyfile);
    qDebug() << Q_FUNC_INFO << "selectfile" << selectedfile << "no_display" << no_display;
    if (no_display || blacklist.contains(selectedfile)) {
        QMessageBox msg(qApp->activeWindow());
        msg.setIcon(QMessageBox::Warning);
        msg.setText(QObject::tr("Programs are not allowed to be added."));
        msg.exec();
    } else {
        QFileDialog::accept();
    }
}

const QDBusArgument &operator<<(QDBusArgument &argument, const AutoApp &app)
{
    argument.beginStructure();
    argument << app.bname;
    argument << app.icon;
    argument << app.name;
    argument << app.hidden;
    argument << app.position;
    argument << app.path;
    argument << app.no_display;
    argument << app.shown;
    argument << app.comment;
    argument << app.exec;
    argument << app.description;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AutoApp &app)
{
    argument.beginStructure();
    argument >> app.bname;
    argument >> app.icon;
    argument >> app.name;
    argument >> app.hidden;
    argument >> app.position;
    argument >> app.path;
    argument >> app.no_display;
    argument >> app.shown;
    argument >> app.comment;
    argument >> app.exec;
    argument >> app.description;
    argument.endStructure();
    return argument;
}
