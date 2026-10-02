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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DBUSADAPTER_H
#define DBUSADAPTER_H

#include <QObject>
#include <QDBusConnection>
#include <QtScript/QScriptContext>
#include <QVariantMap>
#include "globalsignal.h"
//#include "UIControl/player/player.h"
#include "mainwidget.h"

#define UKUI_PANEL_CAN_CONTROL 1

class DbusAdapter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

#if UKUI_PANEL_CAN_CONTROL
    Q_PROPERTY(bool CanControl READ getCanControl)
    Q_PROPERTY(QString PlaybackStatus READ getState)
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
    Q_PROPERTY(long long Position READ getPosition)
#endif

public:
    explicit DbusAdapter(QObject *parent = nullptr);
    void emitSeeked();

public:
    QString getLoopStatus();
    void setLoopStatus(QString s);

    double m_rate;
    double getRate();
    void setRate(double d);

    bool getShuffle();
    void setShuffle(bool b);

    QVariantMap m_metadata;
    QVariantMap getMetadata();
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

#if UKUI_PANEL_CAN_CONTROL
Q_SIGNALS:
    void Seeked(const qint64 url);
    void PropertiesChanged(const QString &interface_name, const QVariantMap &change_properties, const QStringList &invakudate_properties);
#endif

#if UKUI_PANEL_CAN_CONTROL
private:
    bool CanControl = true; // 任务栏预览图添加属性
    QString m_playbackStatus = "Stopped"; // 任务栏预览图播放/暂停属性
#endif

public Q_SLOTS:
    // mpris 使用
    void Stop() const;
    void Next() const;
    void Play() const;
    void Pause() const;
    void PlayPause();
    void Previous() const;
    void Seek(const qint64 Offset);
    void SetPosition(const QString& TrackId, const qint64 Position);
    void OpenUri(const QString& Uri);

    void VolumeUp() const;
    void VolumeDown() const;
    void FullScreen() const;
    void Exit() const;

#if UKUI_PANEL_CAN_CONTROL
    QString getState();
    void notify(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

#endif
};

#endif // DBUSADAPTER_H
