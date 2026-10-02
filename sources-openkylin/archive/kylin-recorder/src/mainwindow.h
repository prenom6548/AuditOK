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
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define KYLINRECORDER "org.kylin-recorder-data.settings"

#define FITCONTROLTRANS "org.ukui.control-center.personalise"

#include <QGSettings>
#include <KWindowSystem>
#include <QWidget>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QPainter>
#include <QStyle>
#include <QToolButton>
#include <QLabel>
#include <QSlider>
#include <QThread>
#include <QMenu>
#include <KWindowEffects>


#include <QTimer>
#include <QTime>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDesktopWidget>
#include <QtMultimedia> //使用多媒体时要引用这个并且在pro中加QT += multimedia multimediawidgets
#include <QAudioRecorder>
#include <QListWidget>

#include <QFile>
#include <QStackedLayout> //此类提供了多页面切换的布局，一次只能看到一个界面
#include <QMouseEvent>

#include <QPainter>
#include <QColor>
#include <QSettings>
#include <QKeyEvent>
#include <QRegExp>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h> //信号处理
// s3s4需要用DBus接口
#include <QDBusConnection>
#include <QDBusInterface>

#include <QScrollBar>
#include <QFileSystemWatcher>
// #include <QX11Info>

#include "mywave.h"
#include "mythread.h"
#include "settings.h"
#include "save.h"
#include "miniwindow.h"
#include "daemondbus.h"
#include "menumodule.h"
#include "ffutil.h"
#include "mmediaplayer.h"
#include "mmediaplaylist.h"
#include "custombutton.h"
#include "fileitem.h"
#include "playcontrol.h"
#include "recorderdatabase.h"
#include "dbusadapter.h"
#include "ukuiwindowhelper.h"

#include <kborderlessbutton.h>
#include <kabase/buried_point.hpp>
#include <kpushbutton.h>

#define INIT_MAINWINDOW_RECTANGLE_COUNT 130 //用于初始化矩形条个数
#define MAX_FLAG_NUM 6
class MainWindow : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.kylin_recorder") //调用DBus一定要加这一行

    friend class TestMainWindow;
private:                            //波形相关
    QList<int> m_maxNum;            //存储振幅的大小的整型列表
    QVector<myWave *> m_customWave; //小矩形框
    QStringList m_filePathList;

public: //放在public都是有原因的因为不同类之间中调用需要公用！！
    QString m_date_format = "";
    MainWindow(QStringList str, QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow *mutual; //！！！指针类型静态成员变量
    QGSettings *m_defaultPathData = nullptr;
    QGSettings *m_themeData = nullptr;             //主题的setting
    QGSettings *m_pGsettingControlTrans = nullptr; //控制面板透明度

    QString m_devMessage = "";
    QString m_limitThemeColor = "";
    QString m_theme = "ukui-default";
    QString m_themeColor = "";

    menuModule *m_menuModule = nullptr;
    FileItem *m_fileItem = nullptr;
    TipWindow *m_tipWindow = nullptr;
    Settings m_setWindow;
    QThread *m_mainThread = nullptr; //主线程
    MyThread *m_subThread = nullptr; //子线程
    MiniWindow m_miniWindow;

    QStackedWidget *m_pStackedWidget = nullptr; //堆叠布局
    QListWidget *m_list = nullptr;

    QMediaPlaylist *m_playList = nullptr;
    QWidget *m_titleRightWidget = nullptr; //右标题栏Wid

    bool m_isRecording = false;  //默认没有开始录音
    bool m_stratOrPause = false; //开始和暂停1
    QTime m_baseTime;            // mini和MainWindow公用计时的时间
    QTime m_diffValueTime;
    qint64 m_recordDurationMs = 0;
    qint64 m_nextRecordSplitMs = 21600000;

    void initGsetting(); //初始化GSettings
    void bindGSetting(); //绑定
    void currentGSettingChangeUI();
    void updateGsetting_ListWidget(); //更新配置文件和ListWidget
    void themeButton(QString themeColor);
    void themeStyle(QString themeColor);
    void themeWindow(QString themeColor);
    void themeFontStyle(QString m_fontType, int m_fontSize);
    void sourceInput(int index);
    void MainWindowLayout();
    QString playerTotalTime(QString filePath);
    void checkSingle(QStringList path); //检查单例模式
    void isFileNull(int n);             //检查文件列表是否为空

    void changePathDataSlot(const QString &key);
    void changeThemeColorSlot(const QString &key);
    void changeControlTransSlot(const QString &key);
    void recorderBuriedPoint(kdk::kabase::BuriedPoint::PT);
    void changePage(int no);
    QStringList getFilePathList();
    void clearFilePathList();
    void removePathInFilePathList(QString path);
    UkuiWindowHelper *m_ukuiWindowHelper {nullptr};
    QGSettings *m_pTransparency;
    double tranSparency;
    QColor m_pSideColor;
    void adjustBackground();

    static long get_sys_runtime(int type);     //获取系统开机后的运行时间，用于准确计算音频录制时间
private:
    QStringList m_argName;
    QString m_fontType = "";
    int m_fontSize;
    double m_tran = 0.60; //透明度
    bool m_effect = false;
    bool m_isMax = false;
    int m_source;                 //输入、输出源
    int m_preCount = 0;           //之前的总数
    int m_nowCount = 0;           //之后的总数
    bool m_finish = true;         //是否录音完成
    bool m_isFirstObject = false; //判断是否是唯一的对象
    int m_index;
    int m_timeTag = 0;
    QString m_titleRightColor; //右标题栏颜色
    bool m_isFirstRun = true;

    QWidget* m_widget { nullptr };

    QTimer *m_delayTimer = nullptr;        //延时Timer用于每隔150ms检测波形
    DaemonDbus *m_DaemonIpcDbus = nullptr; // 用户手册功能

    QLabel *m_seatLabel = nullptr;
    QLabel *m_zeroFileMessageLabel = nullptr; //文件列表为空时的提示信息
    QLabel *m_appLabel = nullptr;
    QLabel *m_showTimeLabel = nullptr; //主界面时间标签

    QPushButton *m_appPicture = nullptr; //应用图标,标题左上角
    QPushButton *m_setButton = nullptr;  //设置按钮
    QMenu *m_menu = nullptr;             //下拉菜单
    QAction *m_actionSet = nullptr;      //设置项
    QAction *m_actionHelp = nullptr;     //帮助项
    QAction *m_actionAbout = nullptr;    //关于项

    QToolButton *m_miniButton = nullptr;  // mini模式切换按钮
    QToolButton *m_minButton = nullptr;   //最小化按钮
    QToolButton *m_maxButton = nullptr;   //最大化按钮
    QToolButton *m_closeButton = nullptr; //关闭按钮

    QPushButton *m_firstRecordButton = nullptr; //首页录音按钮
    QPushButton *m_thirdRecordButton = nullptr;  //录音按钮
    kdk::KPushButton *m_pauseButton = nullptr;       //暂停按钮
    kdk::KPushButton *m_continueButton = nullptr;    //继续按钮
    kdk::KPushButton *m_finishButton = nullptr;      //完成按钮
    kdk::KPushButton *m_flagButton = nullptr;        //标记按钮
    QToolButton *leftButton = nullptr;           //左尖角
    QToolButton *rightButton = nullptr;          //右尖角

    QLabel *m_fileListTitleLabel = nullptr;        //文件列表标题标签
    QWidget *m_fileListTitleLabelWidget = nullptr; //文件列表标签的Wid
    QWidget *m_fileListWidget = nullptr;           //列表Wid

    QWidget *m_leftMainWidget = nullptr;  //主左Wid
    QWidget *m_rightMainWidget = nullptr; //主右Wid
    QWidget *m_titleLeftWidget = nullptr; //左标题栏Wid
    QWidget *m_titleWidget = nullptr;     //标题栏Wid
    QWidget *m_recordButtonWidget = nullptr;

    QWidget *m_firstPageWidget = nullptr;
    QWidget *m_listPageWidget = nullptr;
    QWidget *m_centerWidget = nullptr; //中间Wids

    QHBoxLayout *m_titleLeftLayout = nullptr;  //左标题栏布局
    QHBoxLayout *m_titleRightLayout = nullptr; //右标题栏布局
    QHBoxLayout *m_titleLayout = nullptr;      //总标题栏布局

    QVBoxLayout *m_recordButtonLayout = nullptr;       //录制按钮布局
    QVBoxLayout *m_fileListWidgetLayout = nullptr;     //文件列表布局
    QHBoxLayout *m_fileListTitleLabelLayout = nullptr; //文件列表布局
    QHBoxLayout *m_firstPageLayout = nullptr;          //录制按钮布局
    QHBoxLayout *m_listPageLayout = nullptr;
    QHBoxLayout *m_centerLayout = nullptr; //中间布局

    QWidget *flagWidget = nullptr;        //标记Wid
    QHBoxLayout *flagLayout = nullptr;    //标记布局
    QWidget *elementWidget = nullptr;     //元素标记Wid
    QHBoxLayout *elementLayout = nullptr; //标记元素布局
    QList<kdk::KBorderlessButton *> m_flagList;        //标记List

    QHBoxLayout *m_buttonLayout = nullptr;
    QHBoxLayout *m_leftMainWidLayout = nullptr;  //主Wid的左布局
    QVBoxLayout *m_rightMainWidLayout = nullptr; //主Wid的右布局
    QVBoxLayout *m_mainLayout = nullptr;


    QHBoxLayout *m_showTimelbLayout = nullptr;
    QHBoxLayout *m_monitorWaveLayout = nullptr;
    QHBoxLayout *m_controlPlayAndPauseLayout = nullptr;
    QVBoxLayout *m_pageTwoLayout = nullptr;

    QWidget *m_pageTwoWid = nullptr; //第二个页面
    QWidget *m_showTimelbWid = nullptr;
    QWidget *m_waveWidget = nullptr; //波形图的容器
    QWidget *m_controlPlayAndPauseWid = nullptr;

    QMessageBox *m_warnBox = nullptr;



    //显示的时间
    QString m_timeStr;
    //窗口坐标相关
    bool m_isPress;
    QPoint winPos;
    QPoint dragPos;
    QPoint m_pressPosRecord = QPoint(0, 0);
    bool m_mousePress = false;

    quint64 m_window_id { 0 };

    void x11EventEnd();
    bool event(QEvent *event);
    void keyPressEvent(QKeyEvent *event);       // 键盘响应事件
    void mousePressEvent(QMouseEvent *event);   //鼠标按压事件
    void mouseReleaseEvent(QMouseEvent *event); //鼠标释放事件
    void mouseMoveEvent(QMouseEvent *event);    //鼠标移动事件
    bool eventFilter(QObject *obj, QEvent *event);
    void paintEvent(QPaintEvent *); //毛玻璃绘制
    void transChange();             //监听控制面板的透明度变化

    // DBus相关
    void initDbus();         //初始化dbus
    void initMainWindow();   //初始化MainWindow
    void setTwoPageWindow(); //设置MainWindow布局
    void showNotify(QString message);

    void changeContinuePauseState(bool isContinue);
    void changeContinuePauseBtnThemeColor(QString themeColor);

    void removeFlagLayoutWidget();
    void sendFlagView(); //上传标记埋点数据

    //快捷键处理
    void keySpaceEvent();//空格
    void keyMEvent(); // M 添加标记
    void keyCEvent(); // C 剪辑录音
    void keyOEvent(); // O 在文件夹中显示
    void initShortcutStyle();// 处理焦点

signals: //主线程的信号

    void recordingSignal(bool);
    void startThread();
    void closeSignal();
    void startRecord();
    void stopRecord();
    void playRecord();
    void pauseRecord();
    void pageChange();
    void itemFontStyleSignal(QFont fontStyle);
    void themeColorchange();
    void reRecordSignla();    //满4h后，防止缓冲区满导致录音录不进去，因此需要发出信号，对第二个文件进行读写，最终将多个文件合成输出文件。

    //快捷键信号
    void keySpaceSignal();
    void keyMSignal();
    void keyCSignal();
    void keyOSignal();
    void switchPageSignal();
    void stop_clickedSignal();

public slots:

    int command_Control(QString cmd1); //命令控制
    void recordPaint(int);
    void mainWindow_page2();
    void switchPage(); //选择页面
    void play_pause_clicked();
    void stop_clicked();
    void updateDisplay();
    void updateRecordDuration(qint64 durationUs);
    void changeVoicePicture(); //改变音量图标
    void goset();              //弹出设置窗体
    void miniShow();
    void minShow();
    void maxShow();
    void closeWindow();
    void handlingSlot();
//    void slotListItemAdd(QString fileName, QString recordTime);
    void slotListItemAdd(QString fileName, QDateTime recordTime); 
    void onPrepareForSleep(bool isSleep);     // S3  S4策略
    void onPrepareForShutdown(bool Shutdown); // S3  S4策略
    void inputDevice_get(QString str);
    void outputDevice_get(QString str);
    void monitorStackChanged(int index);
    void handleInsertSlot();
    void findLeftSlot();
    void findRightSlot();
    void updateDateFormat(QString dateFormat);
    void receivepause();
    void onVolumeDeviceChanged(int type, const QString &portName, const QString &cardName);
    void onVolumeDeviceAdjust(int type);
    void onAudioInputError(QAudio::Error error);
    void blurEffect();
    bool isDeviceAdjust();

};

#endif // MAINWINDOW_H
