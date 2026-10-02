#include <QCoreApplication>
#include <QGuiApplication>

#include <QtDBus/QDBusConnection>
#include <QtDebug>

#include "common.h"
#include "screen.h"
#include "audio.h"
#include "wifi.h"
#include "lan.h"
#include "bluetooth.h"
#include "power.h"
#include "touchpad.h"
#include "keyboard.h"
#include "panel_config.h"
#include "mouse.h"
#include "workstation.h"
#include "search-config.h"

#include "theme.h"
#include "background.h"
#include "font.h"
#include "effect.h"
#include "navigations.h"

#include "osdserver.h"
#include "shortcut_navigations.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication a(argc, argv);
    // Todo
    // 启动后台服务，加载Dbus接口,注册DBus对象
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    QDBusConnection::RegisterOptions registerOption =  QDBusConnection::ExportAllSlots |
                                                      QDBusConnection::ExportAllSignals |
                                                      QDBusConnection::ExportAllProperties;
    if (sessionBus.registerService(DBUS_NAME)) {
        sessionBus.registerObject(DEVICES_SCREEN_PATH, new Screen(),
                                 registerOption
                                 );
        sessionBus.registerObject(DEVICES_AUDIO_PATH, new Audio(),
                                 registerOption
                                 );
        BLUETOOTHDBUS::instance();
        sessionBus.registerObject(DEVICES_BLUETOOTH_PATH, BLUETOOTHDBUS::getInstance(),
                                 registerOption
                                 );
        sessionBus.registerObject(DEVICES_WIFI_PATH, new Wifi(),
                                 registerOption
                                 );
        sessionBus.registerObject(DEVICES_LAN_PATH, new Lan(),
                                 registerOption
                                 );
        sessionBus.registerObject(DEVICES_POWER_PATH, new UkuiPowerInterface(),
                                 registerOption
                                 );
        
        sessionBus.registerObject(DEVICES_TOUCHPAD_PATH, new TouchpadManager(),
                                 registerOption
                                 );

        sessionBus.registerObject(DEVICES_KEYBOARD_PATH, new Keyboard(),
                                 registerOption
                                 );

        sessionBus.registerObject(DEVICES_MOUSE_PATH, new MouseManager(),
                                 registerOption
                                 );

        QDBusConnection::RegisterOptions uiOptions = QDBusConnection::ExportAllSlots |
                QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

        sessionBus.registerObject(UI_THEME_PATH, new Theme(&a), uiOptions);
        sessionBus.registerObject(UI_WALLPAPER_PATH, new Background(&a), uiOptions);
        sessionBus.registerObject(UI_FONT_PATH, new Font(&a), uiOptions);
        sessionBus.registerObject(UI_EFFECTS_PATH, new Effect(&a), uiOptions);
        sessionBus.registerObject(UI_Panel_PATH, new ConfigUkPanel(&a), uiOptions);
        sessionBus.registerObject(NAVIGATIONS_PATH, new NavigationUKcc(&a),uiOptions);
        sessionBus.registerObject(SHORTCUT_NAVIGATIONS_PATH, new NavigationShortcut(&a),uiOptions);
        sessionBus.registerObject(UI_OSDSERVER_PATH, new OsdServer(&a), uiOptions);
        sessionBus.registerObject(UI_WORKSTATION_PATH, new ConfigWorkstation(&a), uiOptions);
        sessionBus.registerObject(UI_SEARCH_PATH, new SearchConfig(&a), uiOptions);
    }

    return a.exec();
}
