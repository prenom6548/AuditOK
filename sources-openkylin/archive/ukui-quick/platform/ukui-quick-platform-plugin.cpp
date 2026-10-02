/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "ukui-quick-platform-plugin.h"
#include "window-type.h"
#include "windows/ukui-window.h"
#include "windows/dialog.h"
#include "ukui/ukui-theme-proxy.h"
#include "ukui/settings.h"
#include "ukui/app-launcher.h"
#include "ukui/dt-theme-definition.h"
#include "window-manager.h"
#include "workspace-manager.h"
#include "ukui/dt-theme.h"
#include "ukui/application-icon-proxy.h"
#include <QMetaType>
#include <QQmlEngine>



void UkuiQuickPlatformPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    qmlRegisterModule(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterSingletonType<UkuiQuick::Settings>(uri, 1, 0, "Settings", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return UkuiQuick::Settings::instance();
    });
    qmlRegisterSingletonType<UkuiQuick::AppLauncher>(uri, 1, 0, "AppLauncher", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return UkuiQuick::AppLauncher::instance();
    });

    qmlRegisterExtendedType<UkuiQuick::UKUIWindow, UkuiQuick::UKUIWindowExtension>(uri, 1, 0, "UKUIWindow");
    qmlRegisterSingletonType<UkuiQuick::WindowManager>(uri, 1, 0, "WindowManager", [] (QQmlEngine *, QJSEngine *) -> QObject* {
    return UkuiQuick::WindowManager::self();
    });
    qmlRegisterSingletonType<UkuiQuick::WorkspaceManager>(uri, 1, 0, "WorkspaceManager", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return UkuiQuick::WorkspaceManager::self();
    });
    qmlRegisterType<UkuiQuick::DtThemeDefinition>(uri, 1, 0, "DtThemeDefinition");
    qmlRegisterSingletonType<UkuiQuick::DtTheme>(uri, 1, 0, "GlobalTheme", [] (QQmlEngine *engine, QJSEngine *) -> QObject* {
        return UkuiQuick::DtTheme::self(engine);
    });
    qRegisterMetaType<UkuiQuick::GradientColor>("GradientColor");
    qmlRegisterType<UkuiQuick::GradientColor>(uri, 1, 0, "GradientColor");
    qRegisterMetaType<UkuiQuick::ShadowData>("ShadowData");
    qmlRegisterType<UkuiQuick::ShadowData>(uri, 1, 0, "ShadowData");
    qRegisterMetaType<UkuiQuick::DtColorType>("DtColorType");
    // qRegisterMetaType<UkuiQuick::SlideWindow>("SlideWindow");
    // qmlRegisterType<UkuiQuick::SlideWindow>(uri, 1, 0, "SlideWindow");
    // qRegisterMetaType<UkuiQuick::ApplicationIconProxy>("ApplicationIconProxy");
    // qmlRegisterType<UkuiQuick::ApplicationIconProxy>(uri, 1, 0, "ApplicationIconProxy");
    qRegisterMetaType<UkuiQuick::Region *>("Region *");
    qmlRegisterType<UkuiQuick::RectRegion>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "RectRegion");
    qmlRegisterUncreatableType<UkuiQuick::Region>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Region", QStringLiteral("Used as grouped property"));

    qmlRegisterUncreatableType<UkuiQuick::ApplicationIconProxy>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ApplicationIconProxy", QStringLiteral("Used as grouped property"));
    qmlRegisterUncreatableType<UkuiQuick::SlideWindow>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "SlideWindow", QStringLiteral("Used as grouped property"));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qmlRegisterUncreatableType<UkuiQuick::DtColorType>(uri, 1, 0, "DtColorType", "DtColorType is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<UkuiQuick::Theme>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Theme", "Accessing Theme through Attached Property.");
    qmlRegisterUncreatableType<UkuiQuick::WindowType>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowType",
                                                      "WindowType is a read-only interface used to access enumeration properties.");

    qmlRegisterUncreatableType<UkuiQuick::Decoration>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Decoration",
                                                  "Decoration is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<UkuiQuick::WindowState>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowState",
                                              "WindowState is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<UkuiQuick::WindowProxy>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowProxy",
                                                  "WindowProxy is a read-only interface used to access enumeration properties.");
#else
    qmlRegisterUncreatableMetaObject(UkuiQuick::DtColorType::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "DtColorType",
                                                            "DtColorType is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableMetaObject(UkuiQuick::WindowType::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowType",
                                                            "WindowType is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableMetaObject(UkuiQuick::Decoration::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Decoration",
                                                            "Decoration is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableMetaObject(UkuiQuick::WindowState::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowState",
                                                            "WindowState is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableMetaObject(UkuiQuick::WindowProxy::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowProxy",
                                                            "WindowProxy is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<UkuiQuick::Theme>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Theme", "Accessing Theme through Attached Property.");
    // qmlRegisterUncreatableMetaObject(UkuiQuick::Theme::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Theme",
    //                                                         "Theme is a read-only interface used to access enumeration properties.");
#endif

}
