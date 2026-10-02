/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "keyboard-wayland-manager.h"
#include "clib-syslog.h"
#include "config.h"
#include "rfkillswitch.h"
#include "usd_base_class.h"
#include "clib-syslog.h"

#define USD_KEYBOARD_SCHEMA  "org.ukui.peripherals-keyboard"

#define UKCCOSD_SCHEMA "org.ukui.control-center.osd"
#define KYCCOSD_SCHEMA "org.kylin.control-center.osd"
#define KDSOSD_SCHEMA "org.ukui.kds.osd"

#define SHOW_TIP_KEY "show-lock-tip"

#define KEY_REPEAT           "repeat"
#define KEY_CLICK            "click"
#define KEY_RATE             "rate"
#define KEY_DELAY            "delay"
#define KEY_CLICK_VOLUME     "click-volume"
#define KEY_BELL_PITCH       "bell-pitch"
#define KEY_BELL_DURATION    "bell-duration"
#define KEY_BELL_MODE        "bell-mode"
#define KEY_NUMLOCK_STATE    "numlock-state"
#define KEY_CAPSLOCK_STATE   "capslock-state"
#define KEY_NUMLOCK_REMEMBER "remember-numlock-state"
#define KEY_DEFAULT_RATE 25
#define KEY_DEFAULT_DELAY 660


KeyboardWaylandManager *KeyboardWaylandManager::mKeyboardWaylandManager = nullptr;

KeyboardWaylandManager::KeyboardWaylandManager(QObject * parent)
{

    settings = new QGSettings(USD_KEYBOARD_SCHEMA);

    stInstalled = true;
    time = new QTimer(this);

    const QByteArray id(UKCCOSD_SCHEMA);
    const QByteArray idd(KYCCOSD_SCHEMA);
    const QByteArray iid(KDSOSD_SCHEMA);


    if (QGSettings::isSchemaInstalled(id)){
        ksettings = new QGSettings(id);
    } else if (QGSettings::isSchemaInstalled(idd)){
        ksettings = new QGSettings(idd);
    } else if (QGSettings::isSchemaInstalled(iid)){
        ksettings = new QGSettings(iid);
    } else {
        stInstalled = false;
    }
    m_keyState = new KeyState(this);
    m_capsLock = Keystate::State::Unlocked;
    m_numLock = Keystate::State::Unlocked;

}

KeyboardWaylandManager::~KeyboardWaylandManager()
{
    if (settings) {
        delete settings;
        settings = nullptr;
    }
    if (time) {
        delete time;
        time = nullptr;
    }
    if (ksettings) {
        delete ksettings;
        ksettings = nullptr;
    }
}

KeyboardWaylandManager *KeyboardWaylandManager::KeyboardWaylandManagerNew()
{
    if (nullptr == mKeyboardWaylandManager)
        mKeyboardWaylandManager = new KeyboardWaylandManager(nullptr);
    return  mKeyboardWaylandManager;
}


bool KeyboardWaylandManager::Start()
{
    USD_LOG(LOG_DEBUG,"-- Keyboard wayland Start Manager --");

    connect(time,SIGNAL(timeout()),this,SLOT(start_keyboard_idle_cb()));
    time->start(1500);

    return true;
}

void KeyboardWaylandManager::Stop()
{
    USD_LOG(LOG_DEBUG,"-- Keyboard wayland Stop Manager --");

}



static gboolean xkb_set_keyboard_autorepeat_rate(int delay, int rate)
{
    int interval = (rate <= 0) ? 1000000 : 1000/rate;
    Display *dpy = QX11Info::display();
    if (delay <= 0)
    {
        delay = 1;
    }
    return XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd, delay, interval);
}

void apply_repeat (KeyboardWaylandManager *manager)
{
    bool    repeat;
    int     rate;
    int     delay;
    //Display *dpy = QX11Info::display();

    repeat  = manager->settings->get(KEY_REPEAT).toBool();
    rate    = manager->settings->get(KEY_RATE).toInt();
    delay   = manager->settings->get(KEY_DELAY).toInt();

    if (rate < 0)
        rate = KEY_DEFAULT_RATE;
    if (delay < 0)
        delay = KEY_DEFAULT_DELAY;

    USD_LOG(LOG_DEBUG,"repeat = %d, rate = %d, delay = %d", repeat, rate, delay);

    if (UsdBaseClass::isWayland()) {
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.KWin"),
                                                                QStringLiteral("/KWin"),
                                                                QStringLiteral("org.ukui.KWin"),
                                                                QStringLiteral("setKeyboardRepeat"));
        QList<QVariant> args;
        args.append(repeat);
        args.append(rate);
        args.append(delay);
        message.setArguments(args);
        QDBusConnection::sessionBus().asyncCall(message);
    }
}



void KeyboardWaylandManager::apply_settings (QString keys)
{
    /**
     * Fix by HB* system reboot but rnumlock not available;
    **/

    if (keys.compare(QString::fromLocal8Bit(KEY_CLICK)) == 0||
        keys.compare(QString::fromLocal8Bit(KEY_CLICK_VOLUME)) == 0 ||
        keys.compare(QString::fromLocal8Bit(KEY_BELL_PITCH)) == 0 ||
        keys.compare(QString::fromLocal8Bit(KEY_BELL_DURATION)) == 0 ||
        keys.compare(QString::fromLocal8Bit(KEY_BELL_MODE)) == 0) {
                USD_LOG(LOG_DEBUG,"Bell setting '%s' changed, applying bell settings", keys.toLatin1().data());
//                apply_bell (this);
    } else if (keys.compare(QString::fromLocal8Bit(KEY_NUMLOCK_REMEMBER)) == 0) {
             USD_LOG(LOG_DEBUG,"Remember Num-Lock state '%s' changed, applying num-lock settings",  keys.toLatin1().data());
//            apply_numlock (this);
    } else if (keys.compare(QString::fromLocal8Bit(KEY_NUMLOCK_STATE)) == 0) {
             USD_LOG(LOG_DEBUG,"Num-Lock state '%s' changed, will apply at next startup",  keys.toLatin1().data());

    } else if (keys.compare(QString::fromLocal8Bit(KEY_REPEAT)) == 0 ||
               keys.compare(QString::fromLocal8Bit(KEY_RATE)) == 0 ||
               keys.compare(QString::fromLocal8Bit(KEY_DELAY)) == 0) {
             USD_LOG(LOG_DEBUG,"Key repeat setting '%s' changed, applying key repeat settings",  keys.toLatin1().data());
            apply_repeat (this);

    } else {
//          USD_LOG(LOG_DEBUG,"Unhandled settings change, key '%s'", key);
    }

}

void KeyboardWaylandManager::onKeyStateChange(Keystate::Key key,Keystate::State state)
{
    USD_LOG(LOG_DEBUG,"key:%d state:%d", key, state);
    switch (key) {
    case Keystate::Key::CapsLock:
        if(state != m_capsLock) {
            m_capsLock = state;
            if (stInstalled && !ksettings->get(SHOW_TIP_KEY).toBool()){
                //提示关闭时不弹窗
                return;
            }
            if(m_capsLock == Keystate::State::Locked) {
                showCustomOsdByDbus("ukui-capslock-on");
            } else {
                showCustomOsdByDbus("ukui-capslock-off");
            }
        }
        break;
    case Keystate::Key::NumLock:
        if(state != m_numLock) {
            m_numLock = state;
            if (stInstalled && !ksettings->get(SHOW_TIP_KEY).toBool()){
                //提示关闭时不弹窗
                return;
            }
            if(m_numLock == Keystate::State::Locked) {
                showCustomOsdByDbus("ukui-numlock-on");
            } else {
                showCustomOsdByDbus("ukui-numlock-off");
            }
        }
        break;
    case Keystate::Key::ScrollLock:

        break;
    default:
        break;
    }
}

void KeyboardWaylandManager::start_keyboard_idle_cb ()
{
    //wayland
    time->stop();
    m_keyState->connectInit();
    connect(settings,SIGNAL(changed(QString)),this,SLOT(apply_settings(QString)));
    connect(m_keyState ,&KeyState::keyStateChange,this,&KeyboardWaylandManager::onKeyStateChange);
    apply_repeat (this);
}

KeyState::KeyState(QObject* parent)
{

}

void KeyState::connectInit()
{
    //获取wl_display 连接
    ConnectionThread* connection = ConnectionThread::fromApplication();
    if (connection) {
        m_registry.create(connection);
        m_registry.setup();

        QObject::connect(&m_registry,&Registry::interfacesAnnounced,[this]{
            const bool hasKeystate = m_registry.hasInterface(Registry::Interface::Keystate);
            if(hasKeystate){
                auto keyStateInterface = m_registry.interface(Registry::Interface::Keystate);
                Keystate *keystate = m_registry.createKeystate(keyStateInterface.name,keyStateInterface.version);
                QObject::connect(keystate,&Keystate::stateChanged,keystate,[=](Keystate::Key key,Keystate::State state){
    //                qDebug()<<"key: "<<key<<"===state:"<<state;
                    Q_EMIT keyStateChange(key,state);
                });
//                keystate->fetchStates();
            } else {
                USD_LOG(LOG_DEBUG,"wl_display no interface keystate ");
            }
        });
    } else {
        USD_LOG(LOG_ERR, "get wl_display error .");
    }
}


void KeyboardWaylandManager::showCustomOsdByDbus(const QString& icon)
{
    QDBusMessage message = QDBusMessage::createMethodCall(FRAMEWORK_DBUS_ID,
                                    FRAMEWORK_DBUS_PATH,
                                    FRAMEWORK_DBUS_INTERFACE,
                                    "ShowCustomOSD");

    message << icon;
    QDBusConnection::sessionBus().asyncCall(message);
}


