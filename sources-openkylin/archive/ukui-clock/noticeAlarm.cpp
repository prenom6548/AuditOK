/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "noticeAlarm.h"
#include "ui_noticeAlarm.h"
#include <QPalette>
#include <QSqlTableModel>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDebug>
#include <QPainterPath>
#include <X11/Xlib.h>
#include "clockentitydao.h"
#include "xatom-helper.h"
#include "gsettingsubject.h"
#include <math.h>
#include <QThread>
#include <QBitmap>
#include "noticealarmpoolutil.h"
#include "theme.h"
#include <QDBusMessage>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
/**
 * @brief
 * @param
 * @param      num 闹钟序号 倒计时传-1
 *
 * @return 返回说明
 */
Natice_alarm::Natice_alarm(int close_time, int num, QWidget *parent , QString clockId) :
    QWidget(parent),
    timer_value(close_time),
    num_flag(num),
    m_clockId(clockId)
{
    m_musicPlayer = new MMediaPlayer(this);
    m_musicPlayer->setObjectName("ukui-clock_MMediaPlayer_m_musicPlayer");
    m_playList = new MMediaPlaylist(m_musicPlayer);
    m_playList->setObjectName("ukui-clock_MMediaPlaylist_m_playList");
    m_playList->setPlaybackMode(MMediaPlaylist::CurrentItemInLoop);
    m_selectBtnUtil = new SelectBtnUtil();
    m_selectBtnUtil->setObjectName("ukui-clock_SelectBtnUtil_m_selectBtnUtil");
    // 检查通知中心是否允许闹钟播放声音
    updateDoNotDisturbStatus();

    //保存一个多少秒后关闭，用于稍后提醒的显式
    timer_value2 = timer_value;
    //进行 剩余秒数显示 的定时
    timer = new QTimer();
    timer->setObjectName("ukui-clock_QTimer_timer");
    //剩余秒 减1
    connect(timer, SIGNAL(timeout()), this, SLOT(close_music()));
    timer->setInterval(1000);

    timer_xumhuan = new QTimer();
    timer_xumhuan->setObjectName("ukui-clock_QTimer_timer_xumhuan");
    //稍后提醒的定时
    connect(timer_xumhuan, SIGNAL(timeout()), this, SLOT(ring()));
    timer_xumhuan->setInterval(1000);
    model_clock = clock_sql::getClockTable(this);

    natice_init();
    //数据库
    model_setup = clock_sql::getSetupTable(this);
    //倒计时初始化无稍后提醒
    if(num_flag < 0){
        hideRemindBtn();
    }
    //随机一个id
    m_utilptr = new Utils();
    m_utilptr->setObjectName("ukui-clock_Utils_m_utilptr");
    m_noticeId = m_utilptr->getRandomId().toUInt();
//    m_noticeId = 1;

}




Natice_alarm::~Natice_alarm()
{
    delete timer;
    delete timer_xumhuan;
    if(model_clock != nullptr){
        delete model_clock;
        model_clock = nullptr;
    }
    if(model_setup != nullptr){
        delete model_setup;
        model_setup = nullptr;
    }
    if(m_musicPlayer != nullptr){
        delete m_musicPlayer;
        m_musicPlayer = nullptr;
    }
    if(m_playList != nullptr){
        delete m_playList;
        m_playList = nullptr;
    }
    if(m_selectBtnUtil != nullptr){
        delete m_selectBtnUtil;
        m_selectBtnUtil = nullptr;
    }
    if(m_utilptr != nullptr){
        delete m_utilptr;
        m_utilptr = nullptr;
    }
}
/**
 * @brief 初始化铃声
 * @param
 * @param
 *
 * @return 返回说明
 */
void Natice_alarm::natice_init()
{
    if(num_flag >= 0)
    {
        auto sqlQuery = ClockEntityDao::getClockByPK(m_clockId);
        //更新闹钟名称
        QString info = model_clock->index(num_flag, 0).data().toString();
        info = Utils::changeNumToStr(info.toInt());
        info = info.append(":");
        info = info.append(Utils::changeNumToStr(model_clock->index(num_flag, 1).data().toInt()));
        info = info.append(" ");
        QString clockName = model_clock->index(num_flag, 13).data().toString();
        info = info.append(clockName);
        m_timeEndInfo=info;
        //稍后提醒逻辑
        QString remindLate = model_clock->index(num_flag, 15).data().toString();
        if(remindLate=="none"||remindLate==tr("none")){
            hideRemindBtn();
        }else{
            showRemindBtn();
        }
    } else {
        //倒计时
        m_timeEndInfo=tr("Time out")+"！";
    }
    m_autoCloseTime=(QString::number(60)+tr(" Seconds to close"));
}



void Natice_alarm::callNotice()
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    QStringList actions;
    if(!m_hideRemindLate){
        actions.append("noticeShowAgain");        //按钮动作
        actions.append(QString(tr("Remind later")));  //按钮名称
    }
    m_noticeInfo = m_timeEndInfo+" "+m_autoCloseTime;
    qDebug()<<"dbq-m_noticeInfo"<<m_noticeInfo<<m_noticeId;
    unsigned int id = 0;
    if(num_flag < 0){
        id = m_noticeId;
    }
    QMap<QString, QVariant> hints;
    hints.insert("urgency",1);  //紧急程度，0：低（不提醒收起） 1：默认（停留6秒） 2：重要（驻留）
    hints.insert("x-ukui-popup-timeout", 60*1000);
    args<<QString(tr("Alarm"))                   //应用名
        <<(unsigned int)id           //替换通知ID,不替换通知的话填0。 例如：该字段是5，如果通知中存在ID = 5的弹窗，则ID=5的通知内容将替换成这条通知的内容。
        <<QString("kylin-alarm-clock") //图标
        <<QString(tr("Alarm")) //主题
        <<QString(m_noticeInfo) //正文
        <<actions                       //按钮动作
        <<hints                         //其他参数字段
        <<(int)0;  //超时时间：-1：默认时间（由通知中心决定） 0：常驻通知（没有按钮的通知不要设置成0） 其他：自定义超时时间，单位ms
    QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qInfo() << "发送成功！通知ID：" << response.arguments().takeFirst().toString();
        m_noticeId = response.arguments().takeFirst().toUInt(); //返回的是通知中心为该条消息分配的ID
    } else{
        qInfo() << "发送失败：" << response.type();
    }
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications","/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications","NotificationClosed",
                             this,SLOT(StopSendnotify(uint,uint)));
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications","/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications","ActionInvoked",
                             this,SLOT(showAgainnotify(uint,QString)));
}
void Natice_alarm::StopSendnotify(uint id, uint closeReson)
{
    qDebug()<<"dbq-id"<<id<<"closeReson"<<closeReson;
    if(id==m_noticeId&&closeReson == ClosedReason::ClosedByUser){
        qDebug()<<"通知关闭"<<id;
        NoticeAlarmPoolUtil::getInstance()->deleteNoticesMap(id);
        set_dialog_close();
    }
}

void Natice_alarm::showAgainnotify(uint id, QString actionKey)
{
    if(id==m_noticeId&&actionKey == "noticeShowAgain"){
        qDebug()<<"通知稍后提醒"<<id;
        show_again();
    }
}

void Natice_alarm::updateCanPlayMusicStatus(bool canPlay)
{
    if(m_canPlayMusic != canPlay)
        m_canPlayMusic = canPlay;

    // 如果播放声音的过程中，通知中心发出不允许播放声音的信号，则停止播放
    if(!canPlay && m_musicPlayer->state() == MMediaPlayer::State::PlayingState){
        m_musicPlayer->pauseOnly();
        m_musicPlayer->stop();
    } 
    // 如果正在计时60s，且通知中心发出允许播放声音的信号，则开始播放
    else if(canPlay && timer->isActive()){
        m_musicPlayer->setVolume(100);
        m_musicPlayer->play();
        qInfo() << "canPlay 3" << canPlay << m_musicPlayer->state();

    }
}

//关闭某条通知
void Natice_alarm::closeNotice()
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<m_noticeId; //m_notifyId为要关闭通知的ID号
    iface.callWithArgumentList(QDBus::AutoDetect,"CloseNotification",args);
    qDebug()<<"关闭通知 ID :"<<args.at(0).toUInt();
    m_musicPlayer->pauseOnly();
    m_musicPlayer->stop();
    m_noticeId = 0;
}
void Natice_alarm::setEditDatabase(QString id, bool status)
{
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setObjectName("ukui-clock_QSqlTableModel_model");
    model->setTable("clock");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    QString sql = "update clock set ";
    sql = sql.append(" editstatus=:editstatus , ");
    sql = sql.remove(sql.length()-2,2);
    sql = sql.append(" where Id=:id");
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare(sql);
    sqlQuery.bindValue(":id",id);
    //绑定数据
    sqlQuery.bindValue(":editstatus",status);
    sqlQuery.exec();
    model->submitAll();
}

//重新加载最新音乐
void Natice_alarm::refreshMusic()
{
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setObjectName("ukui-clock_QSqlTableModel_model");
    model->setTable("clock");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model_setup->select();
    QString bellId="";
    if(num_flag >= 0){
        auto sqlQuery = ClockEntityDao::getClockByPK(m_clockId);
        bellId = sqlQuery.value( 2).toString();
    } else {
        bellId = model_setup->index(0, 1).data().toString();
    }
    QString noneId = m_selectBtnUtil->getBellIdByNameEn("none");
    if(bellId!=noneId){
        QString path = m_selectBtnUtil->getBellPathById(bellId);
        m_playList->clear();
        m_playList->addMedia(QUrl::fromLocalFile(path));
        m_musicPlayer->setPlaylist(m_playList);
        int muteOn = model_setup->index(0, 0).data().toInt();
        if(muteOn || !m_canPlayMusic){
            m_musicPlayer->stop();
        }
        else{
            if(m_noticeId == 0){
                m_musicPlayer->pauseOnly();
                m_musicPlayer->stop();
            }else{
                m_musicPlayer->setVolume(100);
                m_musicPlayer->play();
            }
        }
    }
}

void Natice_alarm::updateDoNotDisturbStatus()
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.ukui.NotificationSettings",
                         QDBusConnection::sessionBus());

    QDBusMessage response = iface.call("IsAlarmSoundAllowed");
    if (response.type() == QDBusMessage::ReplyMessage) {
        m_canPlayMusic = response.arguments().first().toBool();
    } else{
        qInfo() << "发送失败：" << response.type();
    }

    qInfo() << "canPlay" <<m_canPlayMusic;
    // 监听“是否允许播放声音”的信号
    bool isConnected = QDBusConnection::sessionBus().connect(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.ukui.NotificationSettings",
        "AlarmSoundAllowedChanged",
        this,
        SLOT(updateCanPlayMusicStatus(bool))
        );
}

void Natice_alarm::hideRemindBtn()
{
    m_hideRemindLate = true;
}

void Natice_alarm::showRemindBtn()
{
    m_hideRemindLate = false;
}

void Natice_alarm::playMusic()
{
    timer_value =  timer_value2;
    m_musicPlayer->setVolume(100);
    alarmAction();
}

void Natice_alarm::alarmAction()
{
    timer->start();
    callNotice();
    refreshMusic();
}






//窗口关闭
/**
 * @brief 铃声、窗体、timer关闭
 * @param
 * @param
 *
 * @return 返回说明
 */
void Natice_alarm::set_dialog_close()
{
    timer->stop();
    m_musicPlayer->pauseOnly();
    m_musicPlayer->stop();
    m_noticeId = 0;
}
//关闭音乐
/**
 * @brief 剩余时间读秒 全关闭
 * @param
 * @param
 *
 * @return 返回说明
 */
void Natice_alarm::close_music()
{
    if (timer_value == 0) {
     //   closeNotice();
        set_dialog_close();
    }else{
        timer_value--;
    }

}

//再次弹出
//Eject again
void Natice_alarm::show_again()
{
    //响铃后，设置闹钟编辑状态为false
    setEditDatabase(m_clockId,false);
    //刷新数据
    model_setup->select();
    QString info = model_clock->index(num_flag, 15).data().toString();
    int remind = getRemindStatusByName(info);
    if (remind == 1) {
        ring_num = 300;
    } else if(remind == 2) {
        ring_num = 600;
    } else if(remind == 3) {
        ring_num = 1200;
    } else if(remind == 4) {
        ring_num = 1800;
    } else if(remind == 5) {
        ring_num = 3600;
    }
    timer_value =  timer_value2;
    timer_xumhuan->start();
    timer->stop();
    m_musicPlayer->pauseOnly();
    m_musicPlayer->stop();
    closeNotice();
}

int Natice_alarm::getRemindStatusByName(QString name)
{
    int status = 0;
    if(name.compare(tr("none")) == 0){
        status = 0;
    }else if(name.compare(tr("five mins late")) == 0){
        status = 1;
    }else if(name.compare(tr("ten mins late")) == 0){
        status = 2;
    }else if(name.compare(tr("twenty mins late")) == 0){
        status = 3;
    }else if(name.compare(tr("thirsty mins late")) == 0){
        status = 4;
    }else if(name.compare(tr("one hour late")) == 0){
        status = 5;
    }
    return status;
}

uint Natice_alarm::getNoticeId() const
{
    return m_noticeId;
}

void Natice_alarm::setNoticeId(const uint &noticeId)
{
    m_noticeId = noticeId;
}

QString Natice_alarm::getClockId() const
{
    return m_clockId;
}
//响铃
//Ring a bell 每一秒减一
void Natice_alarm::ring()
{
    ring_num--;
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("select * from clock where id=:id");
    sqlQuery.bindValue(":id",m_clockId);
    sqlQuery.exec();
    sqlQuery.next();
    bool editstatus = sqlQuery.value(16).toBool();
    //保证被编辑后，旧闹钟稍后提醒消失
    if(editstatus){
        qDebug()<<"zds-闹钟被编辑了";
        ring_num = 0;
    }else{
        qDebug()<<"zds-闹钟没被编辑-啥也不做";
    }
    if (ring_num == 0) {
        timer_xumhuan->stop();
        if(num_flag >= 0){
            bool result = ClockEntityDao::checkClockExist(m_clockId);
            if(result){
                if(editstatus){
                    qDebug()<<"zds-闹钟被编辑了-啥也不做";
                }else{
                    natice_init();
                    alarmAction();
                    }
            }else{
                qDebug()<<"dbq-闹钟被删-啥也不做";
            }
        }else{
            natice_init();
            alarmAction();
        }
    }
}
