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
#include <QScopedPointer>
#include <QTemporaryDir>

#include "../../../src/model/shortcut-filter-model.h"
#include "../../../src/model/shortcut-model.h"
#include "../../../src/model/shortcut-config.h"
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
                            int value = 0,
                            int maxValue = 100,
                            bool disabled = false)
{
    StatusInfo info;
    info.setName(name);
    info.setStatusName(statusName);
    info.setWidgetId(name + QStringLiteral("-widget"));
    info.setIcon(name + QStringLiteral("-icon"));
    info.setToolTip(name + QStringLiteral("-tooltip"));
    info.setColor(Color::BaseColor);
    info.setValue(value);
    info.setMaxValue(maxValue);
    info.setDisable(disabled);
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

    QString m_pluginId;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    StatusInfo m_statusInfo;
    bool m_enabled = true;
    bool m_addedState = false;
    int m_activeCount = 0;
    int m_lastSetValue = -1;
    PluginMetaType::Action m_lastAction = PluginMetaType::Click;
};
}

namespace UkuiShortcut {
ShortcutManager *g_shortcutManagerInstance = nullptr;

class ShortcutManagerPrivate
{
public:
    QList<Shortcut*> m_shortcuts;
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
    return d->m_shortcuts;
}

QPair<bool, QString> ShortcutManager::getWidgetInfo(const QString &widgetId)
{
    for (const auto &shortcut : d->m_shortcuts) {
        if (shortcut->currentStatus().getWidgetId() == widgetId) {
            return QPair<bool, QString>(true, shortcut->currentStatus().getName());
        }
    }
    return QPair<bool, QString>(false, QString());
}
}

class shortcutFilterModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void test_fixFilter_acceptsOnlyVisibleEnabledProgressBarItems();
    void test_data_returnsSourceModelData();
    void test_active_forwardsMappedRowToSourceModel();
    void test_setValue_forwardsMappedRowToSourceModel();
    void test_disableValueUpdate_blocksValueSignalUntilReenabled();
    void test_updateValue_validRow_emitsValueRole();
    void test_updateValue_invalidRow_emitsNothing();
    void test_pluginIndexChange_resortsProxyRows();

private:
    ShortcutModel *m_sourceModel = nullptr;
    ShortcutFilterModel *m_fixModel = nullptr;
    ShortcutFilterModel *m_notFixModel = nullptr;

    FakeShortcut *m_fixVisibleLow = nullptr;
    FakeShortcut *m_fixVisibleHigh = nullptr;
    FakeShortcut *m_fixHidden = nullptr;
    FakeShortcut *m_fixDisabled = nullptr;
    FakeShortcut *m_notFixAlpha = nullptr;
    FakeShortcut *m_notFixBeta = nullptr;

    QStringList proxyIds(ShortcutFilterModel *model) const;
    int rowOfPluginId(ShortcutFilterModel *model, const QString &pluginId) const;
};

void shortcutFilterModel_test::initTestCase()
{
    m_sourceModel = new ShortcutModel(this);
    QVERIFY(m_sourceModel != nullptr);

    m_fixVisibleLow = new FakeShortcut(QStringLiteral("filter-fix-visible-low"),
                                       {
                                           {PluginMetaType::PC, createMetaData(true, -100, PluginMetaType::ProgressBar)}
                                       },
                                       createStatusInfo(QStringLiteral("fix-low"), QStringLiteral("ready"), 10, 100),
                                       true,
                                       m_sourceModel);
    m_fixVisibleHigh = new FakeShortcut(QStringLiteral("filter-fix-visible-high"),
                                        {
                                            {PluginMetaType::PC, createMetaData(true, -50, PluginMetaType::ProgressBar)}
                                        },
                                        createStatusInfo(QStringLiteral("fix-high"), QStringLiteral("ready"), 20, 100),
                                        true,
                                        m_sourceModel);
    m_fixHidden = new FakeShortcut(QStringLiteral("filter-fix-hidden"),
                                   {
                                       {PluginMetaType::PC, createMetaData(false, -200, PluginMetaType::ProgressBar)}
                                   },
                                   createStatusInfo(QStringLiteral("fix-hidden"), QStringLiteral("hidden"), 30, 100),
                                   true,
                                   m_sourceModel);
    m_fixDisabled = new FakeShortcut(QStringLiteral("filter-fix-disabled"),
                                     {
                                         {PluginMetaType::PC, createMetaData(true, -150, PluginMetaType::ProgressBar)}
                                     },
                                     createStatusInfo(QStringLiteral("fix-disabled"), QStringLiteral("disabled"), 40, 100),
                                     false,
                                     m_sourceModel);
    m_notFixAlpha = new FakeShortcut(QStringLiteral("filter-notfix-alpha"),
                                     {
                                         {PluginMetaType::PC, createMetaData(true, 0, PluginMetaType::Icon)}
                                     },
                                     createStatusInfo(QStringLiteral("alpha"), QStringLiteral("idle"), 1, 10),
                                     true,
                                     m_sourceModel);
    m_notFixBeta = new FakeShortcut(QStringLiteral("filter-notfix-beta"),
                                    {
                                        {PluginMetaType::PC, createMetaData(true, 0, PluginMetaType::Icon)}
                                    },
                                    createStatusInfo(QStringLiteral("beta"), QStringLiteral("idle"), 2, 10),
                                    true,
                                    m_sourceModel);

    m_sourceModel->insertShortcut(m_fixVisibleLow);
    m_sourceModel->insertShortcut(m_fixVisibleHigh);
    m_sourceModel->insertShortcut(m_fixHidden);
    m_sourceModel->insertShortcut(m_fixDisabled);
    m_sourceModel->insertShortcut(m_notFixAlpha);
    m_sourceModel->insertShortcut(m_notFixBeta);

    m_fixModel = new ShortcutFilterModel(m_sourceModel, PluginStateType::Fix, this);
    m_notFixModel = new ShortcutFilterModel(m_sourceModel, PluginStateType::NotFix, this);

    QVERIFY(m_fixModel != nullptr);
    QVERIFY(m_notFixModel != nullptr);
}

QStringList shortcutFilterModel_test::proxyIds(ShortcutFilterModel *model) const
{
    QStringList ids;
    for (int row = 0; row < model->rowCount(); ++row) {
        ids.append(model->index(row, 0).data(ShortcutModel::PluginId).toString());
    }
    return ids;
}

int shortcutFilterModel_test::rowOfPluginId(ShortcutFilterModel *model, const QString &pluginId) const
{
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->index(row, 0).data(ShortcutModel::PluginId).toString() == pluginId) {
            return row;
        }
    }
    return -1;
}

void shortcutFilterModel_test::test_fixFilter_acceptsOnlyVisibleEnabledProgressBarItems()
{
    const QStringList ids = proxyIds(m_fixModel);

    QVERIFY(ids.contains(QStringLiteral("filter-fix-visible-low")));
    QVERIFY(ids.contains(QStringLiteral("filter-fix-visible-high")));
    QVERIFY(!ids.contains(QStringLiteral("filter-fix-hidden")));
    QVERIFY(!ids.contains(QStringLiteral("filter-fix-disabled")));
    QVERIFY(!ids.contains(QStringLiteral("filter-notfix-alpha")));
    QVERIFY(!ids.contains(QStringLiteral("filter-notfix-beta")));

    QCOMPARE(ids.indexOf(QStringLiteral("filter-fix-visible-low")), 0);
    QCOMPARE(ids.indexOf(QStringLiteral("filter-fix-visible-high")), 1);
}

void shortcutFilterModel_test::test_data_returnsSourceModelData()
{
    const int row = rowOfPluginId(m_fixModel, QStringLiteral("filter-fix-visible-low"));
    QVERIFY(row >= 0);

    const QModelIndex index = m_fixModel->index(row, 0);
    QCOMPARE(m_fixModel->data(index, ShortcutModel::Name).toString(), QStringLiteral("fix-low"));
    QCOMPARE(m_fixModel->data(index, ShortcutModel::StatusName).toString(), QStringLiteral("ready"));
    QCOMPARE(m_fixModel->data(index, ShortcutModel::PluginId).toString(), QStringLiteral("filter-fix-visible-low"));
    QCOMPARE(m_fixModel->data(index, ShortcutModel::Value).toInt(), 10);
}

void shortcutFilterModel_test::test_active_forwardsMappedRowToSourceModel()
{
    const int row = rowOfPluginId(m_fixModel, QStringLiteral("filter-fix-visible-high"));
    QVERIFY(row >= 0);

    m_fixModel->active(row, PluginMetaType::LongClick);

    QCOMPARE(m_fixVisibleHigh->m_activeCount, 1);
    QCOMPARE(static_cast<int>(m_fixVisibleHigh->m_lastAction), static_cast<int>(PluginMetaType::LongClick));
}

void shortcutFilterModel_test::test_setValue_forwardsMappedRowToSourceModel()
{
    const int row = rowOfPluginId(m_fixModel, QStringLiteral("filter-fix-visible-low"));
    QVERIFY(row >= 0);

    m_fixModel->setValue(row, 77);

    QCOMPARE(m_fixVisibleLow->m_lastSetValue, 77);
}

void shortcutFilterModel_test::test_disableValueUpdate_blocksValueSignalUntilReenabled()
{
    const int row = rowOfPluginId(m_fixModel, QStringLiteral("filter-fix-visible-low"));
    QVERIFY(row >= 0);

    QSignalSpy spy(m_fixModel, &ShortcutFilterModel::dataChanged);

    m_fixModel->disableValueUpdate(true);
    m_fixVisibleLow->emitStatusChanged(createStatusInfo(QStringLiteral("fix-low"), QStringLiteral("ready"), 55, 100));
    QCOMPARE(spy.count(), 0);

    m_fixModel->disableValueUpdate(false);
    m_fixVisibleLow->emitStatusChanged(createStatusInfo(QStringLiteral("fix-low"), QStringLiteral("ready"), 66, 100));
    QVERIFY(spy.count() > 0);
    QCOMPARE(m_fixModel->index(row, 0).data(ShortcutModel::Value).toInt(), 66);
}

void shortcutFilterModel_test::test_updateValue_validRow_emitsValueRole()
{
    const int row = rowOfPluginId(m_fixModel, QStringLiteral("filter-fix-visible-low"));
    QVERIFY(row >= 0);

    QSignalSpy spy(m_fixModel, &ShortcutFilterModel::dataChanged);

    m_fixModel->updateValue(row);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> arguments = spy.takeFirst();
    const QVector<int> roles = qvariant_cast<QVector<int>>(arguments.at(2));
    QCOMPARE(roles.size(), 1);
    QCOMPARE(roles.constFirst(), static_cast<int>(ShortcutModel::Value));
}

void shortcutFilterModel_test::test_updateValue_invalidRow_emitsNothing()
{
    QSignalSpy spy(m_fixModel, &ShortcutFilterModel::dataChanged);

    m_fixModel->updateValue(-1);
    m_fixModel->updateValue(m_fixModel->rowCount());

    QCOMPARE(spy.count(), 0);
}

void shortcutFilterModel_test::test_pluginIndexChange_resortsProxyRows()
{
    const QString alphaId = QStringLiteral("filter-notfix-alpha");
    const QString betaId = QStringLiteral("filter-notfix-beta");

    QVERIFY(rowOfPluginId(m_notFixModel, alphaId) >= 0);
    QVERIFY(rowOfPluginId(m_notFixModel, betaId) >= 0);

    ShortcutConfig::instance().setShortcutOrder(alphaId, 0, PluginStateType::Add);
    ShortcutConfig::instance().setShortcutOrder(betaId, 1, PluginStateType::Add);
    QCoreApplication::processEvents();
    m_notFixModel->sort(0);

    QCOMPARE(rowOfPluginId(m_notFixModel, alphaId) < rowOfPluginId(m_notFixModel, betaId), true);

    const int betaIndex = ShortcutConfig::instance().getShortcutIndex(betaId, PluginStateType::Add);
    QVERIFY(betaIndex >= 0);

    ShortcutConfig::instance().setShortcutOrder(betaIndex, 0, PluginStateType::Add);
    QCoreApplication::processEvents();
    m_notFixModel->sort(0);

    const int alphaRow = rowOfPluginId(m_notFixModel, alphaId);
    const int betaRow = rowOfPluginId(m_notFixModel, betaId);

    QVERIFY(betaRow >= 0);
    QVERIFY(alphaRow >= 0);
    QVERIFY(m_notFixModel->index(betaRow, 0).data(ShortcutModel::PluginIndex).toInt()
            <= m_notFixModel->index(alphaRow, 0).data(ShortcutModel::PluginIndex).toInt());
}

int main(int argc, char *argv[])
{
    setupConfigEnvironment();
    QGuiApplication app(argc, argv);

    shortcutFilterModel_test test;
    return QTest::qExec(&test, argc, argv);
}

#include "shortcut-filter-model-test.moc"
