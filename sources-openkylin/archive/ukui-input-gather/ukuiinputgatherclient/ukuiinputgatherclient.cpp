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
 */

//! \warning Because BOOL and Status definitions conflict,
//! must be include befor other header files
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "ukuiinputgatherclient.h"

#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <cstdio>
#include <string>

/**
 * Unix domain socket (abstract namespace).
 */
static const std::string SOCKET_NAME{"\0/InputGatherServer", 10}; // NOLINT

bool m_flagGesture = true;

void obtainDeviceList(); // 获取当前设备列表并对设备进行遍历
void checkTouchpadState(); // 检测触控板属性(Device Enable)变化情况，属性变化时候禁用手势
void findTouchpadState(Display* display,XDeviceInfo *deviceInfo); // 获取触控板的状态
Atom hasPropertyFromName (Display *display,const char *propertyName); // 根据名称获取属性
bool deviceHasProperty (Display *display,XDevice    *device,char *propertyName);
XDevice* deviceIsTouchpad (Display *display,XDeviceInfo *deviceinfo); //判断当前设备是否是触控板

bool deviceHasProperty (Display *display,XDevice* device,char *property_name)
{
    //LCOV_EXCL_START
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (display, property_name, True);
    if (!prop)
            return false;

    try {
        if ((XGetDeviceProperty (display, device, prop, 0, 1, False,
                                XA_INTEGER, &realtype, &realformat, &nitems,
                                &bytes_after, &data) == Success) && (realtype != None)) {
                XFree (data);
                return true;
        }

    } catch (int x) {

    }
    return false;
    //LCOV_EXCL_STOP
}

XDevice* deviceIsTouchpad (Display *display,XDeviceInfo *deviceinfo)
{

    XDevice *device;
    if(!display)
        return NULL;
    if (deviceinfo->type != XInternAtom (display, XI_TOUCHPAD, True))
            return NULL;
    //LCOV_EXCL_START
    try {
        device = XOpenDevice (display, deviceinfo->id);
        if (device == NULL)
            throw 1;

        if (deviceHasProperty (display,device, (char*)"libinput Tapping Enabled") ||
            deviceHasProperty (display,device, (char*)"Synaptics Off")) {
                return device;
        }
        XCloseDevice (display, device);

    } catch (int x) {
        return NULL;
    }
    return NULL;
    //LCOV_EXCL_STOP
}

Atom hasPropertyFromName (Display *display, const char *propertyName)
{
    //LCOV_EXCL_START
    return XInternAtom (display, propertyName, False);
    //LCOV_EXCL_STOP
}

void findTouchpadState(Display* display, XDeviceInfo *deviceInfo)
{
    XDevice * device = deviceIsTouchpad(display,deviceInfo);
    if(!device)
        return;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;
    Atom realtype, prop;
    //LCOV_EXCL_START
    prop = hasPropertyFromName(display, "Device Enabled");
    if(!prop)
        return;
//    printf("device info name %s\n",device_info->name);
    int iResult = XGetDeviceProperty (display, device, prop, 0, 1, False,XA_INTEGER, &realtype, &realformat, &nitems,
                                      &bytes_after, &data);
    if (iResult== Success) {
//        printf("date = %u\n",data[0]);
        if(data[0] == 1){
            m_flagGesture = true;
        }else{
            m_flagGesture = false;
        }
        XFree(data);
    }
    XCloseDevice (display, device);
    //LCOV_EXCL_STOP
}

void obtainDeviceList()
{
    int numdevices = 0;
    auto display = XOpenDisplay(nullptr);
    if (!display)
        return;

    XDeviceInfo *devicelist = XListInputDevices(display,&numdevices);
    if (devicelist == NULL){
        return;
    }
    for (int i = 0; i < numdevices; i++) {
        findTouchpadState(display, &devicelist[i]);
    }
    XFreeDeviceList (devicelist);
    XCloseDisplay(display);
}
void checkTouchpadState()
{
    //LCOV_EXCL_START
//    printf("checkTouchpadState------每隔一段事件对设备属性状态进行查询----\n");
    std::thread touchpadPropsThread([=]() {
        auto display = XOpenDisplay(nullptr);
        if (display == nullptr) {
            return;
        }

        int xi_opcode, event, error;
        XIEventMask m;

        Atom prop = XInternAtom(display, "Device Enabled", True);
        Atom realtype;
        int realformat;
        unsigned long nitems, bytes_after;
        unsigned char *data;

        if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
            qWarning() << Q_FUNC_INFO << "X Input extension not available.";
        }

        m.deviceid = XIAllDevices;
        m.mask_len = XIMaskLen(XI_LASTEVENT);
        m.mask = static_cast<unsigned char*>(calloc(m.mask_len, sizeof(char)));
        XISetMask(m.mask, XI_PropertyEvent);

        XISelectEvents(display, DefaultRootWindow(display), &m, 1);
        XSync(display, False);
        free(m.mask);

        while (1) {
            XEvent ev;
            XGenericEventCookie *cookie = (XGenericEventCookie*)&ev.xcookie;
            XNextEvent(display, (XEvent*)&ev);

            if (!XGetEventData(display, cookie) ||
                cookie->type != GenericEvent ||
                cookie->extension != xi_opcode ||
                cookie->evtype != XI_PropertyEvent) {

                XFreeEventData(display, cookie);
                continue;
            }

            XIPropertyEvent* event = static_cast<XIPropertyEvent*>(cookie->data);
            char* name = XGetAtomName(display, event->property);
            if (strncmp(name, "Device Enabled", sizeof("Device Enabled")) != 0) {
                XFree(name);
                XFreeEventData(display, cookie);
                continue;
            }
            XFree(name);

            // 在当前设备列表中寻找设备，如果设备已经不在则略过这次事件
            int ndevices;
            XDeviceInfo* infos = XListInputDevices(display, &ndevices);
            XDeviceInfo* eventDeviceInfo = NULL;

            for (int i = 0; i < ndevices; ++i) {
                if (infos[i].id == (XID)event->deviceid)
                    eventDeviceInfo = &infos[i];
            }
            if (!eventDeviceInfo) {
                XFreeEventData(display, cookie);
                continue;
            }

            // 判断设备是否为触控板
            XDevice* device = deviceIsTouchpad(display, eventDeviceInfo);
            if (!device) {
                XFreeDeviceList(infos);
                XFreeEventData(display, cookie);
                continue;
            }

            // 获取当前 Enable 属性
            Status result = XGetDeviceProperty (display, device, prop, 0, 1, False,
                                                XA_INTEGER, &realtype, &realformat, &nitems,
                                                &bytes_after, &data);
            if (result == Success) {
                if (data[0] == 1)
                    m_flagGesture = true;
                else if (data[0] == 0)
                    m_flagGesture = false;
                qDebug() << __FUNCTION__ << __LINE__
                         << "TouchPad Enable property changed to:" << m_flagGesture;
            }
            XCloseDevice(display, device);
            XFree(data);
            XFreeEventData(display, cookie);
        }
        XCloseDisplay(display);
    });
    touchpadPropsThread.detach();
    //LCOV_EXCL_STOP
}


UKUIInputGatherClient::UKUIInputGatherClient()
    :m_event(nullptr), m_sendEventCallBack(nullptr)
{

    m_event = new Event;
    obtainDeviceList();
}

UKUIInputGatherClient::~UKUIInputGatherClient()
{
    if(m_event)
        delete m_event;
}

void UKUIInputGatherClient::setEventCallBack(std::function<void (Event *)> sendEvent)
{
    if(!sendEvent){
        // TOOD log
        return;
    }
//    //手势标志位==false  禁止上传事件
//    if(!m_flagGesture)
//        return;
    m_sendEventCallBack = sendEvent;
}

void UKUIInputGatherClient::startToReceiveEvent()
{
    printf("UKUIInputGatherClient: 开始进行监听Socket!获取事件\n");

    const char *env = std::getenv("XDG_SESSION_TYPE");
    if (env && strcmp(env, "x11") == 0) {
        checkTouchpadState();
    } else {
        qInfo() << "XDG_SESSION_TYPE" << env;
    }

    std::thread receiveThread{[ this ]() {
        bool connected = false;
        while(true)
        {
            if(!connected){
                if(connectToServer()){
                    printf("UKUIInputGatherClient: 服务端链接成功!\n");
                    connected = true;
                }else{
                    connected = false;
                }
            }
            if (connected) {
                int total = read(m_socket, m_event, sizeof(Event));
                if (total <= 0) {
                    connected = false;
                } else {
                    if(m_sendEventCallBack){
                        if(m_flagGesture)
                            m_sendEventCallBack(m_event);
                    }
                }
            }
            if (!connected) {
                close(m_socket);
                int retryTime = 5;
                printf("UKUIInputGatherClient: Socket链接失败！%ds 后开始重新尝试链接！\n", retryTime);
                std::this_thread::sleep_for(std::chrono::seconds(retryTime));
            }
        }
    }};
    receiveThread.detach();
}

bool UKUIInputGatherClient::connectToServer()
{
    bool connected = false;

    m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket == -1) {
        return false;
    }

    struct sockaddr_un addr = {};
    addr.sun_family         = AF_UNIX;
    memset(&addr.sun_path[ 0 ], 0, sizeof(addr.sun_path));
    SOCKET_NAME.copy(&addr.sun_path[ 0 ], SOCKET_NAME.size(), 0);

    auto socketAddr = reinterpret_cast<sockaddr *>(&addr); // NOLINT
    if (connect(m_socket, socketAddr, sizeof(addr)) == -1) {
        connected = false;
    } else {
        connected = true;
    }

    return connected;
}

int UKUIInputGatherClient::libinputTabletSwitchState()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.ukui.libinput.proxy",
                                                      "/org/ukui/libinput/proxy",
                                                      "org.ukui.libinput.proxy",
                                                      "libinputTabletSwitchState");
    QDBusMessage re = QDBusConnection::systemBus().call(msg);
    if (re.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Call method libinputTabletSwitchState failed" << re.errorName() << re.errorMessage();
        return -1;
    }
    else if (re.type() == QDBusMessage::ReplyMessage) {
        return re.arguments().takeFirst().toInt();
    }
    return -1;
}

QJsonArray UKUIInputGatherClient::libinputDeviceList()
{
    QJsonArray deviceInfoJsonArray;
    QDBusMessage msg = QDBusMessage::createMethodCall("org.ukui.libinput.proxy",
                                                      "/org/ukui/libinput/proxy",
                                                      "org.ukui.libinput.proxy",
                                                      "libinputDeviceList");
    QDBusMessage re = QDBusConnection::systemBus().call(msg);
    if (re.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Call method libinputDeviceList failed" << re.errorName() << re.errorMessage();
        return deviceInfoJsonArray;
    }
    else if (re.type() == QDBusMessage::ReplyMessage) {
        QByteArray deviceInfoByteArray = re.arguments().takeFirst().toByteArray();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(deviceInfoByteArray, &err);
        if (doc.isNull() || err.error != QJsonParseError::NoError) {
            qWarning() << "Json Document parse error";
            return deviceInfoJsonArray;
        }

        deviceInfoJsonArray = doc.array();
        return deviceInfoJsonArray;
    }
    return deviceInfoJsonArray;
}
