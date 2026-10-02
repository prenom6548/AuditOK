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

#include "signals.h"

//GlobalUserSignal
GlobalUserSignal* GlobalUserSignal::instance = new GlobalUserSignal;

GlobalUserSignal *GlobalUserSignal::getInstance()
{
    return instance;

}

GlobalUserSignal::GlobalUserSignal(QObject *parent) : QObject(parent)
{
    mMap.clear();
}

bool GlobalUserSignal::getDeviceBusyStatus(QString dev)
{
    if (mMap.contains(dev))
        return mMap.value(dev);
    else {
        return false;
    }
}

void GlobalUserSignal::setDeviceBusyStatus(QString dev,bool result)
{
        mMap.insert(dev,result);
}

void GlobalUserSignal::removeDevice(QString dev,bool result)
{
    if (mMap.contains(dev)) { //判断设备是否在mMap中
        mMap.remove(dev);
    }
}

GlobalUserSignal::~GlobalUserSignal()
{

}

//GlobalCoreSignal
GlobalCoreSignal* GlobalCoreSignal::instance = new GlobalCoreSignal;

GlobalCoreSignal *GlobalCoreSignal::getInstance()
{
    return instance;
}


GlobalCoreSignal::GlobalCoreSignal(QObject *parent) : QObject(parent)
{

}

GlobalCoreSignal::~GlobalCoreSignal()
{

}

//GlobalConfigSignal
GlobalConfigSignal* GlobalConfigSignal::instance = new GlobalConfigSignal;
GlobalConfigSignal *GlobalConfigSignal::getInstance()
{
    return instance;
}

GlobalConfigSignal::GlobalConfigSignal(QObject *parent)
{

}

GlobalConfigSignal::~GlobalConfigSignal()
{

}
