/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifdef SUPPORT_TABLET_MODE
#include "libinputswitchevent.h"
#include <QEventLoop>

typedef std::function<void(Event *)> sendEvent;
LibinputSwitchEvent::LibinputSwitchEvent(QObject *parent)
    : QObject(parent), m_machineModel(MachineModel::getMachineModelInstance())
{
    m_machineType = m_machineModel->getTheMachineType();
    m_gatherClientThread.setGatherClientWorker(&m_gatherClientWorker);
    connect(
        &m_gatherClientWorker,
        &GatherClientWorker::tabletModeStatusChanged,
        this,
        &LibinputSwitchEvent::tabletModeStatusChanged);
    m_gatherClientThread.start();
}

LibinputSwitchEvent::~LibinputSwitchEvent()
{
    m_gatherClientThread.stop();
    m_gatherClientThread.quit();
    m_gatherClientThread.wait();
}

bool LibinputSwitchEvent::geInitDevicesStatus()
{
    // 其他处理
    if (m_machineType == QStringLiteral("SLATE")) {
        return true;
    } else if (m_machineType == "LAPTOP") { // 永久附加键盘笔记本
        // return true;
        return false;
    } else if (m_machineType == "ALLINONE") { // 台式
        return false;
    } else {
        // 0 非平板
        // 1 平板
        //-1 不支持
        //        int status = m_inputGatherClient->libinputTabletSwitchState();
        //        qInfo() << __FILE__ << __LINE__<< "当前设备的状态:" << status;
        //        if(status == 1)
        //            return true;
        //        else
        return false;
    }
}

GatherClientWorker::GatherClientWorker(QObject *parent) : QObject(parent) {}

void GatherClientWorker::start()
{
    sendEvent se = std::bind(&GatherClientWorker::dealEvent, this, std::placeholders::_1);
    m_inputGatherClient = new UKUIInputGatherClient();
    m_inputGatherClient->setEventCallBack(se);
    m_inputGatherClient->startToReceiveEvent();
}

void GatherClientWorker::stop()
{
    Q_EMIT finished();
}

void GatherClientWorker::dealEvent(Event *e)
{
    switch (e->type) {
        case LIBINPUT_EVENT_SWITCH_TOGGLE:
            qInfo() << "=LIBINPUT_EVENT_SWITCH_TOGGLE=";
            if (e->event.switchEventDate.switchType == LIBINPUT_SWITCH_TABLET_MODE) {
                qInfo() << "switch type" << e->event.switchEventDate.switchType;
                qInfo() << "switch status" << e->event.switchEventDate.switchState;
                if (e->event.switchEventDate.switchState == 1) {
                    Q_EMIT tabletModeStatusChanged(1);
                } else {
                    Q_EMIT tabletModeStatusChanged(0);
                }
            }
            break;
        default:
            break;
    }
}

GatherClientThread::GatherClientThread(QObject *parent) : QThread(parent) {}

/**
 * @brief run 线程处理过程
 */
void GatherClientThread::run()
{
    QEventLoop eventLoop; // 线程事件循环
    // 连接worker完成信号，退出事件循环
    if (m_gatherClientWorker) {
        connect(m_gatherClientWorker, &GatherClientWorker::finished, &eventLoop, &QEventLoop::quit);
        m_gatherClientWorker->start();
    }
    // 启动事件循环，直到收到quit
    eventLoop.exec();
}

/**
 * @brief setGatherClientWorker 设置工作者
 * @param worker 工作者实例
 */
void GatherClientThread::setGatherClientWorker(GatherClientWorker *worker)
{
    m_gatherClientWorker = worker;
    m_gatherClientWorker->moveToThread(this);
}

/**
 * @brief stop 通知工作者停止工作
 */
void GatherClientThread::stop()
{
    if (m_gatherClientWorker) {
        m_gatherClientWorker->stop();
    }
}

#endif // SUPPORT_TABLET_MODE
