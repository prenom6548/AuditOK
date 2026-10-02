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
#include "shortcut.h"
#include "realizeshortcutwheel.h"
#include "ukcccommon.h"
using namespace ukcc;
#include <QDebug>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <glib/gi18n.h>

Shortcut::Shortcut() : mFirstLoad(true)
{
    pluginName = tr("Shortcut");
    pluginType = DEVICES;
    qRegisterMetaType<QStringPair>("QStringPair");
    qDBusRegisterMetaType<QStringPair>();
    qRegisterMetaType<QStringPairList>("QStringPairList");
    qDBusRegisterMetaType<QStringPairList>();

    qRegisterMetaType<KeyEntry>("KeyEntry");
    qDBusRegisterMetaType<KeyEntry>();
    qRegisterMetaType<KeyEntryList>("KeyEntryList");
    qDBusRegisterMetaType<KeyEntryList>();
}

Shortcut::~Shortcut()
{
    shortcutUi->deleteLater();
    shortcutUi = nullptr;
}

QString Shortcut::plugini18nName()
{
    return pluginName;
}

int Shortcut::pluginTypes()
{
    return pluginType;
}

QWidget *Shortcut::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        shortcutUi = new ShortcutUi;
        shortcutInterface = new QDBusInterface("org.ukui.ukcc.session",
                                                "/Shortcut",
                                                "org.ukui.ukcc.session.Shortcut",
                                                QDBusConnection::sessionBus(), this);
        if (shortcutInterface->isValid()) { // 判断服务是否存在
            QDBusMessage message = shortcutInterface->call("ping"); // 判断dbus路径是否存在
            if (message.type() == QDBusMessage::ErrorMessage && message.errorMessage().contains("No such object path", Qt::CaseInsensitive)) {
                qWarning()<<shortcutInterface<<":"<<message.errorMessage();
            } else {
                QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                                      "/Shortcut",
                                                      "org.ukui.ukcc.session.Shortcut",
                                                      "changed",
                                                      this,
                                                      SLOT(dataChanged(QString)));
                initContent();
                connectUiSignals();
                connectToServer();
            }
        } else {
            qCritical() << "org.ukui.ukcc.session.Wallpaper DBus error:" << shortcutInterface->lastError();
        }
    }
    return shortcutUi;
}

const QString Shortcut::name() const
{
    return QStringLiteral("Shortcut");
}

bool Shortcut::isShowOnHomePage() const
{
    return false;
}

QIcon Shortcut::icon() const
{
    return QIcon::fromTheme("ukui-shortcut-key-symbolic");
}

bool Shortcut::isEnable() const
{
    return true;
}

void Shortcut::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker, &MThread::keychangedsignal,this,&Shortcut::shortcutChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

void Shortcut::initContent()
{
    shortcutUi->initShortEntry();
    shortcutUi->initSystem();
    shortcutUi->initWindow();
    shortcutUi->initCustom();
}

void Shortcut::connectUiSignals()
{
    connect(shortcutUi, &ShortcutUi::deleteShortcut, this, [=](QString path) {
        deleteCustomShortcut(path);
        shortcutInterface->call("updateShortcut");
    });

    connect(shortcutUi, &ShortcutUi::updateSystemShortcut, this, [=](const QString &key, const QString &value){
        shortcutInterface->call("setSystemShortcut", key, value);
        shortcutInterface->call("updateShortcut");
    });

    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void Shortcut::setEnableBySecurity() {
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("shortcutEnable").toString();
    QStringList setItems = moduleSettings.split(",");
    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        QString key = item.at(0);
        QString value = item.at(1);
        qDebug() << "set item Name: " << key << value << shortcutUi->getCustomFrameList().size();
        if (key == "customFrame") {
            for (QFrame* frame : shortcutUi->getCustomFrameList()) {
                if (frame) {
                    frame->setEnabled(value == "true");
                }
            }
            shortcutUi->getCustomTitle()->setEnabled(value == "true");
            shortcutUi->getAddButton()->setEnabled(value == "true");
        }
    }
}

void Shortcut::deleteCustomShortcut(QString path)
{
    if (path.isEmpty()) {
        return;
    }
    DConfClient *client = dconf_client_new();
    if (!client) {
        qWarning() << Q_FUNC_INFO << "create dconf client failed";
        return;
    }

    auto resetKey = [&](const QString &keyPath) {
        GError *error = nullptr;
        const QByteArray keyPathBytes = keyPath.toLatin1();
        gboolean ok = dconf_client_write_sync(client, keyPathBytes.constData(), nullptr, nullptr, nullptr, &error);
        if (!ok) {
            qWarning() << Q_FUNC_INFO << "dconf reset key failed:" << keyPath
                       << (error ? error->message : "");
            if (error) {
                g_error_free(error);
            }
        }
    };

    resetKey(path + NAME_KEY);
    resetKey(path + ACTION_KEY);
    resetKey(path + BINDING_KEY);
    g_object_unref(client);
}

QString Shortcut::keyToLib(QString key)
{
    if (key.contains("Meta")) {
        key.replace("Meta", "Super");
    }

    if (key.contains("Start")) {
        key.replace("Start", "Super");
    }

    if (key.contains("Print", Qt::CaseInsensitive)) {
        key.replace("Print", "PrtSc", Qt::CaseInsensitive);
    }

    if (key.contains("↑")) {
        key.replace("↑", "Up");
    }
    if (key.contains("→")) {
        key.replace("→", "Right");
    }
    if (key.contains("←")) {
        key.replace("←", "Left");
    }
    if (key.contains("↓")) {
        key.replace("↓", "Down");
    }

    if (key.contains("+")) {
        QStringList keys = key.split("+");
        if (keys.count() == 2) {
            QString lower = keys.at(1);
            QString keyToLib = "<" + keys.at(0) + ">" + lower.toLower();
            qDebug() << "count = 2,keyToLib = " << keyToLib;
            return keyToLib;
        } else if (keys.count() == 3) {
            QString lower = keys.at(2);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + lower.toLower();
            qDebug() << "count = 3,keyToLib = " << keyToLib;
            return keyToLib;
        } else if (keys.count() == 4) {
            QString lower = keys.at(3);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + "<" + keys.at(2) + ">" + lower.toLower();
            qDebug() << "count = 4,keyToLib = " << keyToLib;
            return keyToLib;
        }
    }
    qDebug() << "count = 1,keyToLib = " << key;
    return key;
}

void Shortcut::shortcutChangedSlot()
{
    qInfo() << "receive cloud service signal";
    shortcutUi->clearCustomShorcutFrame();
    shortcutUi->initCustom();
}
