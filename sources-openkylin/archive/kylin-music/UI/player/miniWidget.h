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

#ifndef MINIWIDGET_H
#define MINIWIDGET_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QPixmap>
#include <QPainter>
#include <QBitmap>

#include "UIControl/player/player.h"
#include "UIControl/base/musicDataBase.h"
#include "UI/base/customLabel.h"
#include "UIControl/base/musicfileinformation.h"
#include <kborderlessbutton.h>
//#include <kabase/buried_point.hpp>
#include <kpushbutton.h>
const QString miniMainColor = "QFrame{background:#FFFFFF;}";

#pragma pack(push)
#pragma pack(1)

class MiniWidget : public QFrame
{
    Q_OBJECT

public:
    enum PlaybackMode { CurrentItemOnce=0, CurrentItemInLoop, Sequential, Loop,  Random }; //播放循环模式枚举
    explicit MiniWidget(QWidget *parent = nullptr);
#ifndef WHITE_TEST
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    bool eventFilter(QObject *obj, QEvent *event);
    void songText(QString songName); //mini 正在播放
#endif
    void initAction();
    void initConnect();
    //初始化样式
    void initFocusStyle();
    //通过路径查询歌曲的title和时长
    void songInfo(QString path);
    //显示时长
    void slotPositionChanged(qint64 position);

    void setCloseAndRecoveryButtonStyle();

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    //发送信号mini窗口我喜欢按钮改变
    void signalFavBtnChange(QString filePath);
    //刷新我喜欢列表
    void signalRefreshFav(QString listName);

    //空格键控制播放暂停
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

public Q_SLOTS:
    void playerStateChange(playController::PlayState newState);
    void slotFavExixts();
    void slotFavExixtsDark();
    void slotFavIsExixts(QString filePaths);
    QPixmap setCoverPhotoPixmapRadius(QPixmap pixmap);
    void slotSongInfo(QString path);
    void slotFav();
    void slotSetThemeColor(); // 切换主题色
    //接收播放区我喜欢按钮改变
    void slotFavBtnChange(QString filePath);
    //获取正在播放的title
    void slotPlayingLab(QString playing);
    //获取正在播放的时长
    void slotTimeLab(QString time);
    //设置播放模式
    void slotPlayModeClicked();
    //设置图标
    void setPlayMode(int playModel);
    //没有歌曲在播放
    void slotNotPlaying();
    //判断当前点击的按钮是否为我喜欢（歌单名）
    void slotText(QString btnText);
    //字体
    void slotLableSetFontSize(int size);

public:
    QFrame *m_mainFrame = nullptr;

    QLabel *m_coverLabel = nullptr; // 封面图
    QPushButton *m_loveBtn = nullptr; // 收藏
    QPushButton *m_orderBtn = nullptr; // 播放模式

    QPushButton *m_closeBtn = nullptr;
    QPushButton *m_recoveryWinBtn = nullptr;

    kdk::KPushButton *m_preBtn = nullptr; // 上一首
    kdk::KPushButton *m_playBtn = nullptr; // 播放状态
    kdk::KPushButton *m_nextBtn = nullptr; // 下一首

    CustomLabel *m_songNameLab = nullptr;
    QLabel *m_timeLab = nullptr;

    void initStyle();

    QMenu *playModeMenu = nullptr;
    QAction *playMode_Sequential_Action = nullptr;
    QAction *playMode_Loop_Action = nullptr;
    QAction *playMode_CurrentItemInLoop_Action = nullptr;
    QAction *playMode_Random_Action = nullptr;
    QGSettings *themeData = nullptr;

private:
    PlaybackMode m_playbackMode = Loop; //当前列表的循环模式
    void init_miniWidget();
    bool m_mouseState;

    QPoint  m_WindowPos;
    QPoint m_MousePos;

    QHBoxLayout *m_hMainLayout = nullptr;
    QHBoxLayout *m_hPlayLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

    QFrame *m_palyFrame = nullptr;
    QWidget *coverWid = nullptr;

    QHBoxLayout *m_HMainLayout = nullptr;

    QVBoxLayout *m_vInfoLayout = nullptr;
    QVBoxLayout *m_vSysLayout = nullptr;
    QString filePath;
    //歌单名
    QString listName;
    //正在播放歌曲的歌曲名
    QString m_title;
    //正在播放歌曲的时长
    QString m_time;
};

#pragma pack(pop)

#endif // MINIWIDGET_H
