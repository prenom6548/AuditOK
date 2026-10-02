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
 */
#include "rotatechangeinfo.h"

RotateChangeInfo::RotateChangeInfo(QObject *parent) : QObject(parent)
{
    qDebug() << "rotate object has created!";
    connect(QApplication::primaryScreen(), &QScreen::primaryOrientationChanged, this, &RotateChangeInfo::slotRotateChanged);
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(slotModeChanged(bool)));
    getCurrentScale();
    getCurrentMode();
}

void RotateChangeInfo::slotRotateChanged(){
    QPair<int,int> scale = getCurrentScale();
    qDebug() << "horizontal & vertical mode changed sig has received! current scale: " << scale.first << "*" ;
    emit sigRotationChanged(m_isPCMode,scale.first,scale.second);
}

void RotateChangeInfo::slotModeChanged(bool isPadMode){
    if(isPadMode){
        m_isPCMode = false;
    }else{
        m_isPCMode = true;
    }
    qDebug() << "pc & pad mode changed sig has received! current mode: " << m_isPCMode;
    emit sigRotationChanged(m_isPCMode,m_screenWidth,m_screenHeight);
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
    qDebug() << " ===== INFO ===== " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__ << " getCurrentMode  >> value : " << reply_pcORpad.value()  << " >> isValid :  " << reply_pcORpad.isValid();
    if (!reply_pcORpad.isValid()) {
        m_existDbus = false;
    }
    if (reply_pcORpad.value()) {
        m_isPCMode = false;
    }else{
        m_isPCMode = true;
    }
    return m_isPCMode;
}
