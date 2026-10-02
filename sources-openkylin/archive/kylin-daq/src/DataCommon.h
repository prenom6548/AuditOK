/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef DATACOMMON_H
#define DATACOMMON_H

#include <QObject>
#include <QThread>
#include "CommonInfoModule.h"
#include "DBusModule.h"
#include "CacheModule.h"
#include "TransmitModule.h"
#include "BasicInfoModule.h"
#include "ConfModule.h"
#include "OnlineInfoModule.h"


class DataCommon:public QObject
{
    Q_OBJECT
public:
    DataCommon(QObject *parent=nullptr);
    ~DataCommon();
private:
    void createObject();
    void connectObject();
    void startObject();
private:
    BasicInfoModule *basicInfoModule;
    OnlineInfoModule *onlineInfoModule;
    CommonInfoModule *commonInfoModule;
    DBusModule *dataCollection;
    CacheModule *cacheModule;
    TransmitModule *transmitModule;
    ConfModule *confModule;
    QTimer *m_timer;

    QThread *basicInfoModuleThread;
    QThread *commonInfoModuleThread;
    QThread *onlineInfoModuleThread;
    QThread *dataCollectionThread;
    QThread *cacheModuleThread;
    QThread *transmitModuleThread;
    QThread *confModuleThread;
signals:
    void controlCollectInfo();
};
#endif
