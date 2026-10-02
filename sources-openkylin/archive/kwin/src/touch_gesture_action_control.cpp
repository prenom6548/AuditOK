#include "touch_gesture_action_control.h"

// will error if include after xlib
#include <QDebug>

#include "input.h"
#include "linux/input-event-codes.h"
#include "wayland_server.h"

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include <QByteArray>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QThread>
#include <QVariant>
#include <QtConcurrent/qtconcurrentrun.h>
#include <kysdk/diagnosetest/libkydatacollect.h>

const char *serviceName = "com.kylin.statusmanager.interface";
const char *interfacePath = "/";
const char *interfaceName = "com.kylin.statusmanager.interface";
const char *signalName = "mode_change_signal";
const char *getModeMethod = "get_current_tabletmode";

namespace KWin
{

// 键盘按键映射表
static QMap<QString, quint32> keyCode{
    {"a", KEY_A},
    {"b", KEY_B},
    {"c", KEY_C},
    {"d", KEY_D},
    {"e", KEY_E},
    {"f", KEY_F},
    {"g", KEY_G},
    {"h", KEY_H},
    {"i", KEY_I},
    {"g", KEY_G},
    {"k", KEY_K},
    {"l", KEY_L},
    {"m", KEY_M},
    {"n", KEY_N},
    {"o", KEY_O},
    {"p", KEY_P},
    {"q", KEY_Q},
    {"r", KEY_R},
    {"s", KEY_S},
    {"t", KEY_T},
    {"u", KEY_U},
    {"v", KEY_V},
    {"w", KEY_W},
    {"x", KEY_X},
    {"y", KEY_Y},
    {"z", KEY_Z},
    {"Tab", KEY_TAB},
    {"Super_L", KEY_LEFTMETA},
    {"Alt_L", KEY_LEFTALT},
    {"Left", KEY_LEFT},
    {"Right", KEY_RIGHT},
    {"Down", KEY_DOWN},
    {"Up", KEY_UP},
    {"Shift_L", KEY_LEFTSHIFT},
    {"Control_R", KEY_RIGHTCTRL},
    {"Control_L", KEY_LEFTCTRL},
    {"Alt_R", KEY_RIGHTALT},
    {"Super_R", KEY_RIGHTMETA},
    {"Shift_R", KEY_RIGHTSHIFT}};

static QMap<quint32, quint32> btnCode{
    {1, BTN_LEFT},
    {2, BTN_MIDDLE},
    {3, BTN_RIGHT},
    {4, BTN_FORWARD},
    {5, BTN_BACK}};

KylinStatusInterface *KylinStatusInterface::s_self = nullptr;
bool KylinStatusInterface::isInterfaceInstalled = true;

KylinStatusInterface::KylinStatusInterface(const QDBusConnection &connection, QObject *parent = nullptr)
    : QDBusAbstractInterface(serviceName, interfacePath, interfaceName, connection, parent)
{
    bool ret = QDBusConnection::sessionBus().connect(serviceName, interfacePath,
                                                     interfaceName, signalName, this,
                                                     SLOT(setTabletMode(bool)));
    if (ret) {
        //        qDebug() << "\t信号连接成功！";
    } else {
        //        qDebug() << "\t信号连接失败！";
    }
}

KylinStatusInterface::~KylinStatusInterface()
{
    if (s_self) {
        delete s_self;
        s_self = nullptr;
    }
}

bool KylinStatusInterface::isTabletMode()
{
    if (s_self) {
        QDBusMessage reply = s_self->call(QLatin1String(getModeMethod));
        if (reply.type() == QDBusMessage::ReplyMessage)
            return reply.arguments().takeFirst().toBool();
        else
            return false;
    } else
        return false;
}

bool KylinStatusInterface::setTabletMode(bool isTablet)
{
    GestureAndActionBase::setTabletMode(isTablet);
    return true;
}

KylinStatusInterface *KylinStatusInterface::getInstance()
{
    if (s_self)
        return s_self;
    else if (isInterfaceInstalled) {
        s_self = new KylinStatusInterface(QDBusConnection::sessionBus());
        if (s_self->isValid())
            return s_self;
        else {
            isInterfaceInstalled = false;
            return nullptr;
        }
    } else
        return nullptr;
}

KylinStatusInterface *GestureAndActionBase::s_interface = nullptr;
bool GestureAndActionBase::s_isTabletMode = false;

GestureAndActionBase::GestureAndActionBase(GestureInfo gestureInfo, GestureConfig gestureConfig)
    : m_gestureType(strToGestureType(gestureInfo.type))
    , m_touchDevice(strToTouchDevice(gestureInfo.device))
    , m_fingers(gestureInfo.fingers.toInt())
    , m_gestureDirection(strToGestureDirection(gestureInfo.direction))
    , m_actionType(gestureConfig.action.type)
    , m_gestureAction(gestureConfig.action.settings)
    , m_blacklist(gestureConfig.blacklist)
    , m_timemore(gestureInfo.timemore.toInt())
    , m_timeless(gestureInfo.timeless.toInt())
    , m_swipemore(gestureInfo.swipemore.toInt())
    , m_swipeless(gestureInfo.swipeless.toInt())
    , m_isNeedX(isNeedConnectX11())
//    m_display(nullptr)
{

    //    if(m_isNeedX){
    //        m_display = XOpenDisplay(nullptr);
    //        if (!m_display) {
    //            qDebug() << "\tOpening X11 display failed!";
    //        }
    //    }
    if (!s_interface) {
        s_interface = KylinStatusInterface::getInstance();
        if (s_interface) {
            //            qDebug() << "dbus connect success!";
            s_isTabletMode = s_interface->isTabletMode();
        }
    }
}

GestureAndActionBase::~GestureAndActionBase()
{
    //    if (m_display != nullptr) {
    //        XCloseDisplay(m_display);
    //    }
}

bool GestureAndActionBase::operator==(GestureAndActionBase &value)
{
    if (this->m_gestureAction != value.m_gestureAction) {
        return false;
    }
    if (this->m_fingers != value.m_fingers) {
        return false;
    }
    if (this->m_gestureDirection != value.m_gestureDirection) {
        return false;
    }
    if (this->m_actionType != value.m_actionType) {
        return false;
    }
    if (this->m_gestureAction != value.m_gestureAction) {
        return false;
    }
    if (this->m_touchDevice != value.m_touchDevice) {
        return false;
    }
    return true;
}

bool GestureAndActionBase::isNeedEmulateKeyOrButton()
{
    switch (getGestureActionType()) {
    case ActionType::SEND_KEYS:
    case ActionType::SEND_BUTTON:
        return true;
    default:
        break;
    }
    return false;
}

bool GestureAndActionBase::isNeedConnectX11()
{
    if (!KWin::waylandServer() && isNeedEmulateKeyOrButton())
        return true;
    else
        return false;
}

void GestureAndActionBase::setTabletMode(bool isTablet)
{
    s_isTabletMode = isTablet;
    //    qDebug() << "Current Mode is" << (s_isTabletMode ? "Tablet" : "PC");
}

void GestureAndActionBase::sendKeyEvent(const QStringList &keycodes, bool is_press) const
{
    //    qDebug()<<"keycodes"<<keycodes;
    for (const QString &keycode : keycodes) {

        if (m_isNeedX) {
            Display *m_display = XOpenDisplay(nullptr);
            KeySym sym = XStringToKeysym(keycode.toStdString().c_str());
            KeyCode code = XKeysymToKeycode(m_display, sym);

            XTestFakeKeyEvent(m_display, code, is_press ? true : false, 0);
            XFlush(m_display);

            XCloseDisplay(m_display);
        } else {
            if (keyCode.count(keycode) != 1) {
                qCritical() << "\t Error: Not find the key's map ! key =" << keycode;
                return;
            }
            if (is_press)
                input()->processKeyboardKey(keyCode[keycode], InputRedirection::KeyboardKeyPressed, 0);
            else
                input()->processKeyboardKey(keyCode[keycode], InputRedirection::KeyboardKeyReleased, 0);
            input()->simulateUserActivity();
        }
    }
    return;
}
void GestureAndActionBase::sendButtonEvent(quint32 button, bool is_press) const
{

    if (m_isNeedX) {
        Display *m_display = XOpenDisplay(nullptr);
        XTestFakeButtonEvent(m_display, button, is_press, 0);
        XFlush(m_display);
        XCloseDisplay(m_display);
    } else {
        if (btnCode.count(button) != 1) {
            //            qCritical() << "\t Error: Not find the btn's map ! btn =" << button;
            return;
        }
        input()->processPointerMotion(GestureManager::self()->currentTouchPoint(), 0);
        if (is_press)
            input()->processPointerButton(btnCode[button], InputRedirection::PointerButtonPressed, 0);
        else
            input()->processPointerButton(btnCode[button], InputRedirection::PointerButtonReleased, 0);
        input()->simulateUserActivity();
    }
    return;
}

void GestureAndActionBase::getSignal(bool mode)
{
    //    qDebug() << endl << "\t Get signal from Dbus ! ! !";
    if (mode != s_isTabletMode)
        s_isTabletMode = mode;
    return;
}

quint32 GestureAndActionBase::getTimeOut()
{
    if (m_gestureType == GestureType::LONGPRESS && m_gestureAction.count("timeout") == 1)
        return m_gestureAction["timeout"].toInt();
    else
        return 0;
}

quint32 GestureAndActionBase::getMinSwipeDistance()
{
    if (m_gestureAction.count("distance") == 1)
        return m_gestureAction["distance"].toInt();
    else
        return 50; //! 默认滑动距离为50，当滑动距离为0时，刚开始按下时，便有四个方向的started信号
}

bool GestureAndActionBase::getEventStage(QString str)
{
    bool flag = true;
    if (m_gestureAction.count("eventstage") == 1) {
        if (m_gestureAction["eventstage"] == str)
            flag = true;
        else
            flag = false;
    } else {
        flag = false;
    }
    return flag;
}

bool GestureAndActionBase::isRepeatable()
{
    if (m_gestureAction.count("repeat") == 1)
        return m_gestureAction["repeat"] == "true";
    else
        return false;
}

bool GestureAndActionBase::isOnlyTablet()
{
    if (m_gestureAction.count("onlytablet") == 1)
        return m_gestureAction["onlytablet"] == "true";
    else
        return false;
}

void GestureAndActionBase::onAllDirectionKey(GlobalGesture::GestureDirection direction)
{
    QStringList keys;
    switch (direction) {
    case GlobalGesture::GestureDirection::Down:
        // printf("SwipeSequence: Down\n");
        keys << "Down";
        break;
    case GlobalGesture::GestureDirection::Up:
        // printf("SwipeSequence: Up\n");
        keys << "Up";
        break;
    case GlobalGesture::GestureDirection::Left:
        // printf("SwipeSequence: Left\n");
        keys << "Left";
        break;
    case GlobalGesture::GestureDirection::Right: {
        // printf("SwipeSequence: Right\n");
        keys << "Right";
        break;
    }
    default:
        keys << "";
        break;
    }
    sendKeyEvent(keys, true);
    sendKeyEvent(keys, false);
}

void GestureAndActionBase::burriedTouchpadData(GestureType gestureType, quint32 finger, const QString &triggerTime) const
{
    QList<QPair<QString, QString>> properties = {{"gestureType", gestureTypeToStr(gestureType)},
                                                 {"fingerCount", QString::number(finger)},
                                                 {"triggerTime", triggerTime}};

    burriedData(properties);
}

void GestureAndActionBase::burriedData(const QList<QPair<QString, QString>> &properties) const
{
    QVector<KCustomProperty> property(properties.size());
    for (int i = 0; i < properties.size(); ++i) {

        QByteArray dataKey = properties.at(i).first.toLatin1();
        property[i].key = strdup(dataKey.data());

        QByteArray dataValue = properties.at(i).second.toLatin1();
        property[i].value = strdup(dataValue.data());
    }

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property.data(), properties.size());
    kdk_dia_upload_default(node, (char *)"touchpad_gesture_info", (char *)"global_gesture");
    kdk_dia_data_free(node);
}

void GestureAndActionBase::onGestureActionStart()
{
    // printf("start is ok\n");
    //    qDebug() << endl << "\tGestureAndActionBase::onGestureActionStart()"
    //             << gestureTypeToStr(m_gestureType) << s_isTabletMode << isOnlyTablet();
    if (!s_isTabletMode && isOnlyTablet())
        return;
    if (!getEventStage("start"))
        return;

    if (m_touchDevice == TouchDevice::TouchPad) {
        burriedTouchpadData(m_gestureType, m_fingers, "start");
    }

    switch (m_actionType) {
    case ActionType::SEND_KEYS: {
        QStringList modifiers;
        QStringList keys;
        if (m_gestureAction.count("modifiers") == 1) {
            modifiers = m_gestureAction["modifiers"].split('+');
        }
        if (m_gestureAction.count("keys") == 1) {
            keys = m_gestureAction["keys"].split('+');
        }
        sendKeyEvent(modifiers, true); // 按键被按下 只有在最后的时候才会释放
        sendKeyEvent(keys, true);
        sendKeyEvent(keys, false);
        // printf("send keys is begin\n");
        break;
    }
    case ActionType::RUN_COMMAND: {
        runCommand(m_gestureAction.value("command"));
        break;
    }
    case ActionType::SEND_BUTTON: {
        QString btn;
        if (m_gestureAction.count("button") == 1) {
            btn = m_gestureAction["button"];
        }

        sendButtonEvent(btn.toUInt(), true);
        sendButtonEvent(btn.toUInt(), false);
        break;
    }
    case ActionType::DBUS_ACTION: {
        // printf("dbus\n");
        //  <dbus>session,org.ukui.ScreenSaver,/,org.ukui.ScreenSaver,ShowScreensaver</dbus>
        QStringList dbus;
        if (m_gestureAction.count("dbus") == 1) {
            dbus = m_gestureAction["dbus"].split(',');
        }
        /*
         * TODO:
         * dbus调用对多参数的处理
         */
        //        QStringList args;
        //        if (actionSettings.count("arg") == 1) {
        //            args = actionSettings["arg"].split(',');
        //        }
        //        qDebug() << "\t" << dbus;
        QString &bustype = dbus[0];
        QString &service = dbus[1];
        QString &path = dbus[2];
        QString &interface = dbus[3];
        QString &method = dbus[4];
        QDBusMessage message = QDBusMessage::createMethodCall(service, path, interface, method);
        //        qDebug() << message.arguments() << message.interface() << message.member();
        /*
         * TODO:
         * dbus调用对多参数的处理
         */
        //        QList<QVariant> arguments;
        //        if(args.size() > 0){
        //            foreach (QString& arg, args) {
        //                arguments << QVariant(arg);
        //                message.setArguments(arguments);
        //            }
        //        }
        if (bustype == "session")
            QDBusConnection::sessionBus().call(message);
        else if (bustype == "system")
            QDBusConnection::systemBus().call(message);
        break;
    }
    case ActionType::NOT_SUPPORTED: {
        //        qDebug() << endl
        //                 << "\t" << gestureTypeToStr(getGestureType()) << "\tEmit ActionType::NOT_SUPPORTED";
        break;
    }
    default:
        break;
    }
}

void GestureAndActionBase::onGestureActionReach()
{
    if (!s_isTabletMode && isOnlyTablet())
        return;
    if (!isRepeatable())
        return;
    switch (m_actionType) {
    case ActionType::SEND_KEYS: {
        QStringList keys;
        if (m_gestureAction.count("keys") == 1) {
            keys = m_gestureAction["keys"].split('+');
            //            qDebug()<<"---------";
            //            qDebug()<<"KEYS-"<<keys<<"---";
            //            qDebug()<<"---------";
            //            qDebug()<<"m_gestureAction[]-"<<m_gestureAction["keys"]<<"---";
        }
        sendKeyEvent(keys, true);
        sendKeyEvent(keys, false);
        break;
    }
    case ActionType::RUN_COMMAND: {
        runCommand(m_gestureAction.value("command"));
        break;
    }
    case ActionType::DBUS_ACTION: {
        // <dbus>session,org.ukui.ScreenSaver,/,org.ukui.ScreenSaver,ShowScreensaver</dbus>
        QStringList dbus;
        if (m_gestureAction.count("dbus") == 1) {
            dbus = m_gestureAction["dbus"].split(',');
        }
        /*
         * TODO:
         * dbus调用对多参数的处理
         */
        //        QStringList args;
        //        if (actionSettings.count("arg") == 1) {
        //            args = actionSettings["arg"].split(',');
        //        }
        QString &bustype = dbus[0];
        QString &service = dbus[1];
        QString &path = dbus[2];
        QString &interface = dbus[3];
        QString &method = dbus[4];
        QDBusMessage message = QDBusMessage::createMethodCall(service, path, interface, method);
        //        qDebug() << message.arguments() << message.interface() << message.member();
        /*
         * TODO:
         * dbus调用对多参数的处理
         */
        //        QList<QVariant> arguments;
        //        if(args.size() > 0){
        //            foreach (QString& arg, args) {
        //                arguments << QVariant(arg);
        //                message.setArguments(arguments);
        //            }
        //        }
        if (bustype == "session")
            QDBusConnection::sessionBus().call(message);
        else if (bustype == "system")
            QDBusConnection::systemBus().call(message);
        break;
    }
    case ActionType::NOT_SUPPORTED: {
        //        qDebug() << endl
        //                 << "\t" << gestureTypeToStr(m_gestureType) << "\tEmit ActionType::NOT_SUPPORTED";
        break;
    }
    default:
        break;
    }
}

void GestureAndActionBase::onGestureActionTriggered()
{
    if (!s_isTabletMode && isOnlyTablet()) {
        qDebug() << "It's not in tablet mode now, so  this gesture don't work";
        return;
    }
    if (m_actionType != ActionType::SEND_KEYS) {
        if (!getEventStage("trigger"))
            return;
    }

    if (m_touchDevice == TouchDevice::TouchPad) {
        burriedTouchpadData(m_gestureType, m_fingers, "triggered");
    }

    switch (m_actionType) {
    case ActionType::SEND_KEYS: {
        QStringList modifiers;
        QStringList keys;
        if (m_gestureAction.count("modifiers") == 1) {
            modifiers = m_gestureAction["modifiers"].split('+');
        }
        if (m_gestureAction.count("keys") == 1) {
            keys = m_gestureAction["keys"].split('+');
        }
        if (getEventStage("start")) {
            qDebug() << "Gesture SEND_KEYS"
                     << "modifiers:" << modifiers;
            sendKeyEvent(modifiers, false); // start阶段的时候按键在triggered阶段释放
            break;
        } else {
            qDebug() << "Gesture SEND_KEYS"
                     << "Modifiers:"
                     << modifiers << "Keys:" << keys;
            sendKeyEvent(modifiers, true);
            sendKeyEvent(keys, true);
            sendKeyEvent(keys, false);
            sendKeyEvent(modifiers, false);
            // printf("send keys is ok\n");
        }

        break;
    }
    case ActionType::RUN_COMMAND: {
        runCommand(m_gestureAction.value("command"));
        break;
    }
    case ActionType::SEND_BUTTON: {
        QString btn;
        if (m_gestureAction.count("button") == 1) {
            btn = m_gestureAction["button"];
        }
        qDebug() << "Gesture SEND_BUTTON"
                 << "Button:" << btn;
        sendButtonEvent(btn.toInt(), true);
        sendButtonEvent(btn.toInt(), false);
        break;
    }
    case ActionType::DBUS_ACTION: {
        QtConcurrent::run([this]() {
            QStringList dbus;
            if (m_gestureAction.count("dbus") == 1) {
                dbus = m_gestureAction["dbus"].split(',');
            }

            qDebug() << "Gesture DBUS_ACTION"
                     << "DBus:" << dbus;
            QString &bustype = dbus[0];
            QString &service = dbus[1];
            QString &path = dbus[2];
            QString &interface = dbus[3];
            QString &method = dbus[4];
            QDBusMessage message = QDBusMessage::createMethodCall(service, path, interface, method);

            if (bustype == "session") {
                QDBusConnection::sessionBus().call(message);
            } else if (bustype == "system") {
                QDBusConnection::systemBus().call(message);
            }
        });
        break;
    }
    case ActionType::NOT_SUPPORTED: {
        qDebug() << "Gesture NOT_SUPPORTED";
        break;
    }
    default:
        break;
    }
}

void GestureAndActionBase::runCommand(QString cmdline)
{
    QStringList command = cmdline.split(" ");
    QStringList args;
    QProcess *myProcess = new QProcess(this);

    if (command.count() > 1) {
        for (auto begin = command.begin() + 1; begin != command.end(); ++begin) {
            args << *begin;
        }
    }

    qDebug() << "Gesture run program:" << command[0] << "args:" << args;
    myProcess->startDetached(command[0], args);
}

}
