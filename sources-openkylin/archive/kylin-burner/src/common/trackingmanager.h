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

#ifndef TRACKINGMANAGER_H
#define TRACKINGMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QDateTime>
#include <QMap>

// 包含埋点SDK头文件
#include <kysdk/diagnosetest/libkydatacollect.h>

class TrackingManager : public QObject
{
    Q_OBJECT

public:
    // 页面类型枚举
    enum PageType {
        BurnDataPage = 0,      // 刻录数据页面
        BurnImagePage = 1,     // 刻录镜像页面
        CopyImagePage = 2      // 拷贝镜像页面
    };

    // 埋点事件类型
    struct TrackingEvent {
        PageType pageType;
        qint64 dwellTime;      // 停留时间（毫秒）
        QString eventName;     // 事件名称
        QString eventDesc;     // 事件描述
        QString propertyKey;   // 属性键
        QString propertyValue; // 属性值
    };

    static TrackingManager* instance();
    
    // 设置全局开关
    void setTrackingEnabled(bool enabled);
    bool isTrackingEnabled() const;
    
    // 记录页面停留时间
    void recordPageDwellTime(PageType pageType, qint64 dwellTime);
    
    // 手动触发埋点上传
    void flushTrackingData();

private:
    explicit TrackingManager(QObject *parent = nullptr);
    ~TrackingManager();
    
    // 禁止拷贝和赋值
    TrackingManager(const TrackingManager&) = delete;
    TrackingManager& operator=(const TrackingManager&) = delete;

private slots:
    // 异步处理埋点数据
    void processTrackingData();
    
    // 定时器触发批量上传
    void onBatchUploadTimer();

private:
    // 执行具体的埋点上传
    void uploadTrackingEvent(const TrackingEvent& event);
    
    // 创建埋点数据
    KTrackData* createTrackingData();
    
    // 释放埋点数据
    void freeTrackingData(KTrackData* data);

private:
    static TrackingManager* m_instance;
    static QMutex m_mutex;
    
    bool m_trackingEnabled;           // 全局开关
    QThread* m_workerThread;          // 工作线程
    QQueue<TrackingEvent> m_eventQueue; // 事件队列
    QMutex m_queueMutex;              // 队列互斥锁
    
    // 页面开始时间记录
    QMap<PageType, qint64> m_pageStartTimes;
    QMutex m_timeMutex;               // 时间记录互斥锁
};

// 便捷宏定义
#define TRACKING_MANAGER TrackingManager::instance()

#endif // TRACKINGMANAGER_H
