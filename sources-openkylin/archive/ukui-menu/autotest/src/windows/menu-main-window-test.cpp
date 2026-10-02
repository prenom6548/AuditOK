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

#define protected public
#include "../../../src/windows/menu-main-window.h"
#undef protected

#include <QtTest>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QQmlContext>
#include <QSignalSpy>

#include "../settings/fakes/qgsettings.h"
#include "fakes/context-menu-manager.h"
#include "fakes/security-function-control.h"
#include "fakes/settings.h"
#include "fakes/window-helper.h"

using namespace UkuiMenu;

namespace UkuiMenu {

GlobalSetting *GlobalSetting::instance()
{
    static GlobalSetting setting;
    return &setting;
}

GlobalSetting::GlobalSetting(QObject *parent)
    : QObject(parent)
{
    m_cache.insert(Transparency, 0.45);
    m_cache.insert(EffectEnabled, true);
    m_cache.insert(IsLiteMode, false);
}

QVariant GlobalSetting::get(const GlobalSetting::Key &key)
{
    return m_cache.value(key);
}

void GlobalSetting::setValue(const GlobalSetting::Key &key, const QVariant &value)
{
    m_cache.insert(key, value);
}

void GlobalSetting::emitStyleChanged(const GlobalSetting::Key &key)
{
    Q_EMIT styleChanged(key);
}

MenuSetting *MenuSetting::instance()
{
    static MenuSetting setting;
    return &setting;
}

MenuSetting::MenuSetting(QObject *parent)
    : QObject(parent)
{
    reset();
}

QVariant MenuSetting::get(const QString &key)
{
    return m_cache.value(key);
}

bool MenuSetting::set(const QString &key, const QVariant &value)
{
    m_cache.insert(key, value);
    return true;
}

void MenuSetting::reset()
{
    m_cache.insert(QStringLiteral(FOLLOW_UKUI_PANEL), true);
    m_cache.insert(QStringLiteral(MENU_WIDTH), 320);
    m_cache.insert(QStringLiteral(MENU_HEIGHT), 240);
    m_cache.insert(QStringLiteral(MENU_MARGIN), 12);
}

ContextMenuManager *ContextMenuManager::instance()
{
    static ContextMenuManager manager;
    return &manager;
}

ContextMenuManager::ContextMenuManager()
    : QObject(nullptr)
{
}

void ContextMenuManager::setMainWindow(QWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

bool ContextMenuManager::closeMenu()
{
    ++m_closeCount;
    return true;
}

void ContextMenuManager::reset()
{
    m_mainWindow = nullptr;
    m_closeCount = 0;
}

int ContextMenuManager::closeCount() const
{
    return m_closeCount;
}

QWindow *ContextMenuManager::mainWindow() const
{
    return m_mainWindow;
}

SecurityFunctionControl *SecurityFunctionControl::instance()
{
    static SecurityFunctionControl control;
    return &control;
}

SecurityFunctionControl::SecurityFunctionControl(QObject *parent)
    : QObject(parent)
{
}

bool SecurityFunctionControl::disable() const { return m_disable; }
bool SecurityFunctionControl::disableMouseLeftButton() const { return false; }
bool SecurityFunctionControl::disableMouseRightButton() const { return false; }
bool SecurityFunctionControl::disableMouseHover() const { return false; }
QStringList SecurityFunctionControl::appWhiteList() const { return {}; }
QStringList SecurityFunctionControl::appBlackList() const { return {}; }
bool SecurityFunctionControl::disableUninstallAction() const { return false; }
bool SecurityFunctionControl::windowMaximize() const { return m_windowMaximize; }
bool SecurityFunctionControl::showFoldButton() const { return true; }
QString SecurityFunctionControl::powerButtonSoleOperation() const { return {}; }
bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const { Q_UNUSED(desktop); return true; }

void SecurityFunctionControl::setDisable(bool value)
{
    m_disable = value;
}

void SecurityFunctionControl::setWindowMaximize(bool value)
{
    if (m_windowMaximize == value) {
        return;
    }

    m_windowMaximize = value;
    Q_EMIT windowMaximizeChanged();
}

} // namespace UkuiMenu

namespace UkuiQuick {

QScreen *WindowProxy::s_currentScreen = nullptr;

WindowProxy::WindowProxy(QWindow *window, Operations operations)
    : QObject(window)
{
    Q_UNUSED(operations);
}

void WindowProxy::setPosition(const QPoint &point)
{
    m_position = point;
}

void WindowProxy::setGeometry(const QRect &rect)
{
    m_geometry = rect;
}

void WindowProxy::setWindowType(WindowType::Type type)
{
    m_windowType = type;
}

void WindowProxy::slideWindow(WindowProxy::SlideFromEdge fromEdge, int offset)
{
    m_slideEdge = fromEdge;
    m_slideOffset = offset;
}

void WindowProxy::setBlurRegion(bool enable, const QRegion &region)
{
    m_blurEnabled = enable;
    m_blurRegion = region;
}

QScreen *WindowProxy::currentScreen()
{
    return s_currentScreen ? s_currentScreen : QGuiApplication::primaryScreen();
}

void WindowProxy::setCurrentScreen(QScreen *screen)
{
    s_currentScreen = screen;
}

void WindowProxy::setWindowStates(WindowState::States states, bool enable)
{
    if (enable) {
        m_windowStates = states;
    } else {
        m_windowStates = {};
    }
}

void WindowProxy::setRestoreGeometry(const QRect &rect)
{
    m_restoreGeometry = rect;
}

QPoint WindowProxy::position() const { return m_position; }
QRect WindowProxy::geometry() const { return m_geometry; }
QRect WindowProxy::restoreGeometry() const { return m_restoreGeometry; }
WindowProxy::SlideFromEdge WindowProxy::slideEdge() const { return m_slideEdge; }
int WindowProxy::slideOffset() const { return m_slideOffset; }
bool WindowProxy::blurEnabled() const { return m_blurEnabled; }
QRegion WindowProxy::blurRegion() const { return m_blurRegion; }
WindowType::Type WindowProxy::windowType() const { return m_windowType; }
WindowState::States WindowProxy::windowStates() const { return m_windowStates; }

} // namespace UkuiQuick

namespace {

class TestMenuWindow : public MenuWindow
{
public:
    using MenuWindow::MenuWindow;

    void callInit() { init(); }
    void callInitPanelSetting() { initPanelSetting(); }
    void callUpdateCurrentScreenGeometry() { updateCurrentScreenGeometry(); }
};

class menuMainWindow_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_basePropertiesAndSignals();
    void test_mousePressEvent_closesContextMenu();
    void test_init_bindsContextAndMainWindow();
    void test_initPanelSetting_readsValuesAndReactsToChanges();
    void test_updateCurrentScreenGeometry_skipsWhenScreenInvalid();
    void test_updateCurrentScreenGeometry_classicBottom();
    void test_updateCurrentScreenGeometry_classicRightAndMirroredTop();
    void test_updateCurrentScreenGeometry_islandModes();
    void test_waylandActiveMenuWindow_disableBranchAndUpdateGeometry();
    void test_waylandFullscreenAndSecuritySignal();
    void test_xcbFullscreenBlurAndActivation();
};

void menuMainWindow_test::init()
{
    FakeQGSettingsStore::reset();
    MenuSetting::instance()->reset();
    GlobalSetting::instance()->setValue(GlobalSetting::Transparency, 0.45);
    GlobalSetting::instance()->setValue(GlobalSetting::EffectEnabled, true);
    GlobalSetting::instance()->setValue(GlobalSetting::IsLiteMode, false);
    ContextMenuManager::instance()->reset();
    SecurityFunctionControl::instance()->setDisable(false);
    SecurityFunctionControl::instance()->setWindowMaximize(true);
    UkuiQuick::WindowProxy::setCurrentScreen(QGuiApplication::primaryScreen());
    qGuiApp->setLayoutDirection(Qt::LeftToRight);
}

void menuMainWindow_test::test_basePropertiesAndSignals()
{
    TestMenuWindow window;
    QSignalSpy editSpy(&window, &MenuWindow::editModeChanged);
    QSignalSpy effectSpy(&window, &MenuWindow::effectEnabledChanged);
    QSignalSpy transparencySpy(&window, &MenuWindow::transparencyChanged);

    QCOMPARE(window.panelPos(), 4);
    QCOMPARE(window.transparency(), 0.45);
    QCOMPARE(window.effectEnabled(), true);
    QCOMPARE(window.windowVisible(), false);
    QCOMPARE(window.editMode(), false);
    QCOMPARE(window.isFullScreen(), false);

    window.setEditMode(true);
    window.setEditMode(true);
    QCOMPARE(window.editMode(), true);
    QCOMPARE(editSpy.count(), 1);

    GlobalSetting::instance()->setValue(GlobalSetting::EffectEnabled, false);
    GlobalSetting::instance()->emitStyleChanged(GlobalSetting::EffectEnabled);
    GlobalSetting::instance()->setValue(GlobalSetting::Transparency, 0.75);
    GlobalSetting::instance()->emitStyleChanged(GlobalSetting::Transparency);

    QCOMPARE(effectSpy.count(), 0);
    QCOMPARE(transparencySpy.count(), 0);

    window.callInit();
    GlobalSetting::instance()->setValue(GlobalSetting::EffectEnabled, false);
    GlobalSetting::instance()->emitStyleChanged(GlobalSetting::EffectEnabled);
    GlobalSetting::instance()->setValue(GlobalSetting::Transparency, 0.75);
    GlobalSetting::instance()->emitStyleChanged(GlobalSetting::Transparency);

    QCOMPARE(effectSpy.count(), 1);
    QCOMPARE(transparencySpy.count(), 1);
    QCOMPARE(window.effectEnabled(), false);
    QCOMPARE(window.transparency(), 0.75);
}

void menuMainWindow_test::test_mousePressEvent_closesContextMenu()
{
    TestMenuWindow window;
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    QCOMPARE(ContextMenuManager::instance()->closeCount(), 0);
    window.event(&pressEvent);
    QCOMPARE(ContextMenuManager::instance()->closeCount(), 1);
}

void menuMainWindow_test::test_init_bindsContextAndMainWindow()
{
    TestMenuWindow window;

    window.callInit();

    QCOMPARE(window.rootContext()->contextProperty(QStringLiteral("mainWindow")).value<QObject *>(), &window);
    QCOMPARE(window.rootContext()->contextProperty(QStringLiteral("isLiteMode")).toBool(), false);
    QCOMPARE(ContextMenuManager::instance()->mainWindow(), static_cast<QWindow *>(&window));
    QVERIFY(window.m_windowProxy != nullptr);
    QVERIFY(window.m_screen != nullptr);
    QVERIFY(window.title().contains(QCoreApplication::applicationName()));
}

void menuMainWindow_test::test_initPanelSetting_readsValuesAndReactsToChanges()
{
    FakeQGSettingsStore::setSchemaInstalled("org.ukui.panel.settings", true);
    FakeQGSettingsStore::setSchemaData("org.ukui.panel.settings", {
        { QStringLiteral("panelposition"), 1 },
        { QStringLiteral("panelsize"), 56 },
        { QStringLiteral("paneltype"), 1 },
        { QStringLiteral("dataislandposition"), 1 },
        { QStringLiteral("topbarsize"), 38 }
    });

    TestMenuWindow window;
    window.callInit();

    QCOMPARE(window.m_panelPos, 1);
    QCOMPARE(window.m_panelSize, 56);
    QCOMPARE(window.m_panelType, 1);
    QCOMPARE(window.m_dataIslandPosition, 1);
    QCOMPARE(window.m_topbarSize, 38);

    auto *panelSettings = window.findChild<QGSettings *>();
    QVERIFY(panelSettings != nullptr);

    FakeQGSettingsStore::setSchemaData("org.ukui.panel.settings", {
        { QStringLiteral("panelposition"), 3 },
        { QStringLiteral("panelsize"), 60 },
        { QStringLiteral("paneltype"), 0 },
        { QStringLiteral("dataislandposition"), 0 },
        { QStringLiteral("topbarsize"), 42 }
    });

    emit panelSettings->changed(QStringLiteral("panelposition"));
    emit panelSettings->changed(QStringLiteral("panelsize"));
    emit panelSettings->changed(QStringLiteral("paneltype"));
    emit panelSettings->changed(QStringLiteral("dataislandposition"));
    emit panelSettings->changed(QStringLiteral("topbarsize"));
    emit panelSettings->changed(QStringLiteral("panellength"));

    QCOMPARE(window.m_panelPos, 3);
    QCOMPARE(window.m_panelSize, 60);
    QCOMPARE(window.m_panelType, 0);
    QCOMPARE(window.m_dataIslandPosition, 0);
    QCOMPARE(window.m_topbarSize, 42);
}

void menuMainWindow_test::test_updateCurrentScreenGeometry_skipsWhenScreenInvalid()
{
    TestMenuWindow window;
    window.m_windowProxy = new UkuiQuick::WindowProxy(&window);
    window.m_screen = nullptr;

    window.callUpdateCurrentScreenGeometry();

    QCOMPARE(window.m_normalGeometry, QRect(0, 0, 0, 0));
    QCOMPARE(static_cast<UkuiQuick::WindowProxy *>(window.m_windowProxy)->slideEdge(), UkuiQuick::WindowProxy::NoEdge);
}

void menuMainWindow_test::test_updateCurrentScreenGeometry_classicBottom()
{
    TestMenuWindow window;
    window.m_windowProxy = new UkuiQuick::WindowProxy(&window);
    window.m_screen = QGuiApplication::primaryScreen();
    window.m_panelType = 0;
    window.m_panelPos = 0;
    window.m_panelSize = 48;
    MenuSetting::instance()->set(QStringLiteral(MENU_WIDTH), 320);
    MenuSetting::instance()->set(QStringLiteral(MENU_HEIGHT), 240);
    MenuSetting::instance()->set(QStringLiteral(MENU_MARGIN), 10);

    const QRect screenRect = window.m_screen->geometry();
    window.callUpdateCurrentScreenGeometry();

    const QRect expectedFullRect = QRect(screenRect.x(), screenRect.y(), screenRect.width(), screenRect.height() - 48);
    const QSize expectedSize(qMin(expectedFullRect.width() - 20, 320), qMin(expectedFullRect.height() - 20, 240));
    const QRect expectedMask(QPoint(10, expectedFullRect.height() - expectedSize.height() - 10), expectedSize);

    QCOMPARE(window.m_fullScreenGeometry, expectedFullRect);
    QCOMPARE(window.m_normalMaskGeometry, expectedMask);
    QCOMPARE(window.m_normalGeometry, QRect(expectedMask.topLeft() + expectedFullRect.topLeft(), expectedMask.size()));
    QCOMPARE(static_cast<UkuiQuick::WindowProxy *>(window.m_windowProxy)->slideEdge(), UkuiQuick::WindowProxy::BottomEdge);
}

void menuMainWindow_test::test_updateCurrentScreenGeometry_classicRightAndMirroredTop()
{
    TestMenuWindow rightWindow;
    rightWindow.m_windowProxy = new UkuiQuick::WindowProxy(&rightWindow);
    rightWindow.m_screen = QGuiApplication::primaryScreen();
    rightWindow.m_panelType = 0;
    rightWindow.m_panelPos = 3;
    rightWindow.m_panelSize = 42;
    MenuSetting::instance()->set(QStringLiteral(MENU_WIDTH), 300);
    MenuSetting::instance()->set(QStringLiteral(MENU_HEIGHT), 220);
    MenuSetting::instance()->set(QStringLiteral(MENU_MARGIN), 8);

    const QRect rightScreenRect = rightWindow.m_screen->geometry();
    rightWindow.callUpdateCurrentScreenGeometry();
    QCOMPARE(static_cast<UkuiQuick::WindowProxy *>(rightWindow.m_windowProxy)->slideEdge(), UkuiQuick::WindowProxy::RightEdge);
    const QRect rightFullRect(rightScreenRect.x(), rightScreenRect.y(), rightScreenRect.width() - 42, rightScreenRect.height());
    const QSize rightSize(qMin(rightFullRect.width() - 16, 300), qMin(rightFullRect.height() - 16, 220));
    QCOMPARE(rightWindow.m_normalMaskGeometry.x(), rightFullRect.width() - rightSize.width() - 8);

    TestMenuWindow topWindow;
    topWindow.m_windowProxy = new UkuiQuick::WindowProxy(&topWindow);
    topWindow.m_screen = QGuiApplication::primaryScreen();
    topWindow.m_panelType = 0;
    topWindow.m_panelPos = 1;
    topWindow.m_panelSize = 36;
    qGuiApp->setLayoutDirection(Qt::RightToLeft);

    const QRect topScreenRect = topWindow.m_screen->geometry();
    topWindow.callUpdateCurrentScreenGeometry();

    const QRect fullRect = QRect(topScreenRect.x(), topScreenRect.y() + 36, topScreenRect.width(), topScreenRect.height() - 36);
    const QSize normalSize(qMin(fullRect.width() - 16, 300), qMin(fullRect.height() - 16, 220));
    QCOMPARE(topWindow.m_normalMaskGeometry.x(), fullRect.width() - 8 - normalSize.width());
    QCOMPARE(static_cast<UkuiQuick::WindowProxy *>(topWindow.m_windowProxy)->slideEdge(), UkuiQuick::WindowProxy::TopEdge);
}

void menuMainWindow_test::test_updateCurrentScreenGeometry_islandModes()
{
    TestMenuWindow bottomIsland;
    bottomIsland.m_windowProxy = new UkuiQuick::WindowProxy(&bottomIsland);
    bottomIsland.m_screen = QGuiApplication::primaryScreen();
    bottomIsland.m_panelType = 1;
    bottomIsland.m_panelSize = 44;
    bottomIsland.m_dataIslandPosition = 0;
    bottomIsland.callUpdateCurrentScreenGeometry();
    QCOMPARE(static_cast<UkuiQuick::WindowProxy *>(bottomIsland.m_windowProxy)->slideEdge(), UkuiQuick::WindowProxy::BottomEdge);

    TestMenuWindow topIsland;
    topIsland.m_windowProxy = new UkuiQuick::WindowProxy(&topIsland);
    topIsland.m_screen = QGuiApplication::primaryScreen();
    topIsland.m_panelType = 1;
    topIsland.m_topbarSize = 30;
    topIsland.m_dataIslandPosition = 1;
    qGuiApp->setLayoutDirection(Qt::RightToLeft);
    topIsland.callUpdateCurrentScreenGeometry();
    QCOMPARE(static_cast<UkuiQuick::WindowProxy *>(topIsland.m_windowProxy)->slideEdge(), UkuiQuick::WindowProxy::TopEdge);
}

void menuMainWindow_test::test_waylandActiveMenuWindow_disableBranchAndUpdateGeometry()
{
    SecurityFunctionControl::instance()->setDisable(true);
    MenuWindowWayland window;
    QSignalSpy visibleSpy(&window, &MenuWindow::windowVisibleChanged);

    window.activeMenuWindow(true);
    QCOMPARE(window.windowVisible(), true);
    QCOMPARE(window.isVisible(), false);
    QCOMPARE(visibleSpy.count(), 0);

    SecurityFunctionControl::instance()->setDisable(false);
    window.activeMenuWindow(false);
    window.activeMenuWindow(true);

    QCOMPARE(window.windowVisible(), true);
    QCOMPARE(window.isVisible(), true);
    QVERIFY(window.m_windowProxy != nullptr);
    QVERIFY(static_cast<UkuiQuick::WindowProxy *>(window.m_windowProxy)->blurEnabled());
    QVERIFY(!static_cast<UkuiQuick::WindowProxy *>(window.m_windowProxy)->geometry().isNull());
}

void menuMainWindow_test::test_waylandFullscreenAndSecuritySignal()
{
    SecurityFunctionControl::instance()->setWindowMaximize(false);
    MenuWindowWayland window;
    auto *proxy = static_cast<UkuiQuick::WindowProxy *>(window.m_windowProxy);
    QVERIFY(proxy != nullptr);

    window.m_isFullScreen = true;
    window.setFullScreen(false);
    QCOMPARE(proxy->restoreGeometry(), window.m_normalGeometry);

    SecurityFunctionControl::instance()->setWindowMaximize(true);
    QCoreApplication::processEvents();
    SecurityFunctionControl::instance()->setWindowMaximize(false);
    QVERIFY(proxy->restoreGeometry().isValid());
}

void menuMainWindow_test::test_xcbFullscreenBlurAndActivation()
{
    SecurityFunctionControl::instance()->setWindowMaximize(false);
    MenuWindowXcb window;
    QSignalSpy beforeSpy(&window, &MenuWindowXcb::beforeFullScreenChanged);
    QSignalSpy fullSpy(&window, &MenuWindow::fullScreenChanged);

    window.setFullScreen(true);
    QCOMPARE(beforeSpy.count(), 1);
    QCOMPARE(fullSpy.count(), 1);
    QCOMPARE(window.mask().boundingRect(), QRect(0, 0, window.m_fullScreenGeometry.width(), window.m_fullScreenGeometry.height()));

    window.changeWindowBlurRegion(1, 2, 50, 40, 8);
    auto *proxy = static_cast<UkuiQuick::WindowProxy *>(window.m_windowProxy);
    QVERIFY(proxy->blurEnabled());
    QVERIFY(!proxy->blurRegion().isEmpty());

    window.activeMenuWindow(true);
    QCOMPARE(window.windowVisible(), true);
    QCOMPARE(window.isVisible(), true);
    QCOMPARE(proxy->position(), window.m_fullScreenGeometry.topLeft());

    QSignalSpy exitSpy(&window, &MenuWindowXcb::beforeFullScreenExited);
    window.exitFullScreen();
    QCOMPARE(exitSpy.count(), 1);
}

} // namespace

QTEST_MAIN(menuMainWindow_test)

#include "menu-main-window-test.moc"
