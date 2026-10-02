/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QVariant>
#include <QStandardPaths>
#include "qt5-ukui-platform-theme.h"
#include "settings/ukui-style-settings.h"
#include "effects/highlight-effect.h"
#include "settings/black-list.h"

#include <QFontDatabase>
#include <QApplication>
#include <QTimer>

#include <QPluginLoader>
#include <QIconEnginePlugin>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QFileInfo>
#include <QIcon>
#endif

#include <QApplication>
#include <QWidget>

#include <QDebug>
#include <private/qgenericunixthemes_p.h>
#include <private/qdbustrayicon_p.h>

#include <QQuickStyle>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include "widget/messagebox/message-box.h"
//#include "widget/filedialog/kyfiledialog.h"
#include "platform-theme-fontdata.h"
#include "../qt5-ukui-filedialog/filedialoginterface.h"
#include "widget/filedialog/xdgdesktopportalfiledialog_p.h"

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
static bool isDBusTrayAvailable() {
    static bool dbusTrayAvailable = false;
    static bool dbusTrayAvailableKnown = false;
    if (!dbusTrayAvailableKnown) {
        QDBusMenuConnection conn;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 19))

        if (conn.isWatcherRegistered())
#else
        if (conn.isStatusNotifierHostRegistered())
#endif
            dbusTrayAvailable = true;
        dbusTrayAvailableKnown = true;
    }
    return dbusTrayAvailable;
}
#endif

void Qt5UKUIPlatformTheme::slotChangeStyle(const QString& key)
{
    auto settings = UKUIStyleSettings::globalInstance();
    if (!settings)
        return;
    if (key == "iconThemeName" || key == "icon-theme-name") {
        QString icontheme = settings->get("icon-theme-name").toString();

        QIcon::setThemeName(icontheme);

        if(qApp){
            QIcon icon = qApp->windowIcon();
            qApp->setWindowIcon(QIcon::fromTheme(icon.name()));
        }
        // update all widgets for repaint new themed icons.
        for (auto widget : QApplication::allWidgets()) {
            widget->update();
        }
    }

    if (key == "systemFont" || key == "system-font") {
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app == nullptr)
            return;

        QString font = settings->get("system-font").toString();

        QFontDatabase db;
        int id = 0;
        if (!db.families().contains(font)) {
            PlatformThemeFontData fontData;
            QMap<QString, QString> fontMap = fontData.getAllFontInformation();
            if(fontMap.contains(font)){
                auto iter = fontMap.find(font);
                id = QFontDatabase::addApplicationFont(iter.value());
            }
        }

        QFontDatabase newDb;
        if (newDb.families().contains(font)) {
            QFont oldFont = QApplication::font();
            m_system_font.setFamily(font);
            m_fixed_font.setFamily(font);
            oldFont.setFamily(font);
            QApplication::setFont(oldFont);
        }
    }

    if (key == "systemFontStyle" || key == "system-font-style") {
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app == nullptr || qApp == nullptr)
            return;

        QString fontStyle = settings->get("system-font-style").toString();
        if (!fontStyle.isEmpty()) {
            QFont oldFont = QApplication::font();
            m_system_font.setStyleName(fontStyle);
            m_fixed_font.setStyleName(fontStyle);
            oldFont.setStyleName(fontStyle);
            QApplication::setFont(oldFont);
        }
    }

    if (key == "systemFontSize" || key == "system-font-size") {
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app == nullptr || qApp == nullptr)
            return;

        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            return;
        double fontSize = settings->get("system-font-size").toString().toDouble();
        if (fontSize > 0) {
            QFont oldFont = QApplication::font();
            m_system_font.setPointSize(fontSize);
            m_fixed_font.setPointSize(fontSize*1.2);
            oldFont.setPointSizeF(fontSize);
            QApplication::setFont(oldFont);
        }
    }
}

Qt5UKUIPlatformTheme::Qt5UKUIPlatformTheme(const QStringList &args)
{
    //FIXME:
    Q_UNUSED(args)
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();

        //set font
        auto fontName = settings->get("systemFont").toString();
        auto fontSize = settings->get("systemFontSize").toString().toDouble();
        if (qApp && qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            fontSize = 11;
        m_system_font.setFamily(fontName);
        m_system_font.setPointSizeF(fontSize);

        m_fixed_font.setFamily(fontName);
        m_fixed_font.setPointSizeF(fontSize*1.2);

        /*!
         * \bug
         * if we set app font, qwizard title's font will
         * become very small. I handle the wizard title
         * in ProxyStyle::polish().
         */
        //Skip QGuiApplication avoid it crash when we setfont
        auto *app = qobject_cast<QApplication *>(qApp);
        if(app != nullptr) {
            QApplication::setFont(m_system_font);
        }

        if (app->applicationName().toLower().contains(QLatin1String("kwin"))) {
            QDBusConnection::sessionBus().connect(QString(),
                                                  QStringLiteral("/UKUIPlatformTheme"),
                                                  QStringLiteral("org.ukui.UKUIPlatformTheme"),
                                                  QStringLiteral("refreshFonts"),
                                                  this,
                                                  SLOT(slotChangeStyle(QString)));
        }

        connect(settings, &QGSettings::changed, this, &Qt5UKUIPlatformTheme::slotChangeStyle);
    }

//    // add qqc2 style
//    if (QFile::exists(QString("%1/kf5/kirigami/org.kylin.style.so").arg(QT_PLUGIN_INSTALL_DIRS))) {
//        QQuickStyle::setStyle("org.kylin.style");
//    }
    if(qApp && (qApp->inherits("QApplication") == true || qApp->inherits("QGuiApplication") == true) && qAppName() != "cura") {
        QQuickStyle::setStyle("org.ukui.style");
    }

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    } else {
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    }
    const char* c = UKUIPLATFORMTHEME_DIR;
    QString path = QString::fromUtf8(c);
    if (QFile::exists(QString("%1/platformthemes/libqt5-ukui-filedialog.so").arg(path))) {
        m_loader = new QPluginLoader(QString("%1/platformthemes/libqt5-ukui-filedialog.so").arg(path));
    }

        // Get information about portal version
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.portal.Desktop"),
                                                          QLatin1String("/org/freedesktop/portal/desktop"),
                                                          QLatin1String("org.freedesktop.DBus.Properties"),
                                                          QLatin1String("Get"));
    message << QLatin1String("org.freedesktop.portal.FileChooser") << QLatin1String("version");
    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher *watcher) {
        if (watcher->isError()) {
            // 错误处理
            QDBusError error = watcher->error();
            qWarning() << "D-Bus 调用失败:" << error.message();
        } else {
            QDBusPendingReply<QVariant> reply = *watcher;
            if (reply.isValid()) {
                //qDebug() << "FileChooserPortalVersion......." << reply.value().toUInt();
                m_portalVersion = reply.value().toUInt();
            }
        }
        watcher->deleteLater();
    });


    m_dtConfig = new GlobalDTConfig();
    m_palette = new QPalette(m_dtConfig->appPalette());

}

Qt5UKUIPlatformTheme::~Qt5UKUIPlatformTheme()
{
    if(m_loader){
        m_loader->unload();
        delete m_loader;
        m_loader = nullptr;
    }
    if(m_dtConfig){
        delete m_dtConfig;
        m_dtConfig = nullptr;
    }
    if(m_palette){
        delete m_palette;
        m_palette = nullptr;
    }
}

const QPalette *Qt5UKUIPlatformTheme::palette(Palette type) const
{
    //FIXME:
    if (type == QPlatformTheme::SystemPalette && m_palette) {
        return m_palette;
    }
    return QPlatformTheme::palette(type);
}

const QFont *Qt5UKUIPlatformTheme::font(Font type) const
{
    //FIXME:
    if (type == FixedFont)
        return &m_fixed_font;
    return &m_system_font;
    switch (type) {
    case SystemFont:
        return &m_system_font;
    case TitleBarFont:
    case FixedFont:
    case GroupBoxTitleFont:
        return &m_fixed_font;
    default:
        return &m_system_font;
    }
    return QPlatformTheme::font(type);
}

QVariant Qt5UKUIPlatformTheme::themeHint(ThemeHint hint) const
{
    if(qAppName() == "cura")
        return QPlatformTheme::themeHint(hint);

    switch (hint) {
    case QPlatformTheme::StyleNames:{
//        qDebug() << "Qt5UKUIPlatformTheme....";
//        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
//            if (auto settings = UKUIStyleSettings::globalInstance()) {
//                QString themeName = settings->get("widget-theme-name").toString();
//                if(themeName == "classical")
//                return QStringList()<< "ukui-default";
//            }
//        }
        return QStringList()<< "ukui";
    }

    case QPlatformTheme::SystemIconThemeName: {
        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
            if (auto settings = UKUIStyleSettings::globalInstance()) {
                QString icontheme = settings->get("icon-theme-name").toString();

                return QStringList()<<icontheme;
            }
        }
        return "hicolor";
    }

    case QPlatformTheme::SystemIconFallbackThemeName:
        return "hicolor";
    case QPlatformTheme::IconThemeSearchPaths:
        return xdgIconThemePaths();
//    case WheelScrollLines:
//    {
//        return QVariant(1);
//    }break;
    default:
        break;
    }
    return QPlatformTheme::themeHint(hint);
}

QStringList Qt5UKUIPlatformTheme::xdgIconThemePaths() const {
    QStringList paths;

    // Add home directory first in search path
    const QFileInfo homeIconDir(QDir::homePath() + QLatin1String("/.icons"));
    if (homeIconDir.isDir())
        paths.prepend(homeIconDir.absoluteFilePath());

    paths.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                            QStringLiteral("icons"),
                                            QStandardPaths::LocateDirectory));
    
    return paths;
}

QIconEngine *Qt5UKUIPlatformTheme::createIconEngine(const QString &iconName) const
{
//    QPluginLoader l(XDG_ICON_ENGINE_PATH);
//    if (l.instance()) {
//        auto p = dynamic_cast<QIconEnginePlugin *>(l.instance());
//        auto engine = p->create();
//        qDebug()<<"use my engine";
//        return engine;
//    } else {
//        qDebug()<<"use common engine";
//        return QPlatformTheme::createIconEngine(iconName);
//    }

//    //return new XdgIconLoaderEngine(iconName);

    return QPlatformTheme::createIconEngine(iconName);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
bool Qt5UKUIPlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    switch (type) {
    case QPlatformTheme::FileDialog:
        if (ukuiFiledialogBlackList().contains(qAppName()))
            return false;
        else
            return true;
    case QPlatformTheme::FontDialog:
    case QPlatformTheme::ColorDialog:
        return false;
    case QPlatformTheme::MessageDialog:
        if(qApp && qApp->inherits("QApplication"))
        return true;
//        if (qAppName() == "ukui-control-center")
//            return false;
    default:
        break;
    }

    return false;
}

QPlatformDialogHelper *Qt5UKUIPlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    switch (type) {
    case QPlatformTheme::FileDialog:
        if (!ukuiFiledialogBlackList().contains(qAppName())) {
            if(ukuiXDPFiledialogList().contains(qAppName())){
                return new XdgDesktopPortalFileDialog;
            }
            if (qApp && qApp->inherits("QApplication") && (m_loader && m_loader->instance())) {
                return qobject_cast<FileDialogInterface *>(m_loader->instance())->create();
            } else if (qApp && qApp->inherits("QGuiApplication") && m_portalVersion >= 3) {
                return new XdgDesktopPortalFileDialog;
            }
        }
    case QPlatformTheme::FontDialog:
    case QPlatformTheme::ColorDialog:
        return QPlatformTheme::createPlatformDialogHelper(type);
    case QPlatformTheme::MessageDialog:
        return new MessageBoxHelper;
    default:
        break;
    }

    return nullptr;
}
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
QIcon Qt5UKUIPlatformTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const
{
    //FIXME:
    return QPlatformTheme::fileIcon(fileInfo, iconOptions);
}
#endif

#ifdef GLOBAL_MENU
QPlatformMenuBar *Qt5UKUIPlatformTheme::createPlatformMenuBar() const
{
    return QPlatformTheme::createPlatformMenuBar();
}
#endif

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
QPlatformSystemTrayIcon *Qt5UKUIPlatformTheme::createPlatformSystemTrayIcon() const
{
    if (isDBusTrayAvailable())
        return new QDBusTrayIcon();
    return nullptr;
}

#endif
