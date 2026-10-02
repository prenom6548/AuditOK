/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "../../kysdk-notification/src/knotifier.h"
#include <QtTest>

// add necessary includes here
using namespace kdk;

class NotificationTest : public QObject
{
    Q_OBJECT

private slots:
    void notifierTest();

};

void NotificationTest::notifierTest()
{
    KNotifier notifier;
    notifier.setAppName("ukui-control-center");
    notifier.setAppIcon("ukui-control-center");
    notifier.setBodyText("bodyText");
    notifier.setSummary("summary");
    notifier.setDefaultAction("ukui-control-center");
    notifier.addAction("kylin-music","music");
    notifier.setShowTime(KNotifier::AllTheTime);
    uint id = notifier.notify();

    notifier.setReplaceId(id);
    notifier.closeNotification(id);
}

QTEST_APPLESS_MAIN(NotificationTest)

#include "notificationtest.moc"
