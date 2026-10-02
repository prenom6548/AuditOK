#ifndef MOUSECOMMON_H
#define MOUSECOMMON_H

#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QtX11Extras/QX11Info>
#include <QGSettings/qgsettings.h>
#include <glib.h>
#include <errno.h>
#include <math.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

#include <QDBusInterface>
#include <QDBusReply>

/* Keys with same names for both touchpad and mouse */
#define KEY_LEFT_HANDED                  "left-handed"          /*  a boolean for mouse, an enum for touchpad */
#define KEY_MOTION_ACCELERATION          "motion-acceleration"
#define KEY_MOTION_THRESHOLD             "motion-threshold"

/* Mouse settings */
#define UKUI_MOUSE_SCHEMA                "org.ukui.peripherals-mouse"
#define KEY_MOUSE_LOCATE_POINTER         "locate-pointer"
#define KEY_MIDDLE_BUTTON_EMULATION      "middle-button-enabled"
#define KEY_MOUSE_WHEEL_SPEED            "wheel-speed"
#define KEY_MOUSE_ACCEL                  "mouse-accel"
#define KEY_MOUSE_NATRUAL_SCROLLING      "natural-scroll"

/* Touchpad settings */
#define UKUI_TOUCHPAD_SCHEMA             "org.ukui.peripherals-touchpad"
#define KEY_TOUCHPAD_DISABLE_W_TYPING    "disable-while-typing"
#define KEY_TOUCHPAD_TWO_FINGER_CLICK    "two-finger-click"
#define KEY_TOUCHPAD_THREE_FINGER_CLICK  "three-finger-click"
#define KEY_TOUCHPAD_NATURAL_SCROLL      "natural-scroll"
#define KEY_TOUCHPAD_TAP_TO_CLICK        "tap-to-click"
#define KEY_TOUCHPAD_ONE_FINGER_TAP      "tap-button-one-finger"
#define KEY_TOUCHPAD_TWO_FINGER_TAP      "tap-button-two-finger"
#define KEY_TOUCHPAD_THREE_FINGER_TAP    "tap-button-three-finger"
#define KEY_VERT_EDGE_SCROLL             "vertical-edge-scrolling"
#define KEY_HORIZ_EDGE_SCROLL            "horizontal-edge-scrolling"
#define KEY_VERT_TWO_FINGER_SCROLL       "vertical-two-finger-scrolling"
#define KEY_HORIZ_TWO_FINGER_SCROLL      "horizontal-two-finger-scrolling"
#define KEY_TOUCHPAD_ENABLED             "touchpad-enabled"

#define KEY_TOUCHPAD_DISBLE_O_E_MOUSE    "disable-on-external-mouse"        //插入鼠标，禁用触摸板  true/false
#define KEY_TOUCHPAD_DOUBLE_CLICK_DRAG   "double-click-drag"                //点击两次拖动		true/false
#define KEY_TOUCHPAD_BOTTOM_R_C_CLICK_M  "bottom-right-corner-click-menu"   //右下角点击菜单	true/false
#define KEY_TOUCHPAD_MOUSE_SENSITVITY    "mouse-sensitivity"                //鼠标敏感度	1-4  四个档位  低中高最高


#endif // MOUSECOMMON_H
