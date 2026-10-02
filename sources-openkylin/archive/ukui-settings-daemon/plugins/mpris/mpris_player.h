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

#ifndef MPRISPLAYER_H
#define MPRISPLAYER_H

#include <QObject>
#include <QDBusObjectPath>
#include <memory>
#include "dbus_properties.h"
#include "org_mpris_MediaPlayer2.h"
#include "org_mpris_MediaPlayer2_Player.h"

class MprisPlayerPrivate;

namespace Mpris {
class MprisPlayer;
typedef std::shared_ptr<MprisPlayer> MprisPlayerPtr;
typedef std::weak_ptr<MprisPlayer> MprisPlayerWeakPtr;

class MprisPlayer : public QObject, std::enable_shared_from_this<MprisPlayer>
{
    Q_OBJECT
public:
    explicit MprisPlayer(const QString &busAddress, QObject *parent = nullptr);
    ~MprisPlayer();
    //异步获取对象智能指针时，应使用此方法，不可获取裸指针构造shared_ptr,如：share_ptr<MprisPlayer>(this)；
    MprisPlayerPtr getSelf();
    /**
     * @brief 播放器能力
     */
    enum MprisCap {
        CanQuit = 1 << 0,
        CanRaise = 1 << 1,
        CanSetFullscreen = 1 << 2,
        CanControl = 1 << 3,
        CanPlay = 1 << 4,
        CanPause = 1 << 5,
        CanSeek = 1 << 6,
        CanGoNext = 1 << 7,
        CanGoPrevious = 1 << 8,
    };
    Q_ENUM(MprisCap)
    Q_DECLARE_FLAGS(MprisCaps, MprisCap)

    QString dbusAddress() const;
    OrgFreedesktopDBusPropertiesInterface *propertiesInterface() const;
    OrgMprisMediaPlayer2Interface *rootInterface() const;
    OrgMprisMediaPlayer2PlayerInterface *playerInterface() const;
    MprisCaps capabilities() const;
    uint pid() const;
    QString trackId() const;

    /**
     * player 控制函数
     * 例如： canQuit() == true , 调用quit函数有效
     *
     * canControl() == true ,调用 stop(), setLoopStatus(), setShuffle(), setVolume()应有效,
     */
    Q_INVOKABLE void raise();
    Q_INVOKABLE void quit();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void play();
    Q_INVOKABLE void seek(const qlonglong &offset);
    Q_INVOKABLE void setPosition(const qlonglong &offset);
    Q_INVOKABLE void openUri(const QString &uri);
    Q_INVOKABLE void setLoopStatus(const QString& status);
    Q_INVOKABLE void setShuffle(bool value);
    Q_INVOKABLE void setRate(double value);
    Q_INVOKABLE void setVolume(double value);

    /**
     * 获取player能力
     *
     */
    bool canQuit();
    bool canRaise();
    bool canSetFullscreen();
    bool canControl();
    bool canPlay();
    bool canPause();
    bool canSeek();
    bool canGoNext();
    bool canGoPrevious();
    const QVariantMap data();

Q_SIGNALS:
    void initialFetchFinished();
    void dataUpdate(const QString&, const QVariant&);
private:
    MprisPlayerPrivate *d = nullptr;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Mpris::MprisPlayer::MprisCaps)

#endif // MPRISPLAYER_H
