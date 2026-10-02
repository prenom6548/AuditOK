/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QCoreApplication>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusObjectPath>
#include <QMetaEnum>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest>
#include <utility>

#include "../mpris/player-items-model.h"

namespace {

constexpr auto MprisPath = "/org/mpris/MediaPlayer2";

bool g_sessionBusReady = false;
QString g_sessionBusError;

class PrivateSessionBus
{
public:
    PrivateSessionBus()
        : m_previousAddress(qgetenv("DBUS_SESSION_BUS_ADDRESS"))
    {
        const QString dbusDaemon = QStandardPaths::findExecutable(QStringLiteral("dbus-daemon"));
        if (dbusDaemon.isEmpty()) {
            m_error = QStringLiteral("dbus-daemon is not available");
            return;
        }

        m_daemon.start(dbusDaemon, {QStringLiteral("--session"),
                                    QStringLiteral("--print-address=1"),
                                    QStringLiteral("--nofork")});
        if (!m_daemon.waitForStarted(3000)) {
            m_error = QStringLiteral("failed to start dbus-daemon");
            return;
        }
        if (!m_daemon.waitForReadyRead(3000)) {
            m_error = QStringLiteral("dbus-daemon did not print an address");
            return;
        }

        const QByteArray address = m_daemon.readLine().trimmed();
        if (address.isEmpty()) {
            m_error = QStringLiteral("dbus-daemon printed an empty address");
            return;
        }

        qputenv("DBUS_SESSION_BUS_ADDRESS", address);
        m_ready = true;
    }

    ~PrivateSessionBus()
    {
        if (m_ready) {
            const QString connectionName = QDBusConnection::sessionBus().name();
            QDBusConnection::disconnectFromBus(connectionName);
        }

        if (m_previousAddress.isEmpty()) {
            qunsetenv("DBUS_SESSION_BUS_ADDRESS");
        } else {
            qputenv("DBUS_SESSION_BUS_ADDRESS", m_previousAddress);
        }

        if (m_daemon.state() != QProcess::NotRunning) {
            m_daemon.terminate();
            if (!m_daemon.waitForFinished(3000)) {
                m_daemon.kill();
                m_daemon.waitForFinished(3000);
            }
        }
    }

    bool isReady() const
    {
        return m_ready;
    }

    QString errorString() const
    {
        return m_error;
    }

private:
    bool m_ready = false;
    QByteArray m_previousAddress;
    QString m_error;
    QProcess m_daemon;
};

class RegisteredServiceGuard
{
public:
    RegisteredServiceGuard(QDBusConnection &bus, QString service, QString path)
        : m_bus(bus)
        , m_service(std::move(service))
        , m_path(std::move(path))
    {
    }

    ~RegisteredServiceGuard()
    {
        m_bus.unregisterObject(m_path);
        m_bus.unregisterService(m_service);
    }

private:
    QDBusConnection &m_bus;
    QString m_service;
    QString m_path;
};

class FakeMprisRootAdaptor;
class FakeMprisPlayerAdaptor;

class FakeMprisService : public QObject
{
    Q_OBJECT

public:
    explicit FakeMprisService(const QString &mediaUrl, QObject *parent = nullptr);

    bool canQuit = true;
    bool fullScreen = true;
    bool canSetFullScreen = true;
    bool canRaise = true;
    bool hasTrackList = true;
    QString identity = QStringLiteral("Codex Player");
    QString desktopEntry = QStringLiteral("codex-player");
    QStringList supportedUriSchemes { QStringLiteral("file"), QStringLiteral("https") };
    QStringList supportedMimeTypes { QStringLiteral("audio/mpeg"), QStringLiteral("video/mp4") };

    QString playbackStatus = QStringLiteral("Playing");
    QString loopStatus = QStringLiteral("Playlist");
    double rate = 1.5;
    bool shuffle = true;
    QVariantMap metaData;
    double volume = 0.6;
    qlonglong position = 13579;
    double minimumRate = 0.5;
    double maximumRate = 2.0;
    bool canGoNext = true;
    bool canGoPrevious = false;
    bool canPlay = true;
    bool canPause = true;
    bool canSeek = true;
    bool canControl = true;

    int raiseCount = 0;
    int quitCount = 0;
    int nextCount = 0;
    int previousCount = 0;
    int pauseCount = 0;
    int playPauseCount = 0;
    int stopCount = 0;
    int playCount = 0;
    qlonglong lastSeekOffset = -1;
    QString lastSetPositionTrackId;
    qlonglong lastSetPosition = -1;
    QString lastOpenedUri;
};

class FakeMprisRootAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_PROPERTY(bool CanQuit READ canQuit)
    Q_PROPERTY(bool Fullscreen READ fullScreen WRITE setFullScreen)
    Q_PROPERTY(bool CanSetFullscreen READ canSetFullScreen)
    Q_PROPERTY(bool CanRaise READ canRaise)
    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    Q_PROPERTY(QString Identity READ identity)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)

public:
    explicit FakeMprisRootAdaptor(FakeMprisService *service)
        : QDBusAbstractAdaptor(service)
        , m_service(service)
    {
    }

    bool canQuit() const { return m_service->canQuit; }
    bool fullScreen() const { return m_service->fullScreen; }
    void setFullScreen(bool value) { m_service->fullScreen = value; }
    bool canSetFullScreen() const { return m_service->canSetFullScreen; }
    bool canRaise() const { return m_service->canRaise; }
    bool hasTrackList() const { return m_service->hasTrackList; }
    QString identity() const { return m_service->identity; }
    QString desktopEntry() const { return m_service->desktopEntry; }
    QStringList supportedUriSchemes() const { return m_service->supportedUriSchemes; }
    QStringList supportedMimeTypes() const { return m_service->supportedMimeTypes; }

public slots:
    void Raise() { ++m_service->raiseCount; }
    void Quit() { ++m_service->quitCount; }

private:
    FakeMprisService *m_service;
};

class FakeMprisPlayerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ metaData)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ minimumRate)
    Q_PROPERTY(double MaximumRate READ maximumRate)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)

public:
    explicit FakeMprisPlayerAdaptor(FakeMprisService *service)
        : QDBusAbstractAdaptor(service)
        , m_service(service)
    {
    }

    QString playbackStatus() const { return m_service->playbackStatus; }
    QString loopStatus() const { return m_service->loopStatus; }
    void setLoopStatus(const QString &value) { m_service->loopStatus = value; }
    double rate() const { return m_service->rate; }
    void setRate(double value) { m_service->rate = value; }
    bool shuffle() const { return m_service->shuffle; }
    void setShuffle(bool value) { m_service->shuffle = value; }
    QVariantMap metaData() const { return m_service->metaData; }
    double volume() const { return m_service->volume; }
    void setVolume(double value) { m_service->volume = value; }
    qlonglong position() const { return m_service->position; }
    double minimumRate() const { return m_service->minimumRate; }
    double maximumRate() const { return m_service->maximumRate; }
    bool canGoNext() const { return m_service->canGoNext; }
    bool canGoPrevious() const { return m_service->canGoPrevious; }
    bool canPlay() const { return m_service->canPlay; }
    bool canPause() const { return m_service->canPause; }
    bool canSeek() const { return m_service->canSeek; }
    bool canControl() const { return m_service->canControl; }

public slots:
    void Next() { ++m_service->nextCount; }
    void Previous() { ++m_service->previousCount; }
    void Pause() { ++m_service->pauseCount; }
    void PlayPause() { ++m_service->playPauseCount; }
    void Stop() { ++m_service->stopCount; }
    void Play() { ++m_service->playCount; }
    void Seek(qlonglong offset) { m_service->lastSeekOffset = offset; }
    void SetPosition(const QDBusObjectPath &trackId, qlonglong position)
    {
        m_service->lastSetPositionTrackId = trackId.path();
        m_service->lastSetPosition = position;
    }
    void OpenUri(const QString &uri) { m_service->lastOpenedUri = uri; }

private:
    FakeMprisService *m_service;
};

FakeMprisService::FakeMprisService(const QString &mediaUrl, QObject *parent)
    : QObject(parent)
{
    metaData.insert(QStringLiteral("xesam:url"), mediaUrl);
    new FakeMprisRootAdaptor(this);
    new FakeMprisPlayerAdaptor(this);
}

} // namespace

class player_items_model_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void singletonAndMetadata();
    void serviceLifecycleAndOperations();
};

void player_items_model_test::initTestCase()
{
    qRegisterMetaType<QVector<int>>("QVector<int>");
}

void player_items_model_test::singletonAndMetadata()
{
    auto *first = PlayerItemsModel::self();
    auto *second = PlayerItemsModel::self();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);

    const auto roles = first->roleNames();
    const auto properties = QMetaEnum::fromType<MprisProperties::Properties>();

    QCOMPARE(roles.size(), properties.keyCount());
    QCOMPARE(roles.value(MprisProperties::Pid), QByteArrayLiteral("Pid"));
    QCOMPARE(roles.value(MprisProperties::Valid), QByteArrayLiteral("Valid"));
    QCOMPARE(roles.value(MprisProperties::Identity), QByteArrayLiteral("Identity"));
    QCOMPARE(roles.value(MprisProperties::CanControl), QByteArrayLiteral("CanControl"));

    QVERIFY(!first->index(-1, 0, {}).isValid());
    QVERIFY(!first->index(0, 1, {}).isValid());
    QCOMPARE(first->rowCount(first->index(0, 0, {})), 0);

    QVERIFY(!first->data({}, MprisProperties::Pid).isValid());

    const int outOfRangeRow = first->rowCount({});
    QVERIFY(!first->data(first->index(outOfRangeRow, 0, {}), MprisProperties::Valid).isValid());
}

void player_items_model_test::serviceLifecycleAndOperations()
{
    if (!g_sessionBusReady) {
        const QByteArray reason = g_sessionBusError.isEmpty()
                                  ? QByteArrayLiteral("session bus is not connected")
                                  : g_sessionBusError.toUtf8();
        QSKIP(reason.constData());
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        QSKIP("session bus is not connected");
    }

    auto *model = PlayerItemsModel::self();
    const int baselineCount = model->rowCount({});
    const QString service = QStringLiteral("org.mpris.MediaPlayer2.ukuiQuickAutotest%1")
                                .arg(QCoreApplication::applicationPid());
    const QString trackId = QStringLiteral("/org/mpris/MediaPlayer2/Track/1");
    const QString mediaUrl = QStringLiteral("file:///tmp/codex-test.mp3");
    const QVector<int> changedRoles { MprisProperties::Valid, MprisProperties::Identity };
    QSignalSpy rowsInsertedSpy(model, &QAbstractItemModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(model, &QAbstractItemModel::rowsRemoved);

    {
        FakeMprisService fakeService(mediaUrl);
        QVERIFY2(bus.registerObject(QString::fromLatin1(MprisPath),
                                    &fakeService,
                                    QDBusConnection::ExportAdaptors),
                 qPrintable(bus.lastError().message()));
        QVERIFY2(bus.registerService(service), qPrintable(bus.lastError().message()));
        RegisteredServiceGuard guard(bus, service, QString::fromLatin1(MprisPath));

        QTRY_COMPARE(model->rowCount({}), baselineCount + 1);
        QTRY_COMPARE(rowsInsertedSpy.count(), 1);

        const QModelIndex row = model->index(baselineCount, 0, {});
        QVERIFY(row.isValid());

        QTRY_COMPARE(model->data(row, MprisProperties::Identity).toString(), fakeService.identity);

        QCOMPARE(model->data(row, MprisProperties::Pid).toUInt(),
                 static_cast<uint>(QCoreApplication::applicationPid()));
        QCOMPARE(model->data(row, MprisProperties::Valid).toBool(), true);
        QCOMPARE(model->data(row, MprisProperties::CanQuit).toBool(), fakeService.canQuit);
        QCOMPARE(model->data(row, MprisProperties::FullScreen).toBool(), fakeService.fullScreen);
        QCOMPARE(model->data(row, MprisProperties::CanSetFullScreen).toBool(), fakeService.canSetFullScreen);
        QCOMPARE(model->data(row, MprisProperties::CanRaise).toBool(), fakeService.canRaise);
        QCOMPARE(model->data(row, MprisProperties::HasTrackList).toBool(), fakeService.hasTrackList);
        QCOMPARE(model->data(row, MprisProperties::Identity).toString(), fakeService.identity);
        QCOMPARE(model->data(row, MprisProperties::DesktopEntry).toString(), fakeService.desktopEntry);
        QCOMPARE(model->data(row, MprisProperties::SupportedUriSchemes).toStringList(), fakeService.supportedUriSchemes);
        QCOMPARE(model->data(row, MprisProperties::SupportedMimeTypes).toStringList(), fakeService.supportedMimeTypes);
        QCOMPARE(model->data(row, MprisProperties::PlaybackStatus).toString(), fakeService.playbackStatus);
        QCOMPARE(model->data(row, MprisProperties::LoopStatus).toString(), fakeService.loopStatus);
        QCOMPARE(model->data(row, MprisProperties::Rate).toDouble(), fakeService.rate);
        QCOMPARE(model->data(row, MprisProperties::Shuffle).toBool(), fakeService.shuffle);
        QCOMPARE(model->data(row, MprisProperties::MetaData).toMap(), fakeService.metaData);
        QCOMPARE(model->data(row, MprisProperties::Volume).toDouble(), fakeService.volume);
        QCOMPARE(model->data(row, MprisProperties::Position).toLongLong(), fakeService.position);
        QCOMPARE(model->data(row, MprisProperties::MinimumRate).toDouble(), fakeService.minimumRate);
        QCOMPARE(model->data(row, MprisProperties::MaximumRate).toDouble(), fakeService.maximumRate);
        QCOMPARE(model->data(row, MprisProperties::CanGoNext).toBool(), fakeService.canGoNext);
        QCOMPARE(model->data(row, MprisProperties::CanGoPrevious).toBool(), fakeService.canGoPrevious);
        QCOMPARE(model->data(row, MprisProperties::CanPlay).toBool(), fakeService.canPlay);
        QCOMPARE(model->data(row, MprisProperties::CanPause).toBool(), fakeService.canPause);
        QCOMPARE(model->data(row, MprisProperties::CanSeek).toBool(), fakeService.canSeek);
        QCOMPARE(model->data(row, MprisProperties::CanControl).toBool(), fakeService.canControl);
        QCOMPARE(model->data(row, MprisProperties::IsCurrentMediaVideo).toBool(), false);
        QCOMPARE(model->data(row, MprisProperties::IsCurrentMediaAudio).toBool(), true);
        QVERIFY(!model->data(row, Qt::UserRole).isValid());

        QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
        QVERIFY(QMetaObject::invokeMethod(model, "onDataChanged",
                                          Q_ARG(QString, service),
                                          Q_ARG(QVector<int>, changedRoles)));
        QCOMPARE(dataChangedSpy.count(), 1);

        const auto arguments = dataChangedSpy.takeFirst();
        const auto topLeft = qvariant_cast<QModelIndex>(arguments.at(0));
        const auto bottomRight = qvariant_cast<QModelIndex>(arguments.at(1));
        const auto roles = qvariant_cast<QVector<int>>(arguments.at(2));
        QCOMPARE(topLeft.row(), baselineCount);
        QCOMPARE(topLeft.column(), 0);
        QCOMPARE(bottomRight, topLeft);
        QCOMPARE(roles, changedRoles);

        model->operation({}, MprisProperties::Seek, {});

        const int raiseOrPauseBefore = fakeService.raiseCount + fakeService.pauseCount;
        model->operation(row, MprisProperties::Raise, {});
        QTRY_COMPARE(fakeService.raiseCount + fakeService.pauseCount, raiseOrPauseBefore + 1);

        model->operation(row, MprisProperties::Quit, {});
        QTRY_COMPARE(fakeService.quitCount, 1);

        model->operation(row, MprisProperties::Next, {});
        QTRY_COMPARE(fakeService.nextCount, 1);

        model->operation(row, MprisProperties::Previous, {});
        QTRY_COMPARE(fakeService.previousCount, 1);

        model->operation(row, MprisProperties::Pause, {});
        QTRY_VERIFY(fakeService.pauseCount >= 1);

        model->operation(row, MprisProperties::PlayPause, {});
        QTRY_COMPARE(fakeService.playPauseCount, 1);

        model->operation(row, MprisProperties::Stop, {});
        QTRY_COMPARE(fakeService.stopCount, 1);

        model->operation(row, MprisProperties::Play, {});
        QTRY_COMPARE(fakeService.playCount, 1);

        QTest::ignoreMessage(QtWarningMsg, "Seek without offset!");
        model->operation(row, MprisProperties::Seek, {});

        model->operation(row, MprisProperties::Seek, { QVariant::fromValue<qlonglong>(2468) });
        QTRY_COMPARE(fakeService.lastSeekOffset, 2468LL);

        QTest::ignoreMessage(QtWarningMsg, "setPosition without enough args!");
        model->operation(row, MprisProperties::SetPosition, {});

        model->operation(row, MprisProperties::SetPosition,
                         { trackId, QVariant::fromValue<qlonglong>(97531) });
        QTRY_COMPARE(fakeService.lastSetPositionTrackId, trackId);
        QTRY_COMPARE(fakeService.lastSetPosition, 97531LL);

        QTest::ignoreMessage(QtWarningMsg, "OpenUri without uri!");
        model->operation(row, MprisProperties::OpenUri, {});

        const QString openedUri = QStringLiteral("file:///tmp/opened-by-test.mp3");
        model->operation(row, MprisProperties::OpenUri, { openedUri });
        QTRY_COMPARE(fakeService.lastOpenedUri, openedUri);
    }

    QTRY_COMPARE(model->rowCount({}), baselineCount);
    QTRY_COMPARE(rowsRemovedSpy.count(), 1);

    QSignalSpy removedDataChangedSpy(model, &QAbstractItemModel::dataChanged);
    QVERIFY(QMetaObject::invokeMethod(model, "onDataChanged",
                                      Q_ARG(QString, service),
                                      Q_ARG(QVector<int>, changedRoles)));
    QCOMPARE(removedDataChangedSpy.count(), 0);
}

int main(int argc, char *argv[])
{
    PrivateSessionBus privateBus;
    QCoreApplication app(argc, argv);

    QDBusConnection bus = QDBusConnection::sessionBus();
    g_sessionBusReady = privateBus.isReady() || bus.isConnected();
    if (!g_sessionBusReady) {
        g_sessionBusError = privateBus.errorString();
    }

    player_items_model_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-player-items-model.moc"
