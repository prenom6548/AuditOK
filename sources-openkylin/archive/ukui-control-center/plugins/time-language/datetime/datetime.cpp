/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "datetime.h"
#include <sys/timex.h>
#include <QDBusPendingCall>

#define MAX_TIMEZONES 5

DateTime::DateTime()
    : mFirstLoad(true)
{
    pluginName = tr("Date");
    pluginType = DATETIME;
}

DateTime::~DateTime()
{

}

QString DateTime::plugini18nName()
{
    return pluginName;
}

int DateTime::pluginTypes()
{
    return pluginType;
}

QWidget *DateTime::pluginUi()
{
    if (mFirstLoad) {  
        datetimeUi = new DatetimeUi;
        mFirstLoad = false;
        this->installEventFilter(this);
        datetimeInterface = new QDBusInterface("org.ukui.ukcc.session",
                                               "/Datetime",
                                               "org.ukui.ukcc.session.Datetime",
                                               QDBusConnection::sessionBus(), this);
        m_pUkccSystemDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                               "/",
                                               "com.control.center.interface",
                                               QDBusConnection::systemBus(), this);
        if (datetimeInterface->isValid()) { // 判断服务是否存在
            QDBusMessage message = datetimeInterface->call("ping"); // 判断dbus路径是否存在
            if (message.type() == QDBusMessage::ErrorMessage && message.errorMessage().contains("No such object path", Qt::CaseInsensitive)) {
                qWarning()<<datetimeInterface<<":"<<message.errorMessage();
            } else {
                QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                                      "/Datetime",
                                                      "org.ukui.ukcc.session.Datetime",
                                                      "changed",
                                                      this,
                                                      SLOT(dataChanged(QString)));
                initContent();
            }

        } else {
            qCritical() << "org.ukui.ukcc.session.Datetime DBus error:" << datetimeInterface->lastError();
        }
    } else {
#ifdef Nile
        datetimeUi->resetDateFormat();
        datetimeUi->updateDate();
#endif
    }
    if (datetimeUi) {
        datetimeUi->update();
        datetimeUi->resetDateFormat();
        datetimeUi->updateDate();
        qDebug() << Q_FUNC_INFO << "datetimeUi update";
    }
    return datetimeUi;
}

const QString DateTime::name() const
{

    return QStringLiteral("Date");
}

bool DateTime::isShowOnHomePage() const
{
    return true;
}

QIcon DateTime::icon() const
{
    if (QIcon::hasThemeIcon("ukui-datetime-symbolic")) {
        return QIcon::fromTheme("ukui-datetime-symbolic");
    }
    return QIcon::fromTheme("ukui-datetine-symbolic");  //主题命名错误，兼容
}

bool DateTime::isEnable() const
{
    return true;
}

void DateTime::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&DateTime::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

void DateTime::keyChangedSlot(const QString &key)
{
    if (key == "datetime") {
        initTimer();
        initDate();
        initTimezone();
        initTimeModeSet();
        initNtpServers();
        initNtpServer();
        initOtherTimezone();
    }
}

void DateTime::dataChanged(QString key)
{
    if (toChangeKey != key) {
        if(key == "timezone") {
            initDate();
        } else if (key == "timeMode") {
            initTimeModeSet();
        } else if (key == "ntpServer") {
            initNtpServer();
        } else if (key == "otherTimezones") {
            initOtherTimezone();
        }
    }
    toChangeKey = "";
}

void DateTime::initContent()
{
    initTimer();
    initDate();
    initTimezone();
    initTimeModeSet();
    initNtpServers();
    initNtpServer();
    initOtherTimezone();
    connectUiSignals();
}

void DateTime::initTimer()
{
    if (!timer) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=](){
            struct timex txc = {};
            if (m_uiMode == "manual") {
                timerTimes = 0;
                timer->stop();
                datetimeUi->setSyncResultLabel(2);   //同步时间成功
            } else if (adjtimex(&txc) < 0 || txc.maxerror >= 16000000) {  //未能同步时间
                timerTimes++;
                if (timerTimes >= 120) {
                    datetimeUi->setSyncResultLabel(1);
                } else {
                    datetimeUi->setSyncResultLabel(0);
                }
            } else {
                timerTimes = 0;
                timer->stop();
                datetimeUi->setSyncResultLabel(2);   //同步时间成功
            }
        });
    }
}

void DateTime::initDate()
{
    QStringList list = datetimeInterface->property("timezone").toStringList();
    QTimeZone localTimezone;
    if (list.isEmpty() || list.at(0).isEmpty()) {
        localTimezone = QTimeZone::systemTimeZone();
        qDebug() << Q_FUNC_INFO << "timezone is empty" << localTimezone;
    } else {
        QByteArray timezoneData = list.at(0).toLatin1();
        localTimezone = QTimeZone(timezoneData.constData());
        qDebug() << Q_FUNC_INFO << "timezone is not empty" << list;
    }
    int utcOff = (localTimezone.offsetFromUtc(QDateTime::currentDateTime()))/3600;
    QString gmData;
    if (utcOff >= 0) {
       gmData = QString("(GMT+%1:%2)").arg(utcOff, 2, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    } else {
       gmData = QString("(GMT%1:%2)").arg(utcOff, 3, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    }
    if (list.size() >= 2) {
        datetimeUi->setTimezoneStr(gmData + " " + list.at(1));
    } else {
        qDebug() << Q_FUNC_INFO << "property timezone size error" << list;
    }
}

void DateTime::timezoneChooserConfirmSlot(QString timezone, QString title) {
    qDebug() << Q_FUNC_INFO << "change timezone" << timezone << title;

    if (tr("Change Timezone") == title){
        qDebug() << Q_FUNC_INFO << "change timezone";
        UkccCommon::buriedSettings(name(), "Change Timezone", QString("settings"), timezone);
        if (!m_pUkccSystemDbus || !m_pUkccSystemDbus->isValid()) {
            qWarning() << Q_FUNC_INFO << "system dbus interface is invalid";
            return;
        }

        // Call the privileged system service from the control-center process so
        // polkit keeps authorization only for this control-center instance.
        QDBusReply<bool> reply = m_pUkccSystemDbus->call("setShowTimezone", timezone);
        if (!reply.isValid() || !reply.value()) {
            initDate();
        }
    } else {
        qDebug() << Q_FUNC_INFO << "add timezone";
        UkccCommon::buriedSettings(name(), "Add Timezone", QString("settings"), timezone);
        if (!otherTimezoneList.contains(timezone)) {
            otherTimezoneList.append(timezone);
            toChangeKey = "otherTimezones";
            QDBusReply<bool> reply = datetimeInterface->call("setOtherTimezones", otherTimezoneList);
            if (reply.isValid() && reply.value()) {
                if (otherTimezoneList.size() >= MAX_TIMEZONES) {
                    datetimeUi->setAddTimezoneBtnEnabled(false);
                } else {
                    datetimeUi->setAddTimezoneBtnEnabled(true);
                }
                QDBusReply<QStringList> l = datetimeInterface->call("timezoneName", timezone);
                if (l.isValid() && l.value().size() > 1) {
                    datetimeUi->addOtherTimezone(timezone, l.value().at(1));
                }
            } else {
                otherTimezoneList.removeOne(timezone);
            }
        }
    }
}

void DateTime::initTimezone()
{
    if (m_timeZoneChooserDialog) {
        delete m_timeZoneChooserDialog;
        m_timeZoneChooserDialog = nullptr;
    }
    m_timeZoneChooserDialog = new TimeZoneChooserDialog(datetimeUi);
    connect(m_timeZoneChooserDialog, &TimeZoneChooserDialog::confirmed, this, [=](QString timezone, QString title){
        timezoneChooserConfirmSlot(timezone, title);
    });
}

void DateTime::initTimeModeSet()
{
    QString timeMode = datetimeInterface->property("timeMode").toString();
    if (timeMode == "manual") {
        datetimeUi->setTimeMode("manual");
    } else {
        datetimeUi->setTimeMode("automatic");
    }
}

void DateTime::initNtpServers()
{
    QStringList list = datetimeInterface->property("ntpServers").toStringList();
    datetimeUi->setNtpServers(list);
}

void DateTime::initNtpServer()
{
    datetimeUi->setNtpServer(datetimeInterface->property("ntpServer").toString());
}

void DateTime::initOtherTimezone()
{
    QStringList list = datetimeInterface->property("otherTimezones").toStringList();
    for (QString s1 : list) {
        bool add = true;
        for (QString s2 : otherTimezoneList) {
            if (s2 == s1) {
                add = false;
                break;
            }
        }
        if (add) {
            otherTimezoneList.append(s1);
            if (otherTimezoneList.size() >= MAX_TIMEZONES) {
                datetimeUi->setAddTimezoneBtnEnabled(false);
            } else {
                datetimeUi->setAddTimezoneBtnEnabled(true);
            }
            QDBusReply<QStringList> l = datetimeInterface->call("timezoneName", s1);
            if (l.isValid() && l.value().size() > 1) {
                datetimeUi->addOtherTimezone(s1, l.value().at(1));
            }
        }
    }
}

void DateTime::dataModeChangeSlot(QDBusPendingCallWatcher *call, int toggledBtn, QString mode) {
    if (call == nullptr) {
        return;
    }
    QDBusPendingReply<bool> reply = *call;
    const bool success = reply.isValid() && reply.value();
    if (success) {
        m_uiMode = mode;
        datetimeUi->setTimeMode(mode);
        if (mode == "manual") {
            timerTimes = 0;
            timer->stop();
        } else {
            timer->start(80);
        }
        return;
    }

    const QString previousMode = toggledBtn == 0 ? "manual" : "automatic";
    m_uiMode = previousMode;
    datetimeUi->setTimeMode(previousMode);
    if (previousMode == "manual") {
        timer->stop();
    }
}

void DateTime::connectUiSignals()
{
    connect(datetimeUi, &DatetimeUi::timezoneButtonClicked, this, [=](){
        showTimezoneMap(0);
    });
    connect(datetimeUi, &DatetimeUi::dateTimeChanged, this, [=](QDate d, QTime t){
        QDateTime setdt(d, t);
        const QDBusReply<bool> reply = datetimeInterface->call("setTime", setdt.toSecsSinceEpoch() * G_TIME_SPAN_SECOND);
        if (!reply.isValid() || !reply.value()) {
            datetimeUi->resetDateTime();
        }
    });
    connect(datetimeUi, &DatetimeUi::timeModeChanged, this, [=](int toggledBtn, QString mode){
       UkccCommon::buriedSettings(name(), "Set Time", QString("settings"), mode);
       initNtpServer();
       if (mode == "manual") {
           datetimeUi->setSyncResultLabel(2);
       }
       toChangeKey = "timeMode";

       QDBusPendingCall call =  datetimeInterface->asyncCall("setTimeMode", mode);
       QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, datetimeUi);
       connect(watcher, &QDBusPendingCallWatcher::finished, datetimeUi, [=, toggledBtn, mode](QDBusPendingCallWatcher *call) {
           dataModeChangeSlot(call, toggledBtn, mode);
       });
    });
    connect(datetimeUi, &DatetimeUi::ntpServerChanged, this, [=](QString server ){
        ntpServerChangedSlot(server);
    });
    connect(datetimeUi, &DatetimeUi::addTimezoneButtonClicked, this, [=](){
        showTimezoneMap(1);
    });
    connect(datetimeUi, &DatetimeUi::timezoneRemoved, this, [=](QString t){
        otherTimezoneList.removeOne(t);
        toChangeKey = "otherTimezones";
        QDBusReply<bool> reply = datetimeInterface->call("setOtherTimezones", otherTimezoneList);
        if (reply.isValid() && reply.value()) {
            if (otherTimezoneList.size() >= MAX_TIMEZONES) {
                datetimeUi->setAddTimezoneBtnEnabled(false);
            } else {
                datetimeUi->setAddTimezoneBtnEnabled(true);
            }
        }
    });
    QDBusConnection::systemBus().connect("com.control.center.qt.systemdbus",
                                         "/",
                                         "com.control.center.interface",
                                         "ntpServerChangedDefault",
                                         this,
                                         SLOT(ntpServerChangedDefaultSlot()));
    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void DateTime::ntpServerChangedDefaultSlot() {
    datetimeUi->getComboxWidget()->blockSignals(true);
    datetimeUi->getComboxWidget()->setCurrentIndex(0);
    qDebug() << Q_FUNC_INFO << "change ntpServer to default";
    datetimeUi->getComboxWidget()->blockSignals(false);
}

void DateTime::setEnableBySecurity() {
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("dateEnable").toString();
    QStringList setItems = moduleSettings.split(",");
    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        bool enable = (item.at(1) == "true");
        qDebug() << "set item Name: " << item.at(0) << enable;
        if (item.at(0) == "chgzonebtn") {
            datetimeUi->getChangeTZbutton()->setEnabled(enable);
        }
        if (item.at(0) == "manualTimeButton") {
            datetimeUi->setTimeFrameEnable(enable);
            datetimeUi->getManualRadioButton()->setEnabled(enable);
        }
        if (item.at(0) == "ntpSerWidget") {
            datetimeUi->getComboxWidget()->setEnabled(enable);
        }
        if (item.at(0) == "syncTimeBtn") {
            datetimeUi->getSyncRadioButton()->setEnabled(enable);
        }
    }
}

void DateTime::setNtpSerAddressSlot(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<bool> reply = *watcher;
    watcher->deleteLater();

    bool retReply = reply.value();
    qDebug() << Q_FUNC_INFO << "setNtpSerAddress reply" << retReply;
    if (retReply) {
        qDebug() << Q_FUNC_INFO << "setNtpSerAddress suc";
        timerTimes = 0;
        timer->stop();
        timer->start(80);
    } else {
        qDebug() << Q_FUNC_INFO << "setNtpSerAddress failed";
        initNtpServer();
    }
}

void DateTime::ntpServerChangedSlot(QString server) {
    QString curServer = datetimeInterface->property("ntpServer").toString();
    if (curServer == server || (curServer.isEmpty() && server == "default")) {
        qDebug() << Q_FUNC_INFO << "ntp server not change, skip" << server;
        return;
    }
    qDebug() << Q_FUNC_INFO << "current ntp server" << curServer << server;
    UkccCommon::buriedSettings(name(), "Sync Server", QString("select"), server);
    toChangeKey = "ntpServer";

    if (!m_pUkccSystemDbus || !m_pUkccSystemDbus->isValid()) {
        qWarning() << Q_FUNC_INFO << "m_pUkccSystemDbus invalid, skip setNtpSerAddress";
        initNtpServer();
        return;
    }

    QDBusPendingCall call = m_pUkccSystemDbus->asyncCall("setNtpSerAddress", server);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, datetimeUi);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &DateTime::setNtpSerAddressSlot);
    qDebug() << Q_FUNC_INFO << "setNtpSerAddress async" << server;
}

void DateTime::showTimezoneMap(int flag)
{
    if (!m_timeZoneChooserDialog) {
        qWarning() << Q_FUNC_INFO << "timezone chooser dialog is null";
        return;
    }
    m_timeZoneChooserDialog->setTitle(flag);
    m_timeZoneChooserDialog->show();
}

bool DateTime::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == UkccArgEvent::TYPE) {
        UkccArgEvent *str = dynamic_cast<UkccArgEvent *>(event);
        if (str != nullptr) {
            qDebug() << Q_FUNC_INFO << "event string =" << str->str();
            // ukui-control-center -m Date modifyTimezone
            if (str->str() == "modifyTimezone") {
                showTimezoneMap(0);
            }
        }
    }
    return QObject::eventFilter(watched, event);
}
