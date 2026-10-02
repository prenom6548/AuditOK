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
#include "mouse-wayland-manager.h"
#include "clib-syslog.h"
#include "usd_base_class.h"

typedef enum {
        TOUCHPAD_HANDEDNESS_RIGHT,
        TOUCHPAD_HANDEDNESS_LEFT,
        TOUCHPAD_HANDEDNESS_MOUSE
} TouchpadHandedness;

MouseWaylandManager * MouseWaylandManager::mMouseWaylandManager =nullptr;

MouseWaylandManager::MouseWaylandManager(QObject *parent):QObject (parent)
{
    locate_pointer_spawned = false;
    locate_pointer_pid  = 0;
    settings_mouse  = new QGSettings(UKUI_MOUSE_SCHEMA);
    settings_touchpad = new QGSettings(UKUI_TOUCHPAD_SCHEMA);
    mMouseDeviceIface = new QList<QDBusInterface*>();
}
MouseWaylandManager::~MouseWaylandManager()
{
    delete settings_mouse;
    delete settings_touchpad;
    if(time)
        delete time;
    mMouseDeviceIface->clear();
    delete mMouseDeviceIface;
}

MouseWaylandManager * MouseWaylandManager::MouseWaylandManagerNew()
{
    if(nullptr == mMouseWaylandManager) {
        mMouseWaylandManager = new MouseWaylandManager(nullptr);
    }
    return mMouseWaylandManager;
}

bool MouseWaylandManager::start()
{
    USD_LOG(LOG_DEBUG,"-- wayland Mouse Start Manager --");
    time = new QTimer(this);
    connect(time, &QTimer::timeout, this, &MouseWaylandManager::MouseWaylandManagerIdleCb);
    time->start();
    return true;
}

void MouseWaylandManager::stop()
{

    USD_LOG(LOG_DEBUG,"-- Stopping Mouse Manager --");

    SetLocatePointer(FALSE);

    disconnect(mWaylandIface, SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceChange(QString)));
    disconnect(mWaylandIface, SIGNAL(deviceRemoved(QString)),
            this, SLOT(deviceChange(QString)));

    QObject::disconnect(settings_mouse,SIGNAL(changed(QString)),
                        this,SLOT(MouseCallback(QString)));
    QObject::disconnect(settings_touchpad,SIGNAL(changed(QString)),
                        this,SLOT(TouchpadCallback(QString)));


}

bool MouseWaylandManager::GetTouchpadHandedness (bool  mouse_left_handed)
{
    int a = settings_touchpad->getEnum(KEY_LEFT_HANDED);

    switch (a) {
    case TOUCHPAD_HANDEDNESS_RIGHT:
            return false;
    case TOUCHPAD_HANDEDNESS_LEFT:
            return true;
    case TOUCHPAD_HANDEDNESS_MOUSE:
            return mouse_left_handed;
    default:
            g_assert_not_reached ();
    }
}

void MouseWaylandManager::SetLeftHandedAll (bool mouse_left_handed,
                                     bool touchpad_left_handed)
{
   if(mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()){
        for(int i = 0; i < mMouseDeviceIface->size(); i++){
            mMouseDeviceIface->at(i)->setProperty("leftHanded", mouse_left_handed);
        }
    }
    if(mTouchDeviceFlag && mTouchDeviceIface->isValid()){
        mTouchDeviceIface->setProperty("leftHanded", touchpad_left_handed);
    }
}

void MouseWaylandManager::SetMouseMotion()
{
    if (mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()) {
        double mAccel;
        double motion_acceleration = settings_mouse->get(KEY_MOTION_ACCELERATION).toDouble();
        bool accel = settings_mouse->get(KEY_MOUSE_ACCEL).toBool();
        if (motion_acceleration <= 1.0) {
            motion_acceleration = 1.0;
        } else if (motion_acceleration >= 8.0) {
            motion_acceleration = 8.0;
        }

        mAccel = (motion_acceleration - 1.0) * 2.0 / 7.0 - 1;
        for(int i = 0; i < mMouseDeviceIface->size(); i++){
            mMouseDeviceIface->at(i)->setProperty("pointerAcceleration", mAccel);
            mMouseDeviceIface->at(i)->setProperty("pointerAccelerationProfileAdaptive", accel);
        }
    }
}

void MouseWaylandManager::SetTouchpadMotion()
{
    if (mTouchDeviceFlag && mTouchDeviceIface->isValid()) {
        double mAccel;
        double motion_acceleration = settings_touchpad->get(KEY_MOTION_ACCELERATION).toDouble();

        if (motion_acceleration <= 1.0) {
            motion_acceleration = 1.0;
        } else if (motion_acceleration >= 8.0) {
            motion_acceleration = 8.0;
        }
        mAccel = (motion_acceleration - 1.0) * 2.0 / 7.0 - 1;

        mTouchDeviceIface->setProperty("pointerAcceleration", mAccel);
        mTouchDeviceIface->setProperty("pointerAccelerationProfileAdaptive", true);
    }
}

void MouseWaylandManager::SetMiddleButtonAll (bool middle_button)
{


    if (mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()) {
       for (int i = 0; i < mMouseDeviceIface->size(); i++) {
            mMouseDeviceIface->at(i)->setProperty("MiddleButtonEmulation", middle_button);
        }
    }
}

void MouseWaylandManager::SetNaturalScrollMouse()
{
    bool naturalScroll = settings_mouse->get(KEY_TOUCHPAD_NATURAL_SCROLL).toBool();

    if(mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()){
        for(int i = 0; i < mMouseDeviceIface->size(); i++){
            mMouseDeviceIface->at(i)->setProperty("naturalScroll", naturalScroll);
        }
    }
}

void MouseWaylandManager::SetLocatePointer (bool     state)
{
    if (state) {
        GError *error = NULL;
        char **args;
        int    argc;

        if (locate_pointer_spawned)
            return;
        QString str = "/usr/bin/usd-locate-pointer";
        if( g_shell_parse_argv (str.toLatin1().data(), &argc, &args, NULL)){
            g_spawn_async (g_get_home_dir (),
                           args,
                           NULL,
                           G_SPAWN_SEARCH_PATH,
                           NULL,
                           NULL,
                           &locate_pointer_pid,
                           &error);
            locate_pointer_spawned = (error == NULL);
        }
        if (error) {
            settings_mouse->set(KEY_MOUSE_LOCATE_POINTER,false);
            g_error_free (error);
        }
        g_strfreev (args);
    } else if (locate_pointer_spawned) {
            kill (locate_pointer_pid, SIGHUP);
            g_spawn_close_pid (locate_pointer_pid);
            locate_pointer_spawned = FALSE;
    }
}

void MouseWaylandManager::SetMouseWheelSpeed (int speed)
{
    if(speed <= 0 )
          return;
    if(mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()){

        for(int i = 0; i < mMouseDeviceIface->size(); i++){
            mMouseDeviceIface->at(i)->setProperty("scrollFactor", speed);
        }
    }
}

void MouseWaylandManager::MouseCallback (QString keys)
{
    if (keys.compare(QString::fromLocal8Bit(KEY_LEFT_HANDED))==0){
        bool mouse_left_handed = settings_mouse->get(keys).toBool();
        bool touchpad_left_handed = GetTouchpadHandedness (mouse_left_handed);
        SetLeftHandedAll (mouse_left_handed, touchpad_left_handed);

    } else if ((keys.compare(QString::fromLocal8Bit(KEY_MOTION_ACCELERATION))==0) ||
               (keys.compare(QString::fromLocal8Bit(KEY_MOTION_THRESHOLD))==0) ||
               (keys.compare(QString::fromLocal8Bit(KEY_MOUSE_ACCEL)) == 0)){
        SetMouseMotion();
    } else if (keys.compare(QString::fromLocal8Bit(KEY_MIDDLE_BUTTON_EMULATION))==0){
        SetMiddleButtonAll (settings_mouse->get(keys).toBool());

    } else if (keys.compare(QString::fromLocal8Bit(KEY_MOUSE_LOCATE_POINTER))==0){
        SetLocatePointer (settings_mouse->get(keys).toBool());
    } else if(keys.compare(QString::fromLocal8Bit(KEY_MOUSE_WHEEL_SPEED)) == 0 ) {
        SetMouseWheelSpeed (settings_mouse->get(keys).toInt());
    } else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_NATURAL_SCROLL)) == 0) {
        SetNaturalScrollMouse ();                             //设置上移下滚或上移上滚
        USD_LOG(LOG_DEBUG,"set %s",KEY_TOUCHPAD_NATURAL_SCROLL);
    } else{
        USD_LOG(LOG_DEBUG,"keys:is skip..k%s", keys.toLatin1().data());
    }
}

void MouseWaylandManager::SetDisableWTyping (bool state)
{
    //wayland
    if(mTouchDeviceFlag && mTouchDeviceIface->isValid()){
        mTouchDeviceIface->setProperty("disableWhileTyping", state);
    }
}


void MouseWaylandManager::SetTapToClickAll ()
{
    if(mTouchDeviceFlag && mTouchDeviceIface->isValid()){
        bool tapClick = settings_touchpad->get(KEY_TOUCHPAD_TAP_TO_CLICK).toBool();
        mTouchDeviceIface->setProperty("tapToClick", tapClick);
    }
}

void MouseWaylandManager::SetScrollingAll (QString keys)
{
    // wayland
    if(mTouchDeviceFlag && mTouchDeviceIface->isValid()){
        bool edge, twoFinger;
        if (keys.compare(QString::fromLocal8Bit(KEY_VERT_EDGE_SCROLL)) == 0){
            edge = settings_touchpad->get(keys).toBool();
            mTouchDeviceIface->setProperty("scrollEdge", edge);
        } else if (keys.compare(QString::fromLocal8Bit(KEY_VERT_TWO_FINGER_SCROLL)) == 0){
            twoFinger = settings_touchpad->get(keys).toBool();
            mTouchDeviceIface->setProperty("scrollTwoFinger", twoFinger);
        }
    }
}


void MouseWaylandManager::SetNaturalScrollTouchPad ()
{
    //wayland
    bool naturalScroll = settings_touchpad->get(KEY_TOUCHPAD_NATURAL_SCROLL).toBool();

    if (mTouchDeviceFlag && mTouchDeviceIface->isValid()) {
        mTouchDeviceIface->setProperty("naturalScroll", naturalScroll);
    }
}



void MouseWaylandManager::SetTouchpadEnabledAll (bool state)
{
    //wayland
    if (mTouchDeviceFlag && mTouchDeviceIface->isValid()) {
        mTouchDeviceIface->setProperty("enabled", state);
    }
}

void MouseWaylandManager::SetPlugMouseDisbleTouchpad(QGSettings * settings)
{
    if(mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()) {
       bool state = settings->get(KEY_TOUCHPAD_DISBLE_O_E_MOUSE).toBool();
       if(state){
           if(mMouseDeviceFlag && !mMouseDeviceIface->isEmpty()){
               mTouchDeviceIface->setProperty("enabled", !state);
           } else {
               bool touchpadState = settings->get(KEY_TOUCHPAD_ENABLED).toBool();
               mTouchDeviceIface->setProperty("enabled", touchpadState);

           }
       }else{
           bool touchpadState = settings->get(KEY_TOUCHPAD_ENABLED).toBool();
           mTouchDeviceIface->setProperty("enabled", touchpadState);
       }
    } else {
        SetTouchpadEnabledAll(settings->get(KEY_TOUCHPAD_ENABLED).toBool());
    }
}

void MouseWaylandManager::SetTouchpadDoubleClickAll(bool state)
{
    //wayland
    if(mTouchDeviceFlag && mTouchDeviceIface->isValid()){
        return;
        //mTouchDeviceIface->setProperty("tapDragLock", state);
    }
}

void MouseWaylandManager::TouchpadCallback (QString keys)
{
    if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_DISABLE_W_TYPING))==0) {
        SetDisableWTyping (settings_touchpad->get(keys).toBool());  //设置打字时禁用触摸板

    } else if (keys.compare(QString::fromLocal8Bit(KEY_LEFT_HANDED))== 0) {
        bool mouse_left_handed = settings_mouse->get(keys).toBool();
        bool touchpad_left_handed = GetTouchpadHandedness (mouse_left_handed);
        SetLeftHandedAll (mouse_left_handed, touchpad_left_handed); //设置左右手

    } else if ((keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_TAP_TO_CLICK))    == 0)
            || (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_ONE_FINGER_TAP))  == 0)
            || (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_TWO_FINGER_TAP))  == 0)
            || (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_THREE_FINGER_TAP))== 0)) {
        SetTapToClickAll ();                                        //设置多指手势

    } else if ((keys.compare(QString::fromLocal8Bit(KEY_VERT_EDGE_SCROLL))  == 0)
            || (keys.compare(QString::fromLocal8Bit(KEY_HORIZ_EDGE_SCROLL)) == 0)
            || (keys.compare(QString::fromLocal8Bit(KEY_VERT_TWO_FINGER_SCROLL))  == 0)
            || (keys.compare(QString::fromLocal8Bit(KEY_HORIZ_TWO_FINGER_SCROLL)) == 0)) {
        //SetScrollingAll (settings_touchpad);                //设置滚动
        SetScrollingAll (keys);                //设置滚动
    } else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_NATURAL_SCROLL)) == 0) {
        SetNaturalScrollTouchPad ();                             //设置上移下滚或上移上滚
        USD_LOG(LOG_DEBUG,"set %s",KEY_TOUCHPAD_NATURAL_SCROLL);

    } else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_ENABLED)) == 0) {
        SetTouchpadEnabledAll (settings_touchpad->get(keys).toBool());//设置触摸板开关
             SetDisableWTyping (true);
    } else if (0 == QString::compare(keys, QString(KEY_MOTION_ACCELERATION), Qt::CaseInsensitive)||
              0 == QString::compare(keys, QString(KEY_MOTION_THRESHOLD), Qt::CaseInsensitive)){
        SetTouchpadMotion();                                    //设置鼠标速度

    }else if (keys == "motion-acceleration" || keys==KEY_MOTION_THRESHOLD){
    }else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_DISBLE_O_E_MOUSE)) == 0) {
        SetPlugMouseDisbleTouchpad(settings_touchpad);      //设置插入鼠标时禁用触摸板

    } else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_DOUBLE_CLICK_DRAG)) == 0){
        SetTouchpadDoubleClickAll(settings_touchpad->get(KEY_TOUCHPAD_DOUBLE_CLICK_DRAG).toBool());//设置轻点击两次拖动打开关闭

    } else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_BOTTOM_R_C_CLICK_M)) == 0){
//        SetBottomRightConrnerClickMenu(settings_touchpad->get(KEY_TOUCHPAD_BOTTOM_R_C_CLICK_M).toBool());//打开关闭右下角点击弹出菜单

    } else if (keys.compare(QString::fromLocal8Bit(KEY_TOUCHPAD_MOUSE_SENSITVITY)) == 0){

    } else {
        USD_LOG(LOG_DEBUG,"keys:is skip..k%s", keys.toLatin1().data(),keys.toLatin1().data());
    }
}

void MouseWaylandManager::SetMouseSettings ()
{
    bool mouse_left_handed = settings_mouse->get(KEY_LEFT_HANDED).toBool();
    bool touchpad_left_handed = GetTouchpadHandedness (mouse_left_handed);

    SetLeftHandedAll (mouse_left_handed, touchpad_left_handed);

    SetMouseMotion();
    SetMiddleButtonAll (settings_mouse->get(KEY_MIDDLE_BUTTON_EMULATION).toBool());
    SetMouseWheelSpeed (settings_mouse->get(KEY_MOUSE_WHEEL_SPEED).toInt());
    SetNaturalScrollMouse ();
}

void MouseWaylandManager::SetTouchSettings ()
{
    SetTapToClickAll ();
    //SetScrollingAll (settings_touchpad);
    SetTouchpadMotion();
    SetScrollingAll (KEY_VERT_EDGE_SCROLL);
    SetScrollingAll (KEY_VERT_TWO_FINGER_SCROLL);
    SetNaturalScrollTouchPad ();
    SetTouchpadEnabledAll (settings_touchpad->get(KEY_TOUCHPAD_ENABLED).toBool());
    SetPlugMouseDisbleTouchpad(settings_touchpad);
    SetTouchpadDoubleClickAll(settings_touchpad->get(KEY_TOUCHPAD_DOUBLE_CLICK_DRAG).toBool());
//    SetBottomRightConrnerClickMenu(settings_touchpad->get(KEY_TOUCHPAD_BOTTOM_R_C_CLICK_M).toBool());

}

void MouseWaylandManager::initWaylandMouseStatus()
{
    QVariant deviceReply = mWaylandIface->property("devicesSysNames");
    if (deviceReply.isValid()) {
        QStringList deviceList = deviceReply.toStringList();
        if(!mMouseDeviceIface->isEmpty()){
            mMouseDeviceIface->clear();
        }
        int i = 0;
        for (QString device : deviceList) {
            QDBusInterface *deviceIface = new QDBusInterface("org.ukui.KWin",
                                                             "/org/kde/KWin/InputDevice/"+ device,
                                                             "org.kde.KWin.InputDevice",
                                                             QDBusConnection::sessionBus(),
                                                             this);
            if(!deviceIface->isValid()) {
                deviceIface = new QDBusInterface("org.kde.KWin",
                                                                 "/org/kde/KWin/InputDevice/"+ device,
                                                                 "org.kde.KWin.InputDevice",
                                                                 QDBusConnection::sessionBus(),
                                                                 this);
            }

            if (deviceIface->isValid() &&
                deviceIface->property("pointer").toBool() &&
                !deviceIface->property("keyboard").toBool() &&
                !deviceIface->property("touchpad").toBool()) {
                mMouseDeviceIface->insert(i++, deviceIface);
                mMouseDeviceFlag = true;
//                USD_LOG(LOG_DEBUG,"-MouseDevice-----%s",device.toLatin1().data());

            }

            if (deviceIface->isValid() &&
                deviceIface->property("pointer").toBool() &&
                deviceIface->property("touchpad").toBool()){
                mTouchDeviceIface = deviceIface;
                mTouchDeviceFlag = true;
//                USD_LOG(LOG_DEBUG,"TouchDevice------->%s",device.toLatin1().data());

            }
        }

        if(mTouchDeviceFlag && mTouchDeviceIface){
            QList<QDBusInterface*>::iterator listIterator = mMouseDeviceIface->begin();
            QDBusInterface *data = nullptr;
            while (listIterator != mMouseDeviceIface->end()) {
                data = *listIterator;
                if (data->property("product").toInt() == mTouchDeviceIface->property("product").toInt()) {
                    listIterator = mMouseDeviceIface->erase(listIterator);
                    delete data;
                } else {
                    listIterator++;
                }
            }
        }
    }
}

void MouseWaylandManager::deviceChange(QString dev)
{
    initWaylandMouseStatus();
    SetMouseSettings ();
    SetPlugMouseDisbleTouchpad(settings_touchpad);
}

void MouseWaylandManager::initWaylandDbus()
{
    mWaylandIface = new QDBusInterface("org.ukui.KWin",
                                       "/org/kde/KWin/InputDevice",
                                       "org.kde.KWin.InputDeviceManager",
                                       QDBusConnection::sessionBus(),
                                       this);
    if(!mWaylandIface->isValid()) {
        mWaylandIface = new QDBusInterface("org.kde.KWin",
                                           "/org/kde/KWin/InputDevice",
                                           "org.kde.KWin.InputDeviceManager",
                                           QDBusConnection::sessionBus(),
                                           this);
    }
    if (mWaylandIface->isValid()) {
        connect(mWaylandIface, SIGNAL(deviceAdded(QString)),
                this, SLOT(deviceChange(QString)));
        connect(mWaylandIface, SIGNAL(deviceRemoved(QString)),
                this, SLOT(deviceChange(QString)));

        initWaylandMouseStatus();
    }
}

void MouseWaylandManager::MouseWaylandManagerIdleCb()
{
    initWaylandDbus();
    time->stop();

    QObject::connect(settings_mouse,SIGNAL(changed(QString)),  this,SLOT(MouseCallback(QString)));
    QObject::connect(settings_touchpad,SIGNAL(changed(QString)),this,SLOT(TouchpadCallback(QString)));

    //wayland
    if (!mMouseDeviceIface->isEmpty()) {
        SetMouseSettings ();
    }
    if (mTouchDeviceIface) {
        SetTouchSettings();
    }
    SetLocatePointer (settings_mouse->get(KEY_MOUSE_LOCATE_POINTER).toBool());
}
