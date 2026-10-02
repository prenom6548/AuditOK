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

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/bluetooth-shortcut/bluetooth-connector.h"
#undef private

class BluetoothConnectorTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void cleanupTestCase();

    void test_constructorInitializesWithoutInterface();
    void test_runEmitsReadyWhenInterfaceIsValidImmediately();
    void test_runDeletesInvalidInterfaceBeforeSuccess();
    void test_runEmitsFailedWhenInterfaceNeverBecomesValid();

private:
    int removeGeneratedGcovFiles() const;
    void deleteReadyInterface(BluetoothConnector &connector) const;
};

void BluetoothConnectorTest::init()
{
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    QDBusInterface::resetTestData();
    removeGeneratedGcovFiles();
}

void BluetoothConnectorTest::cleanup()
{
    QDBusInterface::resetTestData();
    removeGeneratedGcovFiles();
}

void BluetoothConnectorTest::cleanupTestCase()
{
    removeGeneratedGcovFiles();
}

int BluetoothConnectorTest::removeGeneratedGcovFiles() const
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

void BluetoothConnectorTest::deleteReadyInterface(BluetoothConnector &connector) const
{
    delete connector.m_interface;
    connector.m_interface = nullptr;
}

void BluetoothConnectorTest::test_constructorInitializesWithoutInterface()
{
    BluetoothConnector connector;

    QVERIFY(connector.m_interface == nullptr);
}

void BluetoothConnectorTest::test_runEmitsReadyWhenInterfaceIsValidImmediately()
{
    QDBusInterface::setDefaultValidity(true);
    BluetoothConnector connector;
    QSignalSpy readySpy(&connector, &BluetoothConnector::ready);
    QSignalSpy failedSpy(&connector, &BluetoothConnector::failed);

    connector.run();

    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(failedSpy.count(), 0);
    QCOMPARE(QDBusInterface::createdCount(), 1);
    QCOMPARE(QDBusInterface::destroyedCount(), 0);
    QVERIFY(connector.m_interface != nullptr);
    QCOMPARE(connector.m_interface->service(), QStringLiteral(BLUETOOTH_SERVICE_NAME));
    QCOMPARE(connector.m_interface->path(), QStringLiteral(BLUETOOTH_SERVICE_PATH));
    QCOMPARE(connector.m_interface->interface(), QStringLiteral(BLUETOOTH_SERVICE_INTERFACE));
    QCOMPARE(connector.m_interface->thread(), QApplication::instance()->thread());

    deleteReadyInterface(connector);
}

void BluetoothConnectorTest::test_runDeletesInvalidInterfaceBeforeSuccess()
{
    QDBusInterface::setValiditySequence({false, true});
    BluetoothConnector connector;
    QSignalSpy readySpy(&connector, &BluetoothConnector::ready);
    QSignalSpy failedSpy(&connector, &BluetoothConnector::failed);

    connector.run();

    QCOMPARE(readySpy.count(), 1);
    QCOMPARE(failedSpy.count(), 0);
    QCOMPARE(QDBusInterface::createdCount(), 2);
    QCOMPARE(QDBusInterface::destroyedCount(), 1);
    QVERIFY(connector.m_interface != nullptr);
    QVERIFY(connector.m_interface->isValid());

    deleteReadyInterface(connector);
}

void BluetoothConnectorTest::test_runEmitsFailedWhenInterfaceNeverBecomesValid()
{
    QDBusInterface::setDefaultValidity(false);
    BluetoothConnector connector;
    QSignalSpy readySpy(&connector, &BluetoothConnector::ready);
    QSignalSpy failedSpy(&connector, &BluetoothConnector::failed);

    connector.run();

    QCOMPARE(readySpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);
    QVERIFY(QDBusInterface::createdCount() > 0);
    QCOMPARE(QDBusInterface::destroyedCount(), QDBusInterface::createdCount());
}

QTEST_MAIN(BluetoothConnectorTest)

#include "bluetooth-connector-test.moc"
