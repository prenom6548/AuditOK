/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#include "touchscreenset.h"

#include <QApplication>
#include <QScreen>

#include <QMutex>
#include <QProcess>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/XInput2.h>



TouchScreenSet* TouchScreenSet::instance(QObject *parent)
{
    static TouchScreenSet* _instance = nullptr;
    QMutex mutex;
    mutex.lock();
    if(!_instance)
        _instance = new TouchScreenSet(parent);
    mutex.unlock();
    return _instance;
}

TouchScreenSet::TouchScreenSet(QObject *parent):
    QObject(parent),
    m_monitorWatcher(new MonitorWatcher(this))
{

}

void TouchScreenSet::init()
{
    setTouchMapScreen();
    connect(m_monitorWatcher, &MonitorWatcher::monitorCountChanged, this, &TouchScreenSet::setTouchMapScreen);
}

bool getTouchScreenID(Display*, XIDeviceInfo *dev)
{
    //QString name = QApplication::primaryScreen()->name();
    //qDebug() << "[][][][][][][][][][][]][]" << name;
    int i;
    if (dev->use != XISlavePointer)
        return false;
    if (!dev->enabled)
    {
        qDebug("info: [TouchScreenSet][getTouchScreenID]: This device is disabled.");
        return false;
    }

    for (i = 0; i < dev->num_classes; i++)
    {
        if (dev->classes[i]->type == XITouchClass)
        {
            XITouchClassInfo *t = (XITouchClassInfo*)dev->classes[i];

            if (t->mode == XIDirectTouch)
            return true;
        }
    }
    return false;
}


void TouchScreenSet::setTouchMapScreen()
{
    int  ndevices = 0;
    Display *dpy = XOpenDisplay(NULL);
    XIDeviceInfo *info = XIQueryDevice(dpy, XIAllDevices, &ndevices);
    QString devicesid="";

    for (int i = 0; i < ndevices; i++)
    {
        XIDeviceInfo* dev = &info[i];
        // 判断当前设备是不是触摸屏
        if(dev->use != XISlavePointer) continue;
        if(!dev->enabled) continue;
        for (int j = 0; j < dev->num_classes; j++)
        {
            if (dev->classes[j]->type == XITouchClass)
            {
                devicesid = tr("%1").arg(dev->deviceid);
                break;//获取第一个触屏设备id
            }
        }
    }

    XIFreeDeviceInfo(info);
    XCloseDisplay(dpy);

    //多屏时，触屏和显示接口映射，由于无法智能，选择主屏幕作为触屏映射，如果多个触屏也会像windows一样出现异常
    QString strScreenName = QApplication::primaryScreen()->name();
    QProcess enableMonitors;
    if (false == devicesid.isEmpty())//有触屏设备才做映射
    {
        //设置映射关系  示例 xinput map-to-output 7 HDMI-1
        QString xinputCmd = "xinput map-to-output " + devicesid + "  " + strScreenName;
        qDebug() << "find touchscreen " << devicesid << " " << strScreenName;
        enableMonitors.start(xinputCmd);
        enableMonitors.waitForFinished(-1);
    }
}

