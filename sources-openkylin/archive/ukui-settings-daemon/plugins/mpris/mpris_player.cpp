/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include "mpris_player.h"
#include <QTimer>
#include "clib-syslog.h"


static const QString MPRIS2_PATH = QStringLiteral("/org/mpris/MediaPlayer2");
using namespace Mpris;

class MprisPlayerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MprisPlayerPrivate(const QString &busAddress, QObject *parent = nullptr);
    ~MprisPlayerPrivate();
private:
    void initPlayer();
    bool initInterfaces();
    void refresh();
    void updatePropsFromMap(const QVariantMap &map);
    void setData(const QString &key, const QVariant &value);
private Q_SLOTS:
    /**
     * @brief 获取相应interface属性
     * @param watcher
     */
    void getPropsFinished(QDBusPendingCallWatcher *watcher);
    /**
     * @brief 属性改变
     * @param interface
     * @param changedProperties 属性值
     * @param invalidatedProperties 无效属性
     */
    void propertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

    void onSeeked(qlonglong offset);
public:
    OrgFreedesktopDBusPropertiesInterface *m_propsIface = nullptr;
    OrgMprisMediaPlayer2Interface *m_rootIface = nullptr;
    OrgMprisMediaPlayer2PlayerInterface *m_playerIface = nullptr;
    QVariantMap m_data;
    QString m_dbusAddress;
    MprisPlayer* q = nullptr;
    int m_fetchesPending = 0;
    MprisPlayer::MprisCaps m_caps;
    QTimer *m_propTimer = nullptr;
    uint m_pid = 0;
    QString m_trackId;

    int m_errorTimes = 0;
};

MprisPlayer::MprisPlayer(const QString &busAddress, QObject *parent)
    : QObject(parent)
    , d(new MprisPlayerPrivate(busAddress, this))
{
    setObjectName(busAddress);
}

MprisPlayer::~MprisPlayer()
{
    qDebug() << dbusAddress() << "析构";
}

MprisPlayerPtr MprisPlayer::getSelf()
{
    return shared_from_this();
}

QString MprisPlayer::dbusAddress() const
{
    return d->m_dbusAddress;
}

OrgFreedesktopDBusPropertiesInterface *MprisPlayer::propertiesInterface() const
{
    return d->m_propsIface;
}

OrgMprisMediaPlayer2Interface *MprisPlayer::rootInterface() const
{
    return d->m_rootIface;
}

OrgMprisMediaPlayer2PlayerInterface *MprisPlayer::playerInterface() const
{
    return d->m_playerIface;
}

MprisPlayer::MprisCaps MprisPlayer::capabilities() const
{
    return d->m_caps;
}

uint MprisPlayer::pid() const
{
    return d->m_pid;
}

QString MprisPlayer::trackId() const
{
    return d->m_trackId;
}

void MprisPlayer::raise()
{
    rootInterface()->Raise();
}

void MprisPlayer::quit()
{
    rootInterface()->Quit();
}

void MprisPlayer::next()
{
    playerInterface()->Next();
}

void MprisPlayer::previous()
{
    playerInterface()->Previous();
}

void MprisPlayer::pause()
{
    playerInterface()->Pause();
}

void MprisPlayer::playPause()
{
    playerInterface()->PlayPause();
}

void MprisPlayer::stop()
{
    playerInterface()->Stop();
}

void MprisPlayer::play()
{
    playerInterface()->Play();
}

void MprisPlayer::seek(const qlonglong &offset)
{
    playerInterface()->Seek(offset);
}

void MprisPlayer::setPosition(const qlonglong &offset)
{
    playerInterface()->SetPosition(QDBusObjectPath(trackId()), offset);
}

void MprisPlayer::openUri(const QString &uri)
{
    playerInterface()->OpenUri(uri);
}

void MprisPlayer::setLoopStatus(const QString &status)
{
    playerInterface()->setLoopStatus(status);
}

void MprisPlayer::setShuffle(bool value)
{
    playerInterface()->setShuffle(value);
}

void MprisPlayer::setRate(double value)
{
    playerInterface()->setRate(value);
}

void MprisPlayer::setVolume(double value)
{
    playerInterface()->setVolume(value);
}

bool MprisPlayer::canQuit()
{
    return capabilities() & CanQuit;
}

bool MprisPlayer::canRaise()
{
    return capabilities() & CanRaise;
}

bool MprisPlayer::canSetFullscreen()
{
    return capabilities() & CanSetFullscreen;
}

bool MprisPlayer::canControl()
{
    return capabilities() & CanControl;
}

bool MprisPlayer::canPlay()
{
    return capabilities() & CanPlay;
}

bool MprisPlayer::canPause()
{
    return capabilities() & CanPause;
}

bool MprisPlayer::canSeek()
{
    return capabilities() & CanSeek;
}

bool MprisPlayer::canGoNext()
{
    return capabilities() & CanGoNext;
}

bool MprisPlayer::canGoPrevious()
{
    return capabilities() & CanGoPrevious;
}

const QVariantMap MprisPlayer::data()
{
    return d->m_data;
}

MprisPlayerPrivate::MprisPlayerPrivate(const QString &busAddress, QObject *parent)
    : QObject(parent)
    , m_dbusAddress(busAddress)
    , q(qobject_cast<MprisPlayer *>(parent))
{
    Q_ASSERT(!busAddress.isEmpty());
    Q_ASSERT(busAddress.startsWith(QLatin1String("org.mpris.MediaPlayer2.")));
    initPlayer();
}

MprisPlayerPrivate::~MprisPlayerPrivate()
{
    if (m_propTimer->isActive()) {
        m_propTimer->stop();
    }
    qDebug() << "MprisPlayerPrivate" << "析构";
}

void MprisPlayerPrivate::initPlayer()
{
    QDBusReply<uint> pidReply = QDBusConnection::sessionBus().interface()->servicePid(m_dbusAddress);
    if (pidReply.isValid()) {
        //通过dbus service 获取pid
        m_pid = pidReply.value();
        setData(QLatin1String("pid"), m_pid);
    }
    m_propTimer = new QTimer(this);
    connect(m_propTimer, &QTimer::timeout, this, &MprisPlayerPrivate::refresh);

    if (initInterfaces()) {
        refresh();
    } else {
        qWarning() << "init mpris interfaces error.";
    }
}

bool MprisPlayerPrivate::initInterfaces()
{
    if (!m_propsIface || (m_propsIface && !m_propsIface->isValid())) {
        m_propsIface = new OrgFreedesktopDBusPropertiesInterface(m_dbusAddress, MPRIS2_PATH, QDBusConnection::sessionBus(), this);
    }
    if (!m_rootIface || (m_rootIface && !m_playerIface->isValid())) {
        m_rootIface = new OrgMprisMediaPlayer2Interface(m_dbusAddress, MPRIS2_PATH, QDBusConnection::sessionBus(), this);
    }
    if (!m_playerIface || (m_playerIface && !m_rootIface->isValid())) {
        m_playerIface = new OrgMprisMediaPlayer2PlayerInterface(m_dbusAddress, MPRIS2_PATH, QDBusConnection::sessionBus(), this);
    }
    //接口是否有效
    if (!m_propsIface->isValid() || !m_playerIface->isValid() || !m_rootIface->isValid()) {
        return false;
    }

    {//org.mpris.MediaPlayer2.TrackList || org.mpris.MediaPlayer2.Playlist

    }
    //properties changed
    connect(m_propsIface, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &MprisPlayerPrivate::propertiesChanged);
    connect(m_playerIface, &OrgMprisMediaPlayer2PlayerInterface::Seeked, this, &MprisPlayerPrivate::onSeeked);
    return true;
}

void MprisPlayerPrivate::refresh()
{
    if (m_propTimer->isActive()) {
        m_propTimer->stop();
    }

    QDBusPendingCall async = m_propsIface->GetAll(OrgMprisMediaPlayer2Interface::staticInterfaceName());
    if (async.isError()) {
        qWarning() << "get props error from " << OrgMprisMediaPlayer2Interface::staticInterfaceName();
    } else {
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &MprisPlayerPrivate::getPropsFinished);
        ++m_fetchesPending;
    }

    async = m_propsIface->GetAll(OrgMprisMediaPlayer2PlayerInterface::staticInterfaceName());
    if (async.isError()) {
        qWarning() << "get props error from " << OrgMprisMediaPlayer2PlayerInterface::staticInterfaceName();

    } else {
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &MprisPlayerPrivate::getPropsFinished);
        ++m_fetchesPending;
    }
    //org.mpris.MediaPlayer2.TrackList || org.mpris.MediaPlayer2.Playlist 属性获取
    {

    }
}

void MprisPlayerPrivate::updatePropsFromMap(const QVariantMap &propsMap)
{
    //属性更新
    QVariantMap::const_iterator it = propsMap.constBegin();
    while (it != propsMap.constEnd()) {
        QVariant value = it.value();
        if (it.value().userType() == qMetaTypeId<QDBusArgument>()) {
            if (it.key() == QLatin1String("Metadata")) {
                const QDBusArgument& arg = propsMap.value(QStringLiteral("Metadata")).value<QDBusArgument>();
                QVariantMap map;
                arg >> map;
                value = QVariant(map);

                const QDBusObjectPath& path = value.toMap().value(QStringLiteral("mpris:trackid")).value<QDBusObjectPath>();
                m_trackId = path.path();
                setData(QStringLiteral("trackid"), m_trackId);
            }
        }
        //播放器能力
        bool ok;
        QMetaEnum metaEnum = QMetaEnum::fromType<MprisPlayer::MprisCap>();
        MprisPlayer::MprisCap cap = MprisPlayer::MprisCap(metaEnum.keyToValue(it.key().toLatin1().data(), &ok));
        if (ok) {
            if (it.value().toBool()) {
                m_caps |= cap;
            } else {
                m_caps &= ~cap;
            }
        }
        {//Position属性有时不会更新，seekd没有信号
        }
        setData(it.key(), value);
        ++it;
    }
}

void MprisPlayerPrivate::setData(const QString &key, const QVariant &value)
{
    if (value.isValid()) {
        m_data.insert(key, value);
//        qDebug() << key << value;
        //数据更新信号
        Q_EMIT q->dataUpdate(key, value);
    } else {
        qWarning() << m_dbusAddress << key << "data is not valid:"  << value;
    }
}

void MprisPlayerPrivate::getPropsFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QVariantMap> propsReply = *watcher;
    watcher->deleteLater();
    if (m_fetchesPending < 1) {
        return;
    }

    if (propsReply.isError()) {
        qWarning() << m_dbusAddress << "does not implement" << OrgFreedesktopDBusPropertiesInterface::staticInterfaceName() << "correctly"
                          << "Error message was" << propsReply.error().name() << propsReply.error().message();
        m_fetchesPending = 0;
        //属性获取失败  celluloid 播放器初始化时错误
        m_errorTimes ++;
        if (m_errorTimes > 5) {
            m_errorTimes = 0;
            SYS_LOG(LOG_WARNING, "Get props error!");
            return;
        }
        SYS_LOG(LOG_NOTICE, "Get props fail! Restart timer %dms", m_errorTimes * 1000);
        m_propTimer->start(m_errorTimes * 1000);
        return;
    }

    updatePropsFromMap(propsReply.value());

    --m_fetchesPending;
    if (m_fetchesPending == 0) {
        //fetch finished
        Q_EMIT q->initialFetchFinished();
    }
}

void MprisPlayerPrivate::propertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(interface)
    updatePropsFromMap(changedProperties);
    if ((interface == OrgMprisMediaPlayer2Interface::staticInterfaceName() ||
         interface == OrgMprisMediaPlayer2PlayerInterface::staticInterfaceName()) &&
         !invalidatedProperties.isEmpty()) {
        m_propTimer->start(10);
        qWarning() << interface << "invalidatedProperties is :" << invalidatedProperties;
    }
}

void MprisPlayerPrivate::onSeeked(qlonglong offset)
{
    setData(QStringLiteral("Position"), offset);
}

#include "mpris_player.moc"
