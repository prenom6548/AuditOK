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
#include <QStandardItemModel>

#include "../../../src/data-entity.h"
#include "../../../src/libappdata/app-group-model.h"

using namespace UkuiMenu;

namespace {

QStandardItem *createGroupedItem(const QString &name, const QString &group)
{
    auto *item = new QStandardItem(name);
    item->setData(name, DataEntity::Name);
    item->setData(group, DataEntity::Group);
    return item;
}

}

class appGroupModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_setSourceModel_buildsGroups();
    void test_mapBetweenProxyAndSource();
    void test_rowsInserted_createsNewGroup();
    void test_rowsRemoved_updatesGroups();
    void test_findLabelIndex_returnsExpectedGroup();
    void test_safeBranches_withoutSourceAndInvalidIndexes();
    void test_sourceSignals_coverResetRelayoutAndDataChanged();
    void test_rowsInsertedAndRemoved_ignoreChildParent();

private:
    QStandardItemModel m_sourceModel;
    AppGroupModel m_model;
};

void appGroupModel_test::init()
{
    m_sourceModel.clear();
    m_sourceModel.setColumnCount(1);
    m_sourceModel.appendRow(createGroupedItem(QStringLiteral("Calculator"), QStringLiteral("Utility")));
    m_sourceModel.appendRow(createGroupedItem(QStringLiteral("Writer"), QStringLiteral("Office")));
    m_sourceModel.appendRow(createGroupedItem(QStringLiteral("Clock"), QStringLiteral("Utility")));
    m_model.setSourceModel(&m_sourceModel);
}

void appGroupModel_test::test_setSourceModel_buildsGroups()
{
    QCOMPARE(m_model.rowCount(QModelIndex()), 2);
    QCOMPARE(m_model.data(m_model.index(0, 0, QModelIndex()), DataEntity::Group).toString(), QStringLiteral("Utility"));
    QCOMPARE(m_model.data(m_model.index(1, 0, QModelIndex()), DataEntity::Name).toString(), QStringLiteral("Office"));
    QCOMPARE(m_model.rowCount(m_model.index(0, 0, QModelIndex())), 2);
    QCOMPARE(m_model.rowCount(m_model.index(1, 0, QModelIndex())), 1);
}

void appGroupModel_test::test_mapBetweenProxyAndSource()
{
    const QModelIndex sourceIndex = m_sourceModel.index(2, 0);
    const QModelIndex proxyChildIndex = m_model.mapFromSource(sourceIndex);

    QVERIFY(proxyChildIndex.isValid());
    QCOMPARE(proxyChildIndex.parent().data(DataEntity::Group).toString(), QStringLiteral("Utility"));
    QCOMPARE(m_model.mapToSource(proxyChildIndex), sourceIndex);
}

void appGroupModel_test::test_rowsInserted_createsNewGroup()
{
    m_sourceModel.appendRow(createGroupedItem(QStringLiteral("Browser"), QStringLiteral("Network")));

    QCOMPARE(m_model.rowCount(QModelIndex()), 3);
    QCOMPARE(m_model.data(m_model.index(1, 0, QModelIndex()), DataEntity::Group).toString(), QStringLiteral("Network"));
    QCOMPARE(m_model.data(m_model.index(2, 0, QModelIndex()), DataEntity::Group).toString(), QStringLiteral("Office"));
}

void appGroupModel_test::test_rowsRemoved_updatesGroups()
{
    m_sourceModel.removeRow(1);

    QCOMPARE(m_model.rowCount(QModelIndex()), 1);
    QCOMPARE(m_model.data(m_model.index(0, 0, QModelIndex()), DataEntity::Group).toString(), QStringLiteral("Utility"));
    QCOMPARE(m_model.rowCount(m_model.index(0, 0, QModelIndex())), 2);
}

void appGroupModel_test::test_findLabelIndex_returnsExpectedGroup()
{
    QCOMPARE(m_model.findLabelIndex(QStringLiteral("Utility")), 0);
    QCOMPARE(m_model.findLabelIndex(QStringLiteral("Office")), 1);
    QCOMPARE(m_model.findLabelIndex(QStringLiteral("Missing")), -1);
}

void appGroupModel_test::test_safeBranches_withoutSourceAndInvalidIndexes()
{
    AppGroupModel emptyModel;

    QCOMPARE(emptyModel.hasChildren(QModelIndex()), false);
    QCOMPARE(emptyModel.rowCount(QModelIndex()), 0);
    QCOMPARE(emptyModel.mapToSource(QModelIndex()), QModelIndex());
    QCOMPARE(emptyModel.mapFromSource(QModelIndex()), QModelIndex());
    QCOMPARE(emptyModel.columnCount(QModelIndex()), 1);
    QStandardItemModel defaultModel;
    QCOMPARE(emptyModel.roleNames(), defaultModel.roleNames());
    QVERIFY(!emptyModel.index(-1, 0, QModelIndex()).isValid());
    QVERIFY(!emptyModel.index(0, 1, QModelIndex()).isValid());

    const QModelIndex rootGroup = m_model.index(0, 0, QModelIndex());
    const QModelIndex child = m_model.index(0, 0, rootGroup);
    QVERIFY(rootGroup.isValid());
    QVERIFY(child.isValid());

    QCOMPARE(m_model.parent(QModelIndex()), QModelIndex());
    QCOMPARE(m_model.parent(rootGroup), QModelIndex());
    QCOMPARE(m_model.parent(child), rootGroup);
    QCOMPARE(m_model.hasChildren(QModelIndex()), true);
    QCOMPARE(m_model.hasChildren(rootGroup), true);
    QCOMPARE(m_model.hasChildren(child), false);
    QCOMPARE(m_model.rowCount(child), 0);
    QCOMPARE(m_model.rowCount(m_model.index(99, 0, QModelIndex())), 0);
    QVERIFY(!m_model.index(99, 0, rootGroup).isValid());
    QVERIFY(!m_model.mapFromSource(QModelIndex()).isValid());
    QVERIFY(!m_model.mapToSource(rootGroup).isValid());
    QVERIFY(!m_model.data(QModelIndex(), DataEntity::Group).isValid());
    QCOMPARE(m_model.data(child, DataEntity::Name).toString(), QStringLiteral("Calculator"));

    m_model.setSourceModel(&m_sourceModel);
    QCOMPARE(m_model.rowCount(QModelIndex()), 2);
}

void appGroupModel_test::test_sourceSignals_coverResetRelayoutAndDataChanged()
{
    QSignalSpy dataSpy(&m_model, &AppGroupModel::dataChanged);
    QSignalSpy resetSpy(&m_model, &AppGroupModel::modelReset);

    m_sourceModel.setData(m_sourceModel.index(0, 0), QStringLiteral("Calculator Updated"), DataEntity::Name);
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(dataSpy.takeFirst().at(0).toModelIndex().data(DataEntity::Name).toString(), QStringLiteral("Calculator Updated"));

    Q_EMIT m_sourceModel.layoutChanged();
    QCOMPARE(resetSpy.count(), 1);
    QCOMPARE(m_model.rowCount(QModelIndex()), 2);

    m_sourceModel.clear();
    m_sourceModel.setColumnCount(1);
    m_sourceModel.appendRow(createGroupedItem(QStringLiteral("Browser"), QStringLiteral("Network")));
    QCOMPARE(resetSpy.count(), 2);
    QCOMPARE(m_model.rowCount(QModelIndex()), 1);

    m_model.setSourceModel(nullptr);
    QCOMPARE(m_model.rowCount(QModelIndex()), 0);
}

void appGroupModel_test::test_rowsInsertedAndRemoved_ignoreChildParent()
{
    const QModelIndex sourceParent = m_sourceModel.index(0, 0);
    QVERIFY(sourceParent.isValid());
    const int initialGroups = m_model.rowCount(QModelIndex());

    m_sourceModel.item(0, 0)->appendRow(createGroupedItem(QStringLiteral("Child"), QStringLiteral("Utility")));
    QCOMPARE(m_model.rowCount(QModelIndex()), initialGroups);

    m_sourceModel.item(0, 0)->removeRow(0);
    QCOMPARE(m_model.rowCount(QModelIndex()), initialGroups);
}

QTEST_MAIN(appGroupModel_test)

#include "app-group-model-test.moc"
