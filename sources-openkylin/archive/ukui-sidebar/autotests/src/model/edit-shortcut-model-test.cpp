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
#include <QGuiApplication>
#include <QQuickItem>
#include <QScopedPointer>
#include <QTemporaryDir>

#include "../../../src/model/edit-shortcut-model.h"
#include "../../../src/model/shortcut-config.h"
#include "../../../src/model/shortcut-model.h"
#include "../../../ukui-shortcut/short-cut-manager.h"

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

StatusInfo createStatusInfo(const QString &name,
                            const QString &statusName,
                            const QString &widgetId,
                            const QString &icon,
                            const QString &tooltip,
                            Color::ColorRole color,
                            int value,
                            int maxValue,
                            bool disabled = false,
                            const QList<StatusInfo::MenuItem> &menu = {})
{
    StatusInfo info;
    info.setName(name);
    info.setStatusName(statusName);
    info.setWidgetId(widgetId);
    info.setIcon(icon);
    info.setToolTip(tooltip);
    info.setColor(color);
    info.setValue(value);
    info.setMaxValue(maxValue);
    info.setDisable(disabled);
    info.setMenu(menu);
    return info;
}

PluginMetaData createMetaData(bool visible,
                              int index,
                              PluginMetaType::PluginType type,
                              PluginMetaType::PredefinedAction preAction = PluginMetaType::NoAction)
{
    PluginMetaData metaData(visible, index, type);
    metaData.setPreAction(preAction);
    return metaData;
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
        ++m_activeCount;
        m_lastAction = action;
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

    QString m_pluginId;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    StatusInfo m_statusInfo;
    bool m_enabled = true;
    bool m_addedState = false;
    int m_activeCount = 0;
    int m_lastSetValue = -1;
    PluginMetaType::Action m_lastAction = PluginMetaType::Click;
};

QQuickItem *createChildItem(QQuickItem *parent, int columnSpan)
{
    auto *item = new QQuickItem(parent);
    item->setProperty("columnSpan", columnSpan);
    return item;
}
}

namespace UkuiShortcut {
ShortcutManager *g_shortcutManagerInstance = nullptr;
QList<Shortcut*> g_fakeShortcuts;

class ShortcutManagerPrivate
{
};

ShortcutManager *ShortcutManager::getInstance()
{
    if (!g_shortcutManagerInstance) {
        g_shortcutManagerInstance = new ShortcutManager;
    }
    return g_shortcutManagerInstance;
}

ShortcutManager::ShortcutManager()
    : d(new ShortcutManagerPrivate)
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
}

ShortcutManager::~ShortcutManager() = default;

QList<Shortcut *> ShortcutManager::getShortcuts()
{
    return g_fakeShortcuts;
}

QPair<bool, QString> ShortcutManager::getWidgetInfo(const QString &widgetId)
{
    for (const auto &shortcut : g_fakeShortcuts) {
        if (shortcut->currentStatus().getWidgetId() == widgetId) {
            return QPair<bool, QString>(true, shortcut->currentStatus().getName());
        }
    }
    return QPair<bool, QString>(false, QString());
}
}

class editShortcutModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void test_addModel_filtersAddedShortcuts();
    void test_notAddModel_filtersNotAddedShortcuts();
    void test_updateItemRect_assignsRectsAndRows();
    void test_updateItemRect_wrapsRowsWhenColumnSpanExceedsColumns();
    void test_findTargetItem_hitsLeftHalf();
    void test_findTargetItem_hitsRightHalf();
    void test_findTargetItem_returnsMinusOneWhenOutside();
    void test_findTargetItem_rightToLeftCoordinates();
    void test_moveItem_updatesAddedOrder();
    void test_setOrder_movesItemAcrossStates();
    void test_changeState_movesItemToBeginningWhenRequested();

private:
    ShortcutModel *m_sourceModel = nullptr;
    EditShortcutModel *m_addModel = nullptr;
    EditShortcutModel *m_notAddModel = nullptr;
};

void editShortcutModel_test::initTestCase()
{
    const auto addMeta = QMap<PluginMetaType::SystemMode, PluginMetaData>{
        {PluginMetaType::PC, createMetaData(true, 0, PluginMetaType::Icon)},
        {PluginMetaType::Tablet, createMetaData(true, 1, PluginMetaType::MenuButton)}
    };
    const auto notAddMeta = QMap<PluginMetaType::SystemMode, PluginMetaData>{
        {PluginMetaType::PC, createMetaData(true, 3, PluginMetaType::Icon)},
        {PluginMetaType::Tablet, createMetaData(true, 3, PluginMetaType::MenuButton)}
    };

    g_fakeShortcuts = {
        new FakeShortcut(QStringLiteral("WiFi"),
                         addMeta,
                         createStatusInfo(QStringLiteral("Add1"),
                                          QStringLiteral("On"),
                                          QStringLiteral("add-widget-1"),
                                          QStringLiteral("add-icon-1"),
                                          QStringLiteral("add-tooltip-1"),
                                          Color::Warning,
                                          10,
                                          20)),
        new FakeShortcut(QStringLiteral("BluetoothShortcut"),
                         {
                             {PluginMetaType::PC, createMetaData(true, 1, PluginMetaType::Icon)},
                             {PluginMetaType::Tablet, createMetaData(true, 2, PluginMetaType::MenuButton)}
                         },
                         createStatusInfo(QStringLiteral("Add2"),
                                          QStringLiteral("On"),
                                          QStringLiteral("add-widget-2"),
                                          QStringLiteral("add-icon-2"),
                                          QStringLiteral("add-tooltip-2"),
                                          Color::HighLight,
                                          5,
                                          10)),
        new FakeShortcut(QStringLiteral("ClipboardShortcut"),
                         notAddMeta,
                         createStatusInfo(QStringLiteral("NotAdd"),
                                          QStringLiteral("Off"),
                                          QStringLiteral("notadd-widget"),
                                          QStringLiteral("notadd-icon"),
                                          QStringLiteral("notadd-tooltip"),
                                          Color::BaseColor,
                                          0,
                                          1))
    };

    setupConfigEnvironment();
    m_sourceModel = new ShortcutModel(this);
    m_addModel = m_sourceModel->getModel(PluginStateType::Add);
    m_notAddModel = m_sourceModel->getModel(PluginStateType::NotAdd);

    QVERIFY(m_sourceModel != nullptr);
    QVERIFY(m_addModel != nullptr);
    QVERIFY(m_notAddModel != nullptr);
}

void editShortcutModel_test::test_addModel_filtersAddedShortcuts()
{
    QVERIFY(m_addModel->rowCount() >= 2);

    for (int row = 0; row < m_addModel->rowCount(); ++row) {
        const QModelIndex index = m_addModel->index(row, 0);
        QCOMPARE(index.data(ShortcutModel::AddState).toInt(), static_cast<int>(PluginStateType::Add));
        QCOMPARE(index.data(ShortcutModel::FixState).toInt(), static_cast<int>(PluginStateType::NotFix));
    }
}

void editShortcutModel_test::test_notAddModel_filtersNotAddedShortcuts()
{
    QVERIFY(m_notAddModel->rowCount() > 0);

    for (int row = 0; row < m_notAddModel->rowCount(); ++row) {
        const QModelIndex index = m_notAddModel->index(row, 0);
        QCOMPARE(index.data(ShortcutModel::AddState).toInt(), static_cast<int>(PluginStateType::NotAdd));
        QCOMPARE(index.data(ShortcutModel::FixState).toInt(), static_cast<int>(PluginStateType::NotFix));
    }
}

void editShortcutModel_test::test_updateItemRect_assignsRectsAndRows()
{
    QQuickItem flowItem;
    flowItem.setWidth(500);

    createChildItem(&flowItem, 1);
    createChildItem(&flowItem, 2);
    createChildItem(&flowItem, 1);
    createChildItem(&flowItem, 0);

    m_addModel->updateItemRect(&flowItem);

    const QList<QQuickItem *> items = flowItem.childItems();
    QCOMPARE(items.at(0)->property("targetRect").toRect(), QRect(0, 0, 80, 72));
    QCOMPARE(items.at(1)->property("targetRect").toRect(), QRect(92, 0, 172, 72));
    QCOMPARE(items.at(2)->property("targetRect").toRect(), QRect(276, 0, 80, 72));
    QVERIFY(!items.at(3)->property("targetRect").isValid());
    QCOMPARE(flowItem.property("rows").toInt(), 1);
}

void editShortcutModel_test::test_updateItemRect_wrapsRowsWhenColumnSpanExceedsColumns()
{
    QQuickItem flowItem;
    flowItem.setWidth(500);

    createChildItem(&flowItem, 3);
    createChildItem(&flowItem, 2);
    createChildItem(&flowItem, 1);

    m_addModel->updateItemRect(&flowItem);

    const QList<QQuickItem *> items = flowItem.childItems();
    QCOMPARE(items.at(0)->property("targetRect").toRect(), QRect(0, 0, 264, 72));
    QCOMPARE(items.at(1)->property("targetRect").toRect(), QRect(0, 84, 172, 72));
    QCOMPARE(items.at(2)->property("targetRect").toRect(), QRect(184, 84, 80, 72));
    QCOMPARE(flowItem.property("rows").toInt(), 2);
}

void editShortcutModel_test::test_findTargetItem_hitsLeftHalf()
{
    QQuickItem flowItem;
    flowItem.setWidth(500);
    createChildItem(&flowItem, 2);
    m_addModel->updateItemRect(&flowItem);

    const QVariantMap result = m_addModel->findTargetItem(20, 20);

    QCOMPARE(result.value("index").toInt(), 0);
    QCOMPARE(result.value("offset").toInt(), 0);
}

void editShortcutModel_test::test_findTargetItem_hitsRightHalf()
{
    QQuickItem flowItem;
    flowItem.setWidth(500);
    createChildItem(&flowItem, 2);
    m_addModel->updateItemRect(&flowItem);

    const QVariantMap result = m_addModel->findTargetItem(150, 20);

    QCOMPARE(result.value("index").toInt(), 0);
    QCOMPARE(result.value("offset").toInt(), 1);
}

void editShortcutModel_test::test_findTargetItem_returnsMinusOneWhenOutside()
{
    QQuickItem flowItem;
    flowItem.setWidth(500);
    createChildItem(&flowItem, 1);
    m_addModel->updateItemRect(&flowItem);

    const QVariantMap result = m_addModel->findTargetItem(300, 200);

    QCOMPARE(result.value("index").toInt(), -1);
    QCOMPARE(result.value("offset").toInt(), 0);
}

void editShortcutModel_test::test_findTargetItem_rightToLeftCoordinates()
{
    QQuickItem flowItem;
    flowItem.setWidth(400);
    createChildItem(&flowItem, 1);
    m_addModel->updateItemRect(&flowItem);

    qGuiApp->setLayoutDirection(Qt::RightToLeft);
    const QVariantMap result = m_addModel->findTargetItem(390, 20);
    QCOMPARE(result.value("index").toInt(), 0);
    QCOMPARE(result.value("offset").toInt(), -1);
    qGuiApp->setLayoutDirection(Qt::LeftToRight);
}

void editShortcutModel_test::test_moveItem_updatesAddedOrder()
{
    QVERIFY(m_addModel->rowCount() >= 2);

    const QString firstId = m_addModel->index(0, 0).data(ShortcutModel::PluginId).toString();
    const QString secondId = m_addModel->index(1, 0).data(ShortcutModel::PluginId).toString();
    const int targetPluginIndex = m_addModel->index(1, 0).data(ShortcutModel::PluginIndex).toInt();

    m_addModel->moveItem(0, 1);

    QCOMPARE(ShortcutConfig::instance().getShortcutIndex(firstId, PluginStateType::Add), targetPluginIndex);
    QVERIFY(ShortcutConfig::instance().getShortcutIndex(secondId, PluginStateType::Add) >= 0);
}

void editShortcutModel_test::test_setOrder_movesItemAcrossStates()
{
    QVERIFY(m_notAddModel->rowCount() > 0);

    const QString pluginId = m_notAddModel->index(0, 0).data(ShortcutModel::PluginId).toString();
    const int targetPluginIndex = m_addModel->index(0, 0).data(ShortcutModel::PluginIndex).toInt();

    m_addModel->setOrder(pluginId, 0);

    QCOMPARE(static_cast<int>(ShortcutConfig::instance().getShortcutState(pluginId)), static_cast<int>(PluginStateType::Add));
    QCOMPARE(ShortcutConfig::instance().getShortcutIndex(pluginId, PluginStateType::Add), targetPluginIndex);
}

void editShortcutModel_test::test_changeState_movesItemToBeginningWhenRequested()
{
    QVERIFY(m_addModel->rowCount() > 0);

    const QString pluginId = m_addModel->index(0, 0).data(ShortcutModel::PluginId).toString();

    m_addModel->changeState(pluginId, PluginStateType::Add, false);

    QCOMPARE(static_cast<int>(ShortcutConfig::instance().getShortcutState(pluginId)), static_cast<int>(PluginStateType::NotAdd));
    QCOMPARE(ShortcutConfig::instance().getShortcutIndex(pluginId, PluginStateType::NotAdd), 0);
}

int main(int argc, char *argv[])
{
    setupConfigEnvironment();
    QGuiApplication app(argc, argv);

    editShortcutModel_test test;
    return QTest::qExec(&test, argc, argv);
}

#include "edit-shortcut-model-test.moc"
