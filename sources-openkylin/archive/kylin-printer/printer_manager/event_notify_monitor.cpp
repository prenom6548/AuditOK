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
#include <QDBusConnection>
#include "event_notify_monitor.h"

static const char * const events[] = {
//    "job-created",
    "job-completed",
    "job-state-changed",
//    "job-state",
    "printer-added",
    "printer-deleted",
    "printer-state-changed"
};

static const int events_num = sizeof(events) / sizeof(char *);

const int SUBSCRIP_DURATION = 24*60*60;

EventNotifyMonitor::EventNotifyMonitor()
    :m_subSequenceNum(1)
{
    m_cupsConnection = new CupsConnection4PPDs();
    m_requestHttp = m_cupsConnection->getCupsConnection();
    m_intervalDuration = new QTimer(this);
    m_intervalDuration->setTimerType(Qt::PreciseTimer);
    m_intervalDuration->setInterval((SUBSCRIP_DURATION - 100) * 1000);
    connect(m_intervalDuration,&QTimer::timeout, this, &EventNotifyMonitor::renewSubscription, Qt::UniqueConnection);
    m_subId = this->creatSubScription();
    m_intervalDuration->start();
}
EventNotifyMonitor::~EventNotifyMonitor()
{
    if (m_cupsConnection)
    {
        delete m_cupsConnection;
        m_cupsConnection = nullptr;
    }
    if (m_interval5)
    {
        m_interval5->stop();
    }
    if (m_intervalDuration)
    {
        m_intervalDuration->stop();
    }
    cancelSubscription();
}
EventNotifyMonitor *EventNotifyMonitor::getInstance()
{
    static EventNotifyMonitor *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new EventNotifyMonitor();
        thread = new QThread();
        instance->moveToThread(thread);
        connect(instance, &EventNotifyMonitor::start,
                instance, &EventNotifyMonitor::detectStart);

        connect(instance, &EventNotifyMonitor::rhPrinterAdded, instance, &EventNotifyMonitor::insertUpdatePrinterName);
        connect(instance, &EventNotifyMonitor::rhPrinterRemoved, instance, &EventNotifyMonitor::insertUpdatePrinterName);
        connect(instance, &EventNotifyMonitor::rhQueueChanged, instance, &EventNotifyMonitor::insertUpdatePrinterName);

        thread->start();
    }
    return instance;
}

void EventNotifyMonitor::detectStart()
{
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "PrinterAdded",    this, SIGNAL(rhPrinterAdded(QString)));
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "QueueChanged",    this, SIGNAL(rhQueueChanged(QString)));
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "PrinterRemoved",  this, SIGNAL(rhPrinterRemoved(QString)));
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "JobQueuedLocal",  this, SIGNAL(rhJobQueuedLocal(QString,uint,QString)));
    QDBusConnection::systemBus().connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "JobStartedLocal", this, SIGNAL(rhJobStartedLocal(QString,uint,QString)));

    m_interval5 = new QTimer(this);
    m_interval5->setInterval(1*1000);
    connect(m_interval5, &QTimer::timeout, this, &EventNotifyMonitor::getAndSendNotify);
    m_interval5->start();
}
void EventNotifyMonitor::getAndSendNotify()
{
    auto res = this->getNotifications();

    int notifySize = res.size();
    for (int i = 0 ; i < notifySize; ++i)
    {
        if (m_subSequenceNum <= res[i].notifySequenceNum)
        {
            // qDebug() << "获取到通知！";
            auto notify = res[i];
            m_subSequenceNum = notify.notifySequenceNum + 1;
            // qDebug() << "当前顺序号为：" << m_subSequenceNum;

            QStringList notifyInfo;
            bool needNotifyFlag = notifyStructAnalysis(notify, notifyInfo);

            if(needNotifyFlag)
            {
                emit getNotify(notifyInfo);
            }

        }
        else
        {
            continue;
        }
    }
    return;
}
int EventNotifyMonitor::creatSubScription()
{
    qDebug() << "create subscription!";
    ipp_t *request = nullptr;
    ipp_t *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    request = ippNewRequest (IPP_CREATE_PRINTER_SUBSCRIPTION);
    /*
     * The "printer-uri" attribute which defines the target for this operation
     */
    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, "/");
    /*
     * The credibility of authentication depends on the mechanism that
     * the Printer uses to obtain the user's name.
     */
    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());
    /*
     * Subscribed Event - an Event that the Subscribing Client expresses
     * interest in by making it a value of the "notify-events" attribute on
     *  a Subscription Object.
     */
    ippAddStrings (request, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, "notify-events", events_num, NULL, events);
    /*
     * This attribute's value is a type2 keyword indicating which
     * Pull Delivery Method is to be used.
     * "notify-recipient-uri" is for Push Delivery Methods only.
     * "notify-pull-method" is for Pull Delivery Methods only.
     */
    ippAddString (request, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, "notify-pull-method", NULL, "ippget");
    /*
     * The requested lease time in seconds for the Subscription Object
     */
    ippAddInteger (request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER, "notify-lease-duration", SUBSCRIP_DURATION);
    
    ippAddString (request, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI, "notify-recipient-uri", NULL, "dbus://");

    answer = cupsDoRequest (m_requestHttp, request, "/");

    if (answer != NULL && ippGetStatusCode (answer) <= IPP_OK_CONFLICT) {
        if ((attr = ippFindAttribute (answer, "notify-subscription-id", IPP_TAG_INTEGER)) == NULL)
        {
            qDebug() << "No notify-subscription-id in response!\n";//这里应该错误退出
            exit(0);
        }
        else
        {
            qDebug() << "create subscription successed!";
            this->m_subId = ippGetInteger (attr, 0);//这个subID是订阅关联值，关键。
            qDebug() << "当前订阅ID为：" << this->m_subId;
        }
    }

    if (answer)
    {
        ippDelete (answer);
    }

    return m_subId;
}

int EventNotifyMonitor::renewSubscription()
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    request = ippNewRequest (IPP_RENEW_SUBSCRIPTION);
    qDebug() << "renew subscription!";

    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, "/");

    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());

    ippAddInteger (request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "notify-subscription-id", m_subId);

    ippAddInteger (request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER, "notify-lease-duration", SUBSCRIP_DURATION);

    answer = cupsDoRequest (m_requestHttp, request, "/");

    if (answer != NULL && ippGetStatusCode (answer) == IPP_OK)
    {
        qDebug() << "renew subscription successed!";
        this->m_subId = ippGetInteger (attr, 0);//这个subID是订阅关联值，关键。
    }
    else
    {
        qDebug() << "renew subscription failed!";
        cancelSubscription();
        this->m_subId = creatSubScription();
        this->m_subSequenceNum = 1;
    }

    if (answer)
    {
        ippDelete (answer);
    }
    return m_subId;
}

void EventNotifyMonitor::cancelSubscription()
{
    ipp_t *request = nullptr, *answer = nullptr;

    request = ippNewRequest (IPP_RENEW_SUBSCRIPTION);
    qDebug() << "cancel subscription!";

    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, "/");

    ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());

    ippAddInteger (request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "notify-subscription-id", m_subId);

    answer = cupsDoRequest (m_requestHttp, request, "/");

    if (answer)
    {
        ippDelete (answer);
    }
    return ;
}

notifyVector EventNotifyMonitor::getNotifications()
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    request = ippNewRequest(IPP_GET_NOTIFICATIONS);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr, "/");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", nullptr, cupsUser());
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "notify-subscription-ids", m_subId);
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "notify-sequence-numbers", m_subSequenceNum);
    ippAddBoolean (request, IPP_TAG_EVENT_NOTIFICATION, "notify-wait", false);
    //Event Notification Attributes Group
    answer = cupsDoRequest(this->m_requestHttp, request, "/");

    if(!answer)
    {
        qDebug() << ("cupsDoRequest请求失败!\n");
    }
    /* the Printer MUST reject the operation and return:
     * 'client-error-forbidden'
     * 'client-error-not-authenticated'
     * 'client-error-not-authorized'
     *  status code as appropriate.
     */
    if(ippFindAttribute(answer,"client-error-forbidden",IPP_TAG_INTEGER) != NULL)
    {
        qDebug() << ("CLIENT被拒绝!\n");
    }
    else if(ippFindAttribute(answer,"client-error-not-authenticated",IPP_TAG_INTEGER) != NULL)
    {
        qDebug() << ("CLIENT被拒绝!\n");
    }
    else if(ippFindAttribute(answer,"client-error-not-authorized",IPP_TAG_INTEGER) != NULL)
    {
        qDebug() << ("CLIENT被拒绝!\n");
    }
    else if(ippFindAttribute(answer,"client-error-not-found",IPP_TAG_INTEGER) != NULL)
    {
        qDebug() << ("CLIENT被拒绝!\n");
    }
    else if(ippFindAttribute(answer,"server-error-busy",IPP_TAG_INTEGER) != NULL)
    {
        qDebug() << ("CLIENT被拒绝!\n");
    }
    notifyVector eventVector = {};
    NotifyStruct event = {};

    bool getNotificationsFlag = false;
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer))
    {
        if (ippGetGroupTag(attr) == IPP_TAG_EVENT_NOTIFICATION)
        {
            getNotificationsFlag = true;
            break;
        }
    }
    if (getNotificationsFlag)
    {
        for (attr = ippFirstAttribute(answer); attr != NULL; attr = ippNextAttribute (answer))
        {
            if (ippGetGroupTag(attr) == IPP_TAG_ZERO)
            {
                //End of Event;
                if (event.isVaild())
                {
                    // qDebug() << ("获取到event_notification\n");
                    // qDebug() << "subsequenceID is ：" << event.subsequenceId;
                    eventVector.push_back(event);

                    event.clear();
                    break;
                }
                else
                {
                    continue;
                }

            }
            if (ippGetGroupTag(attr) == IPP_TAG_EVENT_NOTIFICATION)
            {
                auto AttrName = ippGetName(attr);
                // qDebug() << "AttrName is : " << AttrName ;
                if (!QString::compare(AttrName, ""))
                {
                    // qDebug() << "AttrName is : " << AttrName ;
                    // qDebug() << "subscribedEvent is : " << event.subscribedEvent;
                }
                else if (!QString::compare(AttrName, "notify-charset"))
                {
                    event.charset = ippGetString (attr, 0, NULL);
                    // qDebug() << "event.charset is " << event.charset ;
                }
                else if (!QString::compare(AttrName, "notify-natural-language"))
                {
                    event.lan = ippGetString (attr, 0, NULL);
                    // qDebug() << "event.lan is " << event.lan ;
                }
                else if (!QString::compare(AttrName, "notify-subscription-id"))
                {
                    event.subsequenceId = ippGetInteger(attr, 0);
                    // qDebug() << "event.subsequenceId is " << event.subsequenceId ;
                }
                else if (!QString::compare(AttrName, "notify-sequence-number"))
                {
                    event.notifySequenceNum = ippGetInteger(attr, 0);
                    // qDebug() << "event.notifySequenceNum is " << event.notifySequenceNum ;
                }
                else if (!QString::compare(AttrName, "job-state"))
                {
                    event.jobState = ippGetInteger(attr, 0);
                    // qDebug() << "event.jobState is " << event.jobState ;
                }
                else if (!QString::compare(AttrName, "job-name"))
                {
                    event.jobName = ippGetString (attr, 0, NULL);
                    // qDebug() << "event.jobName is " << event.jobName ;
                }
                else if (!QString::compare(AttrName, "notify-subscribed-event"))
                {
                    event.subscribedEvent = ippGetString (attr, 0, NULL);
                    // qDebug() << "event.subscribedEvent is " << event.subscribedEvent ;
                }
                else if (!QString::compare(AttrName, "printer-up-time"))
                {
                    // qDebug() << "printer-up-time" << ippGetInteger(attr, 0);
                    continue;
                }
                else if (!QString::compare(AttrName, "notify-text"))
                {
                    event.notifyText = ippGetString (attr, 0, NULL);
                    // qDebug() << "event.notifyText is " << event.notifyText ;
                }
                else if (!QString::compare(AttrName, "notify-printer-uri"))
                {
                    // qDebug() << "notify-printer-uri : " << ippGetString (attr, 0, NULL);
                    continue;
                }
                else if (!QString::compare(AttrName, "printer-name"))
                {
                    event.printerName = ippGetString (attr, 0, NULL);
                    // qDebug() << "event.printerName is " << event.printerName ;
                }
                else if (!QString::compare(AttrName, "printer-state"))
                {
                    event.printerState = ippGetInteger(attr, 0);
                    // qDebug() << "event.printerState is " << event.printerState ;
                }
                else if (!QString::compare(AttrName, "printer-state-reasons"))
                {
                    event.printerStateChangedReason = ippGetString (attr, 0, NULL);
                    // qDebug() << "printer-state-reasons : " << ippGetString (attr, 0, NULL);
                }
                else if (!QString::compare(AttrName, "printer-is-accepting-jobs"))
                {
                    // qDebug() << "printer-is-accepting-jobs" << ippGetBoolean(attr, 0);
                    event.isAcceptingJobs = ippGetBoolean(attr, 0);
                    continue;
                }
                else if (!QString::compare(AttrName, "printer-is-accepting-jobs"))
                {
                    // qDebug() << "printer-is-accepting-jobs" << ippGetInteger(attr, 0);
                    continue;
                }
                else
                {
                    // qDebug() << "AttrName is :" << AttrName << "And continue!" ;
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
    }
    if (event.isVaild())
    {
        // qDebug() << ("获取到event_notification\n");
        // qDebug() << "subsequenceID is ：" << event.subsequenceId;
        eventVector.push_back(event);

        event.clear();
    }
    ippDelete(answer);
    return eventVector;
}

bool EventNotifyMonitor::notifyStructAnalysis(const NotifyStruct& curNotify, QStringList &outputInfo)
{
    QString title;
    QString content;
    bool needNotifyFlag = false;
    auto curSubscribedEvent = curNotify.subscribedEvent;
    auto curNotifyText = curNotify.notifyText;
    auto curPrinterName = curNotify.printerName;
    auto curJobName = curNotify.jobName;
    auto curReason = curNotify.printerStateChangedReason;

    title = tr("Printer ") + curPrinterName;

    if (!QString::compare(curSubscribedEvent, "job-state-changed"))
    {
        content = curJobName;
        content += " " + curNotifyText;
        if (!QString::compare((curNotifyText), "Now accepting jobs."))
        {
            needNotifyFlag = false;
        }
        else if (!QString::compare((curNotifyText), "Retrying job on same printer."))
        {
            needNotifyFlag = false;
        }
        else
        {
            needNotifyFlag = true;
        }

    }
    else if (!QString::compare(curSubscribedEvent, "job-created"))
    {
        if ((curNotifyText).contains("Job created."))
        {
            content = tr("Job:");
            content += curJobName;
            content += tr("created！");
        }
        else
        {
            content = curJobName;
            content += " " + curNotifyText;
        }
        needNotifyFlag = true;
    }
    else if (!QString::compare(curSubscribedEvent, "job-completed"))
    {
        if ((curNotifyText).contains("Job completed."))
        {
            content = tr("Job:");
            content += curJobName;
            content += tr("completed！");
        }
        else
        {
            content = curJobName;
            content += " " + curNotifyText;
        }
        needNotifyFlag = true;
    }
    else if (!QString::compare(curSubscribedEvent, "printer-state-changed"))
    {
        title = tr("Printer ") + curPrinterName;
        if (!QString::compare((curNotifyText), "Now accepting jobs."))
        {
            needNotifyFlag = false;
        }
        else if ((curNotifyText).contains("state changed to idle"))
        {
            needNotifyFlag = false;
        }
        else if ((curNotifyText).contains("state changed to processing"))
        {
            needNotifyFlag = false;
        }
        else
        {
            if ((curReason).contains("state changed to idle"))
            {
                needNotifyFlag = false;
            }
            else if ((curReason).contains("toner-low-report"))
            {
                needNotifyFlag = false;
            }
            else if ((curReason).contains("toner-empty-error"))
            {
                needNotifyFlag = false;
            }
            else if ((curReason).contains("toner-empty-warning"))
            {
                needNotifyFlag = false;
            }
            else if ((curReason).contains("Retrying job on same printer"))
            {
                needNotifyFlag = false;
            }
            else if(!QString::compare((curReason), "none"))
            {
                needNotifyFlag = false;
            }
            else
            {
                content = curJobName;
                content += " " + curNotifyText;
                needNotifyFlag = true;
            }
        }
    }
    else if (!QString::compare(curSubscribedEvent, "printer-stopped"))
    {
        needNotifyFlag = false;
    }
    else if (!QString::compare(curSubscribedEvent, "printer-deleted"))
    {
        needNotifyFlag = false;
    }
    else if (!QString::compare(curSubscribedEvent, "printer-added"))
    {
        needNotifyFlag = false;
    }

    outputInfo.push_back(title);
    outputInfo.push_back(content);
    return needNotifyFlag;
}


void  EventNotifyMonitor::insertUpdatePrinterName(const QString &printerName)
{
    // qDebug() << printerName;
}
