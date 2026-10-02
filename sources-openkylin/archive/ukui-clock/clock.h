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
#ifndef CLOCK_H
#define CLOCK_H
#include <QPainterPath>
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QQueue>
#include <QSlider>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QLCDNumber>
#include <QMediaPlayer>
#include <QFrame>
#include <itemNew.h>
#include <QListWidgetItem>
#include <QPaintEvent>
#include <QPointF>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QCloseEvent>
#include <QMenu>
#include <QFontDatabase>
#include <math.h>
#include <QTimerEvent>
#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QMatrix>
#include <QFont>
#include <QMediaPlaylist>
#include <QUrl>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QModelIndex>
#include <QSqlQuery>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <unistd.h>
#include <QMessageBox>
#include <QBitmap>
#include <QProcess>
#include <QScreen>
#include <QScroller>
#include <QTranslator>
#include <QDesktopWidget>
#include <QGraphicsOpacityEffect>
#include <QDBusInterface>
#include <QToolTip>
#include <QListView>
#include<QMediaPlayer>
#include <QList>
#include <QMap>
#include "stopwatchItem.h"
#include "verticalScroll24.h"
#include "verticalScroll60.h"
#include "verticalScroll99.h"
#include "setAlarmRepeatDialog.h"
#include "adaptscreeninfo.h"
#include "connection.h"
#include "noticeAlarm.h"
#include "ui_noticeAlarm.h"
#include "deleteMsg.h"
#include "ui_deleteMsg.h"
#include "selectbtn.h"
#include "constant_class.h"
#include "utils.h"
#include "primarymanager.h"
#include "tinycountdown.h"
#include "theme.h"
#include <QHash>
#include "ClockInterface.h"
#include <string>
#include "CJsonObject.hpp"
#include "commontooltip.h"
#include "selectbtnutil.h"
#include "kwidget.h"
#include "kwindowbuttonbar.h"
#include "gui_g.h"
#include "verticalscrollapm.h"
#include <QPointer>
#include <QSystemTrayIcon>
#include <kpushbutton.h>
#include "noticealarmpoolutil.h"
#include "customcombox.h"

class QDialog;
class QSpinBox;
class QComboBox;
class QLabel;
class QFont;
class QPushButton;
class QMediaPlaylist;
class QSqlTableModel;
class SelectBtn;

using namespace kdk;

namespace Ui {
class Clock;
}

class Clock : public QWidget
{
    Q_OBJECT

public:

    explicit Clock(QWidget *parent = nullptr);
    ~Clock();
    friend class TestClass;  // 核心修改：声明测试类为友元

    void showList();

    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void callUserGuide();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showPaint();
    void showPaint7();
    void updateLabelFront(QLabel * label,int size);                      //修改label字体
    void updateQLineEditFront(QLineEdit * lineEdit,int size);                      //修改QLineEdit字体
    void updateAlarmItemFront(int size);                                         //修改闹钟子项字体
    void updateStopwatchItemFront(int size);                                      //修改秒表子项字体
    void showPopupWithItemTooltip(QComboBox *combo);                             //弹出下拉框并对popup内部QListView安装tooltip事件过滤
    int CURRENT_FONT_SIZE=11;
    enum ScreenPosition {
        SP_LEFT = 1,
        SP_CENTER = 2,
        SP_RIGHT=3,
        UP_LEFT=4,
        UP_CENTER=5,
        UP_RIGHT=6
    };
    enum btnType{
        count_down=1,
        add_clock=2
    };
    QString formatX_h(int x_h);
    Ui::Clock *ui;
    QString m_timeZone;
    QSqlTableModel *model_setup;
    QString addAlarm(clockInterface::RequestParams param);
    QString addAlarmJosn(QString param);
    QString addAlarm(QString clockName, int hour, int minute);
    QString addAlarm(QString clockName, int hour, int minute, QString remindLater, QString repeat);
    QString updateClockByIdJosn(QString param);
    QString selectClockByIdJosn(QString param);
    QString deleteClockByIdJosn(QString param);

    KWindowButtonBar* m_windowButtonBar;
    void showThisWindow();
    void selectAlarmMusic();
    void updateMusicLineEditElidedText(QComboBox *combo);
    void selectRemindLate();
    void countdownMusicSellect();
    void alarmRepeat();
protected:
    void paintEvent1(QPaintEvent *);
    void closeEvent(QCloseEvent *event) override;

public slots:
    void sltMessageReceived(/*const QString &msg*/);                //接收socket信息
    void  CountdownPageSwitch();                                                         // 倒计时切换
                                                                                         // Countdown switch
    void  AlarmPageSwitch();                                                             // 闹钟窗口切换
                                                                                         // Alarm window switching
    void  StopwatchPageSwitch();                                                         // 秒表窗口切换
                                                                                         // Stopwatch window switc
    void  settingsStyle();                                                               // 监听主题

    void  blackStyle();                                                                  // 黑色主题

    void  whiteStyle();                                                                  // 白色主题
    void updateTinyBtn();

    void  drawNoAlarmPrompt();                                                           // 绘制无闹钟提示
                                                                                         // Draw no alarm prompt

private slots:
    void buttonImageInit();                                                              // 闹钟按钮图片初始化
                                                                                         // Alarm button picture initialization
    void CountdownInit();                                                                // 倒计时页初始化

    void textChanged(const QString &arg1);
                                                                                         // Countdown page initialization
    void stopwatchInit();                                                                // 秒表页初始化
                                                                                         // Stopwatch page initialization
    void clockInit();                                                                    // 闹钟页初始化
                                                                                         // Alarm page initialization
    void setupInit();                                                                    // 默认初始设置
    void bellIni();
                                                                                         // Default initial settings
    void noticeDialogShow(int, int,QString id);                                                      // 通知弹窗
                                                                                          // Notification Popup
    void modelSetupSet();                                                                // 默认设置数据库数据初始化
                                                                                         // Default setting database data initialization
    void CountDown();                                                                    // 秒表执行
                                                                                         // Stopwatch execution
    void onPushbuttonStartClicked();                                                     // 秒表开始暂停继续
                                                                                         // Stopwatch start pause continue
    void onPushbuttonRingClicked();                                                      // 计次
    void updateLongestShortLabel();
                                                                                         // times count
    void onPushbuttonTimeselectClicked();                                                // 位
                                                                                         // reset
    void windowClosingClicked();                                                         // 窗口关闭
                                                                                         // window closing
    void windowMinimizingClicked();                                                      // 窗口最小化
    void muteAllBell();
    void ringContinueWhenMute(Natice_alarm *tempdialog);
                                                                                         // window minimizing
    void watchTimeJump();
    void timerUpdate();                                                                  // 动态监控闹钟与本地时间
                                                                                         // Dynamic monitoring alarm clock and local time
    void textTimerupdate();                                                              // 闹钟上方电子表
                                                                                         // Electronic watch above alarm clock
    void setAlarmClock();                                                                // 新建闹钟按钮回调
    void setMusicSelectDialogListById(QString bellId,QComboBox * temp);
    void setRemindLateSelectDialogListByName(QString name,QComboBox * temp);
    int getRemindStatusByName(QString name);
                                                                                         // New alarm button callback
    void updateAlarmClock();                                                             // 重绘窗口，更新闹钟
    void timerAlarmStartUpdate();
                                                                                         // Redraw window, update alarm clock
    void OnOffAlarm();                                                                   // 闹钟开关
                                                                                         // Alarm switch
    void deleteAlarm();                                                                  // 闹钟重编辑页面删除闹钟回调
    void deleteAlarmDatabase(QString id);
    void updateClockDatabase(QString id,QString name,QString hour,QString minute,int onOff);
                                                                                         // Alarm re edit page delete alarm callback
    void listdoubleClickslot();                                                          // 双击闹钟打开重编辑页面

    QString getClockPkByCurrentNum();

    QSqlQuery getClockByPK(QString id);
                                                                                         // Double click the alarm clock to open the re edit page
    void stopwatchStartAnimation();                                                      // 倒计时开始动画移动
                                                                                         // Countdown start animation move
    void stopwatchStopAnimation();                                                       // 倒计时结束动画移动
                                                                                         // Countdown start animation move
    void statCountdownMsec();
    void updateCountdownTime();
                                                                                         // Countdown execution
    void setcoutdownNumber(int h, int m, int s);                                         // 设置倒计时初始时间
                                                                                         // Set the initial countdown time
    void startbtnCountdown();                                                            // 倒计时开始-结束回调
    void tinyCountdownFinish();
                                                                                         // Countdown start end callback
    void onMin_5btnClicked();                                                            // 倒计时5分钟设置回调
                                                                                         // Countdown 5 minutes set callback
    void getCountdownOverTime();                                                         // 获取倒计时结束时间
                                                                                         // Get countdown end time
    void onCountPushClicked();                                                           // 倒计时-暂停继续回调
                                                                                         // Countdown - pause resume callback
    void stopwatchJg();                                                                  // 时间间隔计算执行回调
                                                                                         // Interval calculation execution callback
    void changeTimeNum(int Hour, int Minute);                                             // 修改时间单数 为两位数
                                                                                          // Modify time singular to two digits
    void countdownSetStartTime();                                                        // 倒计时初始数字转盘
                                                                                         // Countdown initial digital dial
    void alarmSetStartTime();                                                            // 闹钟初始化数字转盘绘制
                                                                                         // Alarm clock initialization digital turntable drawing
    void alarmCancelSave();                                                              // 闹钟新建界面取消回调
                                                                                         // Cancel callback in alarm new interface
    void setAlarmSave();                                                                 // 闹钟新建界面保存回调
    void saveClockToDatabase(int rowNum);
    void setTimerAlarmStart();
    QString getSelectBellId(QComboBox * temp);

    QString formatReturnMsg(neb::CJsonObject oJson,clockInterface::STATUS_CODE status,std::string msg);
    QString formatReturnMsg(clockInterface::STATUS_CODE status,std::string msg);
                                                                                         // Alarm new interface save callback
                                                                                         // Alarm clock new and re edit interface remaining time real-time display callback
                                                                                         // Alarm clock initialization workday selection interface drawing callback
    void repeatListclickslot(int num);                                                          // 重复选项单击回调
                                                                                         // Repeat option click callback

    void refreshMusicSelectList(QComboBox * temp);
                                                                                         // Alarm clock initialization music selection interface callback
    void musicListclickslot();                                                           // 闹钟初始化单击选择音乐
    void remindLateListClickSlot(int num);
    QString getRemindLateStrFromNum(int num);
                                                                                         // Alarm initialization Click to select music
                                                                                         // Alarm clock initialization music time selection interface callback
                                                                                         // Click to select music duration callback
                                                                                         // Set page draw callback
                                                                                         // Mute switch callback
                                                                                         // Set volume callback
    void countMusicListclickslot();                                                      // 倒计时音乐选择单机回调
    void playMusicFromPath(QString path);
    void stopHisPlay();
    void addDivBell(QComboBox *temp,btnType type);                                       // Countdown music selection single callback
    void countdownNoticeDialogShow();                                                    // 倒计时通知弹窗

                                                                                         // Countdown notification pop-up
    void offAlarm(int);                                                                  // 重复时单独关闭闹钟
                                                                                         // Turn off the alarm separately if it is not repeated
                                                                                         // Calculate the next alarm ring interval
                                                                                         // Calculate the next alarm ring interval
    QString changeNumToStr(int alarmHour);                                               // 整型转字符
                                                                                         // Integer to character
    void onCustomContextMenuRequested(const QPoint &pos);                                // 闹钟右键删除事件处理函数

    void countStatBtnGray();

    QString get12hourStr(int x_h);
    void createUserGuideDebusClient();
    void onTinyClicked();
    void activeWindow();

signals:
    void timeJump(qint64 timeValue);
private:
    QAction *openClock {nullptr};
    QAction *quitAction {nullptr};
    bool onTablet = false;
    QMessageBox *deleteAlarmBox {nullptr};
    QTime targetTime;                                //倒计时的目标时间
    QWidget *preLoadWidget {nullptr};
    bool comboxclickedflag;                           //多选下拉框是否点击过
    QPoint m_startPoint;
    QTimer *timer = nullptr;
    QTimer *countdown_timer = nullptr;
    QTimer *timer_2 = nullptr;
    int hour, minute, second, pushflag;
    // timer 计时
    uint64 m_runTime { 0 };
    uint64 m_stopWatchLastTime { 0 };
    // timer2 计时
    uint64 m_runTime2 { 0 };
    uint64 m_stopWatchLastTime2 { 0 };


    int stopwatch_minute, stopwatch_second, stopwatch_centi_second;
    int countdown_hour, countdown_minute, countdown_second, countdown_pushflag;
    int countdown_msec=1000;
    qint64 m_countdownLastTime { 0 };
    int alarmHour;
    int alarmMinute;
    int cPauseTime;
    bool isStarted;
    QPushButton* muteBtn;
    QPushButton* closeTitleBtn;
    QPushButton* minmizeBtn;
    KMenuButton* menuBtn;
    /**
 * @brief 倒计时运行标记
 */
    bool countdown_isStarted;
    bool countdown_isStarted_2;
    bool stopwatch_isStarted;

    QMediaPlayer *player;
    QString ring;// 铃声名字
                 // Ring name
    QPixmap pixmap1;
    QPixmap pixmap2;
    QPixmap pixmap3;
    QPixmap pixmap4;
    QPixmap pixmap5;
    QPixmap pixmap6;
    QPixmap pixmap7;
    QPixmap pixmap8;
    QPixmap pixmap9;
    QPixmap pixmap10;
    QPixmap pixmap11;
    QPixmap bgPixmap;
    QPixmap repeat_on_Pixmap;
    QPixmap repeat_off_Pixmap;
    QPixmap hourPixmap;
    QPixmap minutePixmap;
    QPixmap secondPixmap;
    QPixmap delBtnPixmap;
    QPixmap on_pixmap;
    QPixmap off_pixmap;
    QPixmap clock_icon;

    QDialog *dialog;
    QFont alarmFont;
    QSpinBox *hourBox;
    QSpinBox *minuteBox;
    QComboBox *pauseTime;

    QMediaPlayer *player_alarm;
    QMediaPlaylist *mediaList; /*播放列表
                                 playlist*/
    QSqlTableModel *model;  /*数据库
                              data base*/
    QSqlTableModel *watchTimeModel = nullptr;

    QString musicPath;
    qint64 last;
    qint64 updateNowMill();
    item_new *w1[20];
    QListWidgetItem *aItem[20];
    stopwatch_item *stopwatch_w[100];
    QListWidgetItem *stopwatch_aItem[100];
    QString stopwatch_h;
    QString stopwatch_m;
    QString stopwatch_s;
    QString stopwatch_jg_h = "00";
    QString stopwatch_jg_m = "00";
    QString stopwatch_jg_s = "00";
    QString alarmHour_str;
    QString alarmMinute_str;

    int stopwatch_item_flag = 0;
    int clock_num = 0;
    int on_off_flag = 0;
    int add_change_flag = 0;
    int change_alarm_line = 0;
    int medel_flag = 0;
    int continu_flag = 0;
    int alarm_repeat_flag = 0;
    int repeat_day[9];   /*重复日选择保存中介
                           Select and save mediation for duplicate days*/
    int repeat_new_or_edit_flag;  /*重复日判断 是新建,还是重编辑,两者获取数据库号不同;
                                    Whether to create or re edit the duplicate day is determined. The database numbers obtained by the two methods are different*/
    int stopwatch_Animation = 0;
    int system_time_flag;
    int last_day_ring = 0;

    VerticalScroll_99 *hourTimerRing;
    VerticalScroll_60 *minuteTimeRing;
    VerticalScroll_60 *secondTimeRing;
    VerticalScroll_24 *timer_alarm_start24;
    VerticalScroll_60 *timer_alarm_start60;
    VerticalScroll_APM *timer_alarm_startAPM;

    CustomComBox *dialog_repeat = nullptr;
    QComboBox *dialog_remind_late = nullptr;
    QComboBox *count_music_sellect = nullptr;
    QComboBox *dialog_music = nullptr;
    QLabel * clockRemindLabel;
    QLabel * clockRepeatLabel;
    QLabel * clockNamelabel;
    QLabel * clockBellLabel;
    QLabel * countdownBellLabelOnSet;
    QLabel * countdownBellLabelOnRun;

    QSqlTableModel * m_bellQueryModel = nullptr;

    adaptScreenInfo *m_pSreenInfo = nullptr;
    PrimaryManager * primaryManager = nullptr;
    Utils *utils = nullptr;
    QWidget *grand = nullptr;
    QString repeat_str;
    QString repeat_str_model;
    QString remind_late_str_model;
    QString clock_name;
    QPropertyAnimation *animation1;
    QPropertyAnimation *animation2;
    QPropertyAnimation *animation3;

    QPushButton *startCountSingle;
    QWidget *shadow;

    QPoint m_dragPosition;                                            /*拖动坐标*/
    bool mousePressed;                                              /*鼠标是否按下*/

    QLineEdit *clockEditOnClockNew;
    QMenu *m_menu;                                                  /*功能菜单*/

    QMenu *popMenu_In_ListWidget_;                                  /*闹钟右键删除菜单*/
    QAction *action_Delete_In_ListWidget_;
    QAction *action_Clear_In_ListWidget_;                           /*闹钟右键删除动作*/
    QAction *action_edit_In_ListWidget_;                           /*闹钟右键删除动作*/
    Natice_alarm *countdownNoticeDialog = nullptr;
    Natice_alarm *alarmNoticeDialog = nullptr;
    QDBusInterface *userGuideInterface;                                   // 用户手册
    bool refreshCountdownLabel11Flag = false;               //是否刷新，倒计时上的小闹钟时间的数值。因为秒数的变化，如果一直动态计算，会出现1分钟的误差
    int x_h=0, x_m=0 ;
    bool m_selectTinyCountdown = false;
    tinyCountdown * tinycountdownDia = nullptr;

    void listenToGsettings();                                           //监听
    void updateFront(const int size);
    void set24ClockItem(int time_H,int time_M,int time_S,int rowNum);
    void set12ClockItem(int time_H,int time_M,int time_S,int rowNum);
    void clearClockItem(int rowNum);
    void iniSystemTimeFlag();
    bool checkSystem24();
    void muteBtnStyle();
    void minBtnStyle();
    void closeBtnStyle();
    void menuBtnStyle();
    bool checkTinyCountdownDia();
    void navigationBtnStyle(KPushButton * btn);
    theme * currentTheme;
    QHash<int,QString> * indexWeekdayMap = nullptr;
    CommonToolTip * m_commonToolTip;
    bool m_muteOn = false;
    bool onEditPage = false;
    QMap<long,int> * timeSepOrderIndex  = nullptr;
    QList<int> * hisLongShortIndex = nullptr;
    SelectBtnUtil * m_selectBtnUtil = nullptr;
    QMediaPlayer *music = nullptr;

    void updateClockSelectBtnLabel(QLabel * temp,int moveHeight,QString text , bool flag=false);
    void updateClockEditPageLabel(int selectHeight,int selectGap,int labelHeightOffset);
    void updateClockEditPageBtn(int selectMoveWidth,int selectMoveHeight,int selectGap,int btnHeight);

    void widgetListWhiteStyle(QListWidget * listWidget);
    void widgetListBlackStyle(QListWidget * listWidget);
    QMediaPlayer * hisPlayer = nullptr;
    void updateSwitchBtnStyle();
    void setSwitchDefaultColor(KPushButton *btn);
    QColor getButtonActive();
    QColor getHighlightActive();
    void setDefaultIcon(KPushButton *btn);
    void setBtnIcon(KPushButton *btn, QString imgUrl, QString localUrl);
    void changeIconHeight(KPushButton *btn);
    void setSwitchHighlightColor(KPushButton *btn);
    QString getDefaultGreyColor();
    void updateRepeatStr(QLabel *label);
    void updateLabelStrCnEn(QLabel *label,int cnLimit,int enLimit,int boLimit);
    void updateLabelTextByPixel(QLabel *label, int pixelWidthLimit);
    void updateLabelTextByLength(QLabel *label, int limitSize);
    void updateRepeatLineEditStr(QLineEdit *temp);
    void updateLineEditStrCnEn(QLineEdit *temp,int cnLimit,int enLimit);
    void updateLineEditTextByLength(QLineEdit *temp, int limitSize);
    QPointer<QSystemTrayIcon> m_trayIcon;
    QString m_trayIconTooltip = "";
    void enableTrayIcon();
    void disableTrayIcon();
    QString changeNumToStrWithAm(int alarmHour);
    void updateTrayIconTooltip(QString info);
    void setDefaultTrayIconTooltip();
    quint32 m_pid;
    bool m_first_pid = true;
    void iniWaylandWinId();
    QTime musicStartTime;
    QObject *m_obj;
    QObject *m_updateObj;
    QObject *m_deleteObj;
    QObject *m_selectObj;
    void publishDbusInterface();
    void publishDbusUpdateClock();
    void publishDbusSelectClock();
    void publishDbusDeleteClock();
    void publishDbusNoticeShowAgain();
    QString getBellNameById(QString bellId);
};

#endif // CLOCK_H
