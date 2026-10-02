/*
 * Copyright (C) 2023 Tianjin KYLIN Information Technology Co., Ltd.
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

#ifndef LIBINPUTSWITCHEVENT_H
#define LIBINPUTSWITCHEVENT_H

#ifdef SUPPORT_TABLET_MODE
#include <QObject>
#include <QDebug>
#include <QThread>
#include <ukui/event.h>
#include <ukui/ukuiinputgatherclient.h>
#include "machinemodel.h"
//头文件以及顺序不可改，不可删

class GatherClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit GatherClientWorker(QObject* parent = nullptr);

    /**
     * @brief start 启动工作
     */
    void start();
    /**
     * @brief stop 通知工作完成
     */
    void stop();

private:
    void dealEvent(Event *e);

Q_SIGNALS:
    /**
     * @brief tabletModeStatusChanged 平板模式改变信号
     * @param tabletmode 平板模式
     */
    void tabletModeStatusChanged(int tabletmode);
    /**
     * @brief finished 通知线程退出事件循环
     */
    void finished();

private:
    UKUIInputGatherClient *m_inputGatherClient = nullptr;
};

class GatherClientThread : public QThread
{
    Q_OBJECT
public:
    explicit GatherClientThread(QObject *parent = nullptr);
    /**
     * @brief setGatherClientWorker 设置工作者
     * @param worker 工作者实例
     */
    void setGatherClientWorker(GatherClientWorker *worker);

    /**
     * @brief stop 通知工作者停止工作
     */
    void stop();

protected:
    /**
     * @brief run 线程处理过程
     */
    void run() override;

private:
    GatherClientWorker *m_gatherClientWorker = nullptr;
};

class LibinputSwitchEvent : public QObject
{
    Q_OBJECT
public:
    explicit LibinputSwitchEvent(QObject *parent = nullptr);

    ~LibinputSwitchEvent();
    bool geInitDevicesStatus();
signals:
    void tabletModeStatusChanged(int tabletmode);

private:
    QString m_machineType;
    std::shared_ptr<MachineModel> m_machineModel = nullptr;
    GatherClientThread m_gatherClientThread;
    GatherClientWorker m_gatherClientWorker;
};
#endif // SUPPORT_TABLET_MODE

#endif // LIBINPUTSWITCHEVENT_H
