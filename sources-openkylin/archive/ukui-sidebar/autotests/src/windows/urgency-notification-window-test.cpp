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

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QQmlContext>

#include "QGSettings"
#include "QX11Info"
#include "notification-model.h"
#include "urgency-notification-model.h"

#define private public
#define protected public
#include "../../../src/windows/urgency-notification-window.h"
#undef protected
#undef private

using namespace UkuiNotification;

namespace {

const QByteArray kPanelSchema("org.ukui.panel.settings");

class UrgencyNotificationWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_initialStateAndContextProperties();
    void test_setWindowPositionWithoutGSettingsUsesScreenGeometry();
    void test_setWindowPositionPanelTopLeftRightAndDefault();
    void test_setWindowPositionIgnoresIncompleteSchemaKeys();
    void test_setWindowPositionUsesWindowManagerOnNonX11();
    void test_eventExposeRefreshesWindowProperties();
    void test_showAndCloseWindowByModelRows();
    void test_closeKeepsVisibleWhenRowsRemain();

private:
    int removeGeneratedGcovFiles() const;
    QRect expectedPanelRect(int position, int size) const;
    void installPanelSettings(int position, int size) const;
};

} // namespace

void UrgencyNotificationWindowTest::init()
{
    QGSettings::resetTestData();
    QX11Info::setPlatformX11(true);
    NotificationModel::instance()->reset();
}

void UrgencyNotificationWindowTest::cleanup()
{
    NotificationModel::instance()->reset();
    QGSettings::resetTestData();
    QX11Info::setPlatformX11(false);
    removeGeneratedGcovFiles();
}

int UrgencyNotificationWindowTest::removeGeneratedGcovFiles() const
{
    int removed = 0;
    QDirIterator it(QDir::currentPath(),
                    {QStringLiteral("*.gcov")},
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (QFile::remove(it.next())) {
            ++removed;
        }
    }
    return removed;
}

QRect UrgencyNotificationWindowTest::expectedPanelRect(int position, int size) const
{
    const QRect screen = qApp->primaryScreen()->geometry();
    switch (position) {
    case 1:
        return QRect(screen.x(), screen.y() + size, screen.width(), screen.height() - size);
    case 2:
        return QRect(screen.x() + size, screen.y(), screen.width() - size, screen.height());
    case 3:
        return QRect(screen.x(), screen.y(), screen.width() - size, screen.height());
    default:
        return QRect(screen.x(), screen.y(), screen.width(), screen.height() - size);
    }
}

void UrgencyNotificationWindowTest::installPanelSettings(int position, int size) const
{
    QGSettings::installSchema(kPanelSchema, {QStringLiteral("panelposition"), QStringLiteral("panelsize")});
    QGSettings::setValue(kPanelSchema, QStringLiteral("panelposition"), position);
    QGSettings::setValue(kPanelSchema, QStringLiteral("panelsize"), size);
}

void UrgencyNotificationWindowTest::test_initialStateAndContextProperties()
{
    UrgencyNotificationWindow window;

    QCOMPARE(window.resizeMode(), UkuiQuick::SharedEngineView::SizeRootObjectToView);
    QVERIFY(window.m_urgencyModel != nullptr);
    QCOMPARE(window.m_urgencyModel->sourceModel(), NotificationModel::instance());
    QCOMPARE(window.rootContext()->contextProperty("urgencyNotificationWindow").value<QObject *>(),
             static_cast<QObject *>(&window));
    QCOMPARE(window.rootContext()->contextProperty("urgencyNotificationModel").value<QObject *>(),
             static_cast<QObject *>(window.m_urgencyModel));
}

void UrgencyNotificationWindowTest::test_setWindowPositionWithoutGSettingsUsesScreenGeometry()
{
    UrgencyNotificationWindow window;

    window.setWindowPosition();

    QCOMPARE(window.geometry(), qApp->primaryScreen()->geometry());
}

void UrgencyNotificationWindowTest::test_setWindowPositionPanelTopLeftRightAndDefault()
{
    UrgencyNotificationWindow window;

    installPanelSettings(1, 30);
    window.setWindowPosition();
    QCOMPARE(window.geometry(), expectedPanelRect(1, 30));

    installPanelSettings(2, 40);
    window.setWindowPosition();
    QCOMPARE(window.geometry(), expectedPanelRect(2, 40));

    installPanelSettings(3, 50);
    window.setWindowPosition();
    QCOMPARE(window.geometry(), expectedPanelRect(3, 50));

    installPanelSettings(0, 60);
    window.setWindowPosition();
    QCOMPARE(window.geometry(), expectedPanelRect(0, 60));
}

void UrgencyNotificationWindowTest::test_setWindowPositionIgnoresIncompleteSchemaKeys()
{
    QGSettings::installSchema(kPanelSchema, {QStringLiteral("panelposition")});
    QGSettings::setValue(kPanelSchema, QStringLiteral("panelposition"), 1);
    QGSettings::setValue(kPanelSchema, QStringLiteral("panelsize"), 40);
    UrgencyNotificationWindow window;

    window.setWindowPosition();

    QCOMPARE(window.geometry(), qApp->primaryScreen()->geometry());
}

void UrgencyNotificationWindowTest::test_setWindowPositionUsesWindowManagerOnNonX11()
{
    QX11Info::setPlatformX11(false);
    installPanelSettings(1, 24);
    UrgencyNotificationWindow window;

    window.setWindowPosition();

    QCOMPARE(window.geometry(), expectedPanelRect(1, 24));
}

void UrgencyNotificationWindowTest::test_eventExposeRefreshesWindowProperties()
{
    installPanelSettings(2, 22);
    UrgencyNotificationWindow window;

    QExposeEvent event(QRegion(window.geometry()));
    QVERIFY(window.event(&event));
}

void UrgencyNotificationWindowTest::test_showAndCloseWindowByModelRows()
{
    UrgencyNotificationWindow window;
    QList<TestNotificationData> items;
    items.append({1, QStringLiteral("mail"), false, false, true, QStringLiteral("summary"), QStringLiteral("body"), 2, QDateTime::currentDateTime()});

    NotificationModel::instance()->setItems(items);
    QCoreApplication::processEvents();
    QVERIFY(window.isVisible());

    NotificationModel::instance()->setItems({});
    QCoreApplication::processEvents();
    QVERIFY(!window.isVisible());
}

void UrgencyNotificationWindowTest::test_closeKeepsVisibleWhenRowsRemain()
{
    UrgencyNotificationWindow window;
    QList<TestNotificationData> items;
    items.append({1, QStringLiteral("mail"), false, false, true, QStringLiteral("summary"), QStringLiteral("body"), 2, QDateTime::currentDateTime()});
    items.append({2, QStringLiteral("chat"), false, false, true, QStringLiteral("summary"), QStringLiteral("body"), 2, QDateTime::currentDateTime().addSecs(1)});
    NotificationModel::instance()->setItems(items);
    QCoreApplication::processEvents();
    QVERIFY(window.isVisible());

    NotificationModel::instance()->setItems(items.mid(0, 1));
    QCoreApplication::processEvents();

    QVERIFY(window.isVisible());
    QCOMPARE(window.m_urgencyModel->rowCount(), 1);
}

QTEST_MAIN(UrgencyNotificationWindowTest)

#include "urgency-notification-window-test.moc"
