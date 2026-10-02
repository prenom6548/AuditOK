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
#include <QScopedPointer>
#include <QTemporaryDir>

#define private public
#include "../../../src/model/shortcut-config.h"
#undef private

using namespace UkuiShortcut;

namespace {
QScopedPointer<QTemporaryDir> g_tempDir;

void setupConfigEnvironment()
{
    if (!g_tempDir) {
        g_tempDir.reset(new QTemporaryDir);
    }

    const QByteArray configHome = g_tempDir->path().toUtf8();
    qputenv("XDG_CONFIG_HOME", configHome);
    qputenv("HOME", configHome);
}
}

class shortcutConfig_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void test_defaultStateAndIndex();
    void test_recordShortcut_appendsNewKeyToNotAddedList();
    void test_recordShortcut_duplicateKeyKeepsOriginalIndex();
    void test_setShortcutOrder_byIndex_movesAddedItems();
    void test_setShortcutOrder_byIndex_movesNotAddedItems();
    void test_setShortcutOrder_byIndex_ignoresInvalidIndex();
    void test_setShortcutOrder_byIndex_ignoresInvalidNotAddedIndex();
    void test_setShortcutOrder_byId_movesItemIntoAddedList();
    void test_setShortcutOrder_byId_movesItemIntoNotAddedList();
    void test_setShortcutOrder_byId_keepsExistingState();
    void test_changeShortcutState_fromAdd_appendsToEnd();
    void test_changeShortcutState_fromAdd_prependsToStart();
    void test_changeShortcutState_fromNotAdd_prependsToStart();
    void test_changeShortcutState_fromNotAdd_appendsToEnd();
    void test_changeShortcutState_invalidKeyDoesNothing();
    void test_recordShortcut_existingAddedKeyIsIgnored();
    void test_verify_removesDuplicatedNotAddedEntries();
    void test_configChanged_signalIsForwarded();

private:
    ShortcutConfig *m_config = nullptr;
};

void shortcutConfig_test::initTestCase()
{
    m_config = &ShortcutConfig::instance();
    QVERIFY(m_config != nullptr);
}

void shortcutConfig_test::test_defaultStateAndIndex()
{
    QCOMPARE(static_cast<int>(m_config->getShortcutState(QStringLiteral("WiFi"))), static_cast<int>(PluginStateType::Add));
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), 0);
    QCOMPARE(static_cast<int>(m_config->getShortcutState(QStringLiteral("ConnectivityShortcut"))), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("ConnectivityShortcut"), PluginStateType::NotAdd), 5);
    QCOMPARE(static_cast<int>(m_config->getShortcutState(QStringLiteral("UnknownShortcut"))), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("UnknownShortcut"), PluginStateType::Add), -1);
}

void shortcutConfig_test::test_recordShortcut_appendsNewKeyToNotAddedList()
{
    const QString key = QStringLiteral("CustomShortcut");
    QVERIFY(m_config->getShortcutIndex(key, PluginStateType::NotAdd) < 0);

    m_config->recordShortcut(key);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::NotAdd), 6);
}

void shortcutConfig_test::test_recordShortcut_duplicateKeyKeepsOriginalIndex()
{
    const QString key = QStringLiteral("CustomShortcut");
    const int originalIndex = m_config->getShortcutIndex(key, PluginStateType::NotAdd);

    m_config->recordShortcut(key);

    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::NotAdd), originalIndex);
}

void shortcutConfig_test::test_setShortcutOrder_byIndex_movesAddedItems()
{
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), 0);
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("BluetoothShortcut"), PluginStateType::Add), 1);

    m_config->setShortcutOrder(0, 1, PluginStateType::Add);

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("BluetoothShortcut"), PluginStateType::Add), 0);
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), 1);
}

void shortcutConfig_test::test_setShortcutOrder_byIndex_movesNotAddedItems()
{
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("FlightModeShortcut"), PluginStateType::NotAdd), 0);
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("ClipboardShortcut"), PluginStateType::NotAdd), 1);

    m_config->setShortcutOrder(0, 1, PluginStateType::NotAdd);

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("ClipboardShortcut"), PluginStateType::NotAdd), 0);
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("FlightModeShortcut"), PluginStateType::NotAdd), 1);
}

void shortcutConfig_test::test_setShortcutOrder_byIndex_ignoresInvalidIndex()
{
    const int wifiIndex = m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add);

    m_config->setShortcutOrder(-1, 3, PluginStateType::Add);
    m_config->setShortcutOrder(1, 200, PluginStateType::Add);

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), wifiIndex);
}

void shortcutConfig_test::test_setShortcutOrder_byIndex_ignoresInvalidNotAddedIndex()
{
    const int index = m_config->getShortcutIndex(QStringLiteral("FlightModeShortcut"), PluginStateType::NotAdd);

    m_config->setShortcutOrder(0, 200, PluginStateType::NotAdd);

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("FlightModeShortcut"), PluginStateType::NotAdd), index);
}

void shortcutConfig_test::test_setShortcutOrder_byId_movesItemIntoAddedList()
{
    const QString key = QStringLiteral("ConnectivityShortcut");
    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::NotAdd));

    m_config->setShortcutOrder(key, 0, PluginStateType::Add);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::Add));
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::Add), 0);
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::NotAdd), -1);
}

void shortcutConfig_test::test_setShortcutOrder_byId_movesItemIntoNotAddedList()
{
    const QString key = QStringLiteral("WiFi");
    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::Add));

    m_config->setShortcutOrder(key, 0, PluginStateType::NotAdd);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::NotAdd), 0);
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::Add), -1);
}

void shortcutConfig_test::test_setShortcutOrder_byId_keepsExistingState()
{
    m_config->setShortcutOrder(QStringLiteral("WiFi"), 0, PluginStateType::Add);
    const int wifiIndex = m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add);

    m_config->setShortcutOrder(QStringLiteral("WiFi"), 0, PluginStateType::Add);

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), wifiIndex);
}

void shortcutConfig_test::test_changeShortcutState_fromAdd_appendsToEnd()
{
    const QString key = QStringLiteral("ConnectivityShortcut");
    m_config->setShortcutOrder(key, 0, PluginStateType::Add);

    m_config->changeShortcutState(key, PluginStateType::Add, true);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::NotAdd), 6);
}

void shortcutConfig_test::test_changeShortcutState_fromAdd_prependsToStart()
{
    const QString key = QStringLiteral("ClipboardShortcut");
    m_config->setShortcutOrder(key, 0, PluginStateType::Add);

    m_config->changeShortcutState(key, PluginStateType::Add, false);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::NotAdd), 0);
}

void shortcutConfig_test::test_changeShortcutState_fromNotAdd_prependsToStart()
{
    const QString key = QStringLiteral("ConnectivityShortcut");
    QVERIFY(m_config->getShortcutIndex(key, PluginStateType::NotAdd) >= 0);

    m_config->changeShortcutState(key, PluginStateType::NotAdd, false);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::Add));
    QCOMPARE(m_config->getShortcutIndex(key, PluginStateType::Add), 0);
}

void shortcutConfig_test::test_changeShortcutState_fromNotAdd_appendsToEnd()
{
    const QString key = QStringLiteral("WiFi");
    m_config->setShortcutOrder(key, 0, PluginStateType::NotAdd);

    m_config->changeShortcutState(key, PluginStateType::NotAdd, true);

    QCOMPARE(static_cast<int>(m_config->getShortcutState(key)), static_cast<int>(PluginStateType::Add));
    QVERIFY(m_config->getShortcutIndex(key, PluginStateType::Add) >= 0);
}

void shortcutConfig_test::test_changeShortcutState_invalidKeyDoesNothing()
{
    const int wifiIndex = m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add);

    m_config->changeShortcutState(QStringLiteral("MissingShortcut"), PluginStateType::Add, true);

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), wifiIndex);
}

void shortcutConfig_test::test_recordShortcut_existingAddedKeyIsIgnored()
{
    const int addedIndex = m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add);
    const int notAddedIndex = m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::NotAdd);

    m_config->recordShortcut(QStringLiteral("WiFi"));

    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::Add), addedIndex);
    QCOMPARE(m_config->getShortcutIndex(QStringLiteral("WiFi"), PluginStateType::NotAdd), notAddedIndex);
}

void shortcutConfig_test::test_verify_removesDuplicatedNotAddedEntries()
{
    m_config->m_addedList = QStringList{QStringLiteral("WiFi"), QStringLiteral("BluetoothShortcut")};
    m_config->m_notAddedList = QStringList{QStringLiteral("WiFi"), QStringLiteral("VPN")};

    m_config->verify();

    QVERIFY(!m_config->m_notAddedList.contains(QStringLiteral("WiFi")));
    QVERIFY(m_config->m_notAddedList.contains(QStringLiteral("VPN")));
}

void shortcutConfig_test::test_configChanged_signalIsForwarded()
{
    QSignalSpy spy(m_config, SIGNAL(configChanged(QString)));

    m_config->recordShortcut(QStringLiteral("SpyShortcut"));

    QVERIFY(spy.count() > 0 || spy.wait(100));
}

int main(int argc, char *argv[])
{
    setupConfigEnvironment();
    QCoreApplication app(argc, argv);

    shortcutConfig_test test;
    return QTest::qExec(&test, argc, argv);
}

#include "shortcut-config-test.moc"
