/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: Xin Li <lixin@kylinos.cn>
 */

#include "horizontalorverticalmode.h"

static const QString KYLIN_ROTATION_PATH = "/";

static const QString KYLIN_ROTATION_SERVICE = "com.kylin.statusmanager.interface";

static const QString KYLIN_ROTATION_INTERFACE = "com.kylin.statusmanager.interface";

HorizontalOrVerticalMode::HorizontalOrVerticalMode()
{
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                          KYLIN_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (reply_pcORpad.isValid()) {
        m_isPadMode = true;
    }else{
        m_isPadMode = false;
    }
    if(m_isPadMode){
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                              KYLIN_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            m_statePre = PADHorizontalMode;   // 横屏
        } else {
            m_statePre = PADVerticalMode;     //竖屏
        }
    }else{
        m_statePre = PCMode;
    }
    connect(&m_timer,&QTimer::timeout,this,&HorizontalOrVerticalMode::timeOutSlot);

    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("rotations_change_signal"), this, SLOT(rotationChanged(QString)));
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(modeChanged(bool)));
}
deviceMode HorizontalOrVerticalMode::defaultModeCapture()
{ // method
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                          KYLIN_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (!reply_pcORpad.isValid()) {
        return PCMode; //pc模式
    }
    if (reply_pcORpad.value()) {//平板模式
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                              KYLIN_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (!reply.isValid()) {
            return PADHorizontalMode;   //横屏
        }
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            return PADHorizontalMode;   //横屏
        } else {
            return PADVerticalMode;     //竖屏
        }

    } else {
        return PCMode;                  //pc模式
    }
}
void HorizontalOrVerticalMode::rotationChanged(QString res)
{ // signal
    if(m_isPadMode){
        if(res == "normal" || res == "upside-down"){
            m_stateAfter = PADHorizontalMode;
        }else{
            m_stateAfter = PADVerticalMode;
        }
        antiShake();
    }
}
void HorizontalOrVerticalMode::modeChanged(bool res){
    if(res){
        m_isPadMode = true;   //平板模式
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                              KYLIN_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            m_stateAfter = PADHorizontalMode;   // 横屏
        } else {
            m_stateAfter = PADVerticalMode;     //竖屏
        }
    }else{
        m_isPadMode = false;  //pc模式
        m_stateAfter = PCMode;
    }
    antiShake();
}
void HorizontalOrVerticalMode::antiShake(){
    if(m_stateAfter != m_statePre){
        m_statePre = m_stateAfter;
        if(m_timer.isActive()){
            m_timer.stop();
        }
        m_timer.start(200);
    }else{
        m_timer.stop();
        emit RotationSig(m_statePre);
        qDebug()<<"rotation changed sig has emited!";
    }
}
void HorizontalOrVerticalMode::timeOutSlot(){
    m_stateAfter = defaultModeCapture();
    antiShake();
}
