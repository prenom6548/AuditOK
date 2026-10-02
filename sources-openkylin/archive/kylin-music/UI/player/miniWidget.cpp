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

#include "miniWidget.h"
#include "UI/base/widgetstyle.h"
#include "UI/mainwidget.h"
#include <QDebug>
#include <kshadowhelper.h>
#include <ukuiwindowhelper.h>
#include <windowmanager/windowmanager.h>
#include <KWindowEffects>
#include <QMessageBox>
#include "ukuistylehelper/ukuistylehelper.h"
#define PT_9 9

MiniWidget::MiniWidget(QWidget *parent) : QFrame(parent)
{
    setFixedSize(328,60);
    this->setObjectName("miniWidget");

    UkuiWindowHelper* m_windowHelper = new UkuiWindowHelper(this);
    auto states = m_windowHelper->defaultState();
    states |= UkuiWindowHelper::State::KeepAbove;
    m_windowHelper->setWindowState(states);
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId &window_id) {
        if (getpid() == kdk::WindowManager::getPid(window_id)) {
            QString m_window_id = window_id.toString();
            if (kdk::WindowManager::getWindowTitle(m_window_id) == "kylin-music mini") {
                m_windowHelper->removeTitleBar();
                UkuiWindowHelper::States defaultState;
                m_windowHelper->setWindowState(defaultState | UkuiWindowHelper::State::KeepAbove);

            }
        }
    });

    m_mouseState = false;
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
//    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);
    kdk::effects::KShadowHelper::self()->setWidget(this);
    this->setAttribute(Qt::WA_TranslucentBackground, true);     //窗体透明
    this->setWindowTitle(tr("Music Player"));

    setMouseTracking(true);
    init_miniWidget();

    this->setLayout(m_HMainLayout);
    initStyle();

    initAction();
    initConnect();
    initFocusStyle();
}

void MiniWidget::initAction()
{
    playModeMenu = new QMenu(tr("Loop"),this);
    playModeMenu->setAccessibleName("kylin-music_QMenu_playModeMenu");
    playMode_Loop_Action = new QAction(QIcon(":/img/default/listloop.png"), tr("Loop"), playModeMenu);
    playMode_Loop_Action->setObjectName("kylin-music_miniwidget_QAction_playMode_Loop_Action");
    playMode_Random_Action = new QAction(QIcon(":/img/default/random.png"), tr("Random"), playModeMenu);
    playMode_Random_Action->setObjectName("kylin-music_miniwidget_QAction_playMode_Random_Action");
    playMode_Sequential_Action = new QAction(QIcon(":/img/default/sequence.png"), tr("Sequence"), playModeMenu);
    playMode_Sequential_Action->setObjectName("kylin-music_miniwidget_QAction_playMode_Sequential_Action");
    playMode_CurrentItemInLoop_Action = new QAction(QIcon(":/img/default/oneloop.png"), tr("CurrentItemInLoop"), playModeMenu);
    playMode_CurrentItemInLoop_Action->setObjectName("kylin-music_miniwidget_QAction_playMode_CurrentItemInLoop_Action");
    playModeMenu->addAction(playMode_Loop_Action);
    playModeMenu->addAction(playMode_Random_Action);
    playModeMenu->addAction(playMode_Sequential_Action);
    playModeMenu->addAction(playMode_CurrentItemInLoop_Action);
    playModeMenu->addMenu(playModeMenu);
}
#ifndef WHITE_TEST
bool MiniWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::HoverMove){ //鼠标悬停
        if (obj == m_nextBtn){
            m_nextBtn->setIconHighlight(true);
        }else if (obj == m_preBtn){
            m_preBtn->setIconHighlight(true);
        }
        return false;
    }else if (event->type() == QEvent::HoverLeave){ //鼠标离开
        if (obj == m_nextBtn){
            if(WidgetStyle::themeColor == 1) {
                m_nextBtn->setIconHighlight(false);
                m_nextBtn->setTranslucent(true);
                m_nextBtn->setBackgroundColorHighlight(false);
                m_nextBtn->setBackgroundColor("#1d1d1d");
                m_nextBtn->setIconColor(Qt::white);

            } else if(WidgetStyle::themeColor == 0) {
                m_nextBtn->setIconHighlight(false);
                m_nextBtn->setIconColor("#1d1d1d");
                m_nextBtn->setBackgroundColor(Qt::white);
            }
        }else if (obj == m_preBtn){
            if(WidgetStyle::themeColor == 1) {
                m_preBtn->setIconHighlight(false);
                m_preBtn->setIconColor(Qt::white);
                m_preBtn->setBackgroundColor("#1d1d1d");
            } else if(WidgetStyle::themeColor == 0) {
                m_preBtn->setIconHighlight(false);
                m_preBtn->setIconColor("#1d1d1d");
                m_preBtn->setBackgroundColor(Qt::white);
            }
        }
        return false;
    }else{
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
void MiniWidget::mousePressEvent(QMouseEvent *event)
{
    m_WindowPos = this->pos();

    if(QRect(0,0,width(),height()).contains(event->pos()) && event->button() == Qt::LeftButton) {
        m_MousePos = event->globalPos();
        m_mouseState = true;
    }
}

void MiniWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseState) {
        move(m_WindowPos + (event->globalPos() - m_MousePos));
    }
}

void MiniWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseState = false;
}

void MiniWidget::enterEvent(QEvent *)
{
    m_songNameLab->setVisible(false);
    m_timeLab->setVisible(false);
    m_palyFrame->setVisible(true);
    coverWid->setVisible(true);
    m_mainFrame->setFixedSize(300,60);
   this->setFixedWidth(328);
}

void MiniWidget::leaveEvent(QEvent *)
{
    m_songNameLab->setVisible(true);
    m_timeLab->setVisible(true);
    m_palyFrame->setVisible(false);
    coverWid->setVisible(false);
   this->setFixedWidth(328);
   m_mainFrame->setFixedSize(328,60);
}
#endif
void MiniWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space) {
        Q_EMIT playPauseKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Left) {
        qDebug() << "ctrl+left";
        Q_EMIT previousPlayKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Right) {
        qDebug() << "ctrl+right";
        Q_EMIT nextPlayKeySignal();
    }

    if (event->key() == Qt::Key_Up) {
        qDebug() << "up";
        Q_EMIT addVolumeKeySignal();
    }

    if (event->key() == Qt::Key_Down) {
        qDebug() << "down";
        Q_EMIT downVolumeKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_M) {
        qDebug() << "ctrl+m";
        Q_EMIT miniCompleteSwitchKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_L) {
        qDebug() << "ctrl+l";
        Q_EMIT loveSongKeySignal();
    } else if(event->key() == Qt::Key_Right) {
        return;
    } else if(event->key() == Qt::Key_Left) {
        return;
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_T){
        qDebug() << "ctrl+t";
        if (m_playbackMode == CurrentItemInLoop){
            Q_EMIT changePlaybackMode(3);
            m_playbackMode = Loop;
        }
        else if (m_playbackMode == Loop){
            Q_EMIT changePlaybackMode(4);
            m_playbackMode = Random;
        }
        else if (m_playbackMode == Random){
            Q_EMIT changePlaybackMode(1);
            m_playbackMode = CurrentItemInLoop;
        }
    }

    QWidget::keyPressEvent(event);
}

void MiniWidget::slotSetThemeColor()
{
    if(WidgetStyle::themeColor == 1) {
        m_nextBtn->setIconHighlight(false);
        m_nextBtn->setBackgroundColor("#1d1d1d");
        m_nextBtn->setIconColor(Qt::white);

        m_preBtn->setIconHighlight(false);
        m_preBtn->setIconColor(Qt::white);
        m_preBtn->setBackgroundColor("#1d1d1d");
    } else if(WidgetStyle::themeColor == 0) {
        m_nextBtn->setIconHighlight(false);
        m_nextBtn->setBackgroundColor(Qt::white);
        m_nextBtn->setIconColor("#1d1d1d");

        m_preBtn->setIconHighlight(false);
        m_preBtn->setIconColor("#1d1d1d");
        m_preBtn->setBackgroundColor(Qt::white);
    }
    setCloseAndRecoveryButtonStyle();
}

void MiniWidget::initStyle()
{
    playerStateChange(playController::getInstance().getState());

     slotSetThemeColor();

    if(WidgetStyle::themeColor == 1) {
        m_mainFrame->setStyleSheet("#mainFrame{border-radius:6px;background-color:#252526;}");
        m_timeLab->setStyleSheet("QLabel{line-height:12px;color:#8F9399;}");
        m_coverLabel->setStyleSheet("border-image:url(:/img/fengmian.png);");

        m_songNameLab->setStyleSheet("color:#F9F9F9;");
        slotFavExixtsDark();
        coverWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.4);\
                                border-top-right-radius:6px;border-bottom-right-radius:6px;\
                                border-top-left-radius:0px;border-bottom-left-radius:0px}");
    } else if(WidgetStyle::themeColor == 0) {
        m_mainFrame->setStyleSheet("#mainFrame{border-radius:6px;background-color:#FFFFFF;}");
        m_timeLab->setStyleSheet("QLabel{line-height:12px;color:#8F9399;}");
        m_coverLabel->setStyleSheet("border-image:url(:/img/fengmian.png);");
        m_songNameLab->setStyleSheet("color:#303133;");
        slotFavExixts();
        coverWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.4);\
                                border-top-right-radius:6px;border-bottom-right-radius:6px;\
                                border-top-left-radius:0px;border-bottom-left-radius:0px}");
    }

    setCloseAndRecoveryButtonStyle();
}

void MiniWidget::init_miniWidget()
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    m_HMainLayout = new QHBoxLayout();
    m_hLayout = new QHBoxLayout();

    m_mainFrame = new QFrame;
    m_mainFrame->setAccessibleName("kylin-music_miniwidget_QFrame_m_mainFrame");
    // m_mainFrame->setMidLineWidth(300);
   m_mainFrame->setFixedSize(300,60);
    m_mainFrame->setObjectName("mainFrame");

    m_hMainLayout = new QHBoxLayout(m_mainFrame);

    m_coverLabel = new QLabel;
    m_coverLabel->setAccessibleName("kylin-music_QLabel_m_coverLabel");
    m_coverLabel->setFixedSize(40,40);

    m_vInfoLayout = new QVBoxLayout;

    m_songNameLab = new CustomLabel;
    m_songNameLab->setAccessibleName("kylin-music_miniwidget_CustomLabel_m_songNameLab");
    m_songNameLab->setFixedSize(170,28);
//    m_songNameLab->setAlignment(Qt::AlignLeft);

    m_timeLab = new QLabel;
    m_timeLab->setAccessibleName("kylin-music_QLabel_m_timeLab");
//    m_timeLab->setAlignment(Qt::AlignLeft);

    QString playPath = playController::getInstance().getPath();
    if(playPath != "")
    {
        songInfo(playPath);
        //根据保存的路径设置我喜欢按钮图标样式
        filePath = playPath;
        slotPositionChanged(0);
    }
    else
    {
        m_songNameLab->setText(tr("Music Player"));
        m_timeLab->setText(tr("00:00/00:00"));
    }

    QWidget *vWidget = new QWidget(this);
    vWidget->setAccessibleName("kylin-music_miniwidget_QWidget_vWidget");
    vWidget->setFixedHeight(44);
    m_vInfoLayout->addStretch();
    m_vInfoLayout->addWidget(m_songNameLab,0,Qt::AlignVCenter);
//    m_vInfoLayout->addSpacing(6);
    m_vInfoLayout->addWidget(m_timeLab,0,Qt::AlignVCenter);
    m_vInfoLayout->addStretch();
    m_vInfoLayout->setContentsMargins(8,3,0,3);
    m_vInfoLayout->setMargin(0);
    vWidget->setLayout(m_vInfoLayout);
//    m_vInfoLayout->setAlignment(Qt::AlignVCenter);

    m_palyFrame = new QFrame;
    m_palyFrame->setAccessibleName("kylin-music_miniwidget_QFrame_m_palyFrame");
    m_palyFrame->setFixedWidth(120);
    m_palyFrame->setVisible(false);

    m_hPlayLayout = new QHBoxLayout(m_palyFrame);

    m_preBtn = new kdk::KPushButton(this);
    m_preBtn->setAccessibleName("kylin-music_QPushButton_preBtn");
    m_preBtn->setIcon(QIcon::fromTheme("media-skip-backward-symbolic"));
    m_preBtn->setIconHighlight(false);
    m_preBtn->setFixedSize(36, 36);
    m_preBtn->setIconSize(QSize(24, 24));
    m_preBtn->setToolTip(tr("Previous Ctrl+Left"));
    m_preBtn->setTranslucent(true);
    m_preBtn->setBackgroundColorHighlight(false);
    m_preBtn->setBackgroundColor(Qt::white);
    m_preBtn->setIconColor("#1d1d1d");
    m_preBtn->installEventFilter(this);

    m_playBtn = new kdk::KPushButton(this);
    m_playBtn->setAccessibleName("kylin-music_QPushButton_playBtn");
    m_playBtn->setIcon(QIcon::fromTheme("ukui-play-full-symbolic"));
    m_playBtn->setIconHighlight(false);
    m_playBtn->setIconColor(Qt::white);
    m_playBtn->setButtonType(kdk::KPushButton::CircleType);
    m_playBtn->setFixedSize(36, 36);
    m_playBtn->setIconSize(QSize(24, 24));
    m_playBtn->setToolTip(tr("Play Space"));
    m_playBtn->setTranslucent(false);
    m_playBtn->setBackgroundColorHighlight(true);


    m_nextBtn = new kdk::KPushButton(this);
    m_nextBtn->setAccessibleName("kylin-kylin-music_QPushButton_nextBtn");
    m_nextBtn->setIcon(QIcon::fromTheme("media-skip-forward-symbolic"));
    m_nextBtn->setIconHighlight(false);
    m_nextBtn->setFixedSize(36, 36);
    m_nextBtn->setIconSize(QSize(24, 24));
    m_nextBtn->setToolTip(tr("Next Ctrl+Right"));
    m_nextBtn->setTranslucent(true);
    m_nextBtn->setBackgroundColorHighlight(false);
    m_nextBtn->setBackgroundColor(Qt::white);
    m_nextBtn->setIconColor("#1d1d1d");
    m_nextBtn->installEventFilter(this);

    m_hPlayLayout->setMargin(0);
    m_hPlayLayout->setSpacing(9);
    m_hPlayLayout->addWidget(m_preBtn);
    m_hPlayLayout->addWidget(m_playBtn);
    m_hPlayLayout->addWidget(m_nextBtn);

    m_loveBtn = new QPushButton;
    m_loveBtn->setAccessibleName("kylin-music_QPushButton_m_loveBtn");
    m_loveBtn->setFixedSize(36, 36);
//    m_loveBtn->setFocusPolicy(Qt::NoFocus);
    m_loveBtn->setProperty("isWindowButton", 0x1);
    m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    m_loveBtn->setToolTip(tr("Favourite"));  //   这里加个标签，根据bug190406
    m_loveBtn->setFlat(true);

    m_orderBtn = new QPushButton;
    m_orderBtn->setAccessibleName("kylin-music_QPushButton_m_orderBtn");
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

    m_vSysLayout = new QVBoxLayout;
    coverWid = new QWidget(this);
    coverWid->setAccessibleName("kylin-music_miniwidget_QWidget_coverWid");
//    coverWid->setAttribute(Qt::WA_TranslucentBackground, true);     //窗体透明

    m_closeBtn = new QPushButton;
    m_closeBtn->setAccessibleName("kylin-music_QPushButton_m_closeBtn");
    m_closeBtn->setFixedSize(30,30);
    m_closeBtn->setIcon(QIcon(":/img/lightshow/close.svg"));
    m_closeBtn->setIconSize(QSize(18, 18));
    m_closeBtn->setToolTip(tr("Close"));

    connect(m_closeBtn,&QPushButton::clicked,this,&MiniWidget::close);

    m_recoveryWinBtn = new QPushButton;
    m_recoveryWinBtn->setAccessibleName("kylin-music_QPushButton_m_recoveryWinBtn");
    m_recoveryWinBtn->setFixedSize(30,30);
    m_recoveryWinBtn->setIcon(QIcon(":/img/lightshow/maximize.svg"));
    m_recoveryWinBtn->setIconSize(QSize(12, 12));
    m_recoveryWinBtn->setToolTip(tr("normal mode"));


    m_vSysLayout->setSpacing(0);
    m_vSysLayout->setMargin(0);
    m_vSysLayout->addWidget(m_closeBtn);
    m_vSysLayout->addWidget(m_recoveryWinBtn);
    coverWid->setLayout(m_vSysLayout);

//    m_hMainLayout->setMargin(10);
    m_hMainLayout->addWidget(m_coverLabel);
    m_hMainLayout->addWidget(vWidget);
    m_hMainLayout->addStretch();
    m_hMainLayout->addWidget(m_palyFrame);
    m_hMainLayout->addStretch();
    m_hMainLayout->addWidget(m_loveBtn);
    m_hMainLayout->addWidget(m_orderBtn);
//    m_hMainLayout->addLayout(m_vSysLayout);

    m_mainFrame->setLayout(m_hMainLayout);

    m_HMainLayout->setMargin(0);
    m_HMainLayout->addWidget(m_mainFrame);
    m_HMainLayout->addWidget(coverWid);
}

void MiniWidget::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_9;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    m_timeLab->setFont(font);
}

//初始化样式
void MiniWidget::initFocusStyle()
{
    //不接受焦点高亮。解决点击后有蓝框的问题
    QList<QPushButton*> list = this->findChildren<QPushButton*>();

    for(QPushButton *btn :list) {
        btn->setFocusPolicy(Qt::NoFocus);
    }
}

void MiniWidget::initConnect()
{
    connect(&playController::getInstance(),&playController::playerStateChange,this,&MiniWidget::playerStateChange);
    connect(&playController::getInstance(),&playController::singalChangePath,this,&MiniWidget::slotSongInfo);
    connect(m_loveBtn,&QPushButton::clicked,this,&MiniWidget::slotFav);
    connect(m_orderBtn,&QPushButton::clicked,this,&MiniWidget::slotPlayModeClicked);
    connect(&playController::getInstance(),&playController::signalPlayMode,this,&MiniWidget::setPlayMode);
    connect(&playController::getInstance(),&playController::signalNotPlaying,this,&MiniWidget::slotNotPlaying);
    connect(g_user_signal, &GlobalUserSignal::initPlaySongArea, this, &MiniWidget::slotNotPlaying);
}

void MiniWidget::playerStateChange(playController::PlayState newState)
{
    if(newState == playController::PlayState::PLAY_STATE) {
        m_playBtn->setIcon(QIcon::fromTheme("media-playback-pause-symbolic"));
        m_playBtn->setIconColor(Qt::white);
        m_playBtn->setToolTip(tr("Pause Space"));
    }else if((newState == playController::PlayState::PAUSED_STATE) ||(newState == playController::PlayState::STOP_STATE)) {
        m_playBtn->setIcon(QIcon::fromTheme("ukui-play-full-symbolic"));
        m_playBtn->setIconColor(Qt::white);
        m_playBtn->setToolTip(tr("Play Space"));
    }
}

void MiniWidget::slotSongInfo(QString path)
{
    filePath = path.remove("file://");
    QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filePath);

    if(pix.isNull()) {
        pix = QIcon(":/img/fengmian.png").pixmap(QSize(40,40));
    } else {
        pix = setCoverPhotoPixmapRadius(pix);
        pix = pix.scaled(QSize(40,40),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    m_coverLabel->setPixmap(pix);
    slotFavExixts();
}

void MiniWidget::slotText(QString btnText)
{
    listName = btnText;
}

void MiniWidget::slotFav()
{
    if(g_db->checkSongIsInFav(filePath)) {
        QList<musicDataStruct> resList;
        int ref = g_db->getSongInfoListFromDB(resList, FAV);
        int ret = g_db->delMusicFromPlayList(filePath,FAV);

        if(ref == DB_OP_SUCC) {
            //根据歌单名title值查询对应歌单列表
//            int ref = g_db->getSongInfoListFromDB(resList, "我喜欢");
            if(ret == DB_OP_SUCC) {
                for(int i = 0; i < resList.size(); i++) {
                    if(resList.at(i).filepath == filePath) {
                        playController::getInstance().removeSongFromCurList(FAV, i);

                        //当前为我喜欢界面才刷新
                        if(listName == tr("I Love")) {
                            Q_EMIT signalRefreshFav(FAV);
                        }

                        break;
                    }
                }
            }
        }
    } else {
        int ref = g_db->addMusicToPlayList(filePath,FAV);

        if(ref == DB_OP_SUCC) {
            playController::getInstance().addSongToCurList(FAV, filePath);

            if(listName == tr("I Love")) {
                Q_EMIT signalRefreshFav(FAV);
            }
        }
    }
    slotFavExixts();
}

void MiniWidget::slotFavExixts()
{
    if (g_db->checkSongIsInFav(filePath)) {
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }

    Q_EMIT signalFavBtnChange(filePath);
}

void MiniWidget::slotFavExixtsDark()
{
    if(g_db->checkSongIsInFav(filePath)) {
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }

    Q_EMIT signalFavBtnChange(filePath);
}

void MiniWidget::slotFavIsExixts(QString filePaths)
{
    if(g_db->checkSongIsInFav(filePaths)) {
        //由于mini歌曲title是播放区传送故判断 ""
        if(m_songNameLab->text() == "") {
            return;
        }

        if(filePath != filePaths) {
            return;
        }

        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        if(filePath != filePaths) {
            return;
        }

        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

/**
 * @brief miniWidget::setCoverPhotoPixmapRadius 修改封面圆角10px
 * @param pixmap
 * @return
 */
QPixmap MiniWidget::setCoverPhotoPixmapRadius(QPixmap pixmap)
{
    int width = m_coverLabel->width();
    int height = m_coverLabel->height();
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

void MiniWidget::slotFavBtnChange(QString filePath)
{
    if(g_db->checkSongIsInFav(filePath)) {
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

void MiniWidget::songInfo(QString path)
{
    QString filepath = path.remove("file://");

    musicDataStruct musicStruct;
    g_db->getSongInfoFromDB(filepath, musicStruct);
    //使用库解析总时间
    m_time = musicStruct.time;

    if(musicStruct.title == "") {
        m_songNameLab->setText(tr("Music Player"));
        m_coverLabel->setPixmap(QIcon(":/img/fengmian.png").pixmap(QSize(40,40)));
    } else {
        m_songNameLab->setText(musicStruct.title);
        QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filepath);

        if(pix.isNull()) {
            pix = QIcon(":/img/fengmian.png").pixmap(QSize(40,40));
        } else {
            pix = setCoverPhotoPixmapRadius(pix);
            pix = pix.scaled(QSize(40,40),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        }

        m_coverLabel->setPixmap(pix);
    }
}

void MiniWidget::slotPositionChanged(qint64 position)
{
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

    QString length = str_time + "/" + m_time;

    if(m_time == "") {
        m_timeLab->setText("00:00/00:00");
    } else {
        m_timeLab->setText(length);
    }
}

/**
 * @brief MiniWidget::setCloseAndRecoveryButtonStyle 设置最小化的关闭和恢复按钮样式
 *
 * 设置还包括了在按钮上的tooltip样式
 *
 * fix #161294, 处理英文模式下，mini窗口最大化按钮和关闭按钮悬浮显示左侧为方角，预期为圆角问题
 * fix #197138, 因为只有关闭和还原两个按钮是单独设置的属性，导致鼠标在切换到别的按钮时tooltip会收到影响 解决方案是将所有按钮都设置成一个属性
 */
void MiniWidget::setCloseAndRecoveryButtonStyle()
{
    if(WidgetStyle::themeColor == 1) {
        // 黑色背景
        m_closeBtn->setStyleSheet("QPushButton{background:transparent;}");
        m_recoveryWinBtn->setStyleSheet("QPushButton{background-color:transparent;}");
    } else {
        m_closeBtn->setStyleSheet("QPushButton{background:transparent;}");
        m_recoveryWinBtn->setStyleSheet("QPushButton{background-color:transparent;}");
    }
}

void MiniWidget::slotPlayingLab(QString playing)
{
    m_songNameLab->setText(playing);
}

void MiniWidget::slotTimeLab(QString time)
{
    m_timeLab->setText(time);
}

void MiniWidget::slotPlayModeClicked()
{
    int playMode = playController::getInstance().playmode();

    switch (playMode) {
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

void MiniWidget::setPlayMode(int playModel)
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

void MiniWidget::slotNotPlaying()
{
    m_songNameLab->setText(tr("Music Player"));
    m_timeLab->setText("00:00/00:00");
}
#ifndef WHITE_TEST
void MiniWidget::songText(QString songName)
{
    QString show_songName;

    if(songName.length() > 10) {
        show_songName = songName.mid(0,9);
        show_songName.append("...");
        m_songNameLab->setText(show_songName);
        m_songNameLab->setToolTip(songName);
    } else {
        show_songName = "";
        m_songNameLab->setText(songName);
        m_songNameLab->setToolTip(songName);
    }
}
#endif
