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

#define private public
#include "../../src/ukui-menu-application.h"
#undef private

#include <QtTest>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include <QSignalSpy>

#include "application/fakes/settings.h"
#include "application/fakes/power-button.h"
#include "application/fakes/sidebar-button-utils.h"
#include "application/fakes/mask-mouse-area.h"
#include "application/fakes/app-manager.h"
#include "application/fakes/context-menu-manager.h"
#include "application/fakes/security-function-control.h"
#include "application/fakes/app-page-backend.h"
#include "application/fakes/menu-main-window.h"
#include "application/fakes/menu-dbus-service.h"
#include "application/fakes/keep-alive-client.h"
#include "application/fakes/favorite/folder-model.h"
#include "application/fakes/favorite/favorite-filter-model.h"

using namespace UkuiMenu;

Q_DECLARE_METATYPE(UkuiMenu::UkuiMenuApplication::Command)

namespace UkuiMenu {

static QList<QByteArray> g_definedModules;

void WindowModule::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    g_definedModules.append(QByteArray("WindowModule@") + QByteArray(uri) + ':' + QByteArray::number(versionMajor) + '.' + QByteArray::number(versionMinor));
}

void SettingModule::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    g_definedModules.append(QByteArray(uri) + ':' + QByteArray::number(versionMajor) + '.' + QByteArray::number(versionMinor));
}

void PowerButton::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    g_definedModules.append(QByteArray("PowerButton@") + QByteArray(uri) + ':' + QByteArray::number(versionMajor) + '.' + QByteArray::number(versionMinor));
}

void SidebarButtonUtils::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    g_definedModules.append(QByteArray("SidebarButtonUtils@") + QByteArray(uri) + ':' + QByteArray::number(versionMajor) + '.' + QByteArray::number(versionMinor));
}

void MaskMouseArea::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    g_definedModules.append(QByteArray("MaskMouseArea@") + QByteArray(uri) + ':' + QByteArray::number(versionMajor) + '.' + QByteArray::number(versionMinor));
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

QVariant MenuSetting::get(const QString &key) const
{
    return m_cache.value(key);
}

bool MenuSetting::set(const QString &key, const QVariant &value)
{
    m_cache.insert(key, value);
    Q_EMIT changed(key);
    return true;
}

void MenuSetting::reset()
{
    m_cache.insert(QStringLiteral(FOLLOW_UKUI_PANEL), true);
    m_cache.insert(QStringLiteral(MENU_WIDTH), 320);
    m_cache.insert(QStringLiteral(MENU_HEIGHT), 240);
    m_cache.insert(QStringLiteral(MENU_MARGIN), 16);
}

AppManager *AppManager::instance()
{
    static AppManager manager;
    return &manager;
}

AppManager::AppManager(QObject *parent)
    : QObject(parent)
{
}

void AppManager::emitRequest(UkuiMenuApplication::Command command)
{
    Q_EMIT request(command);
}

ContextMenuManager *ContextMenuManager::instance()
{
    static ContextMenuManager manager;
    return &manager;
}

ContextMenuManager::ContextMenuManager(QObject *parent)
    : QObject(parent)
{
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

AppPageBackend *AppPageBackend::instance()
{
    static AppPageBackend backend;
    return &backend;
}

AppPageBackend::AppPageBackend(QObject *parent)
    : QObject(parent)
{
}

MenuDbusService::MenuDbusService(const QString &display, QObject *parent)
    : QObject(parent), m_display(display)
{
}

bool MenuDbusService::visible() const
{
    return m_visible;
}

void MenuDbusService::setVisible(bool visible)
{
    m_visible = visible;
    ++m_visibleSetCount;
}

QString MenuDbusService::display() const
{
    return m_display;
}

int MenuDbusService::visibleSetCount() const
{
    return m_visibleSetCount;
}

MenuWindow::MenuWindow(QWindow *parent)
    : QQuickView(parent)
{
}

MenuWindow::MenuWindow(QQmlEngine *engine, QWindow *parent)
    : QQuickView(engine, parent)
{
}

void MenuWindow::activeMenuWindow(bool active)
{
    m_windowVisible = active;
    m_history.append(active);
    setVisible(active);
    Q_EMIT windowVisibleChanged();
}

bool MenuWindow::windowVisible() const
{
    return m_windowVisible;
}

QList<bool> MenuWindow::history() const
{
    return m_history;
}

FolderModel &FolderModel::instance()
{
    static FolderModel model;
    return model;
}

FolderModel::FolderModel(QObject *parent)
    : QObject(parent)
{
}

FavoriteFilterModel *FavoriteFilterModel::instance()
{
    static FavoriteFilterModel model;
    return &model;
}

FavoriteFilterModel::FavoriteFilterModel(QObject *parent)
    : QObject(parent)
{
}

} // namespace UkuiMenu

namespace {

class ukuiMenuApplication_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_constructor_initializesEngineWindowAndDbus();
    void test_keepAlive_doesNotRestartRegisteredClient();
    void test_execCommand_routesWindowActions();
    void test_initDbusService_menuActiveAndVisiblePropagation();
    void test_processMessage_emitsExpectedCommands();
    void test_preprocessMessage_acceptsSupportedOptions();
};

void ukuiMenuApplication_test::init()
{
    MenuSetting::instance()->reset();
    g_definedModules.clear();
    qApp->setProperty("display", QStringLiteral(":test-display"));
    qRegisterMetaType<UkuiMenuApplication::Command>("UkuiMenuApplication::Command");
}

void ukuiMenuApplication_test::test_constructor_initializesEngineWindowAndDbus()
{
    MenuMessageProcessor processor;
    UkuiMenuApplication application(&processor);

    QVERIFY(application.m_engine != nullptr);
    QVERIFY(application.m_mainWindow != nullptr);
    QVERIFY(application.m_menuDbusService != nullptr);
    QVERIFY(application.m_keepAliveClient != nullptr);
    QVERIFY(g_definedModules.size() >= 4);

    QCOMPARE(application.m_keepAliveClient->appName(), QCoreApplication::applicationName());
    QCOMPARE(application.m_keepAliveClient->startCount(), 1);
    QVERIFY(application.m_keepAliveClient->isStarted());
    QVERIFY(application.m_keepAliveClient->isRegistered());

    auto *window = static_cast<MenuWindow *>(application.m_mainWindow);
    auto *dbusService = static_cast<MenuDbusService *>(application.m_menuDbusService);

    QCOMPARE(dbusService->display(), QStringLiteral(":test-display"));
    QCOMPARE(application.m_engine->importPathList().contains(QStringLiteral("qrc:/qml")), true);
    QCOMPARE(application.m_engine->rootContext()->contextProperty(QStringLiteral("menuSetting")).value<QObject *>(),
             static_cast<QObject *>(MenuSetting::instance()));
    QCOMPARE(application.m_engine->rootContext()->contextProperty(QStringLiteral("menuManager")).value<QObject *>(),
             static_cast<QObject *>(ContextMenuManager::instance()));
    QCOMPARE(application.m_engine->rootContext()->contextProperty(QStringLiteral("appManager")).value<QObject *>(),
             static_cast<QObject *>(AppManager::instance()));
    QCOMPARE(application.m_engine->rootContext()->contextProperty(QStringLiteral("favoriteModel")).value<QObject *>(),
             static_cast<QObject *>(FavoriteFilterModel::instance()));
    QCOMPARE(application.m_engine->rootContext()->contextProperty(QStringLiteral("FolderModel")).value<QObject *>(),
             static_cast<QObject *>(&FolderModel::instance()));
    QCOMPARE(application.m_engine->rootContext()->contextProperty(QStringLiteral("functionControl")).value<QObject *>(),
             static_cast<QObject *>(SecurityFunctionControl::instance()));
    QVERIFY(!window->source().isEmpty());
}

void ukuiMenuApplication_test::test_keepAlive_doesNotRestartRegisteredClient()
{
    MenuMessageProcessor processor;
    UkuiMenuApplication application(&processor);

    application.keepAlive();

    QCOMPARE(application.m_keepAliveClient->startCount(), 1);
}

void ukuiMenuApplication_test::test_execCommand_routesWindowActions()
{
    MenuMessageProcessor processor;
    UkuiMenuApplication application(&processor);
    auto *window = static_cast<MenuWindow *>(application.m_mainWindow);
    auto *engine = application.m_engine;
    QSignalSpy quitSpy(static_cast<QObject *>(engine), SIGNAL(quit()));

    QCOMPARE(window->windowVisible(), false);

    application.execCommand(UkuiMenuApplication::Active);
    QCOMPARE(window->history().last(), true);

    application.execCommand(UkuiMenuApplication::Active);
    QCOMPARE(window->history().last(), false);

    application.execCommand(UkuiMenuApplication::Show);
    QCOMPARE(window->history().last(), true);

    application.execCommand(UkuiMenuApplication::Hide);
    QCOMPARE(window->history().last(), false);

    application.execCommand(UkuiMenuApplication::Quit);
    QCOMPARE(window->history().last(), false);
    QCOMPARE(quitSpy.count(), 1);

    const int before = window->history().size();
    application.execCommand(static_cast<UkuiMenuApplication::Command>(999));
    QCOMPARE(window->history().size(), before);
}

void ukuiMenuApplication_test::test_initDbusService_menuActiveAndVisiblePropagation()
{
    MenuMessageProcessor processor;
    UkuiMenuApplication application(&processor);
    auto *window = static_cast<MenuWindow *>(application.m_mainWindow);
    auto *dbusService = static_cast<MenuDbusService *>(application.m_menuDbusService);

    Q_EMIT dbusService->menuActive();
    QCOMPARE(window->history().isEmpty(), false);
    QCOMPARE(window->history().last(), true);

    window->activeMenuWindow(false);
    QCOMPARE(dbusService->visible(), false);

    window->activeMenuWindow(true);
    QCOMPARE(dbusService->visible(), true);
    QVERIFY(dbusService->visibleSetCount() >= 2);
}

void ukuiMenuApplication_test::test_processMessage_emitsExpectedCommands()
{
    MenuMessageProcessor processor;
    QSignalSpy requestSpy(&processor, &MenuMessageProcessor::request);

    processor.processMessage(QStringLiteral("ukui-menu --show"));
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(requestSpy.takeFirst().at(0).value<UkuiMenuApplication::Command>(), UkuiMenuApplication::Show);

    processor.processMessage(QStringLiteral("ukui-menu --quit"));
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(requestSpy.takeFirst().at(0).value<UkuiMenuApplication::Command>(), UkuiMenuApplication::Quit);

    processor.processMessage(QStringLiteral("ukui-menu --unknown"));
    QCOMPARE(requestSpy.count(), 0);
}

void ukuiMenuApplication_test::test_preprocessMessage_acceptsSupportedOptions()
{
    QVERIFY(MenuMessageProcessor::preprocessMessage({QStringLiteral("ukui-menu"), QStringLiteral("--show")}));
    QVERIFY(MenuMessageProcessor::preprocessMessage({QStringLiteral("ukui-menu"), QStringLiteral("--quit")}));
}

} // namespace

QTEST_MAIN(ukuiMenuApplication_test)

#include "ukui-menu-application-test.moc"
