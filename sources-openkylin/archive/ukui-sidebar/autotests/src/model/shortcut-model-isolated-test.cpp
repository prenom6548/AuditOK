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

#include "../../../src/model/shortcut-model.h"
#include "../../../src/model/shortcut-filter-model.h"
#include "../../../src/model/edit-shortcut-model.h"
#include "../../../src/model/shortcut-config.h"
#include "../../../src/main/global-settings.h"
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

class shortcutModelIsolated_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void test_rowCount_matchesFakeShortcutCount();
    void test_data_returnsAllSupportedRoles();
    void test_data_invalidIndex_returnsEmptyVariant();
    void test_roleNames_containsPluginId_and_omitsFixState();
    void test_insertShortcut_ignoresNullShortcut();
    void test_active_invalidIndex_ignored();
    void test_active_validIndex_forwardsActionAndPredefinedSignal();
    void test_setValue_invalidIndex_ignored();
    void test_setValue_validIndex_forwardsToShortcut();
    void test_updateValue_invalidIndex_ignored();
    void test_updateValue_validIndex_emitsValueRole();
    void test_disableValueUpdate_filtersValueOnlyChange();
    void test_disableValueUpdate_keepsNonValueRoles();
    void test_updateCurrentMode_tabletChange_updatesColumnTypeAndVisibility();
    void test_updateCurrentMode_nonTabletKey_ignored();
    void test_editModel_filtersByAddState();
    void test_editModel_moveItem_updatesOrder();
    void test_editModel_setOrder_movesNotAddedToAdded();
    void test_editModel_changeState_movesItemBackToNotAdded();
    void test_editModel_layoutAndHitDetection();
    void test_progressBarModel_onlyContainsFixedItems();

private:
    ShortcutModel *m_model = nullptr;
    FakeShortcut *m_progressShortcut = nullptr;
    FakeShortcut *m_addShortcut = nullptr;
    FakeShortcut *m_notAddShortcut = nullptr;

    int rowOfPluginId(const QString &pluginId) const;
};

void shortcutModelIsolated_test::initTestCase()
{
    m_progressShortcut = new FakeShortcut(QStringLiteral("isolated-progress"),
                                          {
                                              {PluginMetaType::PC, createMetaData(true, 2, PluginMetaType::ProgressBar)},
                                              {PluginMetaType::Tablet, createMetaData(false, 4, PluginMetaType::MenuButton)}
                                          },
                                          createStatusInfo(QStringLiteral("Progress"),
                                                           QStringLiteral("50%"),
                                                           QStringLiteral("progress-widget"),
                                                           QStringLiteral("progress-icon"),
                                                           QStringLiteral("progress-tooltip"),
                                                           Color::HighLight,
                                                           50,
                                                           100));

    m_addShortcut = new FakeShortcut(QStringLiteral("isolated-add"),
                                     {
                                         {PluginMetaType::PC, createMetaData(true, 0, PluginMetaType::Icon, PluginMetaType::Hide)},
                                         {PluginMetaType::Tablet, createMetaData(true, 1, PluginMetaType::MenuButton, PluginMetaType::Hide)}
                                     },
                                     createStatusInfo(QStringLiteral("AddName"),
                                                      QStringLiteral("On"),
                                                      QStringLiteral("add-widget"),
                                                      QStringLiteral("add-icon"),
                                                      QStringLiteral("add-tooltip"),
                                                      Color::Warning,
                                                      10,
                                                      20,
                                                      false,
                                                      {
                                                          {QStringLiteral("menu-icon"), QStringLiteral("menu-text"), QStringLiteral("menu-status")}
                                                      }));

    m_notAddShortcut = new FakeShortcut(QStringLiteral("isolated-notadd"),
                                        {
                                            {PluginMetaType::PC, createMetaData(true, 0, PluginMetaType::Icon)},
                                            {PluginMetaType::Tablet, createMetaData(false, 2, PluginMetaType::Icon)}
                                        },
                                        createStatusInfo(QStringLiteral("NotAddName"),
                                                         QStringLiteral("Off"),
                                                         QStringLiteral("notadd-widget"),
                                                         QStringLiteral("notadd-icon"),
                                                         QStringLiteral("notadd-tooltip"),
                                                         Color::BaseColor,
                                                         1,
                                                         9));

    g_fakeShortcuts = {m_progressShortcut, m_addShortcut, m_notAddShortcut};

    ShortcutConfig::instance().recordShortcut(QStringLiteral("isolated-add"));
    ShortcutConfig::instance().recordShortcut(QStringLiteral("isolated-notadd"));
    ShortcutConfig::instance().setShortcutOrder(QStringLiteral("isolated-add"), 0, PluginStateType::Add);

    m_model = new ShortcutModel(this);
    QVERIFY(m_model != nullptr);
}

int shortcutModelIsolated_test::rowOfPluginId(const QString &pluginId) const
{
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        if (m_model->index(row, 0).data(ShortcutModel::PluginId).toString() == pluginId) {
            return row;
        }
    }
    return -1;
}

void shortcutModelIsolated_test::test_rowCount_matchesFakeShortcutCount()
{
    QCOMPARE(m_model->rowCount(QModelIndex()), 3);
}

void shortcutModelIsolated_test::test_data_returnsAllSupportedRoles()
{
    const QModelIndex addIndex = m_model->index(rowOfPluginId(QStringLiteral("isolated-add")), 0);
    QVERIFY(addIndex.isValid());

    QCOMPARE(addIndex.data(ShortcutModel::Disabled).toBool(), false);
    QCOMPARE(addIndex.data(ShortcutModel::Name).toString(), QStringLiteral("AddName"));
    QCOMPARE(addIndex.data(ShortcutModel::StatusName).toString(), QStringLiteral("On"));
    QCOMPARE(addIndex.data(ShortcutModel::WidgetId).toString(), QStringLiteral("add-widget"));
    QCOMPARE(addIndex.data(ShortcutModel::Icon).toString(), QStringLiteral("add-icon"));
    QCOMPARE(addIndex.data(ShortcutModel::ToolTip).toString(), QStringLiteral("add-tooltip"));
    QCOMPARE(addIndex.data(ShortcutModel::Color).value<Color::ColorRole>(), Color::Warning);
    QCOMPARE(addIndex.data(ShortcutModel::Value).toInt(), 10);
    QCOMPARE(addIndex.data(ShortcutModel::MaxValue).toInt(), 20);
    QVERIFY(addIndex.data(ShortcutModel::Menu).isValid());
    QCOMPARE(addIndex.data(ShortcutModel::Row).toInt(), 1);
    QCOMPARE(addIndex.data(ShortcutModel::Column).toInt(), 1);
    QCOMPARE(addIndex.data(ShortcutModel::ItemType).toInt(), static_cast<int>(PluginMetaType::Icon));
    QCOMPARE(addIndex.data(ShortcutModel::IsEnable).toBool(), true);
    QCOMPARE(addIndex.data(ShortcutModel::IsVisible).toBool(), true);
    QCOMPARE(addIndex.data(ShortcutModel::AddState).toInt(), static_cast<int>(PluginStateType::Add));
    QCOMPARE(addIndex.data(ShortcutModel::PluginIndex).toInt(), 0);
    QCOMPARE(addIndex.data(ShortcutModel::PluginId).toString(), QStringLiteral("isolated-add"));
    QCOMPARE(addIndex.data(ShortcutModel::FixState).toInt(), static_cast<int>(PluginStateType::NotFix));

    const QModelIndex progressIndex = m_model->index(rowOfPluginId(QStringLiteral("isolated-progress")), 0);
    QCOMPARE(progressIndex.data(ShortcutModel::FixState).toInt(), static_cast<int>(PluginStateType::Fix));
}

void shortcutModelIsolated_test::test_data_invalidIndex_returnsEmptyVariant()
{
    QVERIFY(!m_model->data(m_model->index(-1, 0), ShortcutModel::Name).isValid());
    QVERIFY(!m_model->data(m_model->index(99, 0), ShortcutModel::Name).isValid());
    QVERIFY(!m_model->data(m_model->index(0, 0), 9999).isValid());
}

void shortcutModelIsolated_test::test_roleNames_containsPluginId_and_omitsFixState()
{
    const QHash<int, QByteArray> names = m_model->roleNames();
    QCOMPARE(names.value(ShortcutModel::PluginId), QByteArray("pluginId"));
    QVERIFY(!names.contains(ShortcutModel::FixState));
}

void shortcutModelIsolated_test::test_insertShortcut_ignoresNullShortcut()
{
    const int before = m_model->rowCount(QModelIndex());
    m_model->insertShortcut(nullptr);
    QCOMPARE(m_model->rowCount(QModelIndex()), before);
}

void shortcutModelIsolated_test::test_active_invalidIndex_ignored()
{
    m_model->active(-1, PluginMetaType::Click);
    m_model->active(99, PluginMetaType::Click);
    QCOMPARE(m_addShortcut->m_activeCount, 0);
}

void shortcutModelIsolated_test::test_active_validIndex_forwardsActionAndPredefinedSignal()
{
    int predefinedAction = static_cast<int>(PluginMetaType::NoAction);
    int signalCount = 0;
    connect(m_model, &ShortcutModel::requestExecAction, this, [&predefinedAction, &signalCount] (PluginMetaType::PredefinedAction action) {
        predefinedAction = static_cast<int>(action);
        ++signalCount;
    });
    const int row = rowOfPluginId(QStringLiteral("isolated-add"));
    QVERIFY(row >= 0);

    m_model->active(row, PluginMetaType::LongClick);

    QCOMPARE(m_addShortcut->m_activeCount, 1);
    QCOMPARE(static_cast<int>(m_addShortcut->m_lastAction), static_cast<int>(PluginMetaType::LongClick));
    QCOMPARE(signalCount, 1);
    QCOMPARE(predefinedAction, static_cast<int>(PluginMetaType::Hide));
}

void shortcutModelIsolated_test::test_setValue_invalidIndex_ignored()
{
    m_model->setValue(-1, 12);
    m_model->setValue(99, 12);
    QCOMPARE(m_progressShortcut->m_lastSetValue, -1);
}

void shortcutModelIsolated_test::test_setValue_validIndex_forwardsToShortcut()
{
    const int row = rowOfPluginId(QStringLiteral("isolated-progress"));
    QVERIFY(row >= 0);

    m_model->setValue(row, 77);

    QCOMPARE(m_progressShortcut->m_lastSetValue, 77);
}

void shortcutModelIsolated_test::test_updateValue_invalidIndex_ignored()
{
    QSignalSpy spy(m_model, &ShortcutModel::dataChanged);

    m_model->updateValue(-1);
    m_model->updateValue(99);

    QCOMPARE(spy.count(), 0);
}

void shortcutModelIsolated_test::test_updateValue_validIndex_emitsValueRole()
{
    const int row = rowOfPluginId(QStringLiteral("isolated-progress"));
    QVERIFY(row >= 0);
    QSignalSpy spy(m_model, &ShortcutModel::dataChanged);

    m_model->updateValue(row);

    QCOMPARE(spy.count(), 1);
    const QVector<int> roles = qvariant_cast<QVector<int>>(spy.at(0).at(2));
    QCOMPARE(roles.size(), 1);
    QCOMPARE(roles.constFirst(), static_cast<int>(ShortcutModel::Value));
}

void shortcutModelIsolated_test::test_disableValueUpdate_filtersValueOnlyChange()
{
    const int row = rowOfPluginId(QStringLiteral("isolated-add"));
    QVERIFY(row >= 0);
    QSignalSpy spy(m_model, &ShortcutModel::dataChanged);

    m_model->disableValueUpdate(true);
    m_addShortcut->emitStatusChanged(createStatusInfo(QStringLiteral("AddName"),
                                                      QStringLiteral("On"),
                                                      QStringLiteral("add-widget"),
                                                      QStringLiteral("add-icon"),
                                                      QStringLiteral("add-tooltip"),
                                                      Color::Warning,
                                                      15,
                                                      20));

    QCOMPARE(spy.count(), 0);
    QCOMPARE(m_model->index(row, 0).data(ShortcutModel::Value).toInt(), 15);
}

void shortcutModelIsolated_test::test_disableValueUpdate_keepsNonValueRoles()
{
    const int row = rowOfPluginId(QStringLiteral("isolated-add"));
    QVERIFY(row >= 0);
    QSignalSpy spy(m_model, &ShortcutModel::dataChanged);

    m_model->disableValueUpdate(true);
    m_addShortcut->emitStatusChanged(createStatusInfo(QStringLiteral("AddNameUpdated"),
                                                      QStringLiteral("On"),
                                                      QStringLiteral("add-widget"),
                                                      QStringLiteral("add-icon"),
                                                      QStringLiteral("add-tooltip"),
                                                      Color::Warning,
                                                      18,
                                                      20));

    QVERIFY(spy.count() > 0);
    const QVector<int> roles = qvariant_cast<QVector<int>>(spy.at(0).at(2));
    QVERIFY(roles.contains(ShortcutModel::Name));
    QVERIFY(!roles.contains(ShortcutModel::Value));

    m_model->disableValueUpdate(false);
}

void shortcutModelIsolated_test::test_updateCurrentMode_tabletChange_updatesColumnTypeAndVisibility()
{
    const int addRow = rowOfPluginId(QStringLiteral("isolated-add"));
    const int notAddRow = rowOfPluginId(QStringLiteral("isolated-notadd"));
    QVERIFY(addRow >= 0);
    QVERIFY(notAddRow >= 0);

    QSignalSpy spy(m_model, &ShortcutModel::dataChanged);
    QVERIFY(QMetaObject::invokeMethod(Sidebar::GlobalSettings::globalInstance(),
                                      "updateTabletStatus",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));

    QVERIFY(spy.count() > 0);
    QCOMPARE(m_model->index(addRow, 0).data(ShortcutModel::Column).toInt(), 2);
    QCOMPARE(m_model->index(addRow, 0).data(ShortcutModel::ItemType).toInt(), static_cast<int>(PluginMetaType::MenuButton));
    QCOMPARE(m_model->index(notAddRow, 0).data(ShortcutModel::IsVisible).toBool(), false);

    QVERIFY(QMetaObject::invokeMethod(Sidebar::GlobalSettings::globalInstance(),
                                      "updateTabletStatus",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
}

void shortcutModelIsolated_test::test_updateCurrentMode_nonTabletKey_ignored()
{
    QSignalSpy spy(m_model, &ShortcutModel::dataChanged);

    QVERIFY(QMetaObject::invokeMethod(m_model,
                                      "updateCurrentMode",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QStringLiteral("unrelated-key"))));

    QCOMPARE(spy.count(), 0);
}

void shortcutModelIsolated_test::test_editModel_filtersByAddState()
{
    EditShortcutModel *addModel = m_model->getModel(PluginStateType::Add);
    EditShortcutModel *notAddModel = m_model->getModel(PluginStateType::NotAdd);

    QVERIFY(addModel != nullptr);
    QVERIFY(notAddModel != nullptr);
    QCOMPARE(addModel->rowCount(), 1);
    QCOMPARE(notAddModel->rowCount(), 1);
    QCOMPARE(addModel->index(0, 0).data(ShortcutModel::PluginId).toString(), QStringLiteral("isolated-add"));
    QCOMPARE(notAddModel->index(0, 0).data(ShortcutModel::PluginId).toString(), QStringLiteral("isolated-notadd"));
}

void shortcutModelIsolated_test::test_editModel_moveItem_updatesOrder()
{
    EditShortcutModel *addModel = m_model->getModel(PluginStateType::Add);
    const QString movedId = QStringLiteral("isolated-notadd");

    addModel->setOrder(movedId, 0);
    QCOMPARE(ShortcutConfig::instance().getShortcutState(movedId), PluginStateType::Add);

    QVERIFY(m_addShortcut->m_addedState);
    QCOMPARE(addModel->rowCount(), 2);
    const int beforeIndex = ShortcutConfig::instance().getShortcutIndex(QStringLiteral("isolated-add"), PluginStateType::Add);
    addModel->moveItem(0, 1);
    QVERIFY(ShortcutConfig::instance().getShortcutIndex(QStringLiteral("isolated-add"), PluginStateType::Add) != beforeIndex);
}

void shortcutModelIsolated_test::test_editModel_setOrder_movesNotAddedToAdded()
{
    EditShortcutModel *notAddModel = m_model->getModel(PluginStateType::NotAdd);
    EditShortcutModel *addModel = m_model->getModel(PluginStateType::Add);

    addModel->changeState(QStringLiteral("isolated-notadd"), PluginStateType::Add, true);
    QCOMPARE(ShortcutConfig::instance().getShortcutState(QStringLiteral("isolated-notadd")), PluginStateType::NotAdd);
    QCOMPARE(notAddModel->rowCount(), 1);

    addModel->setOrder(QStringLiteral("isolated-notadd"), 0);
    QCOMPARE(ShortcutConfig::instance().getShortcutState(QStringLiteral("isolated-notadd")), PluginStateType::Add);
    QCOMPARE(addModel->rowCount(), 2);
}

void shortcutModelIsolated_test::test_editModel_changeState_movesItemBackToNotAdded()
{
    EditShortcutModel *addModel = m_model->getModel(PluginStateType::Add);
    EditShortcutModel *notAddModel = m_model->getModel(PluginStateType::NotAdd);

    addModel->changeState(QStringLiteral("isolated-notadd"), PluginStateType::Add, false);

    QCOMPARE(ShortcutConfig::instance().getShortcutState(QStringLiteral("isolated-notadd")), PluginStateType::NotAdd);
    QCOMPARE(ShortcutConfig::instance().getShortcutIndex(QStringLiteral("isolated-notadd"), PluginStateType::NotAdd), 0);
    QVERIFY(notAddModel->rowCount() >= 1);
}

void shortcutModelIsolated_test::test_editModel_layoutAndHitDetection()
{
    EditShortcutModel *addModel = m_model->getModel(PluginStateType::Add);
    QQuickItem flowItem;
    flowItem.setWidth(400);

    createChildItem(&flowItem, 3);
    createChildItem(&flowItem, 2);
    createChildItem(&flowItem, 0);

    addModel->updateItemRect(&flowItem);
    QCOMPARE(flowItem.property("rows").toInt(), 2);

    QVariantMap result = addModel->findTargetItem(10, 10);
    QCOMPARE(result.value("index").toInt(), 0);
    QCOMPARE(result.value("offset").toInt(), 0);

    qGuiApp->setLayoutDirection(Qt::RightToLeft);
    QCoreApplication::processEvents();
    result = addModel->findTargetItem(390, 10);
    QCOMPARE(result.value("index").toInt(), 0);
    qGuiApp->setLayoutDirection(Qt::LeftToRight);
}

void shortcutModelIsolated_test::test_progressBarModel_onlyContainsFixedItems()
{
    ShortcutFilterModel *progressModel = m_model->prosessBarModel();
    QVERIFY(progressModel != nullptr);
    QCOMPARE(progressModel->rowCount(), 1);
    QCOMPARE(progressModel->index(0, 0).data(ShortcutModel::PluginId).toString(), QStringLiteral("isolated-progress"));
}

int main(int argc, char *argv[])
{
    setupConfigEnvironment();
    QGuiApplication app(argc, argv);

    shortcutModelIsolated_test test;
    return QTest::qExec(&test, argc, argv);
}

#include "shortcut-model-isolated-test.moc"
