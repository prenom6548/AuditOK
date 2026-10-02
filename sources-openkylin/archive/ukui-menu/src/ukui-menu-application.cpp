/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "ukui-menu-application.h"
#include "menu-dbus-service.h"
#include "settings.h"
#include "commons.h"
#include "menu-main-window.h"
#include "power-button.h"
#include "app-manager.h"
#include "panel-focus-bridge.h"
#include "context-menu-manager.h"
#include "event-track.h"
#include "sidebar-button-utils.h"
#include "widget-model.h"
#include "app-page-backend.h"
#include "app-group-model.h"
#include "favorite/folder-model.h"
#include "favorite/favorite-filter-model.h"
#include "security-function-control.h"
#include "mask-mouse-area.h"

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>

using namespace UkuiMenu;

UkuiMenuApplication::UkuiMenuApplication(MenuMessageProcessor *processor) : QObject(nullptr)
{
    registerQmlTypes();
    startUkuiMenu();
    initDbusService();
    keepAlive();
    connect(processor, &MenuMessageProcessor::request, this, &UkuiMenuApplication::execCommand);
}

void UkuiMenuApplication::startUkuiMenu()
{
    initQmlEngine();
    loadMenuUI();
}

void UkuiMenuApplication::registerQmlTypes()
{
    const char *uri = "org.ukui.menu.core";
    int versionMajor = 1, versionMinor = 0;
    SettingModule::defineModule(uri, versionMajor, versionMinor);
    WindowModule::defineModule(uri, versionMajor, versionMinor);
    PowerButton::defineModule(uri, versionMajor, versionMinor);
    SidebarButtonUtils::defineModule(uri, versionMajor, versionMinor);
    MaskMouseArea::defineModule(uri, versionMajor, versionMinor);

    qmlRegisterType<WidgetModel>(uri, versionMajor, versionMinor, "WidgetModel");
    qmlRegisterType<AppGroupModel>(uri, versionMajor, versionMinor, "AppGroupModel");
    //qmlRegisterType<AppPageBackend>(uri, versionMajor, versionMinor, "AppPageBackend");
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "AppPageBackend", AppPageBackend::instance());
    qmlRegisterUncreatableType<AppListPluginGroup>(uri, versionMajor, versionMinor, "PluginGroup", "Use enums only.");

    // commons
    qRegisterMetaType<UkuiMenu::DataType::Type>("DataType::Type");
    qRegisterMetaType<UkuiMenu::DataEntity>("DataEntity");
    qRegisterMetaType<UkuiMenu::MenuInfo::Location>("MenuInfo::Location");
    qmlRegisterUncreatableType<Display>(uri, versionMajor, versionMinor, "Display", "Use enums only.");
    qmlRegisterUncreatableType<UkuiMenu::DataType>(uri, versionMajor, versionMinor, "DataType", "Use enums only");
    qmlRegisterUncreatableType<UkuiMenu::MenuInfo>(uri, versionMajor, versionMinor, "MenuInfo", "Use enums only.");
    qmlRegisterUncreatableType<UkuiMenu::LabelItem>(uri, versionMajor, versionMinor, "LabelItem", "Use enums only.");
    qmlRegisterUncreatableType<UkuiMenu::WidgetMetadata>(uri, versionMajor, versionMinor, "WidgetMetadata", "Use enums only.");
//    qmlRegisterUncreatableType<UkuiMenu::DataEntity>(uri, versionMajor, versionMinor, "DataEntity", "unknown");

    qmlRegisterUncreatableType<EventTrack>(uri, versionMajor, versionMinor, "EventTrack", "Attached only.");
}

void UkuiMenuApplication::initQmlEngine()
{
    m_engine = new QQmlEngine(this);
    m_engine->addImportPath("qrc:/qml");

    QQmlContext *context = m_engine->rootContext();
    context->setContextProperty("menuSetting", MenuSetting::instance());
    context->setContextProperty("menuManager", ContextMenuManager::instance());
    context->setContextProperty("appManager", AppManager::instance());
    context->setContextProperty("panelFocusBridge", PanelFocusBridge::instance());

    context->setContextProperty("favoriteModel", FavoriteFilterModel::instance());
    context->setContextProperty("FolderModel", &FolderModel::instance());

    context->setContextProperty("functionControl", SecurityFunctionControl::instance());
    // MenuMainWindow
//    const QUrl url(QStringLiteral("qrc:/qml/MenuMainWindow.qml"));
//    QQmlApplicationEngine *m_applicationEngine{nullptr};
//    m_applicationEngine = new QQmlApplicationEngine(this);
//    QObject::connect(m_applicationEngine, &QQmlApplicationEngine::objectCreated,
//                     this, [url](QObject *obj, const QUrl &objUrl) {
//            if (!obj && url == objUrl)
//                QCoreApplication::exit(-1);
//        }, Qt::QueuedConnection);
//
//    m_applicationEngine->load(url);

    connect(AppManager::instance(), &AppManager::request, this, &UkuiMenuApplication::execCommand);
}

void UkuiMenuApplication::loadMenuUI()
{
    if (QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
        m_mainWindow = new MenuWindowWayland(m_engine, nullptr);
        m_mainWindow->setSource(url);
    } else {
        const QUrl url(QStringLiteral("qrc:/qml/mainXcb.qml"));
        m_mainWindow = new MenuWindowXcb(m_engine, nullptr);
        m_mainWindow->setSource(url);
    }

    connect(m_mainWindow, &QQuickView::activeFocusItemChanged, m_mainWindow, [this] {
        if (m_mainWindow->activeFocusItem()) {
            return;
        }

        execCommand(Hide);
    });
}

void UkuiMenuApplication::initDbusService()
{
    m_menuDbusService = new MenuDbusService(QGuiApplication::instance()->property("display").toString(), this);
    if (m_menuDbusService) {
        connect(m_menuDbusService, &MenuDbusService::menuActive, this, [this] {
            execCommand(Active);
        });
//        connect(m_menuDbusService, &MenuDbusService::reloadConfigSignal, this, [this] {
////            reload ...
//        });

        if (m_mainWindow) {
            connect(m_mainWindow, &MenuWindow::visibleChanged, this, [this] {
                const bool visible = m_mainWindow->isVisible();
                if (!visible) {
                    PanelFocusBridge::instance()->handleMenuHidden();
                }
                m_menuDbusService->setVisible(visible);
            });
        }
    }
}

void UkuiMenuApplication::keepAlive()
{
    if (!m_keepAliveClient) {
        m_keepAliveClient = new UkuiQuick::HeartBeat::KeepAliveClient(QCoreApplication::applicationName() ,this);
    }

    if (m_keepAliveClient->isStarted() && m_keepAliveClient->isRegistered()) {
        return;
    }
    bool success = m_keepAliveClient->start();
    if (!success) {
        qWarning() << "Failed to start keep-alive client";
    }
}

void UkuiMenuApplication::execCommand(Command command)
{
    switch (command) {
        case Active: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(!m_mainWindow->windowVisible());
            }
            break;
        }
        case Show: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(true);
            }
            break;
        }
        case Quit: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(false);
                m_engine->quit();
            }
            QCoreApplication::quit();
            break;
        }
        case Hide: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(false);
            }
            break;
        }
        default:
            break;
    }

    // TODO: 发送窗口显示隐藏信号到插件
}

UkuiMenuApplication::~UkuiMenuApplication()
{
    if (m_mainWindow) {
        m_mainWindow->deleteLater();
    }
}

// == MenuMessageProcessor == //

QCommandLineOption MenuMessageProcessor::showUkuiMenu({"s", "show"}, QObject::tr("Show ukui-menu."));
QCommandLineOption MenuMessageProcessor::quitUkuiMenu({"q", "quit"}, QObject::tr("Quit ukui-menu."));

MenuMessageProcessor::MenuMessageProcessor() : QObject(nullptr) {}

void MenuMessageProcessor::processMessage(const QString &message)
{
    QStringList options = QString(message).split(' ');

    QCommandLineParser parser;
    parser.addOption(MenuMessageProcessor::showUkuiMenu);
    parser.addOption(MenuMessageProcessor::quitUkuiMenu);

    parser.parse(options);

    if (parser.isSet(MenuMessageProcessor::showUkuiMenu)) {
        Q_EMIT request(UkuiMenuApplication::Show);
        return;
    }

    if (parser.isSet(MenuMessageProcessor::quitUkuiMenu)) {
        Q_EMIT request(UkuiMenuApplication::Quit);
        return;
    }
}

/**
 * 已知问题，使用命令quit时，会出现 QFileSystemWatcher::removePaths: list is empty
 * 可见该bug: https://bugreports.qt.io/browse/QTBUG-68607
 * @param message
 * @return
 */
bool MenuMessageProcessor::preprocessMessage(const QStringList& message)
{
    qDebug() << "Hey, there.";

    QCommandLineParser parser;
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();

    parser.addOption(MenuMessageProcessor::showUkuiMenu);
    parser.addOption(MenuMessageProcessor::quitUkuiMenu);

    parser.parse(message);

    if (message.size() <= 1 || parser.isSet(helpOption) || !parser.unknownOptionNames().isEmpty()) {
        if (!parser.unknownOptionNames().isEmpty()) {
            qDebug() << "Unknown options:" << parser.unknownOptionNames();
        }
        parser.showHelp();
        return false;
    }

    if (parser.isSet(versionOption)) {
        parser.showVersion();
        return false;
    }

    return parser.isSet(MenuMessageProcessor::showUkuiMenu) || parser.isSet(MenuMessageProcessor::quitUkuiMenu);
}
