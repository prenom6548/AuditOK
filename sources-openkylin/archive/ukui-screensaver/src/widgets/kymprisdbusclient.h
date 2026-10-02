/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef MPRISDBUSCLIENT_H
#define MPRISDBUSCLIENT_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QDebug>

/*
 * Common mpris interface，reference：https://specifications.freedesktop.org/mpris-spec/latest/Player_Interface.html
 */
class KyMPRISDBusClient : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    KyMPRISDBusClient(
        const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~KyMPRISDBusClient();

    Q_PROPERTY(QString PlaybackStatus READ getPlaybackStatus NOTIFY signalPlaybackStatusChanged)
    QString getPlaybackStatus() const;

    Q_PROPERTY(QString LoopStatus READ getLoopStatus WRITE setLoopStatus NOTIFY signalLoopStatusChanged)
    QString getLoopStatus() const;
    void setLoopStatus(const QString &value);

    Q_PROPERTY(double Rate READ getRate WRITE setRate NOTIFY signalRateChanged)
    double getRate() const;
    void setRate(double value);

    Q_PROPERTY(bool Shuffle READ getShuffle WRITE setShuffle NOTIFY signalShuffleChanged)
    bool getShuffle() const;
    void setShuffle(bool value);

    Q_PROPERTY(QVariantMap Metadata READ getMetadata NOTIFY signalMetadataChanged)
    QVariantMap getMetadata() const;

    Q_PROPERTY(double Volume READ getVolume WRITE setVolume NOTIFY signalVolumeChanged)
    double getVolume() const;
    void setVolume(double value);

    Q_PROPERTY(qlonglong Position READ getPosition NOTIFY signalPositionChanged)
    qlonglong getPosition() const;

    Q_PROPERTY(double MinimumRate READ getMinimumRate NOTIFY signalMinimumRateChanged)
    double getMinimumRate() const;

    Q_PROPERTY(double MaximumRate READ getMaximumRate NOTIFY signalMaximumRateChanged)
    double getMaximumRate() const;

    Q_PROPERTY(bool CanGoNext READ getCanGoNext NOTIFY signalCanGoNextChanged)
    bool getCanGoNext() const;

    Q_PROPERTY(bool CanGoPrevious READ getCanGoPrevious NOTIFY signalCanGoPreviousChanged)
    bool getCanGoPrevious() const;

    Q_PROPERTY(bool CanPlay READ getCanPlay NOTIFY signalCanPlayChanged)
    bool getCanPlay() const;

    Q_PROPERTY(bool CanPause READ getCanPause NOTIFY signalCanPauseChanged)
    bool getCanPause() const;

    Q_PROPERTY(bool CanSeek READ getCanSeek NOTIFY signalCanSeekChanged)
    bool getCanSeek() const;

    Q_PROPERTY(bool CanControl READ getCanControl NOTIFY signalCanControlChanged)
    bool getCanControl() const;

public Q_SLOTS: // METHODS
    void onPropertiesChanged(const QDBusMessage &msg);

    QDBusPendingReply<> doNext();

    QDBusPendingReply<> doPrevious();

    QDBusPendingReply<> doPause();

    QDBusPendingReply<> doPlayPause();

    QDBusPendingReply<> doStop();

    QDBusPendingReply<> doPlay();

    QDBusPendingReply<> doSeek(qlonglong Offset);

    QDBusPendingReply<> doSetPosition(const QDBusObjectPath &TrackId, qlonglong Position);

    QDBusPendingReply<> doOpenUri(const QString &Uri);

Q_SIGNALS: // SIGNALS
    void Seeked(qlonglong Position);
    // begin property changed signals
    void signalRateChanged(double value);
    void signalShuffleChanged(bool value);
    void signalVolumeChanged(double value);
    void signalCanControlChanged(bool value);
    void signalCanGoNextChanged(bool value);
    void signalCanGoPreviousChanged(bool value);
    void signalCanPauseChanged(bool value);
    void signalCanPlayChanged(bool value);
    void signalCanSeekChanged(bool value);
    void signalLoopStatusChanged(const QString &value);
    void signalMaximumRateChanged(double value);
    void signalMetadataChanged(const QVariantMap &value);
    void signalMinimumRateChanged(double value);
    void signalPlaybackStatusChanged(const QString &value);
    void signalPositionChanged(qlonglong value);
};

#endif // MPRISDBUSCLIENT_H
