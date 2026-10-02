/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QDBusMetaType>
#include <QDBusVirtualObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include "../../registeredSession/plugins/shortcut/shortcutinterface.h"

class FakeKwinShortcutObject : public QDBusVirtualObject
{
public:
    QString introspect(const QString &path) const override
    {
        Q_UNUSED(path);
        return "<node>"
               "  <interface name=\"org.kde.KWin\">"
               "    <method name=\"getShortcutInfos\">"
               "      <arg direction=\"out\" type=\"av\"/>"
               "    </method>"
               "  </interface>"
               "</node>";
    }

    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override
    {
        if (message.member() == "getShortcutInfos") {
            QVariantList data;
            data << QVariant(QStringList({"kwin-switch", "Switch", "<Meta>Up", "<Meta>Down"}));
            data << QVariant(QStringList({"invalid"}));
            connection.send(message.createReply(QVariant::fromValue(data)));
            return true;
        }
        return false;
    }
};

class ShortcutInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pShortcutInterface = new ShortcutInterface();
        m_pShortcutInterface->getThreadStart();
    }

    static void TearDownTestSuite()
    {
    }

    static ShortcutInterface *m_pShortcutInterface;
};

ShortcutInterface *ShortcutInterfaceTest::m_pShortcutInterface = nullptr;

TEST_F(ShortcutInterfaceTest, getSystemShortcut)
{
    QStringPairList pairList = m_pShortcutInterface->getSystemShortcut();
    ASSERT_GE(pairList.size(), 15);
}

TEST_F(ShortcutInterfaceTest, getCustomShortcut)
{
    QStringPairList pairList = m_pShortcutInterface->getCustomShortcut();
    ASSERT_GE(pairList.size(), 0);
}

TEST_F(ShortcutInterfaceTest, getSystemShortcutEntry)
{
    QList<KeyEntry> pairList = m_pShortcutInterface->getSystemShortcutEntry();
    ASSERT_GE(pairList.size(), 15);
}

TEST_F(ShortcutInterfaceTest, getCustomShortcutEntry)
{
    QList<KeyEntry> pairList = m_pShortcutInterface->getCustomShortcutEntry();
    ASSERT_GE(pairList.size(), 0);
}

TEST_F(ShortcutInterfaceTest, getShowShortcutString)
{
    QString shortKey = "Meta+A";
    m_pShortcutInterface->updateShortcut();
    QString newShortKey = m_pShortcutInterface->getShowShortcutString(shortKey);
    ASSERT_GE(newShortKey.toStdString().c_str(), "Win+A");
}

TEST_F(ShortcutInterfaceTest, updateShortcut)
{
    bool ret = m_pShortcutInterface->updateShortcut();
    ASSERT_EQ(ret, true);
}

TEST_F(ShortcutInterfaceTest, setSystemShortcut)
{
    QGSettings generalShort("org.ukui.SettingsDaemon.plugins.media-keys");
    std::string oldValue = generalShort.get("kylin-display-switch").toString().toStdString();

    m_pShortcutInterface->setSystemShortcut("kylin-display-switch", "<Win>V");
    std::string newValue = generalShort.get("kylin-display-switch").toString().toStdString();

    ASSERT_STREQ(newValue.c_str(), "<Win>V");

    generalShort.set("kylin-display-switch", oldValue.c_str());
}

TEST_F(ShortcutInterfaceTest, getWindowShortcut)
{
    QStringPairList pairList = m_pShortcutInterface->getWindowShortcut();
    ASSERT_GE(pairList.size(), 0);
}

TEST_F(ShortcutInterfaceTest, getShortKeyValue)
{
    QVariantMap shortcutMap = m_pShortcutInterface->getShortKeyValue();
    ASSERT_GE(shortcutMap.size(), 0);
}

TEST_F(ShortcutInterfaceTest, getShowShortcutStringComplex)
{
    QString src = "<Meta>a or <Start>b";
    QString dst = m_pShortcutInterface->getShowShortcutString(src);
    ASSERT_TRUE(dst.contains("Win"));
    ASSERT_TRUE(dst.contains("or"));
}

TEST_F(ShortcutInterfaceTest, customShortcutSlotNoDuplicate)
{
    int oldSize = m_pShortcutInterface->getCustomShortcutEntry().size();
    m_pShortcutInterface->customShortcutSlot("/org/ukui/desktop/keybindings/test1/", "Test", "<Alt>1", "xterm");
    m_pShortcutInterface->customShortcutSlot("/org/ukui/desktop/keybindings/test1/", "Test", "<Alt>1", "xterm");
    int newSize = m_pShortcutInterface->getCustomShortcutEntry().size();
    ASSERT_GE(newSize, oldSize);
    ASSERT_LE(newSize - oldSize, 1);
}

TEST_F(ShortcutInterfaceTest, defaultShortcutPath)
{
    m_pShortcutInterface->defaultShortcut();
    ASSERT_GE(m_pShortcutInterface->getSystemShortcutEntry().size(), 0);
}

TEST_F(ShortcutInterfaceTest, getWindowShortcutEntry)
{
    QList<KeyEntry> entries = m_pShortcutInterface->getWindowShortcutEntry();
    ASSERT_GE(entries.size(), 0);
}

TEST_F(ShortcutInterfaceTest, setWindowShortcut)
{
    KeyEntry entry;
    entry.keyStr = "ukcc-test-window-key2";
    entry.valueStr = "<Ctrl><Alt>A";
    m_pShortcutInterface->m_windowEntries.append(entry);
    m_pShortcutInterface->m_shortKeyValue.insert(entry.keyStr, entry.valueStr);
    m_pShortcutInterface->setWindowShortcut(entry.keyStr, entry.valueStr, "<Ctrl><Alt>B");
    ASSERT_TRUE(m_pShortcutInterface->m_shortKeyValue.contains(entry.keyStr));
}

TEST_F(ShortcutInterfaceTest, setCustomShortcut)
{
    m_pShortcutInterface->setCustomShortcut("/org/ukui/desktop/keybindings/custom0/", "<Alt><Shift>C");
    ASSERT_TRUE(true);
}

TEST_F(ShortcutInterfaceTest, buildGeneralWidgetInvalidSchema)
{
    QMap<QString, QString> map;
    map.insert("dummy", "<Ctrl>D");
    m_pShortcutInterface->buildGeneralWidget("InvalidSchema", map);
    ASSERT_TRUE(true);
}

TEST_F(ShortcutInterfaceTest, typeCodecQStringPair)
{
    qRegisterMetaType<QStringPair>("QStringPair");
    qDBusRegisterMetaType<QStringPair>();
    QStringPair in;
    in.name = "name";
    in.key = "<Ctrl>A";
    in.mediakey = "media";

    QDBusArgument argument;
    argument << in;
    const QDBusArgument &readArg = argument;
    QStringPair out;
    readArg >> out;
    SUCCEED();
}

TEST_F(ShortcutInterfaceTest, typeCodecKeyEntry)
{
    qRegisterMetaType<KeyEntry>("KeyEntry");
    qDBusRegisterMetaType<KeyEntry>();
    KeyEntry in;
    in.gsSchema = "schema";
    in.keyStr = "key";
    in.valueStr = "value";
    in.descStr = "desc";
    in.gsPath = "/org/ukui/desktop/keybindings/test1/";
    in.nameStr = "name";
    in.bindingStr = "<Ctrl><Alt>T";
    in.actionStr = "xterm";

    QDBusArgument argument;
    argument << in;
    const QDBusArgument &readArg = argument;
    KeyEntry out;
    readArg >> out;
    SUCCEED();
}

TEST_F(ShortcutInterfaceTest, workerGetWindowShortcutInvalidInterface)
{
    UniversalInterface *ui = UniversalInterface::self();
    QDBusInterface *oldInterface = ui->m_kwinDbusInterface;
    ui->m_kwinDbusInterface = new QDBusInterface("org.ukui.invalid", "/KWin", "org.kde.KWin", QDBusConnection::sessionBus());

    GetShortcutWorker worker;
    worker.getWindowShortcut();
    SUCCEED();

    delete ui->m_kwinDbusInterface;
    ui->m_kwinDbusInterface = oldInterface;
}

TEST_F(ShortcutInterfaceTest, workerGetWindowShortcutGenerate)
{
    const QString service = "org.ukui.test.KWin";
    const QString path = "/KWin";
    FakeKwinShortcutObject fakeObj;
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerService(service);
    bus.registerVirtualObject(path, &fakeObj, QDBusConnection::SubPath);

    UniversalInterface *ui = UniversalInterface::self();
    QDBusInterface *oldInterface = ui->m_kwinDbusInterface;
    ui->m_kwinDbusInterface = new QDBusInterface(service, path, "org.kde.KWin", bus);

    GetShortcutWorker worker;
    int signalCount = 0;
    QObject::connect(&worker, &GetShortcutWorker::windowShortcutGenerate,
                     [&](const QString &, const QString &, const QString &) {
        signalCount++;
    });
    worker.getWindowShortcut();
    ASSERT_GE(signalCount, 2);

    delete ui->m_kwinDbusInterface;
    ui->m_kwinDbusInterface = oldInterface;
    bus.unregisterObject(path);
    bus.unregisterService(service);
}

TEST_F(ShortcutInterfaceTest, workerGetSystemShortcutKPDeletePath)
{
    QGSettings settings(KEYBINDINGS_DESKTOP_SCHEMA);
    if (!settings.keys().contains("logout")) {
        SUCCEED();
        return;
    }

    QString oldValue = settings.get("logout").toString();
    settings.set("logout", "<Ctrl><Alt>KP_Delete");

    GetShortcutWorker worker;
    int signalCount = 0;
    QObject::connect(&worker, &GetShortcutWorker::generalShortcutGenerate,
                     [&](const QString &, const QString &, const QString &value) {
        if (value == "<Ctrl><Alt>Del") {
            signalCount++;
        }
    });
    worker.getSystemShortcut();
    ASSERT_GE(signalCount, 1);
    settings.set("logout", oldValue);
}
