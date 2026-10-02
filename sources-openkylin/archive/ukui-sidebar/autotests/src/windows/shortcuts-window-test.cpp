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
#include <QQuickItem>

#define private public
#define protected public
#include "../../../src/windows/shortcuts-window.h"
#undef protected
#undef private

#include "QX11Info"
#include "event-track.h"
#include "global-settings.h"
#include "hand-gesture-helper.h"
#include "screen-monitor.h"
#include "short-cut-manager.h"
#include "shortcut-model.h"
#include "shared-engine-view.h"
#include "window-helper.h"

class ShortcutRootItem : public QQuickItem
{
    Q_OBJECT
public:
    Q_INVOKABLE void initTabletProp()
    {
        ++initCalls;
    }

    Q_INVOKABLE void moveShortcutPanel(const QVariant &value)
    {
        lastMove = value.toInt();
        ++moveCalls;
    }

    Q_INVOKABLE void activePanelOnTablet(const QVariant &value)
    {
        lastActive = value.toBool();
        ++activeCalls;
    }

    int initCalls = 0;
    int moveCalls = 0;
    int activeCalls = 0;
    int lastMove = -1;
    bool lastActive = false;
};

class shortcutsWindow_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_staticHitHelpers();
    void test_initialSettingsAndProperties();
    void test_currentId_signalOnlyOnChange();
    void test_updateGeometry_panelPositions();
    void test_updateGeometry_islandAndRtl();
    void test_updateGeometry_rootObjectSizeAndRtlPanelBranches();
    void test_globalSettingsSignalsCoverAllBranches();
    void test_screenRemovedAndFocusLossHideWindow();
    void test_layoutDirectionSignalUpdatesState();
    void test_tabletModeEffectsAndGestureConnections();
    void test_shortcutModelRequestExecActionHidesWindow();
    void test_moveShortcutPanel_andRelease();
    void test_moveShortcutPanel_emptyRootVisibleAndRtlBranches();
    void test_rightToLeftRelease_inactiveAndNoRootBranches();
    void test_activeShortcutsWindow_branches();
    void test_activeShortcutsWindow_nonTabletActivateAndSameState();
    void test_requestMenuWidget_andBack();
    void test_eventShow_x11Branch();
    void test_setBlurStrength();
};

void shortcutsWindow_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->reset();
    Sidebar::ScreenMonitor::getInstance()->reset();
    Sidebar::HandGestureHelper::getInstance()->reset();
    Sidebar::EventTrack::reset();
    UkuiShortcut::ShortcutManager::getInstance()->reset();
    UkuiQuick::WindowProxy::setCurrentScreen(qApp->primaryScreen());
    QX11Info::setPlatformX11(false);

    auto *settings = Sidebar::GlobalSettings::globalInstance();
    settings->setValue(TABLET_MODE, false);
    settings->setValue(UKUI_PANEL_POSITION_KEY, 0);
    settings->setValue(UKUI_PANEL_SIZE_KEY, 48);
    settings->setValue(UKUI_PANEL_TYPE_KEY, 0);
    settings->setValue(UKUI_SETTINGS_ISLAND_POSITION_KEY, 0);
    settings->setValue(UKUI_TOPBAR_SIZE_KEY, 24);
    settings->setPanelLength(qApp->primaryScreen()->name(), qApp->primaryScreen()->geometry().width() / 2);
}

void shortcutsWindow_test::test_staticHitHelpers()
{
    QVERIFY(ShortcutsWindow::inTargetCircle(3, 4, 0, 0, 5));
    QVERIFY(!ShortcutsWindow::inTargetCircle(6, 0, 0, 0, 5));
    QVERIFY(ShortcutsWindow::inTargetRect(5, 5, QRectF(0, 0, 10, 10)));
    QVERIFY(!ShortcutsWindow::inTargetRect(15, 5, QRectF(0, 0, 10, 10)));
}

void shortcutsWindow_test::test_initialSettingsAndProperties()
{
    ShortcutsWindow window;

    QCOMPARE(window.isTabletMode(), false);
    QCOMPARE(window.windowMargin(), 8);
    QCOMPARE(window.panelPos(), 0);
    QCOMPARE(window.m_panelSize, 48);
    QCOMPARE(window.m_panelType, 0);
    QCOMPARE(window.m_windowProxy->windowType(), UkuiQuick::WindowType::SystemWindow);
}

void shortcutsWindow_test::test_currentId_signalOnlyOnChange()
{
    ShortcutsWindow window;
    QSignalSpy spy(&window, &ShortcutsWindow::currentIdChanged);

    window.setCurrentId("network");
    window.setCurrentId("network");

    QCOMPARE(window.currentId(), QString("network"));
    QCOMPARE(spy.count(), 1);
}

void shortcutsWindow_test::test_updateGeometry_panelPositions()
{
    ShortcutsWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.resize(100, 80);
    const QRect rect = screen->geometry();

    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 1);
    QCOMPARE(window.m_panelPos, 1);
    QCOMPARE(window.geometry().y(), rect.top() + window.windowMargin() + window.m_panelSize);
    QCOMPARE(window.m_windowProxy->lastSlideEdge(), UkuiQuick::WindowProxy::TopEdge);

    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 2);
    QCOMPARE(window.geometry().x(), rect.left() + window.windowMargin() + window.m_panelSize);
    QCOMPARE(window.m_windowProxy->lastSlideEdge(), UkuiQuick::WindowProxy::LeftEdge);

    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 3);
    QCOMPARE(window.geometry().x(), rect.right() - window.width() - window.windowMargin() - window.m_panelSize);
    QCOMPARE(window.m_windowProxy->lastSlideEdge(), UkuiQuick::WindowProxy::RightEdge);
}

void shortcutsWindow_test::test_updateGeometry_islandAndRtl()
{
    ShortcutsWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.resize(100, 80);
    window.m_layoutDirection = Qt::RightToLeft;

    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_PANEL_TYPE_KEY, 1);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_SETTINGS_ISLAND_POSITION_KEY, 1);

    QCOMPARE(window.geometry().x(), screen->geometry().left() + window.windowMargin());
    QCOMPARE(window.geometry().y(), screen->geometry().top() + window.windowMargin() + window.m_topbarSize);
    QCOMPARE(window.m_windowProxy->lastSlideEdge(), UkuiQuick::WindowProxy::TopEdge);
}

void shortcutsWindow_test::test_updateGeometry_rootObjectSizeAndRtlPanelBranches()
{
    ShortcutsWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.resize(100, 80);

    auto *root = new ShortcutRootItem;
    root->setSize(QSizeF(240, 180));
    window.setTestRootObject(root);
    window.m_layoutDirection = Qt::RightToLeft;

    window.m_panelType = 0;
    window.m_panelPos = 0;
    window.updateGeometry();
    QCOMPARE(window.geometry().size(), QSize(240, 180));
    QCOMPARE(window.geometry().x(), screen->geometry().left() + window.windowMargin());

    window.m_panelPos = 1;
    window.updateGeometry();
    QCOMPARE(window.geometry().x(), screen->geometry().left() + window.windowMargin());

    window.m_layoutDirection = Qt::LeftToRight;
    window.m_panelType = 1;
    window.m_settingsIslandPosition = 0;
    window.updateGeometry();
    QCOMPARE(window.m_windowProxy->lastSlideEdge(), UkuiQuick::WindowProxy::BottomEdge);

    window.m_settingsIslandPosition = 1;
    window.updateGeometry();
    QCOMPARE(window.m_windowProxy->lastSlideEdge(), UkuiQuick::WindowProxy::TopEdge);
}

void shortcutsWindow_test::test_globalSettingsSignalsCoverAllBranches()
{
    ShortcutsWindow window;
    window.resize(100, 80);
    QSignalSpy panelSpy(&window, &ShortcutsWindow::panelPosChanged);
    auto *settings = Sidebar::GlobalSettings::globalInstance();

    settings->setValue(UKUI_PANEL_SIZE_KEY, 64);
    QCOMPARE(window.m_panelSize, 64);

    settings->setValue(UKUI_PANEL_TYPE_KEY, 1);
    QCOMPARE(window.m_panelType, 1);

    settings->setValue(UKUI_SETTINGS_ISLAND_POSITION_KEY, 1);
    QCOMPARE(window.m_settingsIslandPosition, 1);

    settings->setValue(UKUI_TOPBAR_SIZE_KEY, 36);
    QCOMPARE(window.m_topbarSize, 36);

    const QRect before = window.geometry();
    settings->setPanelLength(qApp->primaryScreen()->name(), 321);
    QVERIFY(window.geometry().isValid() || !before.isValid());

    settings->setValue(UKUI_PANEL_POSITION_KEY, 2);
    QCOMPARE(window.m_panelPos, 2);
    QCOMPARE(panelSpy.count(), 1);
}

void shortcutsWindow_test::test_screenRemovedAndFocusLossHideWindow()
{
    ShortcutsWindow window;
    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    window.setVisible(true);

    Q_EMIT Sidebar::ScreenMonitor::getInstance()->screenRemoved(qApp->primaryScreen());
    QCOMPARE(window.isVisible(), false);

    window.setVisible(true);
    Q_EMIT window.activeFocusItemChanged();
    QCOMPARE(window.isVisible(), false);
}

void shortcutsWindow_test::test_layoutDirectionSignalUpdatesState()
{
    ShortcutsWindow window;

    QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    QCOMPARE(window.m_layoutDirection, Qt::RightToLeft);

    QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    QCOMPARE(window.m_layoutDirection, Qt::LeftToRight);
}

void shortcutsWindow_test::test_tabletModeEffectsAndGestureConnections()
{
    ShortcutsWindow window;
    QSignalSpy spy(&window, &ShortcutsWindow::tabletModeChanged);

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    QCOMPARE(window.isTabletMode(), true);
    QCOMPARE(window.resizeMode(), UkuiQuick::SharedEngineView::SizeRootObjectToView);
    QCOMPARE(spy.count(), 1);

    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    Sidebar::HandGestureHelper::getInstance()->callControlCenter(10);
    QCOMPARE(root->moveCalls, 1);
}

void shortcutsWindow_test::test_shortcutModelRequestExecActionHidesWindow()
{
    ShortcutsWindow window;
    window.setVisible(true);
    auto *model = window.findChild<UkuiShortcut::ShortcutModel *>();
    QVERIFY(model != nullptr);

    Q_EMIT model->requestExecAction(UkuiShortcut::PluginMetaType::NoAction);
    QCOMPARE(window.isVisible(), true);

    Q_EMIT model->requestExecAction(UkuiShortcut::PluginMetaType::Hide);
    QCOMPARE(window.isVisible(), false);
}

void shortcutsWindow_test::test_moveShortcutPanel_andRelease()
{
    ShortcutsWindow window;
    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    window.m_isTabletMode = true;
    window.resize(200, 100);
    window.setGeometry(100, 0, 200, 100);

    window.moveShortcutPanel(260);
    QCOMPARE(root->initCalls, 1);
    QCOMPARE(root->lastMove, qMax(0, window.geometry().right() - 260));

    window.onRight2LeftReleased(260, 10);
    QCOMPARE(root->activeCalls, 1);
    QCOMPARE(root->lastActive, true);
    QCOMPARE(Sidebar::EventTrack::lastCode(), QString("open_sidebar"));
}

void shortcutsWindow_test::test_moveShortcutPanel_emptyRootVisibleAndRtlBranches()
{
    ShortcutsWindow window;
    window.moveShortcutPanel(10);

    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    window.m_isTabletMode = true;
    window.m_layoutDirection = Qt::RightToLeft;
    window.resize(200, 100);
    window.setGeometry(100, 0, 200, 100);
    window.setVisible(true);

    window.moveShortcutPanel(45);
    QCOMPARE(root->initCalls, 0);
    QCOMPARE(root->lastMove, 45);

    window.m_layoutDirection = Qt::LeftToRight;
    window.moveShortcutPanel(2000);
    QCOMPARE(root->lastMove, 0);
}

void shortcutsWindow_test::test_rightToLeftRelease_inactiveAndNoRootBranches()
{
    ShortcutsWindow window;
    window.onRight2LeftReleased(10, 10);
    QVERIFY(Sidebar::EventTrack::lastCode().isEmpty());

    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    window.resize(200, 100);
    window.setGeometry(100, 0, 200, 100);

    window.onRight2LeftReleased(window.geometry().right(), 10);
    QCOMPARE(root->activeCalls, 1);
    QCOMPARE(root->lastActive, false);
    QVERIFY(Sidebar::EventTrack::lastCode().isEmpty());

    window.m_layoutDirection = Qt::RightToLeft;
    window.onRight2LeftReleased(50, 10);
    QCOMPARE(root->lastActive, true);
    QCOMPARE(Sidebar::EventTrack::lastCode(), QString("open_sidebar"));
}

void shortcutsWindow_test::test_activeShortcutsWindow_branches()
{
    ShortcutsWindow window;

    window.activeShortcutsWindow(true);
    QCOMPARE(window.isVisible(), false);

    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    window.m_isTabletMode = true;
    window.activeShortcutsWindow(true);
    QCOMPARE(root->initCalls, 1);
    QCOMPARE(root->activeCalls, 1);

    window.activeShortcutsWindow(false);
    QCOMPARE(root->lastActive, false);

    window.m_isTabletMode = false;
    window.setVisible(true);
    window.activeShortcutsWindow(false);
    QCOMPARE(window.isVisible(), false);
}

void shortcutsWindow_test::test_activeShortcutsWindow_nonTabletActivateAndSameState()
{
    ShortcutsWindow window;
    auto *root = new ShortcutRootItem;
    window.setTestRootObject(root);
    window.m_isTabletMode = false;
    window.m_screen = nullptr;
    UkuiQuick::WindowProxy::setCurrentScreen(qApp->primaryScreen());

    window.activeShortcutsWindow(true);
    QCOMPARE(window.isVisible(), true);
    QCOMPARE(window.m_screen, qApp->primaryScreen());

    const QPoint position = window.m_position;
    window.activeShortcutsWindow(true);
    QCOMPARE(window.m_position, position);

    window.activeShortcutsWindow(false);
    QCOMPARE(window.isVisible(), false);
}

void shortcutsWindow_test::test_requestMenuWidget_andBack()
{
    ShortcutsWindow window;
    QSignalSpy showSpy(&window, &ShortcutsWindow::showMenuWidget);
    QSignalSpy hideSpy(&window, &ShortcutsWindow::hideMenuWidget);

    QVERIFY(!window.requestMenuWidget("missing", false));

    UkuiShortcut::ShortcutManager::getInstance()->setWidgetInfo("wifi", "Back");
    QVERIFY(window.requestMenuWidget("wifi", true));
    QCOMPARE(showSpy.count(), 1);
    QCOMPARE(showSpy.at(0).at(0).toString(), QString("wifi"));
    QCOMPARE(showSpy.at(0).at(1).toBool(), true);
    QCOMPARE(showSpy.at(0).at(2).toString(), QString("Back"));

    window.backToShortcuts();
    QCOMPARE(hideSpy.count(), 1);
}

void shortcutsWindow_test::test_eventShow_x11Branch()
{
    ShortcutsWindow window;
    QShowEvent event;

    QX11Info::setPlatformX11(false);
    QVERIFY(window.event(&event));

    QX11Info::setPlatformX11(true);
    QVERIFY(window.event(&event));
}

void shortcutsWindow_test::test_setBlurStrength()
{
    ShortcutsWindow window;
    window.setBlurStrength();

    QCOMPARE(window.m_windowProxy->blurEnabled(), true);
}

QTEST_MAIN(shortcutsWindow_test)

#include "shortcuts-window-test.moc"
