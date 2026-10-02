/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "config.h"
#include <QWidget>
#include <QWindow>
#include <KWindowEffects>

#if defined(__has_include) && __has_include("ukuiwindowhelper.h")
    #include <ukuiwindowhelper.h>
#define UKUIWINDOWHELPER
#else
    class UkuiWindowHelper;
#endif


static QGSettings *static_panelGSettings = nullptr;

int envPC = 0;
bool global_rightToleft = false;

//全局变量，判定机型架构
Config::Config(QObject *parent, QString name)
    :QObject(parent)
    ,setting_name(name)
{
    if (QGSettings::isSchemaInstalled(setting_name.toUtf8())) {
        gsetting = new QGSettings(setting_name.toUtf8());
    }
}

Config::~Config()
{
    delete gsetting;
}

void Config::SendNotifyMessage(QString title, QString message, QString soundeffects)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             QDBusConnection::sessionBus());
    QList<QVariant> args;
    QMap<QString, QVariant> vmap;
    vmap.insert("sound-name", QVariant(soundeffects));
    args<<tr("Bluetooth")
        <<((unsigned int) 0)
        <<"bluetooth"
        <<title //显示的是什么类型的信息
        <<message //显示的具体信息
        <<QStringList()
        <<vmap
        <<(int)-1;
    QDBusMessage msg = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    KyDebug() << msg.errorMessage();
}

const QPixmap Config::loadSvg(const QPixmap &source, const PixmapColor &cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                switch (cgColor) {
                case PixmapColor::WHITE:
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLACK:
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::GRAY:
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLUE:
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                    break;
                default:
                    return source;
                    break;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void Config::setKdkGeometry(QWindow *windowHandle, int width, int height, bool center) {
    if (nullptr == windowHandle) {
        KyWarning() << "null windowHandle";
        return;
    }

    int panelPosition = 0;
    int panelSize = 46;
    if ( nullptr == static_panelGSettings) {
        const QByteArray id(PANEL_SETTINGS);
        if (QGSettings::isSchemaInstalled(id)) {
            if (nullptr == static_panelGSettings) {
                static_panelGSettings = new QGSettings(id);
            }
        }
    }
    if (static_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
        panelPosition = static_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        KyDebug() << "panelPosition: " << panelPosition; 
    }

    if (static_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
        panelSize = static_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        KyDebug() << "panelSize: " << panelSize;
    }

    setKdkGeometry(windowHandle, width, height, panelPosition, panelSize, center);
}

void Config::setKdkGeometry(QWindow *windowHandle, int width, int height, int panelPosition, int panelSize, bool center)
{
    if (nullptr == windowHandle) {
        KyWarning() << "null windowHandle";
        return;
    }

    QScreen * qscreen = QGuiApplication::screenAt(QCursor::pos());
    if (nullptr == qscreen) {
        KyWarning() << "null qscreen";
        return;
    }

    QRect availableGeo = qscreen->geometry();
    int x, y;
    int margin = 8;

    if (center) {
        x = (availableGeo.width() - width) / 2;
        y = (availableGeo.height() - height) / 2;

        if (windowHandle) {
            windowHandle->setGeometry(QRect(x, y, width, height));
        }

        return;
    }

    switch (panelPosition)
    {
    case PanelPosition::Top:
        x = availableGeo.x() + availableGeo.width() - width - margin;
        y = availableGeo.y() + panelSize + margin;
        if (global_rightToleft)
            x = availableGeo.x() + availableGeo.width() - x - width;
        break;
    case PanelPosition::Bottom:
        x = availableGeo.x() + availableGeo.width() - width - margin;
        y = availableGeo.y() + availableGeo.height() - panelSize - height - margin;
        if (global_rightToleft)
            x = availableGeo.x() + availableGeo.width() - x - width;
        break;
    case PanelPosition::Left:
        x = availableGeo.x() + panelSize + margin;
        y = availableGeo.y() + availableGeo.height() - height - margin;
        break;
    case PanelPosition::Right:
        x = availableGeo.x() + availableGeo.width() - panelSize - width - margin;
        y = availableGeo.y() + availableGeo.height() - height - margin;
        break;
    }

    KyDebug() <<  "availableGeo.x: " << availableGeo.x() << ", availableGeo.width: " << availableGeo.width() 
             <<  ", availableGeo.y:" << availableGeo.y() << ", availableGeo.height: " << availableGeo.height() 
             << ", width: "<< width << ", height: " << height << ", x: "<< x << ", y: " << y << ", panelSize: " << panelSize
             << ", panelPosition: " << panelPosition;

    if (windowHandle) {
        windowHandle->setGeometry(QRect(x, y, width, height));
    }
}

struct_pos Config::getKdkGeometry(int width, int height, bool center)
{
    struct_pos pos;
    int panelPosition = 0;
    int panelSize = 46;
    if ( nullptr == static_panelGSettings) {
        const QByteArray id(PANEL_SETTINGS);
        if (QGSettings::isSchemaInstalled(id)) {
            if (nullptr == static_panelGSettings) {
                static_panelGSettings = new QGSettings(id);
            }
        }
    }
    if (static_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
        panelPosition = static_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        KyDebug() << "panelPosition: " << panelPosition;
    }

    if (static_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
        panelSize = static_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        KyDebug() << "panelSize: " << panelSize;
    }

    QScreen * qscreen = QGuiApplication::screenAt(QCursor::pos());
    if (nullptr == qscreen) {
        KyWarning() << "null qscreen";
        return pos;
    }

    QRect availableGeo = qscreen->geometry();
    int x, y;
    int margin = 8;

    if (center) {
        x = (availableGeo.width() - width) / 2;
        y = (availableGeo.height() - height) / 2;
        pos.x = x;
        pos.y = y;
        return pos;
    }

    switch (panelPosition)
    {
    case PanelPosition::Top:
        x = availableGeo.x() + availableGeo.width() - width - margin;
        y = availableGeo.y() + panelSize + margin;
        if (global_rightToleft)
            x = availableGeo.x() + availableGeo.width() - x - width;
        break;
    case PanelPosition::Bottom:
        x = availableGeo.x() + availableGeo.width() - width - margin;
        y = availableGeo.y() + availableGeo.height() - panelSize - height - margin;
        if (global_rightToleft)
            x = availableGeo.x() + availableGeo.width() - x - width;
        break;
    case PanelPosition::Left:
        x = availableGeo.x() + panelSize + margin;
        y = availableGeo.y() + availableGeo.height() - height - margin;
        break;
    case PanelPosition::Right:
        x = availableGeo.x() + availableGeo.width() - panelSize - width - margin;
        y = availableGeo.y() + availableGeo.height() - height - margin;
        break;
    }

    pos.x = x;
    pos.y = y;

    KyDebug() <<  "availableGeo.x: " << availableGeo.x() << ", availableGeo.width: " << availableGeo.width()
             <<  ", availableGeo.y:" << availableGeo.y() << ", availableGeo.height: " << availableGeo.height()
             << ", width: "<< width << ", height: " << height << ", x: "<< x << ", y: " << y << ", panelSize: " << panelSize
             << ", panelPosition: " << panelPosition;


    return pos;
}

struct_pos Config::getWidgetPos(int ax, int ay, int aw, int ah, int width)
{
    struct_pos t(ax, ay, aw, ah);
    //从右到左布局，需要重新定位x坐标
    if (global_rightToleft) {
        t.x = width - ax - aw;
    }
    return t;
}

int Config::setSlideWindow(QWidget *window)
{
    int panelPosition = 0;
    if ( nullptr == static_panelGSettings) {
        const QByteArray id(PANEL_SETTINGS);
        if (QGSettings::isSchemaInstalled(id)) {
            if (nullptr == static_panelGSettings) {
                static_panelGSettings = new QGSettings(id);
            }
        }
    }
    if (static_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
        panelPosition = static_panelGSettings->get(PANEL_POSITION_KEY).toInt();
    }

    if(0 == panelPosition)
    {
        KWindowEffects::slideWindow(window, KWindowEffects::BottomEdge);
    }
    else if (1 == panelPosition)
    {
        KWindowEffects::slideWindow(window, KWindowEffects::TopEdge);
    }
    else if(2 == panelPosition)
    {
        KWindowEffects::slideWindow(window, KWindowEffects::LeftEdge);
    }
    else
    {
        KWindowEffects::slideWindow(window, KWindowEffects::RightEdge);
    }

    return 0;
}

static UkuiWindowHelper* getUkuiWindowHelper(QWidget * window)
{
#ifdef UKUIWINDOWHELPER
    UkuiWindowHelper*  windowHelper = nullptr;

    if(nullptr != window) {
        windowHelper = window->findChild<UkuiWindowHelper*>("WindowHelper");
        if (nullptr == windowHelper) {
            KyInfo() << "not find WindowHelper child, create UkuiWindowHelper";
            windowHelper = new UkuiWindowHelper(window);
            windowHelper->setObjectName("WindowHelper");
            windowHelper->setParent(window);
        } else {
            KyInfo() << "find WindowHelper child";
        }
    }
    return windowHelper;
#else
    return NULL;
#endif
}

void Config::setSkipTaskBar(QWidget * window)
{
#ifdef UKUIWINDOWHELPER
    if(nullptr != window) {
        UkuiWindowHelper * windowHelper = getUkuiWindowHelper(window);
        if(windowHelper) {
            windowHelper->setSkipTaskBar(true);
        }
    }
    else {
        KyWarning() << "null window";
    }
#endif
}

void Config::setWindowState(QWidget *window)
{
#ifdef UKUIWINDOWHELPER
    if(nullptr != window) {
        UkuiWindowHelper * windowHelper = getUkuiWindowHelper(window);
        if(windowHelper) {
             UkuiWindowHelper::States defaultstate = UkuiWindowHelper::States(0xff);
             defaultstate = defaultstate & ~UkuiWindowHelper::State::Minimizable;
             defaultstate = defaultstate & ~UkuiWindowHelper::State::Maximizable;
             defaultstate = defaultstate & ~UkuiWindowHelper::State::Fullscreenable;
             defaultstate = defaultstate & ~UkuiWindowHelper::State::Movable;
             defaultstate = defaultstate & ~UkuiWindowHelper::State::Resizable;
             windowHelper->setWindowState(defaultstate);
        }
    }
    else {
        KyWarning() << "null window";
    }
#endif
}

void Config::AppManagerToOpenBluetoothSettings()
{
    KyInfo ();

    QDBusMessage m = QDBusMessage::createMethodCall("com.kylin.AppManager",
                                                    "/com/kylin/AppManager",
                                                    "com.kylin.AppManager",
                                                    "LaunchAppWithArguments");
    m<< QString("ukui-control-center.desktop") << QStringList{"-m","Bluetooth"};
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);

}

void Config::CommandToOpenBluetoothSettings()
{
    KyInfo ();

    static QProcess *process = nullptr;
    if(process) {
        process->close();
        process->deleteLater();
    }
    process = new QProcess();
    QString cmd = "ukui-control-center";
    QStringList arg;
    arg.clear();
    arg << "-m";
    arg << "Bluetooth";
    KyDebug() << arg;
    process->startDetached(cmd,arg);
}

void Config::OpenBluetoothSettings()
{
    if (Environment::MAVIS == envPC){

        QDBusInterface *m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                                                 "/",
                                                                 "com.kylin.statusmanager.interface",
                                                                 QDBusConnection::sessionBus());

        if (m_statusSessionDbus->isValid())
        {
            qInfo() << Q_FUNC_INFO << "is_tabletmode" << __LINE__;

            QDBusReply<bool> is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
            if (is_tabletmode)
                AppManagerToOpenBluetoothSettings();
            else
                CommandToOpenBluetoothSettings();
        }
        else
            CommandToOpenBluetoothSettings();
    }
    else
        CommandToOpenBluetoothSettings();
}

int Config::devConnect(QString address) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("devConnect");
    dbusMsg << address;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Connect Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        case -3:
            DEBUG_PRINT("Adapter doesn't Exist");
            break;
        case -4:
            DEBUG_PRINT("Pair Failed");
            break;
        case -5:
            DEBUG_PRINT("Paring");
            break;
        }
        return res;
    }  else {
        DEBUG_PRINT("Method Call Error");
        return -6;
    }
}

int Config::devDisconnect(QString address) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("devDisconnect");
    dbusMsg << address;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Disconnect Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        }
        return res;
    } else {
        DEBUG_PRINT("Method Call Error");
        return -3;
    }
}

int Config::devRemove(QString address, QString adapter) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("devRemove");
    dbusMsg << address << adapter;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Remove Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        }
        return res;
    } else {
        DEBUG_PRINT("Method Call Error");
        return -3;
    }
}

int Config::activeConnectionReply(QString address, bool accept) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("activeConnectionReply");
    dbusMsg << address << accept;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Remove Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        }
        return res;
    } else {
        DEBUG_PRINT("Method Call Error");
        return -3;
    }
}

int Config::pairFuncReply(QString address, bool accept) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("pairFuncReply");
    dbusMsg << address << accept;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Remove Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        }
        return res;
    } else {
        DEBUG_PRINT("Method Call Error");
        return -3;
    }
}

int Config::replyFileReceiving(QString address, bool accept, QString savePathdir, QString user) {
    QMap<QString, QVariant> sendMsg;
    sendMsg.insert("dev", QVariant(address));
    sendMsg.insert("receive", QVariant(accept));
    sendMsg.insert("savePathdir", QVariant(savePathdir));
    sendMsg.insert("user", user);
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("replyFileReceiving");
    dbusMsg << sendMsg;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Remove Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        }
        return res;
    } else {
        DEBUG_PRINT("Method Call Error");
        return -3;
    }
}

bool Config::setDefaultAdapterAttr(QMap<QString, QVariant> adpAttr) {
    KyDebug() << adpAttr;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("setDefaultAdapterAttr");
    dbusMsg << adpAttr;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toBool();
    } else
        return false;
}

bool Config::getLeaveLockPower()
{
    KyDebug();
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("getLeaveLockPower");
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage) {
        return response.arguments().takeFirst().toBool();
    }

    return false;
}

void Config::setLeaveLockPower(bool v)
{
    KyDebug() << v;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("setLeaveLockPower");
    dbusMsg << v;
    QDBusConnection::systemBus().asyncCall(dbusMsg);
}

void Config::soundComplete() {
    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.sound.theme.player",
                                                    "/org/ukui/sound/theme/player",
                                                    "org.ukui.sound.theme.player",
                                                    "playAlertSound");
    m<< QString("complete");
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void Config::soundWarning() {
    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.sound.theme.player",
                                                    "/org/ukui/sound/theme/player",
                                                    "org.ukui.sound.theme.player",
                                                    "playAlertSound");
    m<< QString("dialog-error");
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}
