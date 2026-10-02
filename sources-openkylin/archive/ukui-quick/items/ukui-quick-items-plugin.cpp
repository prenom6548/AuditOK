/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "ukui-quick-items-plugin.h"
#include <QtQml>
#include <QAction>
#include <QScreen>

#include "margin.h"
#include "types.h"
#include "menu.h"
#include "menu-item.h"
#include "theme-icon.h"
#include "tooltip-proxy.h"
#include "icon.h"
#include "icon-provider.h"
#include "color-mixer.h"
#include "content-window.h"
#include "windows/dialog.h"
#include "tooltip.h"
#include "action-extension.h"
#include "window-blur-behind.h"
#include "gradient-border-item.h"
#include "line-edit-context-menu.h"
#include "shadowed-texture.h"
#include "lottie-attached.h"
#include "lottie-item.h"
#include "lottie-runtime-service_p.h"

#include "ukui/application-icon-proxy.h"

void UkuiQuickItemsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    UkuiQuick::installLottieRuntimeApplicationShutdownHook();
    qmlRegisterModule(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);

    qRegisterMetaType<QScreen*>();
    qRegisterMetaType<UkuiQuick::Types::Pos>();
    qRegisterMetaType<UkuiQuick::Dialog::PopupLocation>();
    qRegisterMetaType<UkuiQuick::Region *>("Region *");

    // value types
    qmlRegisterType<UkuiQuick::Icon>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Icon");
    qmlRegisterType<UkuiQuick::Menu>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Menu");
    qmlRegisterType<UkuiQuick::MenuItem>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "MenuItem");
    qmlRegisterType<UkuiQuick::ThemeIcon>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ThemeIcon");
    qmlRegisterType<UkuiQuick::ColorMix>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ColorMix");
    qmlRegisterType<UkuiQuick::GradientBorderItem>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "GradientBorderItem");
    qmlRegisterType<UkuiQuick::LineEditContextMenu>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "LineEditContextMenu");

    qmlRegisterType<UkuiQuick::LottieItem>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "LottieItem");
    qmlRegisterUncreatableType<UkuiQuick::LottieAttached>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Lottie",
                                                         "Accessing Lottie through Attached property \"Lottie\".");

    qmlRegisterUncreatableType<UkuiQuick::ToolTipAttached>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ToolTip", "Proxy tool pointing to QTooltip, accessed only through Attached property \"ToolTip\".");

    qmlRegisterType<UkuiQuick::Dialog>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Dialog");
    qmlRegisterUncreatableType<UkuiQuick::WindowProxy>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowProxy",
                                              "WindowProxy is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<UkuiQuick::SlideWindow>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "SlideWindow", QStringLiteral("Used as grouped property"));
    qmlRegisterType<UkuiQuick::Tooltip>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Tooltip");
    qmlRegisterType<UkuiQuick::ContentWindow>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ContentWindow");

    // 注册QAction并为它扩展属性
    qmlRegisterExtendedType<QAction, UkuiQuick::ActionExtension>(uri, 1, 0, "Action");

    qmlRegisterType<UkuiQuick::RectRegion>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "RectRegion");
    qmlRegisterType<UkuiQuick::WindowBlurBehind>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowBlurBehind");

    qmlRegisterType<ShadowedRectangle>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ShadowedRectangle");
    qmlRegisterType<ShadowedTexture>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ShadowedTexture");
    qmlRegisterUncreatableType<BorderGroup>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "BorderGroup", QStringLiteral("Used as grouped property"));
    qmlRegisterUncreatableType<UkuiQuick::Region>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Region", QStringLiteral("Used as grouped property"));
    qmlRegisterUncreatableType<ShadowGroup>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ShadowGroup", QStringLiteral("Used as grouped property"));
    qmlRegisterUncreatableType<CornersGroup>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "CornersGroup", QStringLiteral("Used as grouped property"));
    qmlRegisterUncreatableType<UkuiQuick::ApplicationIconProxy>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ApplicationIconProxy", QStringLiteral("Used as grouped property"));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qmlRegisterUncreatableType<UkuiQuick::Types>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Types", "Types is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<UkuiQuick::Margin>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Margin", "Margin is a read-only attribute that can be accessed from the ContentWindow.");

#else
    qmlRegisterUncreatableMetaObject(UkuiQuick::Types::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Types",
                                                        "Types is a read-only interface used to access enumeration properties.");

    qmlRegisterUncreatableMetaObject(UkuiQuick::Margin::staticMetaObject, uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Margin",
                                                        "Margin is a read-only interface used to access enumeration properties.");
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    qmlRegisterRevision<QWindow, 3>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterRevision<QQuickWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#else
    #if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    qmlRegisterRevision<QWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterRevision<QQuickWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    qmlRegisterRevision<QWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterRevision<QQuickWindow, 1>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#else
    qmlRegisterRevision<QWindow, 1>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#endif
#endif
#endif
}

void UkuiQuickItemsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    engine->addImageProvider("theme", new UkuiQuick::IconProvider());

    installTranslations();
}

UkuiQuickItemsPlugin::~UkuiQuickItemsPlugin()
{
    // 检查QCoreApplication是否仍然有效，防止在程序退出时访问已销毁的资源
    if (QCoreApplication::instance() && m_translator != nullptr) {
        QCoreApplication::removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    } else if (m_translator != nullptr) {
        // 如果QCoreApplication已经无效，跳过避免崩溃
        m_translator = nullptr;
    }
}

void UkuiQuickItemsPlugin::installTranslations()
{
    const QString fileName = PLUGIN_INSTALL_TRANSLATIONS_PATH + QLocale::system().name();
    m_translator = new QTranslator(QCoreApplication::instance());
    if (m_translator->load(fileName)) {
        QCoreApplication::installTranslator(m_translator);
    } else {
        delete m_translator;
        m_translator = nullptr;
    }
}
