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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSignalSpy>

#include "../../ukui-shortcut/shortcuts/wifi-shortcut/wifi-dbus-query.cpp"

namespace {

const char kWifiQueryService[] = "com.kylin.network.autotest.query";
const char kWifiQueryPath[] = "/com/kylin/network/autotest/query";
const char kWifiQueryInterface[] = "com.kylin.network.autotest.query";

class FakeWifiNetworkService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.network.autotest.query")
public:
    QVariantList wiredReply {1, 1, QStringLiteral("eth0")};
    QVariantList wirelessReply {1, 1, QStringLiteral("wifi0")};
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(kWifiQueryPath);
        bus.unregisterService(kWifiQueryService);
        if (!bus.registerService(kWifiQueryService)) {
            return false;
        }

        registered = bus.registerObject(kWifiQueryPath, this, QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(kWifiQueryPath);
        bus.unregisterService(kWifiQueryService);
        registered = false;
    }

public Q_SLOTS:
    QVariantList getNetworkDeviceData(int type)
    {
        return type == 0 ? wiredReply : wirelessReply;
    }
};

class WifiDbusQueryTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_nullInterface_doesNotEmit();
    void test_validReplies_emitsCombinedData();
    void test_emptyReplies_fallBackToDefaultPayload();
    void test_invalidReplies_fallBackToDefaultPayload();

private:
    int removeGeneratedGcovFiles() const;
    QVariantList defaultPayload() const;

private:
    FakeWifiNetworkService m_service;
};

} // namespace

void WifiDbusQueryTest::initTestCase()
{
    qRegisterMetaType<QVariantList>("QVariantList");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void WifiDbusQueryTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void WifiDbusQueryTest::cleanup()
{
    removeGeneratedGcovFiles();
}

int WifiDbusQueryTest::removeGeneratedGcovFiles() const
{
    int removed = 0;
    QDir dir(QDir::currentPath());
    const QFileInfoList files = dir.entryInfoList({QStringLiteral("*.gcov")}, QDir::Files);
    for (const QFileInfo &file : files) {
        if (QFile::remove(file.absoluteFilePath())) {
            ++removed;
        }
    }
    return removed;
}

QVariantList WifiDbusQueryTest::defaultPayload() const
{
    return QVariantList{0, 0, QStringLiteral("")};
}

void WifiDbusQueryTest::test_nullInterface_doesNotEmit()
{
    WifiDbusQuery query;
    QSignalSpy spy(&query, &WifiDbusQuery::networkDeviceData);

    query.getNetworkDeviceData(nullptr);

    QCOMPARE(spy.count(), 0);
}

void WifiDbusQueryTest::test_validReplies_emitsCombinedData()
{
    m_service.wiredReply = QVariantList{1, 0, QStringLiteral("wired-name")};
    m_service.wirelessReply = QVariantList{1, 1, QStringLiteral("wireless-name")};

    QDBusInterface interface(kWifiQueryService,
                             kWifiQueryPath,
                             kWifiQueryInterface,
                             QDBusConnection::sessionBus());
    QVERIFY(interface.isValid());

    WifiDbusQuery query;
    QSignalSpy spy(&query, &WifiDbusQuery::networkDeviceData);

    query.getNetworkDeviceData(&interface);

    QCOMPARE(spy.count(), 1);
    const QVariantList payload = spy.takeFirst().at(0).toList();
    QCOMPARE(payload, QVariantList({1, 0, QStringLiteral("wired-name"),
                                    1, 1, QStringLiteral("wireless-name")}));
}

void WifiDbusQueryTest::test_emptyReplies_fallBackToDefaultPayload()
{
    m_service.wiredReply.clear();
    m_service.wirelessReply.clear();

    QDBusInterface interface(kWifiQueryService,
                             kWifiQueryPath,
                             kWifiQueryInterface,
                             QDBusConnection::sessionBus());
    QVERIFY(interface.isValid());

    WifiDbusQuery query;
    QSignalSpy spy(&query, &WifiDbusQuery::networkDeviceData);

    query.getNetworkDeviceData(&interface);

    QCOMPARE(spy.count(), 1);
    const QVariantList payload = spy.takeFirst().at(0).toList();
    QCOMPARE(payload, QVariantList(defaultPayload() + defaultPayload()));
}

void WifiDbusQueryTest::test_invalidReplies_fallBackToDefaultPayload()
{
    m_service.unregisterService();

    QDBusInterface interface(kWifiQueryService,
                             kWifiQueryPath,
                             kWifiQueryInterface,
                             QDBusConnection::sessionBus());

    WifiDbusQuery query;
    QSignalSpy spy(&query, &WifiDbusQuery::networkDeviceData);

    query.getNetworkDeviceData(&interface);

    QCOMPARE(spy.count(), 1);
    const QVariantList payload = spy.takeFirst().at(0).toList();
    QCOMPARE(payload, QVariantList(defaultPayload() + defaultPayload()));

    QVERIFY(m_service.registerService());
}

QTEST_MAIN(WifiDbusQueryTest)

#include "wifi-dbus-query-test.moc"
