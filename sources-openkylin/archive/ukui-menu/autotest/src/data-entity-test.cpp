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

#include "../../src/data-entity.h"

using namespace UkuiMenu;

class dataEntity_test : public QObject
{
    Q_OBJECT

private slots:
    void test_propertyAccessors_roundTripValues();
    void test_copyAndMove_keepState();
    void test_getValue_and_setValue_coverSupportedRoles();
    void test_roleNames_and_defaultBranches();
};

void dataEntity_test::test_propertyAccessors_roundTripValues()
{
    DataEntity entity(DataType::Folder,
                      QStringLiteral("Folder"),
                      QStringLiteral("folder-icon"),
                      QStringLiteral("folder-comment"),
                      QStringLiteral("folder-extra"));

    entity.setId(QStringLiteral("/usr/share/applications/demo.desktop"));
    entity.setTop(2);
    entity.setLock(true);
    entity.setRecentInstall(true);
    entity.setLaunched(1);
    entity.setFavorite(3);
    entity.setLaunchTimes(9);
    entity.setPriority(1.5);
    entity.setInsertTime(QStringLiteral("2024-05-01 08:00:00"));
    entity.setCategory(QStringLiteral("Utility"));
    entity.setFirstLetter(QStringLiteral("D"));
    entity.setType(DataType::Files);
    entity.setIcon(QStringLiteral("demo-icon"));
    entity.setName(QStringLiteral("Demo"));
    entity.setComment(QStringLiteral("comment"));
    entity.setExtraData(QStringLiteral("extra"));
    entity.setGroup(QStringLiteral("Pinned"));
    entity.setRemovable(true);
    entity.setDesktopName(QStringLiteral("manual-name"));
    entity.setDontDisplay(1);
    entity.setAutoStart(2);

    QCOMPARE(entity.id(), QStringLiteral("/usr/share/applications/demo.desktop"));
    QCOMPARE(entity.top(), 2);
    QCOMPARE(entity.isLock(), true);
    QCOMPARE(entity.isRecentInstall(), true);
    QCOMPARE(entity.launched(), 1);
    QCOMPARE(entity.favorite(), 3);
    QCOMPARE(entity.launchTimes(), 9);
    QCOMPARE(entity.priority(), 1.5);
    QCOMPARE(entity.insertTime(), QStringLiteral("2024-05-01 08:00:00"));
    QCOMPARE(entity.category(), QStringLiteral("Utility"));
    QCOMPARE(entity.firstLetter(), QStringLiteral("D"));
    QCOMPARE(entity.type(), DataType::Files);
    QCOMPARE(entity.icon(), QStringLiteral("demo-icon"));
    QCOMPARE(entity.name(), QStringLiteral("Demo"));
    QCOMPARE(entity.comment(), QStringLiteral("comment"));
    QCOMPARE(entity.extraData(), QStringLiteral("extra"));
    QCOMPARE(entity.group(), QStringLiteral("Pinned"));
    QCOMPARE(entity.removable(), true);
    QCOMPARE(entity.desktopName(), QStringLiteral("manual-name"));
    QCOMPARE(entity.dontDisplay(), 1);
    QCOMPARE(entity.autoStart(), 2);

    entity.setId(QStringLiteral("/opt/apps/auto-name.desktop"));
    QCOMPARE(entity.desktopName(), QStringLiteral("auto-name"));
}

void dataEntity_test::test_copyAndMove_keepState()
{
    DataEntity source;
    source.setId(QStringLiteral("/tmp/source.desktop"));
    source.setName(QStringLiteral("Source"));
    source.setFavorite(7);
    source.setRemovable(true);
    source.setDontDisplay(1);
    source.setAutoStart(1);

    DataEntity copied(source);
    QCOMPARE(copied.id(), source.id());
    QCOMPARE(copied.name(), source.name());
    QCOMPARE(copied.favorite(), source.favorite());
    QCOMPARE(copied.removable(), source.removable());
    QCOMPARE(copied.dontDisplay(), source.dontDisplay());
    QCOMPARE(copied.autoStart(), source.autoStart());

    DataEntity assigned;
    assigned = source;
    QCOMPARE(assigned.id(), source.id());
    QCOMPARE(assigned.desktopName(), source.desktopName());

    DataEntity moved(std::move(source));
    QCOMPARE(moved.id(), QStringLiteral("/tmp/source.desktop"));
    QCOMPARE(moved.name(), QStringLiteral("Source"));
    QCOMPARE(moved.favorite(), 7);
    QCOMPARE(moved.removable(), true);

    DataEntity moveAssigned;
    moveAssigned = std::move(copied);
    QCOMPARE(moveAssigned.id(), QStringLiteral("/tmp/source.desktop"));
    QCOMPARE(moveAssigned.name(), QStringLiteral("Source"));
    QCOMPARE(moveAssigned.removable(), true);
}

void dataEntity_test::test_getValue_and_setValue_coverSupportedRoles()
{
    qRegisterMetaType<DataEntity>("DataEntity");
    qRegisterMetaType<DataType::Type>("DataType::Type");

    DataEntity entity;
    entity.setValue(DataEntity::Id, QStringLiteral("/tmp/value.desktop"));
    entity.setValue(DataEntity::Type, QVariant::fromValue(DataType::Label));
    entity.setValue(DataEntity::Icon, QStringLiteral("value-icon"));
    entity.setValue(DataEntity::Name, QStringLiteral("Value"));
    entity.setValue(DataEntity::Comment, QStringLiteral("value-comment"));
    entity.setValue(DataEntity::ExtraData, QStringLiteral("value-extra"));
    entity.setValue(DataEntity::Category, QStringLiteral("Development"));
    entity.setValue(DataEntity::Group, QStringLiteral("Recent"));
    entity.setValue(DataEntity::FirstLetter, QStringLiteral("V"));
    entity.setValue(DataEntity::InstallationTime, QStringLiteral("2024-06-01 10:00:00"));
    entity.setValue(DataEntity::IsLaunched, true);
    entity.setValue(DataEntity::LaunchTimes, 12);
    entity.setValue(DataEntity::IsLocked, true);
    entity.setValue(DataEntity::Favorite, 4);
    entity.setValue(DataEntity::Top, 5);
    entity.setValue(DataEntity::RecentInstall, true);
    entity.setValue(DataEntity::Removable, true);
    entity.setValue(DataEntity::DesktopName, QStringLiteral("manual-desktop"));
    entity.setValue(DataEntity::DontDisplay, 1);
    entity.setValue(DataEntity::AutoStart, 2);

    QCOMPARE(entity.getValue(DataEntity::Id).toString(), QStringLiteral("/tmp/value.desktop"));
    QCOMPARE(entity.getValue(DataEntity::Type).value<DataType::Type>(), DataType::Label);
    QCOMPARE(entity.getValue(DataEntity::Icon).toString(), QStringLiteral("value-icon"));
    QCOMPARE(entity.getValue(DataEntity::Name).toString(), QStringLiteral("Value"));
    QCOMPARE(entity.getValue(DataEntity::Comment).toString(), QStringLiteral("value-comment"));
    QCOMPARE(entity.getValue(DataEntity::ExtraData).toString(), QStringLiteral("value-extra"));
    QCOMPARE(entity.getValue(DataEntity::Category).toString(), QStringLiteral("Development"));
    QCOMPARE(entity.getValue(DataEntity::Group).toString(), QStringLiteral("Recent"));
    QCOMPARE(entity.getValue(DataEntity::FirstLetter).toString(), QStringLiteral("V"));
    QCOMPARE(entity.getValue(DataEntity::InstallationTime).toString(), QStringLiteral("2024-06-01 10:00:00"));
    QCOMPARE(entity.getValue(DataEntity::IsLaunched).toInt(), 1);
    QCOMPARE(entity.getValue(DataEntity::LaunchTimes).toInt(), 12);
    QCOMPARE(entity.getValue(DataEntity::IsLocked).toBool(), true);
    QCOMPARE(entity.getValue(DataEntity::Favorite).toInt(), 4);
    QCOMPARE(entity.getValue(DataEntity::Top).toInt(), 5);
    QCOMPARE(entity.getValue(DataEntity::RecentInstall).toBool(), true);
    QCOMPARE(entity.removable(), true);
    QVERIFY(!entity.getValue(DataEntity::Removable).isValid());
    QCOMPARE(entity.getValue(DataEntity::DesktopName).toString(), QStringLiteral("manual-desktop"));
    QCOMPARE(entity.getValue(DataEntity::DontDisplay).toInt(), 1);
    QCOMPARE(entity.getValue(DataEntity::AutoStart).toInt(), 2);

    const QVariant entityVariant = entity.getValue(DataEntity::Entity);
    QVERIFY(entityVariant.canConvert<DataEntity>());
    QCOMPARE(entityVariant.value<DataEntity>().name(), QStringLiteral("Value"));
}

void dataEntity_test::test_roleNames_and_defaultBranches()
{
    const QHash<int, QByteArray> roleNames = DataEntity::AppRoleNames();

    QCOMPARE(roleNames.value(DataEntity::Id), QByteArray("id"));
    QCOMPARE(roleNames.value(DataEntity::InstallationTime), QByteArray("installationTime"));
    QCOMPARE(static_cast<int>(DataEntity::Entity), 16);
    QCOMPARE(static_cast<int>(DataEntity::Removable), 17);
    QCOMPARE(static_cast<int>(DataEntity::DesktopName), 18);
    QCOMPARE(roleNames.value(DataEntity::DesktopName), QByteArray("desktopName"));
    QCOMPARE(roleNames.contains(DataEntity::Removable), false);
    QCOMPARE(roleNames.value(DataEntity::DontDisplay), QByteArray("dontDisplay"));
    QCOMPARE(roleNames.value(DataEntity::AutoStart), QByteArray("autoStart"));

    DataEntity entity;
    QVERIFY(!entity.getValue(static_cast<DataEntity::PropertyName>(999)).isValid());
    entity.setValue(static_cast<DataEntity::PropertyName>(999), QStringLiteral("ignored"));
    QCOMPARE(entity.name(), QStringLiteral(""));
}

QTEST_MAIN(dataEntity_test)

#include "data-entity-test.moc"
