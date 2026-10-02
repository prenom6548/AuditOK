/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DBUSADAPTER_H
#define DBUSADAPTER_H

#include <QObject>
#include <QVariantMap>
#include "core/mpvtypes.h"
#include "global/globalsignal.h"
#define PLAY_STATUS "Playing"
#define PAUSED_STATUS "Paused"
#define STOP_STATUS "Stopped"

class DbusAdapter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QString PlaybackStatus READ getPlaybackState)
    Q_PROPERTY(QString LoopStatus READ getLoopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ getRate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ getShuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ getMetadata)
    Q_PROPERTY(double Volume READ getVolume WRITE setVolume)
    Q_PROPERTY(double MinimumRate READ getMinimumRate)
    Q_PROPERTY(double MaximumRate READ getMaximumRate)
    Q_PROPERTY(bool CanGoNext READ getCanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ getCanGoPrevious)
    Q_PROPERTY(bool CanPlay READ getCanPlay)
    Q_PROPERTY(bool CanPause READ getCanPause)
    Q_PROPERTY(bool CanSeek READ getCanSeek)
    Q_PROPERTY(bool CanControl READ getCanControl)
    Q_PROPERTY(long long Position READ getPosition)
//    Q_PROPERTY(QString PlaybackStatus READ getState)
public:
    explicit DbusAdapter(QObject *parent = nullptr);
    void emitSeeked();

public:
    void setLoopStatus(QString s);
    QString getLoopStatus();

    double m_rate;
    double getRate();
    void setRate(double d);


    bool getShuffle();
    void setShuffle(bool b);

    QVariantMap m_metadata;
    QVariantMap getMetadata() { return m_metadata; }
    void updateMetadata(const QVariantMap input);

    double getVolume();
    void setVolume(double b);

    long long getPosition();

    double m_minimumRate { 0.5 };
    double getMinimumRate() { return m_minimumRate; }

    double m_maximumRate { 2.0 };
    double getMaximumRate() { return m_maximumRate; }

    bool m_canGoNext { true };
    bool getCanGoNext() { return m_canGoNext; }

    bool m_canGoPrevious { true };
    bool getCanGoPrevious() { return m_canGoPrevious; }

    bool m_canPlay { true };
    bool getCanPlay() { return m_canPlay; }

    bool m_canPause { true };
    bool getCanPause() { return m_canPause; }

    bool m_canSeek { true };
    bool getCanSeek() { return m_canSeek; }

    bool m_canControl { true };
    bool getCanControl() { return m_canControl; }
    void setCanControl(const bool b) { m_canControl = b; }

    void initMetadata();
//Q_SIGNALS:
//    void PropertiesChanged(const QString &interface_name, const QVariantMap &change_properties, const QStringList &invakudate_properties);

public slots:
    // mpris 使用
    QString getPlaybackState();
    void SetPosition(const QString& TrackId, const qint64 Position);
    void OpenUri(const QString Uri);
    void Stop();
    void Next() const;
    // 这个改个名字，不能用 PlayPause，不然会和线控冲突
    void KvPlayPause() const;

    void Play();
    void Pause();
    void PlayPause();

    void Previous() const;
    void AddFile(QString file) const;
    void Seek(const qint64 Offset);
    void VolumeUp() const;
    void VolumeDown() const;
    void FullScreen() const;
    void LoopMode() const;
    void Exit() const;

    int PlayState();
//    QString getState();
    QString CurrentFile();
    void notify(const QString &interfaceName, const QString &key, const QString &value);
    void notify(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
private:
    bool isStopped = true;
    bool CanControl = true;
    QString PlaybackStatus; // 任务栏预览图播放/暂停属性
    QString m_currentFile;
    QString m_playbackStatus = "Stopped";
    double m_position = 0; //播放进度
    PlayOrder  m_playOrder;            // 播放顺序

signals:
};

#endif // DBUSADAPTER_H
