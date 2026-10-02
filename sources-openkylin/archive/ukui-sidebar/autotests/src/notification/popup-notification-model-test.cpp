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

#include "../../../src/notification/popup-notification-model.h"
#include "../../../src/notification/notification-model.h"
#include "stubs/global-settings.h"
#include "stubs/screen-monitor.h"

using namespace UkuiNotification;

class TestPopupNotificationModel : public PopupNotificationModel
{
    Q_OBJECT

public:
    using PopupNotificationModel::filterAcceptsRow;
};

class popupNotificationModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_filterRejectsWhenTabletMode();
    void test_filterRejectsWhenSessionToolsVisible();
    void test_filterRejectsStoredNotification();
    void test_filterHonorsLockScreenVisibility();
    void test_valueChange_updatesFilter();
    void test_sessionToolsChange_updatesFilter();
};

static void setPopupRow(QStandardItemModel &model, int row, bool isStored, bool showOnLockScreen)
{
    model.insertRow(row);
    QModelIndex index = model.index(row, 0);
    model.setData(index, isStored, NotificationItem::IsStored);
    model.setData(index, showOnLockScreen, NotificationItem::ShowOnLockScreen);
}

void popupNotificationModel_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);
    Sidebar::ScreenMonitor::getInstance()->setScreenLockState(false);
    Sidebar::ScreenMonitor::getInstance()->setSessionToolsState(false);
}

void popupNotificationModel_test::test_filterRejectsWhenTabletMode()
{
    QStandardItemModel sourceModel(1, 1);
    setPopupRow(sourceModel, 0, false, true);

    TestPopupNotificationModel model;
    model.setSourceModel(&sourceModel);
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    QCOMPARE(model.filterAcceptsRow(0, QModelIndex()), false);
}

void popupNotificationModel_test::test_filterRejectsWhenSessionToolsVisible()
{
    QStandardItemModel sourceModel(1, 1);
    setPopupRow(sourceModel, 0, false, true);

    Sidebar::ScreenMonitor::getInstance()->setSessionToolsState(true);
    TestPopupNotificationModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.filterAcceptsRow(0, QModelIndex()), false);
}

void popupNotificationModel_test::test_filterRejectsStoredNotification()
{
    QStandardItemModel sourceModel(2, 1);
    setPopupRow(sourceModel, 0, true, true);
    setPopupRow(sourceModel, 1, false, true);

    TestPopupNotificationModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.filterAcceptsRow(0, QModelIndex()), false);
    QCOMPARE(model.filterAcceptsRow(1, QModelIndex()), true);
}

void popupNotificationModel_test::test_filterHonorsLockScreenVisibility()
{
    QStandardItemModel sourceModel(2, 1);
    setPopupRow(sourceModel, 0, false, false);
    setPopupRow(sourceModel, 1, false, true);

    TestPopupNotificationModel model;
    model.setSourceModel(&sourceModel);
    Sidebar::ScreenMonitor::getInstance()->setScreenLockState(true);

    QCOMPARE(model.filterAcceptsRow(0, QModelIndex()), false);
    QCOMPARE(model.filterAcceptsRow(1, QModelIndex()), true);
}

void popupNotificationModel_test::test_valueChange_updatesFilter()
{
    QStandardItemModel sourceModel;
    setPopupRow(sourceModel, 0, false, true);

    PopupNotificationModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.rowCount(), 1);
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);
    QCOMPARE(model.rowCount(), 0);
}

void popupNotificationModel_test::test_sessionToolsChange_updatesFilter()
{
    QStandardItemModel sourceModel;
    setPopupRow(sourceModel, 0, false, true);

    PopupNotificationModel model;
    model.setSourceModel(&sourceModel);

    QCOMPARE(model.rowCount(), 1);
    Sidebar::ScreenMonitor::getInstance()->setSessionToolsState(true);
    QCOMPARE(model.rowCount(), 0);
    Sidebar::ScreenMonitor::getInstance()->setSessionToolsState(false);
    QCOMPARE(model.rowCount(), 1);
}

QTEST_MAIN(popupNotificationModel_test)

#include "popup-notification-model-test.moc"
