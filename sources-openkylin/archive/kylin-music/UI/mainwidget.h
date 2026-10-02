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

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QShortcut>

#include <QPainter>
#include <QPainterPath>
#include <QBrush>
#include <QColor>
#include <QStyleOption>
#include <QPaintEvent>
#include <ukuiwindowhelper.h>
//DBus
#include <QDBusInterface>
#include <QDBusConnection>
//窗口显示在屏幕中心
#include <QApplication>
#include <QScreen>
//单例需要的头文件
#include <fcntl.h>
//窗体拉起
#include <KWindowSystem>
#include <QProcess>

#include "./dbusadapter.h"
#include "UIControl/base/musicDataBase.h"
#include "UI/tableview/tableone.h"
#include "UIControl/tableview/musiclistmodel.h"
#include "UI/tableview/customTreeView.h"
#include "UI/base/widgetstyle.h"
#include "UI/sidebar/sidebarwidget.h"
#include "UI/player/playsongarea.h"
#include "UI/tableview/tablehistory.h"
#include "UI/player/lyricswidget.h"
#include "UI/titlebar/titlebar.h"
#include "UI/player/miniWidget.h"
#include "UIControl/base/daemonipcdbus.h"
#include "UI/player/sliderwidget.h"
#include <ukuiwindowhelper.h>
#include <libkydatacollect.h>

#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
#define FONT_SIZE "system-font-size"
#define THEME_QT_TRANS "org.ukui.control-center.personalise"

#define PLAY_STATUS_PROPERTIES "PlaybackStatus"
#define PLAY_STATUS "Playing"
#define PAUSED_STATUS "Paused"
#define STOP_STATUS "Stopped"

class PlaySongArea;
class TableOne;
class DbusAdapter;

class Widget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.kylin_music.play")

    friend class unitMainWindow;

public:
    enum PlaybackMode { CurrentItemOnce=0, CurrentItemInLoop, Sequential, Loop,  Random }; //播放循环模式枚举
    PlaybackMode playbackMode() const; //获取播放循环模式

    explicit Widget(QStringList strlist, QWidget *parent = nullptr);
    ~Widget();

    //计算播放历史
    void movePlayHistoryWid();
    void moveVolumeWidget();
    void showVolumeWidget();

    void initOpacityGSettings();

    static Widget *mutual;          //指针类型静态成员变量
    QProcess *process;

    QGSettings *m_transparencyGSettings = nullptr; // 控制面板透明度
    double m_transparency = 1.0;  // 透明度
    TableOne *musicListTable = nullptr;
    TitleBar *m_titleBar = nullptr;
    LyricsWidget *m_lyricsWid = nullptr;
    void volumeChange(int volume);
    void showList();
protected:
    void paintEvent(QPaintEvent *eventjk) override;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event) override;

public Q_SLOTS:
    //mini窗口
    void slotShowMiniWidget();
    //关闭主窗体
    void slotClose();
    // 弹出历史列表删除弹窗
    void slotDeleteAllHistoryPlaylistMessageBox();
    //最小化
    void slotShowMinimized();
    //最大化
    void slotShowMaximized();

    ////////// mpris
    // 停止播放
    void Stop() const;
    // 增加音量
    void VolumeUp() const;
    // 降低音量
    void VolumeDown() const;
    // 设置音量
    void setVolume(int v) const;
    // 获取音量
    int getVolume() const;
    // 下一首
    void Next() const;
    // 上一首
    void Previous() const;
    // 播放
    void Play() const;
    // 暂停
    void Pause() const;
    // 播放暂停
    void PlayPause() const;

    // 调整播放位置
    void Seek(const qint64 Offset) const;

    //获取元数据
    QVariantMap getMetadata() const;

    // 调整播放速度
    void setPlayRate(double s) const;

    // 获取播放速度
    double getPlayRate() const;

    // 获取当前播放位置
    qint64 getPosition() const;
    // 得到当前状态
    QString getStates() const;

    //获取当前播放歌曲的状态
    QString getState();

    //音量调高
    void volumeIncrease() const;
    //音量调低
    void volumeReduce() const;
    //系统音乐音量改变
    void slotVolume(int volume);

    //mini
    void slotRecoverNormalWidget();
    //mini 关闭窗体
    void slotCloseMiniWidget();
    //获取当前播放歌曲的名称
    void slotPlayingTitle(QString title);
    //当前播放歌曲的名称
    QString getTitle();
    //命令参数
    int kylin_music_play_request(QString cmd1, QString cmd2 = "", QString cmd3 = "");
    //判断当前点击的按钮是否为歌曲列表（歌单名）
    void slotText(QString btnText);
    //
    void slotReturnPressed();
    //构造完成之后的流程事件函数
    void creartFinish();
    //构造完成后需要弹窗
    void setCreatFinishMsg(QString msg);
    //如果正在播放，阻止锁屏
    void slotStateChanged(playController::PlayState state);
    // 获取播放模式
    QString getPlayMode() const;
    //设置当前播放状态
    void setPlayMode(QString state);
private Q_SLOTS:
    void onScreenLock();
    void onScreenUnlock();
    void onPrepareForShutdown(bool Shutdown);
    void onPrepareForSleep(bool isSleep);
    void client_get(QString str);
    void inputDevice_get(QString str);
    void slotPrepareForSwitchuser();

Q_SIGNALS:
    // Space 控制播放/暂停
    void playPauseKeySignal();

    // Ctrl+Left 控制上一首
    void previousPlayKeySignal();

    // Ctrl+Right 控制下一首
    void nextPlayKeySignal();

    // Ctrl+Up 控制音量增加
    void addVolumeKeySignal();

    // Ctrl+down 控制音量降低
    void downVolumeKeySignal();

    // Ctrl+M 切换 Mini/完整模式
    void miniCompleteSwitchKeySignal();

    // Ctrl+L 将歌曲添加到喜欢歌单
    void loveSongKeySignal();

    //Ctrl+T 切换播放模式
    void changePlaybackMode(int mode);
    void minisiglyricsHide();
    void normalsiglyricsHide();

private:
    //初始化dbus
    void initDbus();
    void initStyle();
    //单例
    void Single(QStringList path);
    //处理双击文件打开时间
    void stateMusicFile(QStringList args);
    void initAllComponent();
    void allConnect();
    void initGSettings();
    void moveWidget(QString newWidth, QString newHeight);
    void changeDarkTheme();
    void changeLightTheme();
    void importFile(QStringList list);
    QStringList getPath(QString playListName);
    void initThemeColor();
    //初始化音乐
    void initMusic();
    PlaybackMode m_playbackMode = Loop; //当前列表的循环模式
    void initLayOut();
    QStringList m_inputList;
private:
    // QGSettings
    QGSettings *m_gsTransOpacity = nullptr;
    qreal m_curTransOpacity = 1;

    QDBusInterface *interface;
    quint32 m_inhibitValue = 0;             // 阻止锁屏cookie
    QFocusEvent::Type type =  QEvent::FocusIn;

    QVBoxLayout *mainVBoxLayout = nullptr;
    QHBoxLayout *mainHBoxLayout = nullptr;
    TableHistory *historyListTable = nullptr;
    SliderWidget *m_volSliderWid = nullptr; // 音量弹出窗口
    QGSettings *themeData = nullptr;
    QGSettings *themeColor = nullptr;
    SideBarWidget *sideBarWid = nullptr;
    PlaySongArea *playSongArea = nullptr;
    MiniWidget *m_miniWidget = nullptr;
    QWidget *rightVWidget = nullptr;

    bool isMaximize = false;       //最大化和还原俩个状态
    bool isMinimode = false;

    //判断是否为第一个实例
    bool isFirstObject = false;
    QStringList argName;
    //歌单名
    QString listName;
    //正在播放歌曲的title
    QString m_playTitle;
    //避免初始化流程中触发点击事件
    bool m_initFinish = false;
    //主界面初始化完成后执行的操作
    enum CreatFinishEnum{NONE=0,MESSAGE,OTHER} m_creatFinishEnum = NONE;
    QString m_creatFinishMsg;

    // ctrl+q 退出窗口快捷键
    QShortcut *m_quitWindow;
    UkuiWindowHelper* m_windowHelper { nullptr };

    // wayland 窗口id
    QString m_windowId = "";
    QWidget *m_widget = nullptr;
    QWidget *m_bufferWidget = nullptr;

    DbusAdapter *m_dbsAdapter { nullptr };

Q_SIGNALS:
    void signalShowGuide();
    //刷新歌曲列表界面
    void signalRefreshList(QString listName);
    //字体
    void signalSetFontSize(int);
    // 主题强调色
    void signalSetThemeColor();
    // 图标强调色
    void signalSetThemeIcon(QString themeIcon);
};
#endif // WIDGET_H
