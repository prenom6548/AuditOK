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

#include "../../../src/notification/notification-group-model.h"
#include "../../../src/notification/notification-model.h"

using namespace UkuiNotification;

namespace UkuiNotification {

static QList<uint> g_removedIds;
static NotificationModel *g_fakeModel = reinterpret_cast<NotificationModel *>(0x1);

NotificationModel *NotificationModel::instance()
{
    return g_fakeModel;
}

void NotificationModel::removeNotification(uint id)
{
    g_removedIds.append(id);
}

} // namespace UkuiNotification

class notificationGroupModel_test : public QObject
{
    Q_OBJECT

private slots:
    void test_setSourceModel_buildsGroups();
    void test_setSourceModel_sameModel_noChange();
    void test_setSourceModel_nullModel();
    void test_setData_updatesExpandState();
    void test_differentApp_createsSeparateGroup();
    void test_rowsInserted_updatesGrouping();
    void test_rowsRemoved_updatesGroups();
    void test_mapAndParentFunctions();
    void test_hasChildrenAndCounts();
    void test_dataChanged_reemitsForChild();
    void test_dataChanged_regroupsOnAppNameChange();
    void test_dataChanged_ignoredAndCrossParentBranches();
    void test_middleInsertAdjustsSourceIndexesAndMovesGroup();
    void test_noFoldGroupingUsesId();
    void test_singleGroupRemovalAndIgnoredParentRemoval();
    void test_invalidIndexesAndFallbacks();
    void test_clearGroup_callsRemoveNotification();
    void test_clearGroup_ignoresInvalidIndex();
    void test_stripRichText();
};

static QStandardItem *createGroupItem(const QString &appName, uint id, bool noFold = false)
{
    auto *item = new QStandardItem(appName);
    item->setData(id, NotificationItem::Id);
    item->setData(appName, NotificationItem::AppName);
    item->setData(noFold, NotificationItem::NoFold);
    item->setData(QString("<b>%1</b>").arg(appName), NotificationItem::Body);
    return item;
}

void notificationGroupModel_test::test_setSourceModel_buildsGroups()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));
    sourceModel.appendRow(createGroupItem("chat", 3));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupName).toString(), QString("chat"));
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0, QModelIndex()), NotificationItem::GroupName).toString(), QString("mail"));
    QCOMPARE(model.rowCount(model.index(1, 0, QModelIndex())), 2);
}

void notificationGroupModel_test::test_setSourceModel_sameModel_noChange()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    const int rowCount = model.rowCount(QModelIndex());

    model.setSourceModel(&sourceModel);

    QCOMPARE(model.rowCount(QModelIndex()), rowCount);
}

void notificationGroupModel_test::test_setSourceModel_nullModel()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    model.setSourceModel(nullptr);

    QCOMPARE(model.rowCount(QModelIndex()), 0);
    QCOMPARE(model.columnCount(QModelIndex()), 1);
}

void notificationGroupModel_test::test_setData_updatesExpandState()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QModelIndex groupIndex = model.index(0, 0, QModelIndex());
    QVERIFY(model.setData(groupIndex, true, NotificationItem::GroupIsExpand));
    QCOMPARE(model.data(groupIndex, NotificationItem::GroupIsExpand).toBool(), true);
}

void notificationGroupModel_test::test_differentApp_createsSeparateGroup()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("chat", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 1);
}

void notificationGroupModel_test::test_rowsInserted_updatesGrouping()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QSignalSpy spy(&model, &QAbstractItemModel::rowsInserted);
    sourceModel.insertRow(1, createGroupItem("mail", 2));

    QVERIFY(spy.count() >= 1);
    QCOMPARE(model.rowCount(QModelIndex()), 1);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 2);
}

void notificationGroupModel_test::test_rowsRemoved_updatesGroups()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    sourceModel.removeRow(0);

    QCOMPARE(model.rowCount(QModelIndex()), 1);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 1);
}

void notificationGroupModel_test::test_mapAndParentFunctions()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QModelIndex groupIndex = model.index(0, 0, QModelIndex());
    QModelIndex childIndex = model.index(0, 0, groupIndex);
    QModelIndex sourceChild = sourceModel.index(1, 0);

    QVERIFY(groupIndex.isValid());
    QVERIFY(childIndex.isValid());
    QCOMPARE(model.parent(childIndex), groupIndex);
    QCOMPARE(model.mapToSource(childIndex), sourceChild);
    QCOMPARE(model.mapFromSource(sourceChild), childIndex);
}

void notificationGroupModel_test::test_hasChildrenAndCounts()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QModelIndex groupIndex = model.index(0, 0, QModelIndex());
    QModelIndex childIndex = model.index(0, 0, groupIndex);

    QVERIFY(model.hasChildren(QModelIndex()));
    QVERIFY(model.hasChildren(groupIndex));
    QCOMPARE(model.hasChildren(childIndex), false);
    QCOMPARE(model.rowCount(childIndex), 0);
    QCOMPARE(model.columnCount(groupIndex), 1);
}

void notificationGroupModel_test::test_dataChanged_reemitsForChild()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);

    sourceModel.setData(sourceModel.index(1, 0), QString("updated"), NotificationItem::Summary);

    QVERIFY(spy.count() >= 1);
}

void notificationGroupModel_test::test_dataChanged_regroupsOnAppNameChange()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    sourceModel.setData(sourceModel.index(1, 0), QString("chat"), NotificationItem::AppName);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
}

void notificationGroupModel_test::test_dataChanged_ignoredAndCrossParentBranches()
{
    QStandardItemModel sourceModel;
    auto *parentA = createGroupItem("parentA", 10);
    parentA->appendRow(createGroupItem("childA", 11));
    auto *parentB = createGroupItem("parentB", 20);
    parentB->appendRow(createGroupItem("childB", 21));
    sourceModel.appendRow(parentA);
    sourceModel.appendRow(parentB);

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);

    sourceModel.setData(parentA->child(0)->index(), QString("ignored"), NotificationItem::Summary);
    QCOMPARE(spy.count(), 0);

    Q_EMIT sourceModel.dataChanged(sourceModel.index(0, 0), sourceModel.index(1, 0),
                                   {NotificationItem::Summary});
    QVERIFY(spy.count() >= 1);
}

void notificationGroupModel_test::test_middleInsertAdjustsSourceIndexesAndMovesGroup()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("chat", 2));
    sourceModel.appendRow(createGroupItem("mail", 3));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    QCOMPARE(model.rowCount(QModelIndex()), 2);

    QSignalSpy moveSpy(&model, &QAbstractItemModel::rowsMoved);
    sourceModel.insertRow(1, createGroupItem("chat", 4));

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupName).toString(),
             QString("chat"));
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 2);
    QVERIFY(moveSpy.count() >= 1);
    QCOMPARE(model.mapToSource(model.index(0, 0, model.index(0, 0, QModelIndex()))),
             sourceModel.index(1, 0));
}

void notificationGroupModel_test::test_noFoldGroupingUsesId()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("1", 1, true));
    sourceModel.appendRow(createGroupItem("1", 1, true));
    sourceModel.appendRow(createGroupItem("2", 2, true));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0, QModelIndex()), NotificationItem::GroupCount).toInt(), 2);
}

void notificationGroupModel_test::test_singleGroupRemovalAndIgnoredParentRemoval()
{
    QStandardItemModel sourceModel;
    auto *parent = createGroupItem("parent", 10);
    parent->appendRow(createGroupItem("child", 11));
    sourceModel.appendRow(parent);
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("chat", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    sourceModel.item(0)->removeRow(0);
    QCOMPARE(model.rowCount(QModelIndex()), 3);

    sourceModel.removeRow(2);
    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.data(model.index(0, 0, QModelIndex()), NotificationItem::GroupName).toString(),
             QString("mail"));
}

void notificationGroupModel_test::test_invalidIndexesAndFallbacks()
{
    NotificationGroupModel emptyModel;
    QCOMPARE(emptyModel.hasChildren(QModelIndex()), false);
    QCOMPARE(emptyModel.rowCount(QModelIndex()), 0);
    QVERIFY(!emptyModel.index(-1, 0, QModelIndex()).isValid());
    QVERIFY(!emptyModel.index(0, 1, QModelIndex()).isValid());
    QVERIFY(!emptyModel.mapToSource(QModelIndex()).isValid());
    QVERIFY(!emptyModel.mapFromSource(QModelIndex()).isValid());

    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);

    QModelIndex groupIndex = model.index(0, 0, QModelIndex());
    QModelIndex childIndex = model.index(0, 0, groupIndex);
    QVERIFY(!model.parent(groupIndex).isValid());
    QCOMPARE(model.rowCount(model.index(8, 0, QModelIndex())), 0);
    QVERIFY(!model.mapToSource(groupIndex).isValid());
    QVERIFY(!model.mapFromSource(sourceModel.index(0, 1)).isValid());
    QCOMPARE(model.data(QModelIndex(), NotificationItem::GroupName), QVariant());
    QCOMPARE(model.data(childIndex, NotificationItem::AppName).toString(), QString("mail"));
    QCOMPARE(model.data(groupIndex, NotificationItem::Body), QVariant());
    QCOMPARE(model.setData(QModelIndex(), true, NotificationItem::GroupIsExpand), false);
    QCOMPARE(model.setData(childIndex, true, NotificationItem::GroupIsExpand), false);
    QCOMPARE(model.setData(groupIndex, true, NotificationItem::Summary), false);
}

void notificationGroupModel_test::test_clearGroup_callsRemoveNotification()
{
    g_removedIds.clear();

    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));
    sourceModel.appendRow(createGroupItem("mail", 2));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    model.clearGroup(model.index(0, 0, QModelIndex()));

    QCOMPARE(g_removedIds.size(), 2);
}

void notificationGroupModel_test::test_clearGroup_ignoresInvalidIndex()
{
    g_removedIds.clear();
    QStandardItemModel sourceModel;
    sourceModel.appendRow(createGroupItem("mail", 1));

    NotificationGroupModel model;
    model.setSourceModel(&sourceModel);
    model.clearGroup(QModelIndex());

    QCOMPARE(model.rowCount(QModelIndex()), 1);
    QCOMPARE(g_removedIds.size(), 0);
}

void notificationGroupModel_test::test_stripRichText()
{
    NotificationGroupModel model;

    QCOMPARE(model.stripRichText("<p>Hello <b>Qt</b></p>"), QString("Hello Qt"));
    QCOMPARE(model.stripRichText("plain"), QString("plain"));
}

QTEST_MAIN(notificationGroupModel_test)

#include "notification-group-model-test.moc"
