/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sessionsettings.h"
#include "utils.h"
#include "kmreenv.h"
#include "dbus/dbusclient.h"

#include <chrono>
#include <thread>

#include <QString>
#include <QByteArray>
#include <QSettings>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>

#include <xcb/xcb.h>
#include <sys/syslog.h>
#include <wayland-client.h>

#include "drm-client-protocol.h"

using namespace std::chrono_literals;


static bool wayland_has_plasma_shell = false;
static bool wayland_has_plasma_window_management = false;
static bool wayland_has_ukui_decoration = false;
static bool wayland_has_xdg_shell_v6 = false;
static bool wayland_has_xdg_wm_base = false;
static bool wayland_has_xdg_activation_v1 = false;
static bool wayland_has_gtk_shell1 = false;
static bool wayland_has_remote_access = false;
static bool has_xdg_shell_support = false;


static void registry_add_object (void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void) data;
    (void) registry;
    (void) name;
    (void) version;
    if (!strcmp(interface, "org_kde_plasma_shell")) {
        wayland_has_plasma_shell = true;
    } else if (!strcmp(interface, "org_kde_plasma_window_management")) {
        wayland_has_plasma_window_management = true;
    } else if (!strcmp(interface, "ukui_decoration")) {
        wayland_has_ukui_decoration = true;
    } else if (!strcmp(interface, "org_kde_kwin_remote_access_manager")) {
        if (version >= 2) {
            wayland_has_remote_access = true;
        }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    } else if (!strcmp(interface, "zxdg_shell_v6")) {
        wayland_has_xdg_shell_v6 = true;
        has_xdg_shell_support = true;
    } else if (!strcmp(interface, "xdg_wm_base")) {
        if (version >= 2) {
            wayland_has_xdg_wm_base = true;
            has_xdg_shell_support = true;
        }
    } else if (!strcmp(interface, "xdg_activation_v1")) {
        wayland_has_xdg_activation_v1 = true;
    } else if (!strcmp(interface, "gtk_shell1")) {
        if (version >= 4) {
            wayland_has_gtk_shell1 = true;
        }
#endif
    }
}

static void registry_remove_object (void *data, struct wl_registry *registry, uint32_t name)
{
    (void) data;
    (void) registry;
    (void) name;
}

static struct wl_registry_listener registry_listener = {
    &registry_add_object,
    &registry_remove_object,
};

static bool hasWaylandInterfacesSupport()
{
    int result = false;
    int count = 0;
    int try_times = 5;

    struct wl_display* awl_display = NULL;
    struct wl_registry* awl_registry = NULL;
    struct wl_event_queue* awl_queue = NULL;

    awl_display = wl_display_connect(NULL);
    if (!awl_display) {
        return false;
    }

    awl_queue = wl_display_create_queue(awl_display);
    if (!awl_queue) {
        goto out;
    }

    awl_registry = wl_display_get_registry(awl_display);
    if (!awl_registry) {
        goto out;
    }
    wl_proxy_set_queue((struct wl_proxy*)awl_registry, awl_queue);
    wl_registry_add_listener(awl_registry, &registry_listener, NULL);

    wl_display_roundtrip_queue(awl_display, awl_queue);

    while (count++ < try_times) {
        if ((wayland_has_plasma_shell &&
                wayland_has_plasma_window_management &&
                wayland_has_ukui_decoration) ||
                (has_xdg_shell_support &&
                 wayland_has_xdg_activation_v1 &&
                 wayland_has_gtk_shell1)) {
            result = true;
            break;
        }

        wl_display_roundtrip_queue(awl_display, awl_queue);
    }

out:
    if (awl_registry) {
        wl_registry_destroy(awl_registry);
        awl_registry = NULL;
    }

    if (awl_queue) {
        wl_event_queue_destroy(awl_queue);
        awl_queue = NULL;
    }

    if (awl_display) {
        wl_display_disconnect(awl_display);
        awl_display = NULL;
    }

    return result;
}

kmre::sp<DisplayInfo> getDisplayInfoFromDbus()
{
    QString infoStr;
    for (int i = 0; i < 50; ++i) {
        infoStr = kmre::DbusClient::getInstance()->GetDisplayInformation();
        if (!infoStr.isEmpty()) {
            break;
        }
        else {
            std::this_thread::sleep_for(100ms);
        }
    }

    if (infoStr.isEmpty()) {
        syslog(LOG_ERR, "[%s] GetDisplayInformation from dbus failed!", __func__);
        return nullptr;
    }

    int virtualWidth = 0;
    int virtualHeight = 0;
    int physicalWidth = 0;
    int physicalHeight = 0;
    QString displayTypeName = "unknow";
    QJsonParseError parseJsonErr;

    QJsonDocument document = QJsonDocument::fromJson(infoStr.toLocal8Bit(), &parseJsonErr);
    if (parseJsonErr.error != QJsonParseError::NoError) {
        syslog(LOG_ERR, "[%s] Invalid display information!", __func__);
        return nullptr;
    }

    auto displayInfo = std::make_shared<DisplayInfo>();

    QJsonObject obj = document.object();
    if (obj.contains(QStringLiteral("window_display.width"))) {
        virtualWidth = obj.value("window_display.width").toInt();
    }

    if (obj.contains(QStringLiteral("window_display.height"))) {
        virtualHeight = obj.value("window_display.height").toInt();
    }

    if (obj.contains(QStringLiteral("physical_display.width"))) {
        physicalWidth = obj.value("physical_display.width").toInt();
    }

    if (obj.contains(QStringLiteral("physical_display.height"))) {
        physicalHeight = obj.value("physical_display.height").toInt();
    }

    if (obj.contains(QStringLiteral("supports_threaded_opengl"))) {
        displayInfo->supportsThreadedOpenGL = obj.value("supports_threaded_opengl").toBool();
    }

    if (physicalWidth > 0 && physicalHeight > 0) {
        displayInfo->physicalDisplaySize = QSize(physicalWidth, physicalHeight);
    } 
    else {
        displayInfo->physicalDisplaySize = QSize(1920, 1080);
    }

    if (virtualWidth > 0 && virtualHeight > 0) {
        displayInfo->virtualDisplaySize = QSize(virtualWidth, virtualHeight);
    } 
    else {
        displayInfo->virtualDisplaySize = QSize(720, 1280);
    }

    if (obj.contains(QStringLiteral("display_type"))) {
        displayTypeName = obj.value("display_type").toString();
        if (displayTypeName == "drm") {
            displayInfo->displayType = DisplayType::TYPE_DRM;
        } 
        else if (displayTypeName == "sw") {
            displayInfo->displayType = DisplayType::TYPE_SW;
        } 
        else if (displayTypeName == "emugl") {
            displayInfo->displayType = DisplayType::TYPE_EMUGL;
        }
    }

    kmre::utils::saveDisplayTypeToShm(displayTypeName);
    
    syslog(LOG_INFO, "[%s] physical size: width = %d, height = %d", 
        __func__, displayInfo->physicalDisplaySize.width(), displayInfo->physicalDisplaySize.height());
    syslog(LOG_INFO, "[%s] display size: width = %d, height = %d", 
        __func__, displayInfo->virtualDisplaySize.width(), displayInfo->virtualDisplaySize.height());
    syslog(LOG_INFO, "[%s] display type: '%s'", 
        __func__, displayTypeName.toStdString().c_str());

    return displayInfo;
}

static bool windowCouldUseX11()
{
    xcb_connection_t *conn = xcb_connect(NULL, NULL);

    if (!conn) {
        return false;
    }

    if (xcb_connection_has_error(conn)) {
        xcb_disconnect(conn);
        return false;
    }

    xcb_disconnect(conn);

    return true;
}

static bool windowTryWayland()
{
    bool result = false;

    QSettings s(KmreEnv::GetKmreConfigFile(), QSettings::IniFormat);
    QStringList groups;
    QStringList keys;

    groups = s.childGroups();
    if (groups.contains("window")) {
        s.beginGroup("window");
        keys = s.childKeys();
        if (keys.contains("try_wayland")) {
            result = s.value("try_wayland", false).toBool();
        }
        s.endGroup();
    }

    return result;
}

SessionSettings::SessionSettings()
    : mDisplayInfo(getDisplayInfoFromDbus())
    , mDisplayPlatform(DisplayPlatform::NONE)
    , mWindowPlatform(DisplayPlatform::NONE)
{
    if (!mDisplayInfo || (mDisplayInfo->displayType == DisplayType::TYPE_UNKNOWN)) {
        syslog(LOG_CRIT, "[%s] Init Session Settings failed! Exit now...", __func__);
        exit(-1);
    }

    QString xdgSessionType = qgetenv("XDG_SESSION_TYPE");
    if (xdgSessionType == "x11") {
        mDisplayPlatform = DisplayPlatform::X11;
    } else if (xdgSessionType == "wayland") {
        mDisplayPlatform = DisplayPlatform::WAYLAND;
    }

    if (mDisplayPlatform == DisplayPlatform::X11) {
        mWindowPlatform = DisplayPlatform::X11;
    } else if (mDisplayPlatform == DisplayPlatform::WAYLAND) {
        if (!windowCouldUseX11()) {
            mWindowPlatform = DisplayPlatform::WAYLAND;
        } else {
            if (windowCouldUseWayland() && windowTryWayland()) {
                mWindowPlatform = DisplayPlatform::WAYLAND;
            } else {
                mWindowPlatform = DisplayPlatform::X11;
            }
        }
    }
}

bool SessionSettings::windowCouldUseWayland()
{
    if (!hasWaylandInterfacesSupport()) {
        return false;
    }

    if (mDisplayInfo->displayType != DisplayType::TYPE_DRM) {
        return false;
    }

    if (!mDisplayInfo->supportsThreadedOpenGL) {
        return false;
    }

    return true;
}

bool SessionSettings::windowUsePlatformX11()
{
    return mWindowPlatform == DisplayPlatform::X11;
}

bool SessionSettings::windowUsePlatformWayland()
{
    return mWindowPlatform == DisplayPlatform::WAYLAND;
}

bool SessionSettings::hasWaylandPlasmaShellSupport()
{
    return wayland_has_plasma_shell;
}

bool SessionSettings::hasWaylandPlasmaWindowManagementSupport()
{
    return wayland_has_plasma_window_management;
}

bool SessionSettings::hasWaylandUKUIDecorationSupport()
{
    return wayland_has_ukui_decoration;
}

bool SessionSettings::hasWaylandXdgShellV6Support()
{
    return wayland_has_xdg_shell_v6;
}

bool SessionSettings::hasWaylandXdgShellSupport()
{
    return wayland_has_xdg_wm_base;
}

bool SessionSettings::hasWaylandXdgActivationV1Support()
{
    return wayland_has_xdg_activation_v1;
}

bool SessionSettings::hasWaylandGtkShell1Support()
{
    return wayland_has_gtk_shell1;
}

bool SessionSettings::hasWaylandRemoteAccessSupport()
{
    return wayland_has_remote_access;
}

bool SessionSettings::supportsWaylandXdgShell()
{
    return has_xdg_shell_support;
}
