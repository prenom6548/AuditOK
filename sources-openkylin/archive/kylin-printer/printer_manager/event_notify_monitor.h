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
#ifndef EVENT_NOTIFY_MONITOR_H
#define EVENT_NOTIFY_MONITOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include "cupsconnection4ppds.h"
struct NotifyStruct
{
    NotifyStruct()
    {
        clear();
    }
    bool isVaildFlag = false;
    QString charset;
    QString lan;
    QString jobName;
    int jobState;
    int notifySequenceNum;
    QString subscribedEvent;
    QString notifyText;
    int printerisAcceptingJob;
    QString printerName;
    int printerState;
    QString printerStateChangedReason;
    int subsequenceId;
    int subId;
    bool isAcceptingJobs;
    void clear()
    {
        isVaildFlag = false;
        charset = "";
        lan = "";
        jobName = "";
        jobState = INT_MIN;
        notifySequenceNum = INT_MIN;
        subscribedEvent = "";
        notifyText = "";
        printerisAcceptingJob = INT_MIN;
        printerName = "";
        printerState = INT_MIN;
        isAcceptingJobs = false;
        printerStateChangedReason = "";
    }
    bool isVaild()
    {
        if (
            notifySequenceNum == INT_MIN ||
            subscribedEvent == "" ||
            notifyText == "" ||
            printerName == "" ||
            printerState == INT_MIN)
        {
            isVaildFlag = false;
            return isVaildFlag;
        }
        else
        {
            isVaildFlag = true;
            return isVaildFlag;
        }
    }
};
using notifyVector = std::vector<NotifyStruct>;

class EventNotifyMonitor : public QThread
{
    Q_OBJECT
public:
    static EventNotifyMonitor *getInstance();
    ~EventNotifyMonitor();
    int creatSubScription();
    notifyVector getNotifications();
private:
    int m_subId;
    int m_subSequenceNum;
private:
    EventNotifyMonitor();
    bool notifyStructAnalysis(const NotifyStruct& curNotify, QStringList &outputInfo);
    void cancelSubscription();
    CupsConnection4PPDs *m_cupsConnection;
    http_t *m_requestHttp;
    QTimer *m_interval5;
    QTimer *m_intervalDuration;

signals:
    void start();
    void getNotify(QStringList);
    void rhPrinterAdded(const QString &queueName);
    void rhPrinterRemoved(const QString &queueName);
    void rhQueueChanged(const QString &queueName);
private slots:
    void detectStart();
    void getAndSendNotify();
    int renewSubscription();
    void insertUpdatePrinterName(const QString &printerName);
};

#endif // EVENT_NOTIFY_MONITOR_H
