/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "media-key-block-shutcut.h"
#include <QDir>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#define MEDIA_KEY_DEFAULT_NODE "org.ukui.SettingsDaemon.plugins.customized"
#define MEDIA_KEY_DEFAULT_FILE "/usr/share/ukui-config/%1/org.ukui.SettingsDaemon.plugins.customized.json"

class MediaKeyBlockShortcutPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MediaKeyBlockShortcutPrivate(QObject* parent = nullptr);
    ~MediaKeyBlockShortcutPrivate(){USD_LOG(LOG_DEBUG,"析构..");}

Q_SIGNALS:
    void blockChanged(QMap<QString, QStringList> blockName);
    void unblockChanged(QMap<QString, QStringList> blockName);
private Q_SLOTS:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void onConfigChanged(QString userName, QStringList fileList);
    void onSecurityExit();
public:
    void connectSignal();
    QMap<QString, QStringList> getBlockList();
private:
    void addBlock(const QString& service);
    void removeBlock(const QString& service);
    static QStringList getAllBlocklist(const QString& fileName);
    QString getSecurityFileByDbus();

    void updateBlockList();

private:
    QMap<QString, QStringList> m_blockList;
    QMap<QString, QStringList> m_unblockList;

};

MediaKeyBlockShortcutPrivate::MediaKeyBlockShortcutPrivate(QObject *parent)
  : QObject(parent)
{
    QDBusConnection::systemBus().connect(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                         SETTINGS_DAEMON_SYSTEMDBUS_PATH_SECURITY, \
                                         SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE,"ConfigChanged",
                                         this,
                                         SLOT(onConfigChanged(QString, QStringList)));

    QDBusConnection::systemBus().connect(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                         SETTINGS_DAEMON_SYSTEMDBUS_PATH_SECURITY, \
                                         SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE,"SecurityExit",
                                         this,
                                         SLOT(onSecurityExit()));
}

void MediaKeyBlockShortcutPrivate::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (name.isEmpty()) {
        return;
    }

    if (!name.startsWith(USD_BLOCK_SETTINGS_DAEMON_STARTWITH)) {
        //过滤不符合快捷键场景协议的seivice
        return;
    }
    if (!oldOwner.isEmpty()) {
        //dbus 注销  对应server的场景
        removeBlock(name);
    }
    if (!newOwner.isEmpty()) {
        //dbus 注册  对应server的场景
        addBlock(name);
    }
}

QString MediaKeyBlockShortcutPrivate::getSecurityFileByDbus()
{
    QDBusInterface sercurityManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME,
                                    SETTINGS_DAEMON_SYSTEMDBUS_PATH_SECURITY,
                                    SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE,
                                    QDBusConnection::systemBus());
    if (!sercurityManager.isValid()) {
        USD_LOG(LOG_ERR,"securityManager dbus interface error.");
        return QString();
    }

    QDBusReply<QString> reply = sercurityManager.call("GetConfigPath", QString(qgetenv("USER")), MEDIA_KEY_DEFAULT_NODE);
    if (!reply.isValid()) {
        USD_LOG(LOG_ERR,"securityManager dbus reply error.");
        return QString();
    }

    return reply.value();
}

void MediaKeyBlockShortcutPrivate::updateBlockList()
{
    QString filePath = getSecurityFileByDbus();
    if (filePath.isEmpty()) {
        return;
    }

    if (!QFile::exists(filePath)) {
        USD_LOG(LOG_DEBUG,"delete :%s's block", MEDIA_KEY_DEFAULT_NODE);
        m_blockList.remove(MEDIA_KEY_DEFAULT_NODE);
        Q_EMIT blockChanged(m_blockList);
        return;
    }

    QStringList blockList = MediaKeyBlockShortcutPrivate::getAllBlocklist(filePath);

    m_blockList.insert(MEDIA_KEY_DEFAULT_NODE, blockList);
    Q_EMIT blockChanged(m_blockList);
}

void MediaKeyBlockShortcutPrivate::onConfigChanged(QString userName,  QStringList fileList)
{
    //更新组件列表不包含本组件则不更新
    if (!fileList.contains(MEDIA_KEY_DEFAULT_NODE)) {
        return;
    }
    //用户名不为空且不等于当前用户名则不更新，用户名为空也更新
    if (!userName.isEmpty() && userName != qgetenv("USER")) {
        return;
    }

    updateBlockList();
}

void MediaKeyBlockShortcutPrivate::onSecurityExit()
{
    m_blockList.remove(MEDIA_KEY_DEFAULT_NODE);
    Q_EMIT blockChanged(m_blockList);
}

void MediaKeyBlockShortcutPrivate::connectSignal()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    auto watcher = new QDBusServiceWatcher(QStringLiteral(USD_BLOCK_SETTINGS_DAEMON_SHORTCUT), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &MediaKeyBlockShortcutPrivate::serviceOwnerChanged);
#else
    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &MediaKeyBlockShortcutPrivate::serviceOwnerChanged);
#endif
    //获取列表
    QDBusPendingCall async = QDBusConnection::sessionBus().interface()->asyncCall(QStringLiteral("ListNames"));
    QDBusPendingCallWatcher *asyncWatcher = new QDBusPendingCallWatcher(async, this);
    connect(asyncWatcher, &QDBusPendingCallWatcher::finished, [=](QDBusPendingCallWatcher* watcher){
        QDBusPendingReply<QStringList> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            //error
            USD_LOG(LOG_DEBUG,"Error message:%s-->%s", reply.error().name().toLatin1().data(), reply.error().message().toLatin1().data());
        } else {
            for (const QString& serviceName : reply.value()) {
                if (serviceName.startsWith(QLatin1String(USD_BLOCK_SETTINGS_DAEMON_STARTWITH))) {
                    addBlock(serviceName);
                }
            }
        }
    });

    updateBlockList();
}

QMap<QString, QStringList> MediaKeyBlockShortcutPrivate::getBlockList()
{
    updateBlockList();
    return m_blockList;
}

void MediaKeyBlockShortcutPrivate::addBlock(const QString &service)
{
    QDBusInterface blockIter(service,
                             USD_BLOCK_SETTINGS_DAEMON_PATH,
                             USD_BLOCK_SETTINGS_DAEMON_INTERFEACE,
                            QDBusConnection::sessionBus());
    if (!blockIter.isValid()) {
        USD_LOG(LOG_DEBUG,"create interface error![%s]", service.toLatin1().data());
        return;
    }


    QDBusPendingCall pendingCall = blockIter.asyncCall(USD_BLOCK_SETTINGS_DAEMON_METHOD);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall,this);
    watcher->setProperty("serviceName", service);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this,watcher](){
        QDBusMessage msg  = watcher->reply();
        QString service = watcher->property("serviceName").toString();
        if (msg.type() == QDBusMessage::ReplyMessage) {
            if (msg.arguments().isEmpty() == false) {
                QStringList value = msg.arguments().takeFirst().toStringList();
                m_blockList.insert(service, value);
            }
        }
        Q_EMIT blockChanged(m_blockList);
        watcher->deleteLater();
    });

    QDBusPendingCall pendingUnblockCall = blockIter.asyncCall(USD_UNBLOCK_SETTINGS_DAEMON_METHOD);
    QDBusPendingCallWatcher *UnBlockWatcher = new QDBusPendingCallWatcher(pendingUnblockCall,this);
    UnBlockWatcher->setProperty("serviceName", service);

    connect(UnBlockWatcher, &QDBusPendingCallWatcher::finished, this, [this,UnBlockWatcher](){
        USD_LOG(LOG_DEBUG,".");
        QDBusMessage msg  = UnBlockWatcher->reply();
        QString service = UnBlockWatcher->property("serviceName").toString();
        qDebug()<<msg;
        if (msg.type() == QDBusMessage::ReplyMessage) {
            if (msg.arguments().isEmpty() == false) {
                QStringList value = msg.arguments().takeFirst().toStringList();
                m_unblockList.insert(service, value);
            }
            Q_EMIT unblockChanged(m_unblockList);
        }
        UnBlockWatcher->deleteLater();
    });
}

void MediaKeyBlockShortcutPrivate::removeBlock(const QString &service)
{
    m_blockList.remove(service);
    m_unblockList.remove(service);
    Q_EMIT blockChanged(m_blockList);
    Q_EMIT unblockChanged(m_unblockList);
}

QStringList MediaKeyBlockShortcutPrivate::getAllBlocklist(const QString &fileName)
{
    QStringList blockList;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        USD_LOG(LOG_WARNING,"Cannot open file for reading: %s cuz:[%s]" ,
                fileName.toLatin1().data(), file.errorString().toLatin1().data());
        return QStringList();
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());

    if (!jsonDoc.isNull() && jsonDoc.isObject()) {

        QJsonObject jsonObj = jsonDoc.object();
        // 访问"org.ukui.SettingsDaemon.plugins.customized"对象
        QJsonObject customizedObj = jsonObj.value("org.ukui.SettingsDaemon.plugins.customized").toObject();
        // 访问"shortcutkey-black-list"的值
        QJsonValue blacklistValue = customizedObj.value("shortcutkey-black-list");

        // 输出值
        if (blacklistValue.isString()) {
            blockList = blacklistValue.toString().split(",");
            qDebug() << "Shortcut key black list:" << blockList;
        } else {
            USD_LOG(LOG_ERR,"The value of 'shortcutkey-black-list' is not a string.");
        }
    } else {
        USD_LOG(LOG_ERR,"Failed to parse JSON or JSON is not an object.");
    }

    return blockList;
}

MediaKeyBlockShutcut::MediaKeyBlockShutcut(QObject *parent)
    : QObject(parent)
    , d(new MediaKeyBlockShortcutPrivate(this))
{
    QTimer::singleShot(1*1000, [=](){
        d->connectSignal();
    });

    connect(d, &MediaKeyBlockShortcutPrivate::blockChanged, this, &MediaKeyBlockShutcut::blockChanged);
    connect(d, &MediaKeyBlockShortcutPrivate::unblockChanged, this, &MediaKeyBlockShutcut::unblockChanged);
}

MediaKeyBlockShutcut::~MediaKeyBlockShutcut()
{
    USD_LOG(LOG_DEBUG,".");
}

QMap<QString, QStringList> MediaKeyBlockShutcut::getBlockList()
{
    return d->getBlockList();
}

#include "media-key-block-shutcut.moc"
