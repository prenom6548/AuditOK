/* powerwindow.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QListWidgetItem>
#include <QListWidget>
#include <QEvent>
#include <QException>
#include <QDebug>
#include <QDBusReply>
#include <QFile>
#include <QDBusInterface>
#include "powerwindow.h"
#include "lockinfo.h"
#include "common/configuration.h"
#include "klabel.h"

#define FREEDESKTOP_LOGIN1                  "org.freedesktop.DBus.Properties"
#define LOGIN1_PATH                         "/org/freedesktop/login1"
#define LOGIN1_SERVICE                      "org.freedesktop.login1"

#define NORMAL "normal"
#define FOCUS "focus"

PowerManager::PowerManager(int num, QWidget *parent)
 : QListWidget(parent),
   m_power(new QLightDM::PowerInterface(this)),
   lasttime(QTime::currentTime()),
   m_configuration(Configuration::instance())
{
    setFlow(QListWidget::LeftToRight);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QListWidget::NoSelection);

    QFile qss(":/ukui-greeter.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(qss.readAll());

    //QObject::connect(this,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(powerClicked(QListWidgetItem*)));
    initUI();

    connect(XEventMonitor::instance(), SIGNAL(keyPress(const QString &)), this,
            SLOT(onGlobalKeyPress(const QString &)));
    connect(XEventMonitor::instance(), SIGNAL(keyRelease(const QString &)), this,
            SLOT(onGlobalkeyRelease(const QString &)));
    // 监听程序阻塞关机/睡眠
    QDBusConnection::systemBus().connect(
            LOGIN1_SERVICE, LOGIN1_PATH, FREEDESKTOP_LOGIN1, "PropertiesChanged", this, SLOT(onHibitedWatcherMessage(void)));
    loginNum = num;
}

QSize PowerManager::windowSize()
{
    //return QSize((ITEM_WIDTH+ITEM_SPACING*2)*this->count()-ITEM_SPACING*2, ITEM_HEIGHT+ITEM_SPACING*2);
    return QSize((ITEM_WIDTH)*this->count(), ITEM_HEIGHT);
}


void PowerManager::powerClicked(QListWidgetItem *item)
{
    int interval = lasttime.msecsTo(QTime::currentTime());
    if(interval < 200 && interval > -200)
        return ;
    lasttime = QTime::currentTime();
/*
    int x = row(item);

    switch (x) {
    case 0:
        switchWidgetClicked();
        break;
    case 1:
        rebootWidgetClicked();
        break;
    case 2:
        shutdownWidgetClicked();
        break;
    case 3:
        if(m_power->canSuspend())
            suspendWidgetCliced();
        else if(m_power->canHibernate())
            hibernateWidgetClicked();
        break;
    case 4:
        if(m_power->canHibernate())
            hibernateWidgetClicked();
    default:
        break;
    }
*/
    QString name = itemWidget(item)->objectName();
    if(switchWidget && name == switchWidget->objectName())
        switchWidgetClicked();
    else if(name == rebootWidget->objectName())
        rebootWidgetClicked();
    else if(name == shutdownWidget->objectName())
        shutdownWidgetClicked();
    else if(suspendWidget &&  name == suspendWidget->objectName())
        suspendWidgetClicked();
    else if(hibernateWidget && name == hibernateWidget->objectName())
        hibernateWidgetClicked();

}

bool PowerManager::eventFilter(QObject *obj, QEvent *event)
{
    QString name = obj->objectName();

    if (event->type() == QEvent::ToolTip) {
        if(name == "rebootFace")
            emit setKToolTip(tr("Close all apps, turn off your computer, and then turn your computer back on"));
        else if(name == "shutdownFace")
            emit setKToolTip(tr("Close all apps, and then shut down your computer"));
        else if(name == "suspendFace") {
            Q_EMIT setKToolTip(tr("The computer stays on, but consumes less power. The app stays open and can quickly wake up and revert to where you left off"));
        }
        else if(name == "hibernateFace")
            emit setKToolTip(tr("Turn off your computer, but the app stays open. When the computer is turned on, it can be restored to the state you left"));
    } else if (event->type() == QEvent::Leave) {
        if (name == "rebootFace" || name == "shutdownFace" || name == "suspendFace" || name == "hibernateFace") {
            emit hideKTooltip();
        }
    }

    /*使用鼠标点击事件时，会概率性出现打断睡眠的问题，改为鼠标释放事件时无此问题*/
    if(event->type() != QEvent::MouseButtonRelease){
        return QListWidget::eventFilter(obj, event);
    }

    if(name == "switchFace")
        switchWidgetClicked();
    else if(name == "rebootFace")
        rebootWidgetClicked();
    else if(name == "shutdownFace")
        shutdownWidgetClicked();
    else if(name == "suspendFace")
        suspendWidgetClicked();
    else if(name == "hibernateFace")
        hibernateWidgetClicked();

    return QListWidget::eventFilter(obj, event);
}

QStringList PowerManager::getLoginedUsers()
{
}

void PowerManager::onGlobalKeyPress(const QString &key)
{
}

void PowerManager::onGlobalkeyRelease(const QString &key)
{
//    if (key == "Escape") {
//        this->hide();
//    } else if (key == "Left" || key == "Right" || key == "Return" || key == "KP_Enter") {
//        keyBdRelease(key);
//    }
}

void PowerManager::keyBdRelease(int key)
{
    QString focus = "focus";
    QString normal = "normal";
    if(key == Qt::Key_Right){
        if(nowAt == -1){
            nowAt = 0;
            setButtonStyle(FOCUS);
        } else if(nowAt == 0){
            setButtonStyle(NORMAL);
            nowAt = 1;
            setButtonStyle(FOCUS);
        } else if(nowAt == 1){
            setButtonStyle(NORMAL);
            nowAt = 2;
            setButtonStyle(FOCUS);
        } else if(nowAt == 2){
            setButtonStyle(NORMAL);
            nowAt = 3;
            setButtonStyle(FOCUS);
        } else if(nowAt == 3){
            setButtonStyle(NORMAL);
            nowAt = 0;
            setButtonStyle(FOCUS);
        }
    } else if(key == Qt::Key_Left) {
        if(nowAt == -1){
            nowAt = 3;
            setButtonStyle(FOCUS);
        } else if(nowAt == 3){
            setButtonStyle(NORMAL);
            nowAt = 2;
            setButtonStyle(FOCUS);
        } else if(nowAt == 2){
            setButtonStyle(NORMAL);
            nowAt = 1;
            setButtonStyle(FOCUS);
        } else if(nowAt == 1){
            setButtonStyle(NORMAL);
            nowAt = 0;
            setButtonStyle(FOCUS);
        } else if(nowAt == 0){
            setButtonStyle(NORMAL);
            nowAt = 3;
            setButtonStyle(FOCUS);
        }
    } else if(key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space){
        if(nowAt == 0 && this->isVisible())
            hibernateWidgetClicked();
        else if(nowAt == 1 && this->isVisible())
            suspendWidgetClicked();
        else if(nowAt == 2 && this->isVisible())
            rebootWidgetClicked();
        else if(nowAt == 3 && this->isVisible())
            shutdownWidgetClicked();
    }
}

void PowerManager::clearStatus()
{
    for(nowAt = 0; nowAt < listLabel.count(); nowAt++) {
        setButtonStyle(NORMAL);
    }
    nowAt = -1;
}

void::PowerManager::setButtonStyle(QString Style)
{
    if(Style == "normal")
        listLabel.at(nowAt)->setStyleSheet("background-color: rgba(255, 255, 255, 15%);\
                                            QPushButton:hover{background-color: rgba(255, 255, 255, 40%)}");
    else if(Style == "focus")
        listLabel.at(nowAt)->setStyleSheet("background-color: rgba(255, 255, 255, 20%);border: 1px solid #296CD9; border-radius: 64px;");
}

void PowerManager::doEvent(int type)
{
    switch (type) {
    case REBOOT:
        try{
            m_power->restart();
            close();
        }catch(QException &e) {
            qWarning() << e.what();
        }
    case SHOTDOWN:
        try{
            qDebug() << "shutdown";
            m_power->shutdown();
            close();
        }catch(QException &e) {
            qWarning() << e.what();
        }
    default:
        break;
    }
}

void PowerManager::cancelClicked()
{
    blockInhibited = NOTHING;
}

void PowerManager::switchWidgetClicked()
{
    emit switchToUser();
}

void PowerManager::onHibitedWatcherMessage()
{
    QDBusInterface iface(LOGIN1_SERVICE, LOGIN1_PATH, FREEDESKTOP_LOGIN1, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.login1.Manager", "BlockInhibited");
    if (reply.isValid()) {
        QString lidState = reply.value().toString();
//          Q_EMIT lidStateChanged(m_lidState);
        switch (blockInhibited) {
        case SHOTDOWN:
            if(!lidState.contains("shutdown")) {
                shutdownWidgetClicked();
            }
            break;
        case REBOOT:
            if(!lidState.contains("shutdown")) {
                rebootWidgetClicked();
            }
            break;
        case SLEEP:
            if(!lidState.contains("sleep")) {
                suspendWidgetClicked();
            }
            break;
        case HIBERNATE:
            if(!lidState.contains("sleep")) {
                hibernateWidgetClicked();
            }
            break;
        default:
            break;
        }
        qDebug() << "lid state:" << lidState;
    } else {
        qDebug() << "Failed to get lid closed event!";
    }
}

void PowerManager::shutdownWidgetClicked()
{
    if(lockinfo::getSystemdInhibitor(QString("shutdown"))){
        Q_EMIT is_blockInhibited(SHOTDOWN, 1);
        blockInhibited = SHOTDOWN;
        return;
    }
    if(loginNum > 1){
        Q_EMIT is_blockInhibited(SHOTDOWN, 0);
        return;
    }
    try{
        qDebug() << "shutdown";
        m_power->shutdown();
        close();
    }catch(QException &e) {
        qWarning() << e.what();
    }
}

void PowerManager::rebootWidgetClicked()
{
    if (lockinfo::getSystemdInhibitor(QString("shutdown"))) {
        Q_EMIT is_blockInhibited(REBOOT, 1);
        blockInhibited = REBOOT;
        return;
    }
    if (loginNum > 1) {
        Q_EMIT is_blockInhibited(REBOOT, 0);
        return;
    }
    try {
        m_power->restart();
        close();
    } catch (QException &e) {
        qWarning() << e.what();
    }
}

void PowerManager::suspendWidgetClicked()
{
    if(lockinfo::getSystemdInhibitor(QString("sleep"))){
        Q_EMIT is_blockInhibited(SLEEP, 1);
        blockInhibited = SLEEP;
        return;
    }
    try{
        emit switchToUser();
	m_power->suspend();
    }catch(QException &e) {
        qWarning() << e.what();
    }
}

void PowerManager::hibernateWidgetClicked()
{
    if(lockinfo::getSystemdInhibitor(QString("sleep"))){
        Q_EMIT is_blockInhibited(HIBERNATE, 1);
        blockInhibited = HIBERNATE;
        return;
    }
    try{
        emit switchToUser();
    m_power->hibernate();
    }catch(QException &e) {
        qWarning() << e.what();
    }
}

void PowerManager::refreshTranslate()
{
      switchLabel->setText(tr("Switch User"));
      rebootLabel->setText(tr("Restart"));
      shutdownLabel->setText(tr("Power Off"));
}

void PowerManager::initUI()
{
    //setSpacing(30);
    curFontSize = m_configuration->getUserConfig("fontSize").toDouble();

//    actService = new QDBusInterface("org.freedesktop.Accounts",
//                                    "/org/freedesktop/Accounts",
//                                    "org.freedesktop.Accounts",
//                                    QDBusConnection::systemBus());

//    QDBusMessage ret = actService->call("ListCachedUsers");
//    QList<QVariant> outArgs = ret.arguments();
//    QVariant first = outArgs.at(0);
//    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
//    dbusArgs.beginArray();
//    QDBusObjectPath path;
//    int userCount =0;
//    while (!dbusArgs.atEnd())
//    {
//        userCount++;
//        dbusArgs >> path;
//    }
//    dbusArgs.endArray();
//    if(userCount > 1){
//        switchWidget = new QWidget(this);
//        switchWidget->setObjectName("switchWidget");
//        QLabel *switchFace = new QLabel(this);
//        switchLabel =  new QLabel(this);
//        switchFace->setAlignment(Qt::AlignCenter);
//	switchFace->setObjectName("switchFace");
//	switchFace->installEventFilter(this);
//        switchLabel->setAlignment(Qt::AlignCenter);
//        switchFace->setPixmap(QPixmap(":/images/avatar.png").scaled(58,58));
//        switchLabel->setText(tr("Switch User"));
//        switchWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
//        QVBoxLayout *switchlayout = new QVBoxLayout(switchWidget);
//        switchlayout->addWidget(switchFace);
//        switchlayout->addWidget(switchLabel);
//    }
    rebootWidget = new QWidget(this);
    rebootWidget->setObjectName("rebootWidget");
    QWidget *rbLabelWidget = new QWidget(this);
    rbLabelWidget->setFixedSize(130, 130);
    QPushButton *rebootFace = new QPushButton(rbLabelWidget);
    rebootLabel = new KLabel(this);
    rebootFace->setObjectName("rebootFace");
    rebootFace->installEventFilter(this);
    rebootLabel->setAlignment(Qt::AlignCenter);
    rebootFace->setIcon(QIcon(":/images/reboot.svg"));
    rebootFace->setIconSize(QSize(48, 48));
    rebootFace->setFixedWidth(130);
    rebootLabel->setFontSize(16 + curFontSize);
    rebootLabel->setText(tr("Restart"));
    rebootWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *rebootlayout = new QVBoxLayout(rebootWidget);
    rebootlayout->addWidget(rbLabelWidget);
    rebootlayout->setAlignment(Qt::AlignHCenter);
    rebootlayout->addWidget(rebootLabel);
    
    shutdownWidget = new QWidget(this);
    shutdownWidget->setObjectName("shutdownWidget");
    QWidget *shLabelWidget = new QWidget(this);
    shLabelWidget->setFixedSize(130, 130);
    QPushButton *shutdownFace  = new QPushButton(shLabelWidget);
    shutdownLabel = new KLabel(this);
    shutdownLabel->setAlignment(Qt::AlignCenter);
    shutdownFace->setObjectName("shutdownFace");
    shutdownFace->installEventFilter(this);
    shutdownFace->setIcon(QIcon(":/images/shutdown.svg"));
    shutdownFace->setIconSize(QSize(48, 48));
    shutdownFace->setFixedWidth(130);
    shutdownLabel->setFontSize(16 + curFontSize);
    shutdownLabel->setText(tr("Shut Down"));
    shutdownWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    QVBoxLayout *shutdownlayout = new QVBoxLayout(shutdownWidget);
    shutdownlayout->addWidget(shLabelWidget);
    shutdownlayout->setAlignment(Qt::AlignHCenter);
    shutdownlayout->addWidget(shutdownLabel);

    if(m_power->canHibernate()) {
        hibernateWidget = new QWidget(this);
        hibernateWidget->setObjectName("hibernateWidget");
        QWidget *hbLabelWidget = new QWidget(this);
        hbLabelWidget->setFixedSize(130, 130);
        QPushButton *hibernateFace = new QPushButton(hbLabelWidget);
        KLabel *hibernateLabel = new KLabel(this);
        hibernateFace->setObjectName("hibernateFace");
        hibernateFace->installEventFilter(this);
        hibernateLabel->setAlignment(Qt::AlignCenter);
        hibernateFace->setIcon(QIcon(":/images/hibernate.svg"));
        hibernateFace->setIconSize(QSize(48, 48));
        hibernateLabel->setFixedWidth(130);
        hibernateLabel->setFontSize(16 + curFontSize);
        hibernateLabel->setText(tr("Hibernate"));
        hibernateWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
        QVBoxLayout *hibernatelayout = new QVBoxLayout(hibernateWidget);
        hibernatelayout->addWidget(hbLabelWidget);
        hibernatelayout->setAlignment(Qt::AlignHCenter);
        hibernatelayout->addWidget(hibernateLabel);
        listLabel.append(hibernateFace);
    }

    if(m_power->canSuspend()){
        suspendWidget = new QWidget(this);
        suspendWidget->setObjectName("suspendWidget");
        QWidget *spLabelWidget = new QWidget(this);
        spLabelWidget->setFixedSize(130, 130);
        QPushButton *suspendFace = new QPushButton(spLabelWidget);
        KLabel *suspendLabel = new KLabel(this);
        suspendFace->setObjectName("suspendFace");
        suspendFace->installEventFilter(this);
        suspendLabel->setAlignment(Qt::AlignCenter);
        suspendFace->setIcon(QIcon(":/images/suspend.svg"));
        suspendFace->setIconSize(QSize(48, 48));
        suspendLabel->setFixedWidth(130);
        suspendLabel->setFontSize(16 + curFontSize);
        suspendLabel->setText(tr("Suspend"));
    	suspendWidget->setFixedSize(ITEM_WIDTH,ITEM_HEIGHT);
    	QVBoxLayout *suspendlayout = new QVBoxLayout(suspendWidget);
        suspendlayout->addWidget(spLabelWidget);
        suspendlayout->setAlignment(Qt::AlignHCenter);
    	suspendlayout->addWidget(suspendLabel);
        listLabel.append(suspendFace);
    }


//   if(userCount>1){
//        QListWidgetItem *item0 = new QListWidgetItem();
//        item0->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
//        insertItem(this->count(), item0);
//        setItemWidget(item0, switchWidget);
//    }
    if(m_power->canHibernate()){
        QListWidgetItem *item1 = new QListWidgetItem();
        item1->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        insertItem(this->count(), item1);
        setItemWidget(item1, hibernateWidget);
    }

    if(m_power->canSuspend()){
        QListWidgetItem *item0 = new QListWidgetItem();
        item0->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        insertItem(this->count(), item0);
        setItemWidget(item0, suspendWidget);
    }

    QListWidgetItem *item2 = new QListWidgetItem();
    item2->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item2);
    setItemWidget(item2, rebootWidget);
    listLabel.append(rebootFace);

    QListWidgetItem *item3 = new QListWidgetItem();
    item3->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    insertItem(this->count(), item3);
    setItemWidget(item3, shutdownWidget);
    listLabel.append(shutdownFace);

    adjustSize();

}
