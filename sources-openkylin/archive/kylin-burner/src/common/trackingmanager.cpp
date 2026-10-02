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

#include "trackingmanager.h"
#include "src/frame/logger.h"
#include "src/frame/configuration.h"

#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>


// 静态成员初始化
TrackingManager* TrackingManager::m_instance = nullptr;
QMutex TrackingManager::m_mutex;

TrackingManager::TrackingManager(QObject *parent)
    : QObject(parent)
    , m_trackingEnabled(true)  // 默认开启埋点
    , m_workerThread(nullptr)
{
    // 从配置文件读取埋点开关状态
    m_trackingEnabled = Configuration::instance()->get("Tracking", "Enabled", true).toBool();
    
    // 创建工作线程
    m_workerThread = new QThread();
    this->moveToThread(m_workerThread);
    
    // 连接线程信号
    connect(m_workerThread, &QThread::started, this, &TrackingManager::processTrackingData);
    connect(m_workerThread, &QThread::finished, this, [this]() {
        // 线程结束时处理剩余数据
        flushTrackingData();
    });
    
    // 启动工作线程
    m_workerThread->start();
    
    // 初始化页面开始时间
    m_pageStartTimes[BurnDataPage] = 0;
    m_pageStartTimes[BurnImagePage] = 0;
    m_pageStartTimes[CopyImagePage] = 0;
    
    qDebug() << "TrackingManager initialized, tracking enabled:" << m_trackingEnabled;
}

TrackingManager::~TrackingManager()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
    }
    
    // 保存埋点开关状态到配置文件
    Configuration::instance()->set("Tracking", "Enabled", m_trackingEnabled);
    
    qDebug() << "TrackingManager destroyed";
}

TrackingManager* TrackingManager::instance()
{
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);
        if (m_instance == nullptr) {
            m_instance = new TrackingManager();
        }
    }
    return m_instance;
}

void TrackingManager::setTrackingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_trackingEnabled = enabled;
    
    // 保存到配置文件
    Configuration::instance()->set("Tracking", "Enabled", enabled);
    
    qDebug() << "Tracking enabled changed to:" << enabled;
    
    if (!enabled) {
        // 关闭埋点时清空队列
        QMutexLocker queueLocker(&m_queueMutex);
        m_eventQueue.clear();
    }
}

bool TrackingManager::isTrackingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_trackingEnabled;
}

void TrackingManager::recordPageDwellTime(PageType pageType, qint64 dwellTime)
{
    if (!m_trackingEnabled) {
        return;
    }
    
    TrackingEvent event;
    event.pageType = pageType;
    event.dwellTime = dwellTime;
    
    // 根据页面类型设置事件信息
    switch (pageType) {
    case BurnDataPage:
        event.eventName = "burn_data_entrance_dwell_time_event";
        event.eventDesc = "entryPage ";
        event.propertyKey = "burnDataEntranceDwellTime";
        break;
    case BurnImagePage:
        event.eventName = "burn_image_entrance_dwell_time_event";
        event.eventDesc = "entryPage ";
        event.propertyKey = "burnImageEntranceDwellTime";
        break;
    case CopyImagePage:
        event.eventName = "copy_image_entrance_dwell_time_event";
        event.eventDesc = "entryPage ";
        event.propertyKey = "copyImageEntranceDwellTime";
        break;
    default:
        return;
    }
    
    event.propertyValue = QString::number(dwellTime) + "ms";
    
    // 将事件添加到队列
    QMutexLocker locker(&m_queueMutex);
    m_eventQueue.enqueue(event);
    
    qDebug() << "Added tracking event to queue:" << event.eventName << "dwell time:" << dwellTime << "ms";
}

void TrackingManager::flushTrackingData()
{
    if (!m_trackingEnabled) {
        return;
    }
    
    QMutexLocker locker(&m_queueMutex);
    if (m_eventQueue.isEmpty()) {
        return;
    }
    
    // 批量处理队列中的事件
    while (!m_eventQueue.isEmpty()) {
        TrackingEvent event = m_eventQueue.dequeue();
        uploadTrackingEvent(event);
    }

     kdk_dia_upload_program_exit();
}

void TrackingManager::processTrackingData()
{
    // 这个方法在工作线程中运行
    qDebug() << "Tracking worker thread started";

    //上传启动数据
    if(m_trackingEnabled){
        kdk_dia_upload_program_startup();
    }
    
    // 线程主循环
    while (m_workerThread->isRunning()) {
        // 检查队列中是否有数据需要处理
        {
            QMutexLocker locker(&m_queueMutex);
            if (!m_eventQueue.isEmpty()) {
                TrackingEvent event = m_eventQueue.dequeue();
                uploadTrackingEvent(event);
            }
        }
        
        // 休眠100毫秒，避免过度占用CPU
        QThread::msleep(200);
    }
}

void TrackingManager::onBatchUploadTimer()
{
    // 定时器触发时，发送信号到工作线程处理数据
    QMetaObject::invokeMethod(this, "flushTrackingData", Qt::QueuedConnection);
}

void TrackingManager::uploadTrackingEvent(const TrackingEvent& event)
{
    // 创建埋点数据
    KTrackData* trackData = createTrackingData();
    if (!trackData) {
        qWarning() << "Failed to create tracking data for event:" << event.eventName;
        return;
    }

    // 设置自定义属性
    KCustomProperty property;
    property.key = strdup(event.propertyKey.toStdString().c_str());
    property.value = strdup(event.propertyValue.toStdString().c_str());

    // 添加自定义属性
    kdk_dia_append_custom_property(trackData, &property, 1);

    // 上传埋点数据
    kdk_dia_upload_default(trackData,
                           event.eventName.toUtf8().data(),
                           event.eventDesc.toUtf8().data());

    // 释放资源
    freeTrackingData(trackData);

    qDebug() << "Successfully uploaded tracking event:" << event.eventName
             << "property:" << event.propertyKey << "=" << event.propertyValue;

}

KTrackData* TrackingManager::createTrackingData()
{
    return kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
}

void TrackingManager::freeTrackingData(KTrackData* data)
{
    if (data) {
        kdk_dia_data_free(data);
    }
}
