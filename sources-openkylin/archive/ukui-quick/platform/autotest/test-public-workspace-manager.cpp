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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "workspace-manager.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>
#include <QVariant>

namespace UkuiQuick::Test {

class PublicWorkspaceManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testWorkspaceValueDefaultsAndMetaTypes();
    void testWorkspaceManagerMetaObjectAndSingleton();
    void testWorkspaceManagerRejectsRequestsWhenUnavailable();
};

void PublicWorkspaceManagerTest::initTestCase()
{
    qRegisterMetaType<UkuiQuick::Workspace>("UkuiQuick::Workspace");
    qRegisterMetaType<UkuiQuick::WorkspaceList>("UkuiQuick::WorkspaceList");
    qRegisterMetaType<uint32_t>("uint32_t");
}

void PublicWorkspaceManagerTest::testWorkspaceValueDefaultsAndMetaTypes()
{
    Workspace workspace;
    QVERIFY(workspace.id.isEmpty());
    QVERIFY(workspace.name.isEmpty());
    QCOMPARE(workspace.position, -1);
    QCOMPARE(workspace.active, false);

    const QVariant workspaceVariant = QVariant::fromValue(workspace);
    QVERIFY(workspaceVariant.isValid());
    QCOMPARE(workspaceVariant.value<Workspace>().position, -1);

    WorkspaceList workspaces;
    workspaces.append({QStringLiteral("workspace-1"), QStringLiteral("Workspace 1"), 1, true});

    const QVariant listVariant = QVariant::fromValue(workspaces);
    QVERIFY(listVariant.isValid());
    QCOMPARE(listVariant.value<WorkspaceList>().size(), 1);
    QCOMPARE(listVariant.value<WorkspaceList>().constFirst().id, QStringLiteral("workspace-1"));
}

void PublicWorkspaceManagerTest::testWorkspaceManagerMetaObjectAndSingleton()
{
    auto *manager = WorkspaceManager::self();
    QVERIFY(manager);
    QCOMPARE(WorkspaceManager::self(), manager);

    const int availableProperty = manager->metaObject()->indexOfProperty("available");
    QVERIFY(availableProperty >= 0);

    const QMetaProperty property = manager->metaObject()->property(availableProperty);
    QCOMPARE(property.read(manager).toBool(), manager->isAvailable());
    QVERIFY(property.hasNotifySignal());

    QSignalSpy availableSpy(manager, &WorkspaceManager::availableChanged);
    QSignalSpy initializedSpy(manager, &WorkspaceManager::initialized);
    QSignalSpy workspacesSpy(manager, &WorkspaceManager::workspacesChanged);
    QSignalSpy addedSpy(manager, &WorkspaceManager::workspaceAdded);
    QSignalSpy removedSpy(manager, &WorkspaceManager::workspaceRemoved);
    QSignalSpy nameSpy(manager, &WorkspaceManager::workspaceNameChanged);
    QSignalSpy positionSpy(manager, &WorkspaceManager::workspacePositionChanged);
    QSignalSpy activeSpy(manager, &WorkspaceManager::workspaceActiveChanged);
    QSignalSpy currentSpy(manager, &WorkspaceManager::currentWorkspaceChanged);

    QVERIFY(availableSpy.isValid());
    QVERIFY(initializedSpy.isValid());
    QVERIFY(workspacesSpy.isValid());
    QVERIFY(addedSpy.isValid());
    QVERIFY(removedSpy.isValid());
    QVERIFY(nameSpy.isValid());
    QVERIFY(positionSpy.isValid());
    QVERIFY(activeSpy.isValid());
    QVERIFY(currentSpy.isValid());
}

void PublicWorkspaceManagerTest::testWorkspaceManagerRejectsRequestsWhenUnavailable()
{
    auto *manager = WorkspaceManager::self();
    QVERIFY(manager);

    if (manager->isAvailable()) {
        QSKIP("Workspace protocol is available in this session; unavailable-state checks are not applicable.");
    }

    QCOMPARE(manager->isInitialized(), false);
    QVERIFY(manager->workspaces().isEmpty());
    QVERIFY(!manager->requestCreateWorkspace(QStringLiteral("Workspace"), 0));
    QVERIFY(!manager->requestRemoveWorkspace(QStringLiteral("missing-workspace")));
    QVERIFY(!manager->requestRenameWorkspace(QStringLiteral("missing-workspace"), QStringLiteral("Renamed")));
    QVERIFY(!manager->requestMoveWorkspace(QStringLiteral("missing-workspace"), 1));
    QVERIFY(!manager->requestActivateWorkspace(QStringLiteral("missing-workspace")));
}

} // namespace UkuiQuick::Test

QTEST_MAIN(UkuiQuick::Test::PublicWorkspaceManagerTest)

#include "test-public-workspace-manager.moc"
