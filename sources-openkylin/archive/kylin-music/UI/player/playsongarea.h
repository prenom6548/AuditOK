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

#ifndef PLAYSONGAREA_H
#define PLAYSONGAREA_H

#include <QWidget>
#include <QTimerEvent>
#include <QTimer>
#include <QEventLoop>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QBitmap>
#include <QPainter>
#include <QVariant>
#include <QShortcut>
#include <QMenu>
#include <QDebug>
#include "UI/dbusadapter.h"

#include "sliderwidget.h"
#include "playbackmodewidget.h"
#include "musicslider.h"
#include "UIControl/player/player.h"
#include "UIControl/tableview/musiclistmodel.h"
#include "UI/tableview/tableone.h"
#include "UI/base/customLabel.h"
#include "UI/player/sliderwidget.h"
#include "UI/player/lyricsdialog.h"
#include "UI/player/lyricswidget.h"
#include <kborderlessbutton.h>
//#include <kabase/buried_point.hpp>
#include <kpushbutton.h>
class CoverLabel : public QLabel
{
    Q_OBJECT

    friend class unitMainWindow;

public:
    explicit CoverLabel(QWidget *parent = nullptr) : QLabel(parent){}

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void clicked();
};


class PlaySongArea : public QWidget
{
    Q_OBJECT

    friend class unitMainWindow;


public:
    explicit PlaySongArea(QWidget *parent = nullptr);
    ~PlaySongArea();

    // SliderWidget *m_volSliderWid = nullptr;
    PlayBackModeWidget *m_playBackModeWid = nullptr;

    QPushButton *listBtn = nullptr; //播放列表
    QPushButton *volumeBtn = nullptr; //音量
    QString m_playbackStatus = "Stopped";
    bool isMax = false;
    LyricsDialog * m_lyricsDialog = nullptr;

    void initStyle();
    void initWidget();
    void initConnect();
    //移动音量
    // void moveVolSliderWid();

    //计算新的播放模式菜单
    void listBtnClicked();
    void playMeta();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void showHistoryListBtnClicked();
    //发送信号播放区我喜欢按钮改变
    void signalFavBtnChange(QString filePath);
    //正在播放title
    void signalPlayingLab(QString playing);
    //正在播放时长
    void signalTimeLab(QString time);
    //刷新我喜欢界面
    void signalRefreshFav(QString listName);
//    //按钮添加到我喜欢刷新界面
//    void signalAddFromFavButton(QString listName);
//    //按钮从我喜欢删除刷新界面
//    void signalDelFromFavButton(QString listName);
    void showVolumeBtnClicked();
    void showLyricsClicked();
    void dlgisPlaying(qint64 position);
    void dlgisStoping();

public Q_SLOTS:
    //上一首
    void slotPrevious();
    void delayMsecond(unsigned int msec); // 延时函数，不阻塞线程
    //播放/暂停
    void slotPlayClicked();
    //下一首
    void slotNext();
    void slotSetThemeColor(); // 切换主题色
    //播放通过路径判断我喜欢是否存在此歌曲
    void slotFavExixts();
    //深色模式图标样式
    void slotFavExixtsDark();
    //动态添加判断我喜欢是否存在
    void slotFavIsExixts(QString filePaths);
    //显示音量界面
    void slotVolSliderWidget();
    //音量调高
    // void volumeIncrease();
    //音量调低
    // void volumeReduce();
    //改变音量
    void slotVolumeChanged(int values);
    //系统音乐音量改变
    // void slotVolume(int volume);
    //系统音乐音量是否静音（ mute = true 静音 ）
    void slotMute(bool mute);
    //我喜欢按钮
    void slotFav();
    //显示歌词dialog
    void slotShowLyricDlg();
    void lyricsWidHide();
    //设置播放模式
    void slotPlayModeClicked();
    //判断当前点击的按钮是否为我喜欢（歌单名）
    void slotText(QString btnText);

    void setPlayMode(int playModel);
    //获取当前播放歌曲的路径
    void slotSongInfo(QString path);
    QPixmap setCoverPhotoPixmapRadius(QPixmap pixmap);
    void setCoverPhotoPixmap(QPixmap pixmap);
    void playerStateChange(playController::PlayState newState);

    //播放滑动条相关
    void slotPositionChanged(qint64 position); // 播放进度改变
    void slotDurationChanged(qint64 duration); // 播放总时长改变
    void setPosition(int position);
    void slotSlidePressd();
    void slotSlideReleased();
    //接收mini窗口我喜欢按钮改变
    void slotFavBtnChange(QString filePath);

    //没有歌曲在播放
    void slotNotPlaying();
    //设置进度条归 0
    void slotSetValue();
    //播放历史按钮选中状态改变
    void slotHistoryBtnChecked(bool checked);
    void coverBtnClicked();
    //字体
    void slotLableSetFontSize(int size);
    //更新
    void updatePlayArea();

    // 切换平板模式
    void switchPCAndTabletMode(bool isTablet);

private:
    MusicSlider *hSlider = nullptr; //播放滑动条
    SideBarWidget *m_sideBarWid = nullptr;
    QHBoxLayout *m_hmainLayout = nullptr; // 进度滑动区域布局

    kdk::KPushButton *preBtn = nullptr;  //上一首
    kdk::KPushButton *playBtn = nullptr; //播放/暂停
    kdk::KPushButton *nextBtn = nullptr; //下一首

    QPushButton *favBtn = nullptr; //我喜欢
    QPushButton *m_orderBtn = nullptr; // 播放模式
    QPushButton *m_lrcBtn = nullptr; // 歌词按钮

    LyricsWidget *m_lyricsWid = nullptr;
    bool is_lrcBtnCheck = false;

    qint64 m_nowtime = 0;
    CoverLabel *coverPhotoBtn = nullptr; //封面
    CustomLabel *playingLabel = nullptr; //正在播放
    QLabel *timeLabel = nullptr; //时长
    QString m_time = nullptr; //保存库解析总时间
    QString filePath = nullptr; //路径
    QString listName = nullptr; //歌单名

    QHBoxLayout *m_mainLayout = nullptr; // 上一首、播放、下一首等区域布局

    QVBoxLayout *m_vmainLayout = nullptr; // 整个播放区域布局

    void closeLyricsDialog();
    void updateThemeColor(QObject *obj, QEvent *event);
};

#endif // PLAYSONGAREA_H
