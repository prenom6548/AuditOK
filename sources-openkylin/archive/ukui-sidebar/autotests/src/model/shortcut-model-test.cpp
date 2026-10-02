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
#include <QSignalSpy>
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

class shortcutModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void test_role_disabled_isZero();
    void test_role_name_isOne();
    void test_role_statusName_isTwo();
    void test_role_widgetId_isThree();
    void test_role_icon_isFour();
    void test_role_tooltip_isFive();
    void test_role_color_isSix();
    void test_role_value_isSeven();
    void test_role_maxValue_isEight();
    void test_role_menu_isNine();
    void test_role_row_isTen();
    void test_role_column_isEleven();
    void test_role_itemType_isTwelve();
    void test_role_isEnable_isThirteen();
    void test_role_isVisible_isFourteen();
    void test_role_addState_isFifteen();
    void test_role_pluginIndex_isSixteen();
    void test_role_pluginId_isSeventeen();
    void test_role_fixState_isEighteen();

    void test_construction_notNull();
    void test_rowCount_validIndex();
    void test_rowCount_invalidIndex();
    void test_roleNames_notEmpty();
    void test_roleNames_containsDisabled();
    void test_roleNames_containsName();
    void test_roleNames_containsIcon();
    void test_disableValueUpdate_true();
    void test_disableValueUpdate_false();
    void test_disableValueUpdate_toggle();
    void test_getModel_addReturnsNonNull();
    void test_getModel_notAddReturnsNonNull();
    void test_prosessBarModel_notNull();
    void test_editIconName_notEmpty();

private:
    ShortcutModel *m_model = nullptr;
    FakeShortcut *m_progressShortcut = nullptr;
    FakeShortcut *m_addShortcut = nullptr;
    FakeShortcut *m_notAddShortcut = nullptr;
};

void shortcutModel_test::initTestCase()
{
    m_progressShortcut = new FakeShortcut(QStringLiteral("model-progress"),
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

    m_addShortcut = new FakeShortcut(QStringLiteral("model-add"),
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
                                                      20));

    m_notAddShortcut = new FakeShortcut(QStringLiteral("model-notadd"),
                                        {
                                            {PluginMetaType::PC, createMetaData(true, 3, PluginMetaType::Icon)},
                                            {PluginMetaType::Tablet, createMetaData(true, 3, PluginMetaType::MenuButton)}
                                        },
                                        createStatusInfo(QStringLiteral("NotAddName"),
                                                         QStringLiteral("Off"),
                                                         QStringLiteral("notadd-widget"),
                                                         QStringLiteral("notadd-icon"),
                                                         QStringLiteral("notadd-tooltip"),
                                                         Color::BaseColor,
                                                         0,
                                                         1));

    g_fakeShortcuts = {m_progressShortcut, m_addShortcut, m_notAddShortcut};

    setupConfigEnvironment();
    m_model = new ShortcutModel(this);
}

void shortcutModel_test::test_role_disabled_isZero()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Disabled), 0);
}

void shortcutModel_test::test_role_name_isOne()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Name), 1);
}

void shortcutModel_test::test_role_statusName_isTwo()
{
    QCOMPARE(static_cast<int>(ShortcutModel::StatusName), 2);
}

void shortcutModel_test::test_role_widgetId_isThree()
{
    QCOMPARE(static_cast<int>(ShortcutModel::WidgetId), 3);
}

void shortcutModel_test::test_role_icon_isFour()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Icon), 4);
}

void shortcutModel_test::test_role_tooltip_isFive()
{
    QCOMPARE(static_cast<int>(ShortcutModel::ToolTip), 5);
}

void shortcutModel_test::test_role_color_isSix()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Color), 6);
}

void shortcutModel_test::test_role_value_isSeven()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Value), 7);
}

void shortcutModel_test::test_role_maxValue_isEight()
{
    QCOMPARE(static_cast<int>(ShortcutModel::MaxValue), 8);
}

void shortcutModel_test::test_role_menu_isNine()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Menu), 9);
}

void shortcutModel_test::test_role_row_isTen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Row), 10);
}

void shortcutModel_test::test_role_column_isEleven()
{
    QCOMPARE(static_cast<int>(ShortcutModel::Column), 11);
}

void shortcutModel_test::test_role_itemType_isTwelve()
{
    QCOMPARE(static_cast<int>(ShortcutModel::ItemType), 12);
}

void shortcutModel_test::test_role_isEnable_isThirteen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::IsEnable), 13);
}

void shortcutModel_test::test_role_isVisible_isFourteen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::IsVisible), 14);
}

void shortcutModel_test::test_role_addState_isFifteen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::AddState), 15);
}

void shortcutModel_test::test_role_pluginIndex_isSixteen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::PluginIndex), 16);
}

void shortcutModel_test::test_role_pluginId_isSeventeen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::PluginId), 17);
}

void shortcutModel_test::test_role_fixState_isEighteen()
{
    QCOMPARE(static_cast<int>(ShortcutModel::FixState), 18);
}

void shortcutModel_test::test_construction_notNull()
{
    QVERIFY(m_model != nullptr);
}

void shortcutModel_test::test_rowCount_validIndex()
{
    QCOMPARE(m_model->rowCount(QModelIndex()), 3);
}

void shortcutModel_test::test_rowCount_invalidIndex()
{
    QModelIndex child = m_model->index(0, 0);
    QCOMPARE(m_model->rowCount(child), 3);
}

void shortcutModel_test::test_roleNames_notEmpty()
{
    QHash<int, QByteArray> names = m_model->roleNames();
    QVERIFY(!names.isEmpty());
}

void shortcutModel_test::test_roleNames_containsDisabled()
{
    QVERIFY(m_model->roleNames().contains(ShortcutModel::Disabled));
}

void shortcutModel_test::test_roleNames_containsName()
{
    QVERIFY(m_model->roleNames().contains(ShortcutModel::Name));
}

void shortcutModel_test::test_roleNames_containsIcon()
{
    QVERIFY(m_model->roleNames().contains(ShortcutModel::Icon));
}

void shortcutModel_test::test_disableValueUpdate_true()
{
    m_model->disableValueUpdate(true);
    QVERIFY(true);
}

void shortcutModel_test::test_disableValueUpdate_false()
{
    m_model->disableValueUpdate(false);
    QVERIFY(true);
}

void shortcutModel_test::test_disableValueUpdate_toggle()
{
    m_model->disableValueUpdate(true);
    m_model->disableValueUpdate(false);
    m_model->disableValueUpdate(true);
    QVERIFY(true);
}

void shortcutModel_test::test_getModel_addReturnsNonNull()
{
    QVERIFY(m_model->getModel(PluginStateType::Add) != nullptr);
}

void shortcutModel_test::test_getModel_notAddReturnsNonNull()
{
    QVERIFY(m_model->getModel(PluginStateType::NotAdd) != nullptr);
}

void shortcutModel_test::test_prosessBarModel_notNull()
{
    QVERIFY(m_model->prosessBarModel() != nullptr);
}

void shortcutModel_test::test_editIconName_notEmpty()
{
    QVERIFY(!m_model->editIconName().isNull());
}

int main(int argc, char *argv[])
{
    setupConfigEnvironment();
    QGuiApplication app(argc, argv);

    shortcutModel_test test;
    return QTest::qExec(&test, argc, argv);
}

#include "shortcut-model-test.moc"
