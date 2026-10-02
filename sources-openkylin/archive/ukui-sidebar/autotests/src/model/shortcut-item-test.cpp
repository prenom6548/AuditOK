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

#include "../../../src/model/shortcut-item.h"
#include "../../../src/model/shortcut-config.h"
#include "../../../src/model/shortcut-model.h"

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

StatusInfo createStatusInfo(bool disabled,
                            const QString &name,
                            const QString &statusName,
                            const QString &widgetId,
                            const QString &icon,
                            const QString &tooltip,
                            Color::ColorRole color,
                            int value,
                            int maxValue,
                            const QList<StatusInfo::MenuItem> &menu = {})
{
    StatusInfo info;
    info.setDisable(disabled);
    info.setName(name);
    info.setStatusName(statusName);
    info.setWidgetId(widgetId);
    info.setIcon(icon);
    info.setToolTip(tooltip);
    info.setColor(color);
    info.setValue(value);
    info.setMaxValue(maxValue);
    info.setMenu(menu);
    return info;
}

class FakeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit FakeShortcut(const QString &pluginId,
                          const QMap<PluginMetaType::SystemMode, PluginMetaData> &metaData,
                          const StatusInfo &statusInfo,
                          bool enabled = true,
                          QObject *parent = nullptr)
        : Shortcut(parent)
        , m_pluginId(pluginId)
        , m_metaData(metaData)
        , m_statusInfo(statusInfo)
        , m_enabled(enabled)
    {
    }

    QString pluginId() override
    {
        return m_pluginId;
    }

    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override
    {
        return m_metaData;
    }

    void active(PluginMetaType::Action action) override
    {
        m_lastAction = action;
        ++m_activeCount;
    }

    void setValue(int value) override
    {
        m_lastSetValue = value;
    }

    const StatusInfo currentStatus() override
    {
        return m_statusInfo;
    }

    bool isEnable() override
    {
        return m_enabled;
    }

    void setAddedState(bool added) override
    {
        m_addedState = added;
    }

    void emitStatusChanged(const StatusInfo &info)
    {
        m_statusInfo = info;
        Q_EMIT statusChanged(info);
    }

    void emitEnableStatusChanged(bool enabled)
    {
        m_enabled = enabled;
        Q_EMIT enableStatusChanged(enabled);
    }

    QString m_pluginId;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    StatusInfo m_statusInfo;
    bool m_enabled = true;
    bool m_addedState = false;
    int m_lastSetValue = -1;
    int m_activeCount = 0;
    PluginMetaType::Action m_lastAction = PluginMetaType::Click;
};

PluginMetaData createMetaData(bool visible,
                              int index,
                              PluginMetaType::PluginType type,
                              PluginMetaType::PredefinedAction preAction = PluginMetaType::NoAction)
{
    PluginMetaData metaData(visible, index, type);
    metaData.setPreAction(preAction);
    return metaData;
}

bool containsRole(const QVector<int> &roles, ShortcutModel::Role role)
{
    return roles.contains(static_cast<int>(role));
}
}

class shortcutItem_test : public QObject
{
    Q_OBJECT

private slots:
    void test_progressBarShortcut_initializesFixedState();
    void test_normalShortcut_readsStatusAndGetters();
    void test_enableStatusChanged_emitsIsEnableRole();
    void test_statusChanged_emitsAllChangedRoles();
    void test_statusChanged_sameInfo_emitsNothing();
    void test_configChanged_updatesAddStateAndIndex();
    void test_configChanged_withoutRelevantDataChange_emitsNothing();
    void test_active_withPredefinedAction_emitsRequestAndCallsShortcut();
    void test_active_withoutPredefinedAction_onlyCallsShortcut();
    void test_setValue_forwardsToShortcut();
};

void shortcutItem_test::test_progressBarShortcut_initializesFixedState()
{
    const StatusInfo info = createStatusInfo(false,
                                             QStringLiteral("Progress"),
                                             QStringLiteral("50%"),
                                             QStringLiteral("progressWidget"),
                                             QStringLiteral("progress-icon"),
                                             QStringLiteral("progress-tooltip"),
                                             Color::BaseColor,
                                             50,
                                             100);

    FakeShortcut shortcut(QStringLiteral("progress-shortcut"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 3, PluginMetaType::ProgressBar)},
                              {PluginMetaType::Tablet, createMetaData(false, 4, PluginMetaType::MenuButton)}
                          },
                          info,
                          false);

    ShortcutItem item(&shortcut);

    QCOMPARE(item.pluginId(), QStringLiteral("progress-shortcut"));
    QCOMPARE(item.isEnable(), false);
    QCOMPARE(item.isVisible(PluginMetaType::PC), true);
    QCOMPARE(item.isVisible(PluginMetaType::Tablet), false);
    QCOMPARE(item.itemType(PluginMetaType::PC), PluginMetaType::ProgressBar);
    QCOMPARE(item.itemType(PluginMetaType::Tablet), PluginMetaType::MenuButton);
    QCOMPARE(item.row(), 1);
    QCOMPARE(item.column(PluginMetaType::PC), 1);
    QCOMPARE(item.column(PluginMetaType::Tablet), 2);
    QCOMPARE(static_cast<int>(item.addState()), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(item.pluginIndex(), 3);
    QCOMPARE(static_cast<int>(item.fixState()), static_cast<int>(PluginStateType::Fix));
    QCOMPARE(shortcut.m_addedState, true);
}

void shortcutItem_test::test_normalShortcut_readsStatusAndGetters()
{
    const QList<StatusInfo::MenuItem> menu = {
        {QStringLiteral("icon-1"), QStringLiteral("text-1"), QStringLiteral("status-1")},
        {QStringLiteral("icon-2"), QStringLiteral("text-2"), QStringLiteral("status-2")}
    };
    const StatusInfo info = createStatusInfo(true,
                                             QStringLiteral("WiFi"),
                                             QStringLiteral("Connected"),
                                             QStringLiteral("toggle"),
                                             QStringLiteral("network-wireless"),
                                             QStringLiteral("network-tooltip"),
                                             Color::Warning,
                                             30,
                                             120,
                                             menu);
    const QString pluginId = QStringLiteral("shortcut-item-getter-test");

    FakeShortcut shortcut(pluginId,
                          {
                              {PluginMetaType::PC, createMetaData(true, 2, PluginMetaType::Icon)}
                          },
                          info);

    ShortcutItem item(&shortcut);

    QCOMPARE(item.pluginId(), pluginId);
    QCOMPARE(item.disabled(), true);
    QCOMPARE(item.name(), QStringLiteral("WiFi"));
    QCOMPARE(item.statusName(), QStringLiteral("Connected"));
    QCOMPARE(item.widgetId(), QStringLiteral("toggle"));
    QCOMPARE(item.icon(), QStringLiteral("network-wireless"));
    QCOMPARE(item.tooltip(), QStringLiteral("network-tooltip"));
    QCOMPARE(item.color(), Color::Warning);
    QCOMPARE(item.value(), 30);
    QCOMPARE(item.maxValue(), 120);
    QCOMPARE(item.itemType(PluginMetaType::PC), PluginMetaType::Icon);
    QCOMPARE(static_cast<int>(item.addState()), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(static_cast<int>(item.fixState()), static_cast<int>(PluginStateType::NotFix));
    QCOMPARE(shortcut.m_addedState, false);

    const QList<StatusInfo::MenuItem> returnedMenu = item.menu().value<QList<StatusInfo::MenuItem>>();
    QCOMPARE(returnedMenu.size(), 2);
    QCOMPARE(returnedMenu.at(0).text, QStringLiteral("text-1"));
    QCOMPARE(returnedMenu.at(1).statusIcon, QStringLiteral("status-2"));
}

void shortcutItem_test::test_enableStatusChanged_emitsIsEnableRole()
{
    FakeShortcut shortcut(QStringLiteral("shortcut-item-enable-test"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::Icon)}
                          },
                          createStatusInfo(false, QStringLiteral("name"), QStringLiteral("status"), QStringLiteral("widget"), QStringLiteral("icon"), QStringLiteral("tooltip"), Color::BaseColor, 1, 10),
                          true);
    ShortcutItem item(&shortcut);

    QVector<QVector<int>> emissions;
    connect(&item, &ShortcutItem::itemDataChanged, this, [&emissions] (QVector<int> &roles) {
        emissions.append(roles);
    });

    shortcut.emitEnableStatusChanged(false);

    QCOMPARE(item.isEnable(), false);
    QCOMPARE(emissions.size(), 1);
    QCOMPARE(emissions.at(0).size(), 1);
    QVERIFY(containsRole(emissions.at(0), ShortcutModel::IsEnable));
}

void shortcutItem_test::test_statusChanged_emitsAllChangedRoles()
{
    FakeShortcut shortcut(QStringLiteral("shortcut-item-status-test"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::Icon)}
                          },
                          createStatusInfo(false,
                                           QStringLiteral("name-a"),
                                           QStringLiteral("status-a"),
                                           QStringLiteral("widget-a"),
                                           QStringLiteral("icon-a"),
                                           QStringLiteral("tooltip-a"),
                                           Color::BaseColor,
                                           1,
                                           10));
    ShortcutItem item(&shortcut);

    QVector<QVector<int>> emissions;
    connect(&item, &ShortcutItem::itemDataChanged, this, [&emissions] (QVector<int> &roles) {
        emissions.append(roles);
    });

    const StatusInfo changedInfo = createStatusInfo(true,
                                                    QStringLiteral("name-b"),
                                                    QStringLiteral("status-b"),
                                                    QStringLiteral("widget-b"),
                                                    QStringLiteral("icon-b"),
                                                    QStringLiteral("tooltip-b"),
                                                    Color::Warning,
                                                    9,
                                                    99,
                                                    {
                                                        {QStringLiteral("menu-icon"), QStringLiteral("menu-text"), QStringLiteral("menu-status")}
                                                    });

    shortcut.emitStatusChanged(changedInfo);

    QCOMPARE(emissions.size(), 1);
    const QVector<int> roles = emissions.constFirst();
    QCOMPARE(roles.size(), 9);
    QVERIFY(containsRole(roles, ShortcutModel::Name));
    QVERIFY(containsRole(roles, ShortcutModel::StatusName));
    QVERIFY(containsRole(roles, ShortcutModel::WidgetId));
    QVERIFY(containsRole(roles, ShortcutModel::Color));
    QVERIFY(containsRole(roles, ShortcutModel::ToolTip));
    QVERIFY(containsRole(roles, ShortcutModel::Icon));
    QVERIFY(containsRole(roles, ShortcutModel::Disabled));
    QVERIFY(containsRole(roles, ShortcutModel::Value));
    QVERIFY(containsRole(roles, ShortcutModel::MaxValue));

    QCOMPARE(item.disabled(), true);
    QCOMPARE(item.name(), QStringLiteral("name-b"));
    QCOMPARE(item.statusName(), QStringLiteral("status-b"));
    QCOMPARE(item.widgetId(), QStringLiteral("widget-b"));
    QCOMPARE(item.icon(), QStringLiteral("icon-b"));
    QCOMPARE(item.tooltip(), QStringLiteral("tooltip-b"));
    QCOMPARE(item.color(), Color::Warning);
    QCOMPARE(item.value(), 9);
    QCOMPARE(item.maxValue(), 99);
}

void shortcutItem_test::test_statusChanged_sameInfo_emitsNothing()
{
    const StatusInfo info = createStatusInfo(false,
                                             QStringLiteral("same-name"),
                                             QStringLiteral("same-status"),
                                             QStringLiteral("same-widget"),
                                             QStringLiteral("same-icon"),
                                             QStringLiteral("same-tooltip"),
                                             Color::BaseColor,
                                             8,
                                             80);
    FakeShortcut shortcut(QStringLiteral("shortcut-item-same-status-test"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::Icon)}
                          },
                          info);
    ShortcutItem item(&shortcut);

    int emissionCount = 0;
    connect(&item, &ShortcutItem::itemDataChanged, this, [&emissionCount] (QVector<int> &) {
        ++emissionCount;
    });

    shortcut.emitStatusChanged(info);

    QCOMPARE(emissionCount, 0);
}

void shortcutItem_test::test_configChanged_updatesAddStateAndIndex()
{
    const QString pluginId = QStringLiteral("shortcut-item-config-change-test");
    FakeShortcut shortcut(pluginId,
                          {
                              {PluginMetaType::PC, createMetaData(true, 6, PluginMetaType::Icon)}
                          },
                          createStatusInfo(false, QStringLiteral("config"), QStringLiteral("status"), QStringLiteral("widget"), QStringLiteral("icon"), QStringLiteral("tooltip"), Color::BaseColor, 1, 10));
    ShortcutItem item(&shortcut);

    QVector<QVector<int>> emissions;
    connect(&item, &ShortcutItem::itemDataChanged, this, [&emissions] (QVector<int> &roles) {
        emissions.append(roles);
    });

    QCOMPARE(static_cast<int>(item.addState()), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(item.pluginIndex(), ShortcutConfig::instance().getShortcutIndex(pluginId, PluginStateType::NotAdd));

    ShortcutConfig::instance().setShortcutOrder(pluginId, 0, PluginStateType::Add);

    QCOMPARE(static_cast<int>(item.addState()), static_cast<int>(PluginStateType::Add));
    QCOMPARE(item.pluginIndex(), 0);
    QCOMPARE(shortcut.m_addedState, true);
    QCOMPARE(emissions.size(), 1);
    QVERIFY(containsRole(emissions.at(0), ShortcutModel::AddState));
    QVERIFY(containsRole(emissions.at(0), ShortcutModel::PluginIndex));
}

void shortcutItem_test::test_configChanged_withoutRelevantDataChange_emitsNothing()
{
    const QString pluginId = QStringLiteral("shortcut-item-config-stable-test");
    FakeShortcut shortcut(pluginId,
                          {
                              {PluginMetaType::PC, createMetaData(true, 7, PluginMetaType::Icon)}
                          },
                          createStatusInfo(false, QStringLiteral("stable"), QStringLiteral("status"), QStringLiteral("widget"), QStringLiteral("icon"), QStringLiteral("tooltip"), Color::BaseColor, 2, 20));
    ShortcutItem item(&shortcut);

    int emissionCount = 0;
    connect(&item, &ShortcutItem::itemDataChanged, this, [&emissionCount] (QVector<int> &) {
        ++emissionCount;
    });

    ShortcutConfig::instance().recordShortcut(QStringLiteral("shortcut-item-config-other-test"));

    QCOMPARE(emissionCount, 0);
    QCOMPARE(static_cast<int>(item.addState()), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(shortcut.m_addedState, false);
}

void shortcutItem_test::test_active_withPredefinedAction_emitsRequestAndCallsShortcut()
{
    FakeShortcut shortcut(QStringLiteral("shortcut-item-active-pre-test"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::Icon, PluginMetaType::Hide)}
                          },
                          createStatusInfo(false, QStringLiteral("active"), QStringLiteral("status"), QStringLiteral("widget"), QStringLiteral("icon"), QStringLiteral("tooltip"), Color::BaseColor, 1, 10));
    ShortcutItem item(&shortcut);

    QVector<int> predefinedActions;
    connect(&item, &ShortcutItem::requestExecAction, this, [&predefinedActions] (PluginMetaType::PredefinedAction action) {
        predefinedActions.append(static_cast<int>(action));
    });

    item.active(PluginMetaType::LongClick, PluginMetaType::PC);

    QCOMPARE(predefinedActions.size(), 1);
    QCOMPARE(predefinedActions.constFirst(), static_cast<int>(PluginMetaType::Hide));
    QCOMPARE(shortcut.m_activeCount, 1);
    QCOMPARE(static_cast<int>(shortcut.m_lastAction), static_cast<int>(PluginMetaType::LongClick));
}

void shortcutItem_test::test_active_withoutPredefinedAction_onlyCallsShortcut()
{
    FakeShortcut shortcut(QStringLiteral("shortcut-item-active-no-pre-test"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::Icon, PluginMetaType::NoAction)}
                          },
                          createStatusInfo(false, QStringLiteral("active"), QStringLiteral("status"), QStringLiteral("widget"), QStringLiteral("icon"), QStringLiteral("tooltip"), Color::BaseColor, 1, 10));
    ShortcutItem item(&shortcut);

    int predefinedActionCount = 0;
    connect(&item, &ShortcutItem::requestExecAction, this, [&predefinedActionCount] (PluginMetaType::PredefinedAction) {
        ++predefinedActionCount;
    });

    item.active(PluginMetaType::MenuRequest, PluginMetaType::PC);

    QCOMPARE(predefinedActionCount, 0);
    QCOMPARE(shortcut.m_activeCount, 1);
    QCOMPARE(static_cast<int>(shortcut.m_lastAction), static_cast<int>(PluginMetaType::MenuRequest));
}

void shortcutItem_test::test_setValue_forwardsToShortcut()
{
    FakeShortcut shortcut(QStringLiteral("shortcut-item-set-value-test"),
                          {
                              {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::ProgressBar)}
                          },
                          createStatusInfo(false, QStringLiteral("value"), QStringLiteral("status"), QStringLiteral("widget"), QStringLiteral("icon"), QStringLiteral("tooltip"), Color::BaseColor, 1, 10));
    ShortcutItem item(&shortcut);

    item.setValue(77);

    QCOMPARE(shortcut.m_lastSetValue, 77);
}

int main(int argc, char *argv[])
{
    setupConfigEnvironment();
    QCoreApplication app(argc, argv);

    shortcutItem_test test;
    return QTest::qExec(&test, argc, argv);
}

#include "shortcut-item-test.moc"
