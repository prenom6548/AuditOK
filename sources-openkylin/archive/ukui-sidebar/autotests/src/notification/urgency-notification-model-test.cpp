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

#include "../../../src/notification/urgency-notification-model.h"

using namespace UkuiNotification;

namespace UkuiNotification {

NotificationModel *NotificationModel::instance()
{
    return nullptr;
}

void NotificationModel::closeNotification(uint)
{
}

void NotificationModel::execAction(uint, QString)
{
}

} // namespace UkuiNotification

class TestUrgencyNotificationModel : public UrgencyNotificationModel
{
    Q_OBJECT

public:
    using UrgencyNotificationModel::filterAcceptsRow;
    using UrgencyNotificationModel::lessThan;
};

class urgencyNotificationModel_test : public QObject
{
    Q_OBJECT

private slots:
    void test_forwardingMethods_doNotCrash();
    void test_filterAcceptsCriticalUrgency();
    void test_filterRejectsNonCriticalUrgency();
    void test_lessThan_usesCreateTime();
    void test_sort_placesEarlierNotificationFirst();
};

static void setNotificationRow(QStandardItemModel &model,
                               int row,
                               int urgency,
                               const QDateTime &createdTime)
{
    model.insertRow(row);
    QModelIndex index = model.index(row, 0);
    model.setData(index, urgency, NotificationItem::Urgency);
    model.setData(index, createdTime, NotificationItem::CreateTime);
}

void urgencyNotificationModel_test::test_forwardingMethods_doNotCrash()
{
    UrgencyNotificationModel model;

    model.closeNotification(42);
    model.execAction(42, "default");

    QVERIFY(true);
}

void urgencyNotificationModel_test::test_filterAcceptsCriticalUrgency()
{
    QStandardItemModel sourceModel(1, 1);
    setNotificationRow(sourceModel, 0, PopupNotification::CriticalUrgency, QDateTime::currentDateTime());

    TestUrgencyNotificationModel model;
    model.setSourceModel(&sourceModel);

    QVERIFY(model.filterAcceptsRow(0, QModelIndex()));
}

void urgencyNotificationModel_test::test_filterRejectsNonCriticalUrgency()
{
    QStandardItemModel sourceModel(2, 1);
    setNotificationRow(sourceModel, 0, PopupNotification::LowUrgency, QDateTime::currentDateTime());
    setNotificationRow(sourceModel, 1, PopupNotification::NormalUrgency, QDateTime::currentDateTime().addSecs(1));

    TestUrgencyNotificationModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.filterAcceptsRow(0, QModelIndex()), false);
    QCOMPARE(model.filterAcceptsRow(1, QModelIndex()), false);
}

void urgencyNotificationModel_test::test_lessThan_usesCreateTime()
{
    QStandardItemModel sourceModel(2, 1);
    const QDateTime earlyTime(QDate(2024, 1, 1), QTime(8, 0, 0));
    const QDateTime lateTime(QDate(2024, 1, 1), QTime(9, 0, 0));

    setNotificationRow(sourceModel, 0, PopupNotification::CriticalUrgency, lateTime);
    setNotificationRow(sourceModel, 1, PopupNotification::CriticalUrgency, earlyTime);

    TestUrgencyNotificationModel model;
    model.setSourceModel(&sourceModel);

    QVERIFY(model.lessThan(sourceModel.index(1, 0), sourceModel.index(0, 0)));
    QCOMPARE(model.lessThan(sourceModel.index(0, 0), sourceModel.index(1, 0)), false);
}

void urgencyNotificationModel_test::test_sort_placesEarlierNotificationFirst()
{
    QStandardItemModel sourceModel(3, 1);
    const QDateTime baseTime(QDate(2024, 1, 1), QTime(10, 0, 0));

    setNotificationRow(sourceModel, 0, PopupNotification::NormalUrgency, baseTime.addSecs(30));
    setNotificationRow(sourceModel, 1, PopupNotification::CriticalUrgency, baseTime.addSecs(20));
    setNotificationRow(sourceModel, 2, PopupNotification::CriticalUrgency, baseTime.addSecs(10));

    TestUrgencyNotificationModel model;
    model.setSourceModel(&sourceModel);
    model.sort(0, Qt::AscendingOrder);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.index(0, 0).data(NotificationItem::CreateTime).toDateTime(), baseTime.addSecs(10));
    QCOMPARE(model.index(1, 0).data(NotificationItem::CreateTime).toDateTime(), baseTime.addSecs(20));
}

QTEST_MAIN(urgencyNotificationModel_test)

#include "urgency-notification-model-test.moc"
