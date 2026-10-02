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
#include <QStandardItemModel>

#include "../../../src/notification/group-cache-proxy-model.h"
#include "../../../src/notification/notification-model.h"

using namespace UkuiNotification;

class groupCacheProxyModel_test : public QObject
{
    Q_OBJECT

private slots:
    void test_defaultState();
    void test_setRootIndex_populatesItems();
    void test_mapBetweenSourceAndProxy();
    void test_data_forwardsSourceRole();
    void test_removeItem_removesMatchedRow();
    void test_rowsInserted_updatesCache();
    void test_rowsRemoved_emitsItemRemoved();
    void test_dataChanged_isForwarded();
};

static QStandardItem *createNotificationLeaf(const QString &appName, uint id)
{
    auto *item = new QStandardItem(QString::number(id));
    item->setData(appName, NotificationItem::AppName);
    item->setData(QString("summary-%1").arg(id), NotificationItem::Summary);
    return item;
}

static QModelIndex prepareRootWithChildren(QStandardItemModel &sourceModel, int childCount = 2)
{
    auto *groupItem = new QStandardItem("group");
    for (int i = 0; i < childCount; ++i) {
        groupItem->appendRow(createNotificationLeaf(QString("app-%1").arg(i), static_cast<uint>(i + 1)));
    }
    sourceModel.appendRow(groupItem);
    return sourceModel.index(0, 0, QModelIndex());
}

void groupCacheProxyModel_test::test_defaultState()
{
    GroupCacheProxyModel model;

    QCOMPARE(model.rowCount(QModelIndex()), 0);
    QCOMPARE(model.columnCount(QModelIndex()), 1);
    QVERIFY(model.index(0, 0, QModelIndex()).isValid());
    QVERIFY(!model.parent(QModelIndex()).isValid());
}

void groupCacheProxyModel_test::test_setRootIndex_populatesItems()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 2);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.getRootIndex(0), rootIndex);
}

void groupCacheProxyModel_test::test_mapBetweenSourceAndProxy()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 2);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QModelIndex firstSourceChild = sourceModel.index(0, 0, rootIndex);
    QModelIndex firstProxyChild = model.mapFromSource(firstSourceChild);

    QVERIFY(firstProxyChild.isValid());
    QCOMPARE(model.mapToSource(firstProxyChild), firstSourceChild);
    QVERIFY(!model.mapFromSource(rootIndex).isValid());
}

void groupCacheProxyModel_test::test_data_forwardsSourceRole()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 1);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::Summary).toString(),
             QString("summary-1"));
}

void groupCacheProxyModel_test::test_removeItem_removesMatchedRow()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 3);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QCOMPARE(model.rowCount(QModelIndex()), 3);

    const uint removableId = model.data(model.index(1, 0, QModelIndex()), NotificationItem::Id).toUInt();
    model.removeItem(removableId);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QVERIFY(model.index(0, 0, QModelIndex()).isValid());
    QVERIFY(model.index(1, 0, QModelIndex()).isValid());
}

void groupCacheProxyModel_test::test_rowsInserted_updatesCache()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 1);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QSignalSpy spy(&model, &QAbstractItemModel::rowsInserted);
    sourceModel.itemFromIndex(rootIndex)->appendRow(createNotificationLeaf("new-app", 9));
    const QModelIndex insertedSourceChild = sourceModel.index(1, 0, rootIndex);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QVERIFY(model.mapFromSource(insertedSourceChild).isValid());
}

void groupCacheProxyModel_test::test_rowsRemoved_emitsItemRemoved()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 2);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QSignalSpy spy(&model, &GroupCacheProxyModel::itemRemoved);
    sourceModel.itemFromIndex(rootIndex)->removeRow(0);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
}

void groupCacheProxyModel_test::test_dataChanged_isForwarded()
{
    QStandardItemModel sourceModel;
    QModelIndex rootIndex = prepareRootWithChildren(sourceModel, 1);

    GroupCacheProxyModel model;
    model.setSourceModel(&sourceModel);
    model.setRootIndex(rootIndex);

    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);
    QModelIndex sourceChild = sourceModel.index(0, 0, rootIndex);
    sourceModel.setData(sourceChild, QString("summary-updated"), NotificationItem::Summary);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::Summary).toString(),
             QString("summary-updated"));
}

QTEST_MAIN(groupCacheProxyModel_test)

#include "group-cache-proxy-model-test.moc"
