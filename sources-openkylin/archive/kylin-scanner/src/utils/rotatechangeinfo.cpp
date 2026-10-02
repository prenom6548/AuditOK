/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "rotatechangeinfo.h"

RotateChangeInfo::RotateChangeInfo(QObject *parent) : QObject(parent)
{
    qDebug() << "rotate object has created!";
    connect(QApplication::primaryScreen(),&QScreen::primaryOrientationChanged,
            this,&RotateChangeInfo::rotateChanged);
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(modeChanged(bool)));
    getCurrentScale();
    getCurrentMode();
}
void RotateChangeInfo::rotateChanged(){
    QPair<int,int> scale = getCurrentScale();
    qDebug() << "horizontal & vertical mode changed sig has received! current scale: " << scale.first << "*" ;
    emit rotationChanged(m_isPCMode,scale.first,scale.second);
}
void RotateChangeInfo::modeChanged(bool isPadMode){
    if(isPadMode){
        m_isPCMode = false;
    }else{
        m_isPCMode = true;
    }
    qDebug() << "pc & pad mode changed sig has received! current mode: " << m_isPCMode;
    emit rotationChanged(m_isPCMode,m_screenWidth,m_screenHeight);
}
QPair<int, int> RotateChangeInfo::getCurrentScale(){
    QScreen *screen = qApp->primaryScreen();
    m_screenWidth = screen->size().width();
    m_screenHeight = screen->size().height();
    QPair<int,int> scale;
    scale.first = m_screenWidth;
    scale.second = m_screenHeight;
    qDebug() << "screen size: " << m_screenWidth << "*" << m_screenHeight;
    return scale;
}
bool RotateChangeInfo::getCurrentMode(){
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                          KYLIN_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (reply_pcORpad.value()) {
        m_isPCMode = false;
    }else{
        m_isPCMode = true;
    }
    return m_isPCMode;
}
