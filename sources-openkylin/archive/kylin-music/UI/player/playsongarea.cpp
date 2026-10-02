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

#include <ukui-log4qt.h>
#include <QApplication>
#include <QPixmap>
#include <QTimer>
#include "UI/dbusadapter.h"
#include "UI/mainwidget.h"
#include "UI/globalsignal.h"
#include "playsongarea.h"
#include "UI/base/widgetstyle.h"
#include "UI/base/xatom-helper.h"
#include "UIControl/base/musicDataBase.h"
#include <KWindowEffects>
#include <QMessageBox>
#include "ukuistylehelper/ukuistylehelper.h"

#define VOLUME 5
#define PT_9 9

PlaySongArea::PlaySongArea(QWidget *parent) : QWidget(parent)
{
    resize(750,88);
    setAttribute(Qt::WA_StyledBackground,true);
    this->setObjectName("PlaySongArea");
    initWidget();
    initConnect();
    initStyle();
    m_lyricsDialog = new LyricsDialog();
    m_lyricsDialog->setFixedSize(950, 96);
}

PlaySongArea::~PlaySongArea()
{
/*
    if (m_volSliderWid) {
        m_volSliderWid->deleteLater();
    }#include <KPushButton>
*/
}

void PlaySongArea::initWidget()
{
    preBtn = new kdk::KPushButton(this);
    preBtn->setAccessibleName("kylin-music_QPushButton_preBtn");
    preBtn->setIcon(QIcon::fromTheme("media-skip-backward-symbolic"));
    preBtn->setIconHighlight(false);
    preBtn->setFixedSize(38, 38);
    preBtn->setIconSize(QSize(24, 24));
    preBtn->setToolTip(tr("Previous Ctrl+Left"));
    preBtn->setTranslucent(true);
    preBtn->setBackgroundColorHighlight(false);
    preBtn->setBackgroundColor(Qt::white);
    preBtn->setIconColor("#1d1d1d");
    preBtn->installEventFilter(this);

    playBtn = new kdk::KPushButton(this);
    playBtn->setAccessibleName("kylin-music_QPushButton_playBtn");
    playBtn->setIcon(QIcon::fromTheme("ukui-play-full-symbolic"));
    playBtn->setIconHighlight(false);
    playBtn->setIconColor(Qt::white);
    playBtn->setButtonType(kdk::KPushButton::CircleType);
    playBtn->setFixedSize(36, 36);
    playBtn->setIconSize(QSize(24, 24));
    playBtn->setToolTip(tr("Play Space"));
    playBtn->setTranslucent(false);
    playBtn->setBackgroundColorHighlight(true);

    nextBtn = new kdk::KPushButton(this);
    nextBtn->setAccessibleName("kylin-kylin-music_QPushButton_nextBtn");
    nextBtn->setIcon(QIcon::fromTheme("media-skip-forward-symbolic"));
    nextBtn->setIconHighlight(false);
    nextBtn->setFixedSize(36, 36);
    nextBtn->setIconSize(QSize(24, 24));
    nextBtn->setToolTip(tr("Next Ctrl+Right"));
    nextBtn->setTranslucent(true);
    nextBtn->setBackgroundColorHighlight(false);
    nextBtn->setBackgroundColor(Qt::white);
    nextBtn->setIconColor("#1d1d1d");
    nextBtn->installEventFilter(this);

    volumeBtn = new QPushButton;
    volumeBtn->setAccessibleName("kylin-music_QPushButton_volumeBtn");
    volumeBtn->installEventFilter(this);
    volumeBtn->setFixedSize(36, 36);
    volumeBtn->setToolTip(tr("Volume"));    //音量
    int volume = playController::getInstance().getVolume();

    if(volume == 0) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    } else if(volume > 0 && volume <= 33) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-low-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    } else if(volume > 33 && volume <= 77) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-medium-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    } else {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-high-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    }

/*
    m_volSliderWid = new SliderWidget(); // 不用指定this，指定后音量控件不显示

    MotifWmHints hint;
    hint.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hint.functions = MWM_FUNC_ALL;
    hint.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(m_volSliderWid->winId(), hint);

    moveVolSliderWid();
    m_volSliderWid->hide();
*/

    m_playBackModeWid = new PlayBackModeWidget(this);
    m_playBackModeWid->setAccessibleName("kylin-music_playsongarea_PlayBackModeWidget_m_playBackModeWid");
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(m_playBackModeWid->winId(), hints);
    m_playBackModeWid->hide();

    hSlider = new MusicSlider(this);
    hSlider->setAccessibleName("kylin-music_playsongarea_MusicSlider_hSlider");
    hSlider->setPosition(0);
    hSlider->setDisabled(true);

    // 收藏
    favBtn = new QPushButton;
    favBtn->setAccessibleName("kylin-music_QPushButton_favBtn");
    favBtn->installEventFilter(this);
    favBtn->setFixedSize(36, 36);
    favBtn->setFocusPolicy(Qt::NoFocus);
    favBtn->setToolTip(tr("Favourite"));
    favBtn->setProperty("isWindowButton", 0x1);
    favBtn->setProperty("useIconHighlightEffect", 0x2);
    favBtn->setFlat(true);

    // 播放模式
    m_orderBtn = new QPushButton;
    m_orderBtn->setAccessibleName("kylin-music_QPushButton_m_orderBtn");
    m_orderBtn->installEventFilter(this);
    m_orderBtn->setFixedSize(36, 36);

    switch (playController::getInstance().mode()) {
    case 1:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
        m_orderBtn->setProperty("isWindowButton", 0x1);
        m_orderBtn->setProperty("useIconHighlightEffect", 0x2);
        m_orderBtn->setFlat(true);
        m_orderBtn->setToolTip(tr("CurrentItemInLoop"));
        playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
        break;
    case 3:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
        m_orderBtn->setProperty("isWindowButton", 0x1);
        m_orderBtn->setProperty("useIconHighlightEffect", 0x2);
        m_orderBtn->setFlat(true);
        m_orderBtn->setToolTip(tr("Loop"));
        playController::getInstance().setPlaymode(playController::Loop);
        break;
    case 4:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
        m_orderBtn->setProperty("isWindowButton", 0x1);
        m_orderBtn->setProperty("useIconHighlightEffect", 0x2);
        m_orderBtn->setFlat(true);
        m_orderBtn->setToolTip(tr("Random"));
        playController::getInstance().setPlaymode(playController::Random);
        break;
    default:
        break;
    }

    //歌词
    m_lrcBtn = new QPushButton;
    m_lrcBtn->setAccessibleName("kylin-music_QPushButton_m_lrcBtn");
    m_lrcBtn->installEventFilter(this);
    m_lrcBtn->setFixedSize(36, 36);
    m_lrcBtn->setFocusPolicy(Qt::NoFocus);
    QFont font1;
    font1.setPointSizeF(10);
    m_lrcBtn->setFont(font1);
    QFontMetrics fm = m_lrcBtn->fontMetrics();
    QString showText = fm.elidedText(tr("Lyric"), Qt::ElideRight, 26);
    m_lrcBtn->setText(showText);
    m_lrcBtn->setToolTip(tr("Lyric"));

    m_lrcBtn->setProperty("isWindowButton", 0x1);
    m_lrcBtn->setProperty("useIconHighlightEffect", 0x2);
    m_lrcBtn->setFlat(true);


    //历史播放列表
    listBtn = new QPushButton;
    listBtn->setAccessibleName("kylin-music_QPushButton_listBtn");
    listBtn->installEventFilter(this);
    listBtn->setFixedSize(36, 36);
//    listBtn->setCheckable(true); //按钮是否是可点击状态，默认不点击 fix: #114350, 历史播放列表图标点击弹出列表后是没有焦点区域
    listBtn->setChecked(false);    //只可检查按钮是否是点击状态，保存点击的状态
    listBtn->setIcon(QIcon::fromTheme("ukui-play-list-symbolic"));
    listBtn->setProperty("isWindowButton", 0x1);
    listBtn->setProperty("useIconHighlightEffect", 0x2);
    listBtn->setFlat(true);
    listBtn->setToolTip(tr("Play List"));

    coverPhotoBtn = new CoverLabel(this);
    coverPhotoBtn->setAccessibleName("kylin-music_CoverLabel_coverPhotoBtn");
    coverPhotoBtn->setFixedSize(40,40);

    playingLabel = new CustomLabel(this);
    playingLabel->setAccessibleName("kylin-music_playsongarea_CustomLabel_playingLabel");
    playingLabel->setFixedHeight(28);
//    playingLabel->setText(tr("Music Player"));

    timeLabel = new QLabel(this);
    timeLabel->setAccessibleName("kylin-music_QLabel_timeLabel");

    QString playPath = playController::getInstance().getPath();
    if(playPath != "") {
        slotSongInfo(playPath);
        slotPositionChanged(0);
    } else {
        playingLabel->setText(tr("Music Player"));
        timeLabel->setText(tr("00:00/00:00"));
    }

    QWidget *letfWid = new QWidget(this);
    letfWid->setAccessibleName("kylin-music_playsongarea_QWidget_letfWid");
    QWidget *vWidget = new QWidget(letfWid);
    vWidget->setAccessibleName("kylin-music_playsongarea_QWidget_vWidget");
    vWidget->setFixedHeight(44);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addStretch();
    vLayout->addWidget(playingLabel,0,Qt::AlignVCenter);
    vLayout->addWidget(timeLabel,0,Qt::AlignVCenter);
    vLayout->addStretch();
    vLayout->setContentsMargins(8,3,0,3);
    vLayout->setMargin(0);
    vWidget->setLayout(vLayout);

    QHBoxLayout *leftLayout = new QHBoxLayout(letfWid);
    leftLayout->addWidget(coverPhotoBtn,Qt::AlignVCenter);
    leftLayout->addWidget(vWidget,Qt::AlignVCenter);
    leftLayout->setMargin(0);

    QWidget *centreWid = new QWidget(this);
    centreWid->setAccessibleName("kylin-music_playsongarea_QWidget_centreWid");
    QHBoxLayout *playLayout = new QHBoxLayout(centreWid);
    playLayout->addStretch(0);
    playLayout->addWidget(preBtn,Qt::AlignHCenter);
    playLayout->addSpacing(12);
    playLayout->addWidget(playBtn,Qt::AlignHCenter);
    playLayout->addSpacing(12);
    playLayout->addWidget(nextBtn,Qt::AlignHCenter);
    playLayout->addStretch();
    playLayout->setMargin(0);

    QWidget *rightWid = new QWidget(this);
    rightWid->setAccessibleName("kylin-music_playsongarea_QWidget_rightWid");
    QHBoxLayout *rightLayout = new QHBoxLayout(rightWid);
    rightWid->setLayout(rightLayout);
    rightLayout->setSpacing(0);
    rightLayout->addStretch(0);
    rightLayout->addWidget(volumeBtn,Qt::AlignRight);
    rightLayout->addSpacing(2);
    rightLayout->addWidget(favBtn,Qt::AlignRight);
    rightLayout->addSpacing(2);
    rightLayout->addWidget(m_orderBtn,Qt::AlignRight);
    rightLayout->addSpacing(2);
    rightLayout->addWidget(m_lrcBtn,Qt::AlignRight);
    rightLayout->addSpacing(2);
    rightLayout->addWidget(listBtn,Qt::AlignRight);
    rightLayout->setContentsMargins(0,14,30,14);
    rightLayout->setMargin(0);

    letfWid->setLayout(leftLayout);
    centreWid->setLayout(playLayout);

    m_hmainLayout = new QHBoxLayout();
    m_hmainLayout->addWidget(hSlider);
    m_hmainLayout->setMargin(0);
    m_hmainLayout->setSpacing(0);

    m_mainLayout = new QHBoxLayout();
    m_mainLayout->addWidget(letfWid);
    m_mainLayout->addWidget(centreWid);
    m_mainLayout->addWidget(rightWid);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setStretchFactor(letfWid,1);
    m_mainLayout->setStretchFactor(centreWid,1);
    m_mainLayout->setStretchFactor(rightWid,1);
    m_mainLayout->setContentsMargins(25,0,25,0);

    m_vmainLayout = new QVBoxLayout();
    m_vmainLayout->addLayout(m_hmainLayout);
    m_vmainLayout->addLayout(m_mainLayout);
    m_vmainLayout->setContentsMargins(0,0,0,8);
    m_vmainLayout->setSpacing(0);
    this->setLayout(m_vmainLayout);
}

void PlaySongArea::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_9;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    timeLabel->setFont(font);
}

void PlaySongArea::updatePlayArea()
{
    QTimer::singleShot(100, [&](){
        // qDebug() << "will update1";
        this->update();
    });

    QTimer::singleShot(100, [&](){
        // qDebug() << "will update2";
        this->update();
    });
}

/**
 * @brief PlaySongArea::switchPCAndTabletMode 切换PC和平板模式
 *
 * 平板模式：需要处理滑动条的控件问题,主要是parent设置
 *
 * @param isTablet 当前模式是否是平板
 */
void PlaySongArea::switchPCAndTabletMode(bool isTablet)
{
    qDebug() << "isTablet: " << isTablet;
}

void PlaySongArea::initConnect()
{
    connect(preBtn,&kdk::KPushButton::clicked,this,&PlaySongArea::slotPrevious);
    connect(playBtn,&kdk::KPushButton::clicked,this,&PlaySongArea::slotPlayClicked);
    connect(nextBtn,&kdk::KPushButton::clicked,this,&PlaySongArea::slotNext);
    connect(listBtn,&QPushButton::pressed,this,&PlaySongArea::listBtnClicked); // 播放历史按钮
    connect(listBtn, &QPushButton::toggled, this, [&](){
        QTimer::singleShot(200, [&](){
            // qDebug() << "will update1";
            this->update();
        });
    });
    connect(volumeBtn,&QPushButton::clicked,this,&PlaySongArea::slotVolSliderWidget);
    connect(volumeBtn, &QPushButton::released, this, [&](){
        QTimer::singleShot(200, [&](){
            // qDebug() << "will update2";
            this->update();
        });
    });
    connect(favBtn,&QPushButton::clicked,this,&PlaySongArea::slotFav);
    connect(m_lrcBtn,&QPushButton::clicked,this,&PlaySongArea::slotShowLyricDlg);

    connect(m_orderBtn,&QPushButton::clicked,this,&PlaySongArea::slotPlayModeClicked);
    connect(m_orderBtn, &QToolButton::released, this, [&](){
        QTimer::singleShot(200, [&](){
            // qDebug() << "will update3";
            this->update();
        });
    });
    connect(&playController::getInstance(),&playController::singalChangePath,this,&PlaySongArea::slotSongInfo);
    connect(&playController::getInstance(),&playController::playerStateChange,this,&PlaySongArea::playerStateChange);
    connect(playController::getInstance().getMpvCorePointer(),SIGNAL(positionChanged(qint64)),this,SLOT(slotPositionChanged(qint64)));
    connect(playController::getInstance().getMpvCorePointer(),SIGNAL(notifyDurationChanged(qint64)),this,SLOT(slotDurationChanged(qint64)));
    connect(hSlider,SIGNAL(sliderPressed()),this,SLOT(slotSlidePressd()));
    connect(hSlider,SIGNAL(sliderReleased()),this,SLOT(slotSlideReleased()));
    connect(hSlider,&MusicSlider::valueChanged,this,&PlaySongArea::setPosition);
    connect(&playController::getInstance(),&playController::signalPlayMode,this,&PlaySongArea::setPlayMode);
    connect(&playController::getInstance(),&playController::signalNotPlaying,this,&PlaySongArea::slotNotPlaying);
    connect(g_user_signal, &GlobalUserSignal::initPlaySongArea       , this, &PlaySongArea::slotNotPlaying);
    connect(&playController::getInstance(),&playController::signalSetValue,this,&PlaySongArea::slotSetValue);
    // connect(&playController::getInstance(),&playController::signalVolume,this,&PlaySongArea::slotVolume);
    connect(&playController::getInstance(),&playController::signalMute,this,&PlaySongArea::slotMute);
    //connect(g_user_signal, &GlobalUserSignal::sigSwitchTabletMode, this, &PlaySongArea::switchPCAndTabletMode);
    connect(coverPhotoBtn, &CoverLabel::clicked, this, &PlaySongArea::coverBtnClicked);
}

void PlaySongArea::slotVolumeChanged(int values)
{
    playController::getInstance().setVolume(values);
    int volume = playController::getInstance().getVolume();
    KyInfo() << "volume = " << volume;

    if(volume == 0) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
    } else if(volume > 0 && volume <= 33) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-low-symbolic"));
    } else if(volume > 33 && volume <= 77) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-medium-symbolic"));
    } else {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-high-symbolic"));
    }
}

/*
void PlaySongArea::volumeIncrease()
{
    int value = m_volSliderWid->vSlider->value() + VOLUME;

    if(value > 100)
        value = 100;
    m_volSliderWid->vSlider->setValue(value);
}

void PlaySongArea::volumeReduce()
{
    int value = m_volSliderWid->vSlider->value() - VOLUME;

    if(value < 0) {
        value = 0;
    }

    m_volSliderWid->vSlider->setValue(value);
}
*/

void PlaySongArea::slotVolSliderWidget()
{
    Q_EMIT showVolumeBtnClicked();
}

void PlaySongArea::slotMute(bool mute)
{
    if (mute) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
    }
}

void PlaySongArea::slotText(QString btnText)
{
    listName = btnText;
}

void PlaySongArea::slotFav()
{
    QString playListName = playController::getInstance().getPlayListName();
    if(g_db->checkSongIsInFav(filePath)) {
        QList<musicDataStruct> resList;
        int ref = g_db->getSongInfoListFromDB(resList, FAV);
        int ret = g_db->delMusicFromPlayList(filePath,FAV);

        if(ref == DB_OP_SUCC && ret == DB_OP_SUCC) {
            for(int i = 0; i < resList.size(); i++) {
                if(resList.at(i).filepath == filePath) {
                    playController::getInstance().removeSongFromCurList(FAV, i);

                    if(listName == tr("I Love")){
                        //若清空的是列表中最后一首歌曲，则filePath置空
                        if(playListName == tr("I Love") && resList.size() == 1){
                            filePath.clear();
                        }
                        //刷新我喜欢界面
                        Q_EMIT signalRefreshFav(FAV);
                    }
                    break;
                }
            }
        }
    } else {
        int ref = g_db->addMusicToPlayList(filePath,FAV);

        if(ref == DB_OP_SUCC) {
            playController::getInstance().addSongToCurList(FAV, filePath);
            if(listName == tr("I Love")){
                Q_EMIT signalRefreshFav(FAV);
            }
        }
    }

    slotFavExixts();
}

void PlaySongArea::lyricsWidHide()
{
    qDebug() << "收起歌词界面";
    if (m_lyricsWid != nullptr) {
        m_lyricsWid->close();
        delete m_lyricsWid;  // 确保释放资源
        m_lyricsWid = nullptr;  // 避免悬空指针
    }
}
void PlaySongArea::slotShowLyricDlg()
{
    if (is_lrcBtnCheck == false) {
        is_lrcBtnCheck = true;

        // 如果 m_lyricsDialog 已经存在，则不需要重新创建
        if (m_lyricsDialog == nullptr) {
            m_lyricsDialog = new LyricsDialog();
            m_lyricsDialog->setFixedSize(950, 96);
        }

        // 获取 PlaySongArea 在屏幕上的全局位置
        QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
        m_lyricsDialog->move((screenGeometry.width()- m_lyricsDialog->width()) / 2, screenGeometry.height() - m_lyricsDialog->height() - 20);
        m_lyricsDialog->show();

        QString playPath = playController::getInstance().getPath();
        m_lyricsDialog->loadLyricsForSongDlg(playPath);

        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE){
            qDebug() << "点出歌词之前就已经开始播放了";
            qint64 movePos = playController::getInstance().getMpvCorePointer()->position();
            m_lyricsDialog->onPlayButtonClicked(movePos);
        }
        connect(m_lyricsDialog, &LyricsDialog::dlgCLoseSig, [this]() {
            is_lrcBtnCheck = false;
            QTimer::singleShot(0, this, [this]() {
                closeLyricsDialog();
            });
        });
    } else {
        is_lrcBtnCheck = false;
        closeLyricsDialog();
    }
    connect(&playController::getInstance(),&playController::singalChangePath,m_lyricsDialog,&LyricsDialog::loadLyricsForSongDlg);
    connect(this, &PlaySongArea::dlgisPlaying, m_lyricsDialog, &LyricsDialog::onPlayButtonClicked);
    connect(this, &PlaySongArea::dlgisStoping, m_lyricsDialog, &LyricsDialog::onStopButtonClicked);
    connect(playController::getInstance().getMpvCorePointer(),&MpvCore::widpositionChanged,m_lyricsDialog, &LyricsDialog::onPlayButtonClicked);
}

void PlaySongArea::closeLyricsDialog()
{
    if (m_lyricsDialog != nullptr) {
        m_lyricsDialog->close();
        delete m_lyricsDialog;  // 确保释放资源
        m_lyricsDialog = nullptr;  // 避免悬空指针
    }
}

/**
 * @brief PlaySongArea::listBtnClicked 切换歌词大屏界面
 */
void PlaySongArea::coverBtnClicked()
{
    qDebug() << "发出显示全屏歌词界面信号";
    Q_EMIT showLyricsClicked();
}

void PlaySongArea::slotPlayModeClicked()
{
    int playMode = playController::getInstance().playmode();
    switch (playMode)
    {
    case 1:
        ++playMode;
        ++playMode;
        break;
    case 3:
        ++playMode;
        break;
    case 4:
        playMode = 1;
    default:
        break;
    }

    setPlayMode(playMode);
    playController::getInstance().setMode(static_cast<playController::PlayMode>(playMode));
}


void PlaySongArea::setPlayMode(int playModel)
{
    switch (playModel) {
    case 1:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
        m_orderBtn->setToolTip(tr("CurrentItemInLoop"));
        playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
        break;
    case 3:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
        m_orderBtn->setToolTip(tr("Loop"));
        playController::getInstance().setPlaymode(playController::Loop);
        break;
    case 4:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
        m_orderBtn->setToolTip(tr("Random"));
        playController::getInstance().setPlaymode(playController::Random);
        break;
    default:
        break;
    }

    playController::getInstance().setMode(static_cast<playController::PlayMode>(playModel));
}

/**
 * @brief PlaySongArea::slotSongInfo 从音频文件绝对路径所在数据库获取音频信息
 * @param path
 */
void PlaySongArea::slotSongInfo(QString path)
{
    musicDataStruct musicStruct;
    if(!path.isEmpty()) {
        filePath = path.remove("file://");
        g_db->getSongInfoFromDB(filePath, musicStruct);
    }

    //使用库解析总时间，但有时也会解析失败，特别是针对ftp服务器的ogg音视频文件
    m_time = musicStruct.time;

    if(musicStruct.title == "") {
        playingLabel->setText(tr("Music Player"));
        coverPhotoBtn->setPixmap(QIcon(":/img/fengmian.png").pixmap(QSize(40,40)));
        Q_EMIT signalPlayingLab(tr("Music Player"));
    } else {
        hSlider->isPlaying(true);
        playingLabel->setText(musicStruct.title);
        QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filePath);
        setCoverPhotoPixmap(pix);
        Q_EMIT signalPlayingLab(musicStruct.title);
    }

    slotFavExixts();
}

/**
 * @brief PlaySongArea::setCoverPhotoPixmapRadius 修改图片，形成圆角
 * @param pixmap
 * @return
 */
QPixmap PlaySongArea::setCoverPhotoPixmapRadius(QPixmap pixmap)
{
    int width = coverPhotoBtn->width();
    int height = coverPhotoBtn->height();
    QSize size(width, height);

    QBitmap mask(size);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRoundedRect(0, 0, size.width(), size.height(), 10, 10); //修改这个值，可以改弧度，和直径相等就是圆形

    QPixmap image = pixmap.scaled(size);
    image.setMask(mask);

    return image;
}

void PlaySongArea::setCoverPhotoPixmap(QPixmap pixmap)
{
    if(pixmap.isNull()) {
        pixmap = QIcon(":/img/fengmian.png").pixmap(QSize(40,40));
    } else {
        pixmap = setCoverPhotoPixmapRadius(pixmap);
        pixmap = pixmap.scaled(QSize(40,40),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    coverPhotoBtn->setPixmap(pixmap);
}


/**
 * @brief PlaySongArea::playerStateChange 切换播放、暂停按钮样式
 * @param newState
 */
void PlaySongArea::playerStateChange(playController::PlayState newState)
{
    if(newState == playController::PlayState::PLAY_STATE) {
        playBtn->setIcon(QIcon::fromTheme("media-playback-pause-symbolic"));
        playBtn->setIconColor(Qt::white);
        playBtn->setToolTip(tr("Pause Space"));
    }else if((newState == playController::PlayState::PAUSED_STATE)
        || (newState == playController::PlayState::STOP_STATE)) {
        playBtn->setIcon(QIcon::fromTheme("ukui-play-full-symbolic"));
        playBtn->setIconColor(Qt::white);
        playBtn->setToolTip(tr("Play Space"));
    }
}

/**
 * @brief PlaySongArea::slotPositionChanged 播放进度改变
 *
 * 随着播放进度的改变，同时也会更新界面的音频总时间，因此总时长m_timer需要实时更改成最长的时间
 * @param position 播放进度位置
 */
void PlaySongArea::slotPositionChanged(qint64 position)
{
    m_nowtime = position;
//    qDebug() << "当前播放进度： " << position; // 151076, 151, 1500996
    QString str_time;
    int pos = position / 1000;
    int hour = static_cast<int>(pos / 3600);
    int minutes = static_cast<int>(pos % 3600 / 60);
    int seconds = static_cast<int>(pos % 60);

    QTime duration(hour, minutes, seconds);
    QStringList s = m_time.split(":");

    if(s.size() == 3) {
        str_time = duration.toString("hh:mm:ss");
    } else {
        str_time = duration.toString("mm:ss");
    }

//    qDebug() << "m_time: " << m_time;

    QString length = str_time + "/" + m_time;

    if(m_time == "") {
        timeLabel->setText("00:00/00:00");
        Q_EMIT signalTimeLab("00:00/00:00");
    } else {
        timeLabel->setText(length);
        Q_EMIT signalTimeLab(length);
    }

    if(hSlider->signalsBlocked()) {
        return;
    }

    hSlider->blockSignals(true);
    hSlider->setPosition(static_cast<int>(position));

    hSlider->blockSignals(false);
    hSlider->update();
}

void PlaySongArea::slotNotPlaying()
{
    //滑块可以点击，无法跳转
    hSlider->isPlaying(false);
    //禁用
    hSlider->setDisabled(true);
    hSlider->setPosition(0);
    playingLabel->setText(tr("Music Player"));
    timeLabel->setText("00:00/00:00");
}

void PlaySongArea::slotSetValue()
{
    hSlider->setPosition(0);
}

/**
 * @brief PlaySongArea::slotDurationChanged 播放总时长改变
 * @param duration mpvcore获取的总时长
 */
void PlaySongArea::slotDurationChanged(qint64 duration)
{
    qDebug() << "111 duration: " << duration;

    QString str_time;
    int pos = duration / 1000;
    int hour = static_cast<int>(pos / 3600);
    int minutes = static_cast<int>(pos % 3600 / 60);
    int seconds = static_cast<int>(pos % 60);

    QTime durationTime(hour, minutes, seconds);

    if (hour > 0) {
        str_time = durationTime.toString("hh:mm:ss");
    } else {
        str_time = durationTime.toString("mm:ss");
    }
    m_time = str_time;

    hSlider->setRange(0,static_cast<int>(duration));
    hSlider->setEnabled(duration>0);
    hSlider->setPageStep(static_cast<int>(duration)/10);
}

void PlaySongArea::slotSlidePressd()
{
    playController::getInstance().getMpvCorePointer()->pause();
}

void PlaySongArea::slotSlideReleased()
{
    playController::getInstance().getMpvCorePointer()->play();
}

void PlaySongArea::setPosition(int position)
{
//    qDebug() << "position" << position;
//    qDebug() << "playController::getInstance().getMpvCorePointer()->position()" << playController::getInstance().getMpvCorePointer()->position();
//    qDebug() << "playController::getInstance().getMpvCorePointer()->position() - position" << playController::getInstance().getMpvCorePointer()->position() - position;
    //判断播放的位置 - 滑块的位置是否大于0.1s
    if (qAbs(playController::getInstance().getMpvCorePointer()->position() - position) > 99) {
       playController::getInstance().getMpvCorePointer()->setPosition(position);
    }
}

void PlaySongArea::resizeEvent(QResizeEvent *event)
{
    // moveVolSliderWid();
    playingLabel->setFixedWidth(this->width()/3.6);
    QWidget::resizeEvent(event);
}
/*
*处理上一首／下一首鼠标悬停事件
*/
void PlaySongArea::updateThemeColor(QObject *obj, QEvent *event){

    if (event->type() == QEvent::HoverMove){ //鼠标悬停
        if (obj == nextBtn){
            nextBtn->setIconHighlight(true);
        }else if (obj == preBtn){
            preBtn->setIconHighlight(true);
        }
    }else if (event->type() == QEvent::HoverLeave){ //鼠标离开
        if (obj == nextBtn){
            if(WidgetStyle::themeColor == 1) {
                nextBtn->setIconHighlight(false);
                nextBtn->setTranslucent(true);
                nextBtn->setBackgroundColorHighlight(false);
                nextBtn->setBackgroundColor("#1d1d1d");
                nextBtn->setIconColor(Qt::white);

            } else if(WidgetStyle::themeColor == 0) {
                nextBtn->setIconHighlight(false);
                nextBtn->setIconColor("#1d1d1d");
                nextBtn->setBackgroundColor(Qt::white);
            }
        }else if (obj == preBtn){
            if(WidgetStyle::themeColor == 1) {
                preBtn->setIconHighlight(false);
                preBtn->setIconColor(Qt::white);
                preBtn->setBackgroundColor("#1d1d1d");
            } else if(WidgetStyle::themeColor == 0) {
                preBtn->setIconHighlight(false);
                preBtn->setIconColor("#1d1d1d");
                preBtn->setBackgroundColor(Qt::white);
            }
        }
    }
}

bool PlaySongArea::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == preBtn || obj == nextBtn){
        updateThemeColor(obj, event);
        return false;
    }
    if (event->type() == QEvent::HoverLeave) {
        if (obj == volumeBtn || obj == listBtn || obj ==  favBtn || obj == m_orderBtn) { // TODO: all button
            // QTimer::singleShot(100, [&](){this->update();});
            this->updatePlayArea();
        }
        return false;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void PlaySongArea::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    this->updatePlayArea();
}

void PlaySongArea::slotPlayClicked()
{

    KyInfo() << QString("%1 %2")
                .arg("slotPlayCliked: ")
                .arg(playController::getInstance().getMpvCorePointer()->state());

    if(playController::getInstance().getMpvCorePointer()->state() == MpvCore::PlayingState) {

        playController::getInstance().getMpvCorePointer()->pause();
        Q_EMIT dlgisStoping();
    }
    else if(playController::getInstance().getMpvCorePointer()->state() == MpvCore::PausedState) {
 //       if (qAbs(playController::getInstance().getMpvCorePointer()->position()) > 99) {

        playController::getInstance().getMpvCorePointer()->play();
        Q_EMIT dlgisPlaying(m_nowtime);
        int currentVolume = playController::getInstance().getVolume();
        KyInfo() << "currentVolume = " << currentVolume
                 << "playState = " << playController::getInstance().getMpvCorePointer()->state();

        delayMsecond(100);
        playController::getInstance().setVolume(currentVolume);
//        }
//        else{
//            playMeta();

//            int currentVolume = playController::getInstance().getVolume();
//            KyInfo() << "currentVolume = " << currentVolume
//                     << "playState = " << playController::getInstance().getMpvCorePointer()->state();

//            delayMsecond(100);
//            playController::getInstance().setVolume(currentVolume);
//        }
    } else if(playController::getInstance().getMpvCorePointer()->state() == MpvCore::StoppedState) {
        playMeta();
        Q_EMIT dlgisPlaying(m_nowtime);
        int currentVolume = playController::getInstance().getVolume();
        KyInfo() << "currentVolume = " << currentVolume
                 << "playState = " << playController::getInstance().getMpvCorePointer()->state();

        delayMsecond(100);
        playController::getInstance().setVolume(currentVolume);
    }


}

void PlaySongArea::playMeta()
{
    QString playListName = playController::getInstance().getPlayListName();
    QString playPath = playController::getInstance().getPath();
    QList<musicDataStruct> resList;
    QStringList filePaths;

    int ret = g_db->getSongInfoListFromDB(resList, playListName);

    if(playPath != "") {
        if(ret == DB_OP_SUCC) {
            if(resList.size() == 0) {
                return;
            }

            int index;

            for(int i = 0; i < resList.size(); i++) {
                filePaths << resList.at(i).filepath;

                if(resList.at(i).filepath == playPath) {
                    index = i;
                }
            }

            playController::getInstance().setCurPlaylist(playListName, filePaths);
            playController::getInstance().play(playListName, index);
        }
    } else {
        QList<musicDataStruct> resMusicList;
        int ref = g_db->getSongInfoListFromDB(resMusicList,ALLMUSIC);

        if(ref == DB_OP_SUCC) {
            if(resMusicList.size() == 0) {
                return;
            }

            if(resMusicList.size() >= 1) {
                QStringList paths;

                for(int i = 0; i < resMusicList.size(); i++) {
                    paths << resMusicList.at(i).filepath;
                }

                playController::getInstance().setCurPlaylist(ALLMUSIC,paths);
                playController::getInstance().play(ALLMUSIC,0);
            }
        }
    }
}

void PlaySongArea::slotPrevious()
{
    QString playPath = playController::getInstance().getPath();
    if(playPath != "")
        playController::getInstance().onPreviousSong();
}

void PlaySongArea::delayMsecond(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void PlaySongArea::slotNext()
{
    QString playPath = playController::getInstance().getPath();
    if(playPath != "")
        playController::getInstance().onNextSong();
}


/**
 * @brief PlaySongArea::slotSetThemeColor 设置按钮图标跟随主题变化
 *
 * 设置上一首、下一首图标跟随系统面板的高亮色主题变化，由于主题sdk包影响，各种色值的icon需要和设计沟通得到，
 * 而不能根据一张svg图片自行加载切换不同颜色的图片。
 *
 * @param themeHighlightColor 不同的主题高亮色颜色名称
 */
void PlaySongArea::slotSetThemeColor()
{
    if(WidgetStyle::themeColor == 1) {
        nextBtn->setIconHighlight(false);
        nextBtn->setBackgroundColor("#1d1d1d");
        nextBtn->setIconColor(Qt::white);

        preBtn->setIconHighlight(false);
        preBtn->setIconColor(Qt::white);
        preBtn->setBackgroundColor("#1d1d1d");
    } else if(WidgetStyle::themeColor == 0) {
        nextBtn->setIconHighlight(false);
        nextBtn->setBackgroundColor(Qt::white);
        nextBtn->setIconColor("#1d1d1d");

        preBtn->setIconHighlight(false);
        preBtn->setIconColor("#1d1d1d");
        preBtn->setBackgroundColor(Qt::white);
    }
}


/**
 * @brief PlaySongArea::listBtnClicked 播放历史按钮点击
 */
void PlaySongArea::listBtnClicked()
{
    Q_EMIT showHistoryListBtnClicked();
}

void PlaySongArea::slotFavExixts()
{
    if (g_db->checkSongIsInFav(filePath)) {
        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        favBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }

    Q_EMIT signalFavBtnChange(filePath);
}

void PlaySongArea::slotFavExixtsDark()
{
    if(g_db->checkSongIsInFav(filePath)) {
        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        favBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }

    Q_EMIT signalFavBtnChange(filePath);
}

void PlaySongArea::slotHistoryBtnChecked(bool checked)
{
    listBtn->setChecked(checked);
}

void PlaySongArea::slotFavIsExixts(QString filePaths)
{
    if(g_db->checkSongIsInFav(filePaths)) {
        if(playingLabel->text() == tr("Music Player")) {
            return;
        }

        //用于判断添加到我喜欢的歌曲是否是当前播放的歌曲， 如果是刷新， 如果不是不做操作
        if(filePath != filePaths) {
            return;
        }

        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        //用于判断添加到我喜欢的歌曲是否是当前播放的歌曲， 如果是刷新， 如果不是不做操作
        if(filePath != filePaths) {
            return;
        }

        favBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

void PlaySongArea::slotFavBtnChange(QString filePath)
{
    if(g_db->checkSongIsInFav(filePath)) {
        if(playingLabel->text() == tr("Music Player")) {
            return;
        }

        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        favBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

void PlaySongArea::initStyle()
{
    playerStateChange(playController::getInstance().getState());

    slotSetThemeColor();

    if(WidgetStyle::themeColor == 1) {
        this->setStyleSheet("#PlaySongArea{background-color:#1d1d1d;}");

        slotFavExixtsDark();

        coverPhotoBtn->setStyleSheet("background:transparent;border-image:url(:/img/fengmian.png);");

        playingLabel->setStyleSheet("color:#F9F9F9;line-height:14px;");

        hSlider->initStyle();
    } else if(WidgetStyle::themeColor == 0) {
        this->setStyleSheet("#PlaySongArea{background-color:#FFFFFF;}");

        slotFavExixts();

        coverPhotoBtn->setStyleSheet("background:transparent;border-image:url(:/img/fengmian.png);");

        playingLabel->setStyleSheet("line-height:14px;color:#303133;");

        timeLabel->setStyleSheet("line-height:12px;color:#8F9399;");

        hSlider->initStyle();
    }
}

void CoverLabel::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        Q_EMIT clicked();
    }
    QLabel::mouseReleaseEvent(event);
}
