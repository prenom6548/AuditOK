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
#ifndef NATICE_ALARM_H
#define NATICE_ALARM_H

#include <QWidget>
#include <QDialog>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStyleOption>
#include <QSqlTableModel>
#include <QMediaPlayer>
#include "selectbtnutil.h"
#include <QLabel>
#include "coreplayer/playcontroller.h"
#include <QDBusInterface>
#include "primarymanager.h"
#include "utils.h"
namespace Ui {
class Natice_alarm;
}
/**
 * @brief 右下角通知弹窗
 */
class Natice_alarm : public QWidget
{
    Q_OBJECT

public:
    explicit Natice_alarm( int close_time, int num,QWidget *parent = nullptr,QString clockId = nullptr);
    ~Natice_alarm();
     QTimer *timer = nullptr;
     QTimer *timer_xumhuan = nullptr;
     MMediaPlayer* m_musicPlayer  = nullptr;
     MMediaPlaylist* m_playList  = nullptr;
     int timer_value;
     void refreshMusic();
     void updateDoNotDisturbStatus();
     void hideRemindBtn();
     void showRemindBtn();
     void playMusic();
     void alarmAction();
     //再次弹出    Eject again
     void show_again();

     uint getNoticeId() const;
     void setNoticeId(const uint &noticeId);
     QString getClockId() const;
     enum ClosedReason {
         TimeOut = 1,
         ClosedByUser = 2,
         ClosedByApp = 3,
         Undefined = 4,
         HandledByUser = 20
     };

     void closeNotice();
     //设置编辑状态
     void setEditDatabase(QString id, bool status);
     uint m_noticeId = 0;
protected:

private slots:
     //窗口关闭    window closing
     void set_dialog_close();
    //关闭音乐    Turn off music
    void close_music();
    //响铃       Ring a bell
    void ring();
    void StopSendnotify(uint id, uint closeReson);
    void showAgainnotify(uint id, QString actionKey);
    void updateCanPlayMusicStatus(bool canPlay);
private:
    void natice_init();
    void callNotice();
    int num_flag;
    int ring_num;
    int timer_value2;
    int full_flag=1;
    QSqlTableModel *model_setup;
    QString m_clockId;
    QSqlTableModel *model_clock;
    SelectBtnUtil * m_selectBtnUtil = nullptr;
    int getRemindStatusByName(QString name);
//    uint m_noticeId = 0;
    QString m_timeEndInfo;
    QString m_autoCloseTime;
    QString m_noticeInfo;
    bool m_hideRemindLate = true;
    bool m_canPlayMusic = true;   //  通知中心是否允许闹钟播放声音
    Utils * m_utilptr = nullptr;
};

#endif // NATICE_ALARM_H
