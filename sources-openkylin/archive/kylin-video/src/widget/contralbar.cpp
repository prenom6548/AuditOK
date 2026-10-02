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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "contralbar.h"
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QSlider>
#include <QToolTip>
#include <QGSettings>
#include <QListWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QPalette>
#include <QFontMetrics>
#include <QDBusVariant>

#include <ukui-log4qt.h>
#include "global/globalsignal.h"
#include "global/global.h"
#include "sliderbar/timeslider.h"
#include "filletwidget.h"
//#include "videopreview.h"
#include "core/dbusadapter.h"

// 无障碍（Accessibility）支持：
// 为悬浮的音量弹窗（FilletWidget）提供自定义 QAccessible 接口，
// 主动把内部的 QSlider（音量条）暴露为子元素，便于 accerciser 抓取。
#include <QAccessible>
#include <QAccessibleWidget>
#include <QApplication>

const QSize VIDEO_PREVIEW_SIZE(192, 108);

namespace {

// 自定义音量弹窗的无障碍包装：暴露内部 QSlider 为子元素
class VolumeWidgetAccessible : public QAccessibleWidget
{
public:
    explicit VolumeWidgetAccessible(FilletWidget *w)
        : QAccessibleWidget(w, QAccessible::Pane) {}

    FilletWidget *volumeWidget() const { return static_cast<FilletWidget*>(object()); }

    int childCount() const override
    {
        if (!volumeWidget()) return 0;
        QSlider *slider = volumeWidget()->findChild<QSlider*>();
        return slider ? 1 : 0;
    }

    QAccessibleInterface *child(int index) const override
    {
        if (index != 0 || !volumeWidget()) return nullptr;
        QSlider *slider = volumeWidget()->findChild<QSlider*>();
        if (!slider) return nullptr;
        return QAccessible::queryAccessibleInterface(slider);
    }

    int indexOfChild(const QAccessibleInterface *c) const override
    {
        if (!c || !volumeWidget()) return -1;
        QSlider *slider = volumeWidget()->findChild<QSlider*>();
        if (!slider) return -1;
        return c->object() == slider ? 0 : -1;
    }
};

// 无障碍工厂：仅为音量弹窗（通过 accessibleName 区分）返回自定义接口
static QAccessibleInterface *contralbarAccessibleFactory(const QString &className, QObject *object)
{
    if (className == QStringLiteral("FilletWidget")) {
        QWidget *w = static_cast<QWidget*>(object);
        if (w && w->accessibleName() == QLatin1String("kylin-video_contralbar_FilletWidget_m_volumeWidget")) {
            return new VolumeWidgetAccessible(static_cast<FilletWidget*>(object));
        }
    }
    // 将弹出的音量滑条作为“音量按钮”的可访问子元素，便于从按钮路径直接展开
    if (className == QStringLiteral("QPushButton")) {
        QWidget *btn = static_cast<QWidget*>(object);
        if (btn && btn->accessibleName() == QLatin1String("kylin-video_QPushButton_m_volumeButton")) {
            class VolumeButtonAccessible : public QAccessibleWidget {
            public:
                explicit VolumeButtonAccessible(QWidget *w) : QAccessibleWidget(w, QAccessible::Button) {}
                QSlider* locateSlider() const {
                    const auto widgets = QApplication::allWidgets();
                    for (QWidget *w : widgets) {
                        if (w && w->accessibleName() == QLatin1String("kylin-video_QSlider_m_volumeSlider"))
                            return qobject_cast<QSlider*>(w);
                    }
                    return nullptr;
                }
                int childCount() const override {
                    return locateSlider() ? 1 : 0;
                }
                QAccessibleInterface *child(int index) const override {
                    if (index != 0) return nullptr;
                    QSlider *s = locateSlider();
                    if (!s) return nullptr;
                    return QAccessible::queryAccessibleInterface(s);
                }
                int indexOfChild(const QAccessibleInterface *c) const override {
                    QSlider *s = locateSlider();
                    if (!s || !c) return -1;
                    return c->object() == s ? 0 : -1;
                }
            };
            return new VolumeButtonAccessible(btn);
        }
    }
    return nullptr;
}

} // namespace

using namespace Global;

QString int2ts(int64_t time, bool use_msec)
{
    if (time <= 0) {
        return QString("--:--:--");
    }
    // 当前是秒
    time *= 1000;

    int64_t hour = 60*60*1000;
    int64_t minute = 60*1000;
    int64_t second = 1*1000;

    int64_t h = time/hour;
    int64_t m = time%hour/minute;
    int64_t s = time%minute/second;
    int64_t ms = time%1000;
    char res[16];

    if(use_msec)
        sprintf(res, "%02ld:%02ld:%02ld.%03ld", h, m, s, ms);
    else
        sprintf(res, "%02ld:%02ld:%02ld", h, m, s);
    return QString(res);
}

ContralBar::ContralBar(QWidget *parent) :
    ThemeWidget(parent),
    m_isMute(false)
{
    // 安装无障碍工厂：确保音量弹窗中的滑动条可被辅助工具发现
    QAccessible::installFactory(contralbarAccessibleFactory);
    m_speed = 1.0;
    m_seekTime = 0;
    m_duration = 0;
    m_isChangeTime = false;
    m_isFullScreen = false;
    m_isMouseEnter = false;
    m_isMiniMode = false;
    m_canChangeDuration = true;
    m_mouseOnSlider = false;
    m_speedList = nullptr;

    m_isTabletMode = false;

    m_isSeeking = false;
    m_seekTimer = new QTimer;
    m_seekTimer->setObjectName("kylin-video_QTimer_m_seekTimer");
    m_seekTimer->setInterval(200);
    connect(m_seekTimer, &QTimer::timeout, this, [this](){m_isSeeking = false;});

    if (g_settings->contains("General/volume"))
    {
        m_volume = g_settings->value("General/volume").toInt();
        g_user_signal->setVolume(m_volume);
    }
    else
    {
        m_volume = 100;
        g_settings->setValue("General/volume", m_volume);
    }

    if (g_gsettings->keys().contains("themeColor"))
    {
        m_highlight_color_name = g_gsettings->get("themeColor").toString();
    } else {
        m_highlight_color_name = "daybreakBlue";
    }

    initLayout();
    initConnect();
    initGlobalSig();
    initAnimation();
    m_timeSlider->installEventFilter(this);

    connectThemeSetting();

    // 设置进度条为伸缩量，不设置的话会和其他控件平分剩下空间
//    ((QBoxLayout*)m_widget->layout())->setStretchFactor(m_timeSlider, 1);

    QFont f;
    f.setPixelSize(14);
    m_posLabel->setFont(f);
}

ContralBar::~ContralBar()
{
}

void ContralBar::setDuration(int duration)
{
    m_duration = duration;
    if (m_duration > 0)
        m_toolList->item(1)->setFlags(m_toolList->item(1)->flags() | Qt::ItemIsEnabled);
    m_timeSlider->setRange(0, m_duration * 100);
    m_posLabel->setText(QString("00:00:00").append("/").append(int2ts(m_duration, false)));
}

void ContralBar::setCurrentTime(int currentTime)
{
    if(m_isChangeTime)
        return;
    m_currentTime = currentTime;
    m_timeSlider->setValue(currentTime);
    m_posLabel->setText(int2ts(currentTime, false).append("/").append(int2ts(m_duration, false)));
}

void ContralBar::setPreviewSize(QSize size)
{
//    m_videoPreview->resize(size);
}

void ContralBar::setHide()
{
    m_volumeWidget->hide();
    m_speedWidget->hide();
    hide();
}

void ContralBar::setShow()
{
    if(!m_isMiniMode)
        show();
}

void ContralBar::setFullScreenState(bool full)
{
    m_isFullScreen = full;
    updateIcon();
}

void ContralBar::setUIMode(bool isTabletMode)
{
    // 是否平板模式，平板模式不显示进度预览
    m_isTabletMode = isTabletMode;
    if (isTabletMode) {
        if (m_fullscreenButton) {
            m_fullscreenButton->hide();
        }
    }
}

void ContralBar::setMainWidgetClickPos(QPoint pos)
{
    if (!Global::isTablet)
        return;
    // 如果有弹出点击位置不是弹出窗口则隐藏
    if (!m_speedWidget->isHidden()) {
        if (pos.x() < m_speedWidget->pos().x()
                || pos.x() > (m_speedWidget->pos().x() + m_speedWidget->width())
                || pos.y() < m_speedWidget->pos().y()
                || pos.y() > (m_speedWidget->pos().y() + m_speedWidget->height())) {
            m_speedWidget->hide();
        }
    }
    else if (!m_volumeWidget->isHidden()) {
        if (pos.x() < m_volumeWidget->pos().x()
                || pos.x() > (m_volumeWidget->pos().x() + m_volumeWidget->width())
                || pos.y() < m_volumeWidget->pos().y()
                || pos.y() > (m_volumeWidget->pos().y() + m_volumeWidget->height())) {
            m_volumeWidget->hide();
        }
    }
    else if (!m_toolWidget->isHidden()) {
        if (pos.x() < m_toolWidget->pos().x()
                || pos.x() > (m_toolWidget->pos().x() + m_toolWidget->width())
                || pos.y() < m_toolWidget->pos().y()
                || pos.y() > (m_toolWidget->pos().y() + m_toolWidget->height())) {
            m_toolWidget->hide();
        }
    }
}

void ContralBar::setBlackTheme()
{
    m_toolButton->setProperty("isWindowButton", 0x1);
    m_toolButton->setProperty("useIconHighlightEffect", 0x2);
    m_toolButton->setFlat(true);
    m_theme = STYLE_UKUI_BLACK;
    m_preButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico/lastsong_h_3x.png);}"));

    m_nextButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico/nextsong_h_3x.png);}"));

    m_toolButton->setIcon(QIcon::fromTheme("ukui-full-backup-symbolic"));

    m_posLabel->setStyleSheet("color:#d9d9d9;");

    m_timeSlider->setBlackTheme();

    m_toolList->setStyleSheet("QListWidget{background-color: rgba(0, 0, 0, 0);color:#d9d9d9;}"
                               "QListWidget::item::hover{background-color:rgba(255,255,255,0.1);color:#d9d9d9;}"
                               "QListWidget::item::selected{background-color:rgba(255,255,255,0.1);color:#d9d9d9;}"
                               "QListWidget::item::disabled{background-color:rgba(0,0,0,0);color:#9f9f9f;}");
    updateIcon();
}

void ContralBar::setDefaultTheme()
{
    m_toolButton->setProperty("isWindowButton", 0x1);
    m_toolButton->setProperty("useIconHighlightEffect", 0x2);
    m_toolButton->setFlat(true);
    m_theme = STYLE_UKUI_LIGHT;
    m_preButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico_light/lastsong-h.png);}"));

    m_nextButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico_light/nextsong-h.png);}"));

    m_toolButton->setIcon(QIcon::fromTheme("ukui-full-backup-symbolic"));

    m_posLabel->setStyleSheet("color:#262626;");

    m_timeSlider->setLightTheme();

    m_toolList->setStyleSheet("QListWidget{background-color: rgba(255, 255, 255, 0);color:#262626;}"
                              "QListWidget::item::hover{background-color:rgba(0,0,0,0.1);color:#262626;}"
                              "QListWidget::item::selected{background-color:rgba(0,0,0,0);color:#262626;}"
                              "QListWidget::item::disabled{background-color:rgba(0,0,0,0);color:#9f9f9f;}");
    updateIcon();
}

void ContralBar::clearMark()
{
    m_mutexMark.lock();
    m_markMap.clear();
    m_timeSlider->clearMark();
    m_mutexMark.unlock();
}

void ContralBar::updateMarks()
{
    m_mutexMark.lock();
    QMap<int, QString>::iterator iter = m_markMap.begin();
    while (iter != m_markMap.end())
    {
        m_timeSlider->addMark(iter.key(), iter.value());
        iter++;
    }
    m_mutexMark.unlock();
}

void ContralBar::addMark(int mark_pos, QString desc)
{
    m_mutexMark.lock();
    m_markMap[mark_pos] = desc;
    m_mutexMark.unlock();
}

void ContralBar::insertMark(int mark_pos, QString desc)
{
    m_mutexMark.lock();
    m_markMap[mark_pos] = desc;
    m_timeSlider->addMark(mark_pos, desc);
    m_mutexMark.unlock();
}

void ContralBar::deleteMark(int mark_pos)
{
    m_timeSlider->deleteMark(mark_pos);
}

void ContralBar::initGlobalSig()
{
    // 播放状态改变
    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, this, &ContralBar::slotPlayStateChange);

    // 文件信息改变
    connect(g_core_signal, &GlobalCoreSignal::sigFileInfoChange, this, [&](Mpv::FileInfo fi){
        if (fi.length == 0) {
            m_timeSlider->setRange(0, 1);
            m_timeSlider->setValue(0);
            m_timeSlider->setEnabled(false);
        }
        else
            m_toolList->item(1)->setFlags(m_toolList->item(1)->flags() | Qt::ItemIsEnabled);
        m_videoId = -1;

        if (fi.video_params.codec.indexOf("jpeg") >= 0 ||
                fi.video_params.codec.indexOf("png") >= 0 ||
                fi.video_params.codec == "") {

            m_isVideo = false;

            if (fi.video_params.codec == "") {
                // 设置截图不可用
                m_toolList->item(0)->setFlags(Qt::NoItemFlags);
            }
            else {
                m_toolList->item(0)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            }
        }
        else {
            m_toolList->item(0)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            m_isVideo = true;
        }
    });

    connect(g_user_signal, &GlobalUserSignal::sigNoMarkMode, this, [&](bool noMark){
        m_toolList->setEnabled(!noMark);
    });

    connect(g_core_signal, &GlobalCoreSignal::sigDuration, [&](QString file, double duration){
        if ((m_playingFile == file && duration < m_duration) || duration == 0)
            return;
        m_playingFile = file;
        m_videoPreviewFf->setShowFile(m_playingFile);
        m_duration = duration;
        if (m_duration > 0)
            m_toolList->item(1)->setFlags(m_toolList->item(1)->flags() | Qt::ItemIsEnabled);

        m_timeSlider->setEnabled(true);
        m_timeSlider->setRange(0, m_duration * 1000);
        m_posLabel->setText(QString("00:00:00").append("/").append(int2ts((int)m_duration, false)));

    });

    connect(g_core_signal, &GlobalCoreSignal::sigVideoIdChange, [&](int vid){
        // 打开视频全屏功能删除
        m_videoId = vid;
        return;

        // 打开视频时全屏功能
        if (vid >= 0 && m_videoId != vid) {
            if(g_config->fullScreenWhenPlay.first)
                if(!m_isFullScreen)
                    slotFullScreen();
        }

        m_videoId = vid;
        if(m_videoId < 0){
            m_toolButton->setEnabled(false);
            m_fullscreenButton->setEnabled(false);
        }
        else{
            m_toolButton->setEnabled(true);
            m_fullscreenButton->setEnabled(true);
        }
    });

    // 更新播放时间
    connect(g_core_signal, &GlobalCoreSignal::sigCurrentTime, [&](double time){
        if(m_isChangeTime)
            return;
        m_canChangeDuration = false;
        m_currentTime = time;
        m_timeSlider->setValue(time * 1000);
        if (m_currentTime >= 1)
            m_posLabel->setText(int2ts((int)m_currentTime, false).append("/").append(int2ts((int)m_duration, false)));
    });

    // 音量改变
    connect(g_core_signal, &GlobalCoreSignal::sigVolumeChange, [&](int volume){
        if(g_playstate < 0)
            return;
        m_volume = volume;
        g_settings->setValue("General/volume", m_volume);
        updateIcon();
    });

    // 静音
    connect(g_core_signal, &GlobalCoreSignal::sigMuteChange, [&](bool mute){
        if(g_playstate < 0)
            return;
        m_isMute = mute;
        g_settings->setValue("General/mute", m_isMute);
        updateIcon();
    });

    // 快捷键设置静音走这里中转
    connect(g_user_signal, SIGNAL(sigMute()), this, SLOT(slotMute()));

    // 播放速度改变
    connect(g_core_signal, &GlobalCoreSignal::sigSpeedChange, this, [&](double speed){
        if(g_playstate < 0)
            return;
        m_speed = speed;
        int row = 7-(int)(speed*4);
        row = row<0.2 ? 0 : row;
        m_speedList->setCurrentRow(row);
        g_settings->setValue("General/speed", m_speed);
        updateIcon();
    });

    // 隐藏 显示
//    connect(g_user_signal, &GlobalUserSignal::sigHideBar, [&](bool is_hide){
//        if(is_hide)
//        {
//            if(m_isMouseEnter)
//                ((QWidget*)parent())->setCursor(QCursor(Qt::ArrowCursor));
//            else
//            {
//                hide();
//                m_volumeWidget->hide();
//                m_speedWidget->hide();
//            }
//        }
//        else
//        {
//            if(!m_isMiniMode)
//                show();
//        }
//    });

    // 快捷键全屏
    connect(g_user_signal, &GlobalUserSignal::sigFullScreen, this, &ContralBar::slotFullScreen);

    connect(g_core_signal,&GlobalCoreSignal::sigAudioIdChange,this,[this](int aid){
        m_aid = aid;
        m_volumeButton->setEnabled(aid >= 0);
    });
}

void ContralBar::initLayout()
{
    {
        QVBoxLayout *lay = new QVBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);

        m_widget = new FilletWidget;
        m_widget->setAccessibleName("kylin-video_contralbar_FilletWidget_m_widget");
        lay->addWidget(m_widget);

        QHBoxLayout *lay_widget = new QHBoxLayout(m_widget);
        lay_widget->setContentsMargins(15, 0, 0, 0);
        lay_widget->setSpacing(16);

        m_preButton = new QPushButton;
        m_preButton->setAccessibleName("kylin-video_QPushButton_m_preButton");
        m_preButtonOpacityEffect = new QGraphicsOpacityEffect(m_preButton);
        m_preButtonOpacityEffect->setOpacity(0.7);
        m_preButton->setGraphicsEffect(m_preButtonOpacityEffect);
        m_preButton->installEventFilter(this);
        lay_widget->addWidget(m_preButton);

        m_playPauseButton = new QPushButton;
        m_playPauseButton->setAccessibleName("kylin-video_QPushButton_m_playPauseButton");
        m_playPauseButtonOpacityEffect = new QGraphicsOpacityEffect(m_playPauseButton);
        m_playPauseButtonOpacityEffect->setOpacity(0.7);
        m_playPauseButton->setGraphicsEffect(m_playPauseButtonOpacityEffect);
        m_playPauseButton->installEventFilter(this);
        lay_widget->addWidget(m_playPauseButton);

        m_nextButton = new QPushButton;
        m_nextButton->setAccessibleName("kylin-video_QPushButton_m_nextButton");
        m_nextButtonOpacityEffect = new QGraphicsOpacityEffect(m_nextButton);
        m_nextButtonOpacityEffect->setOpacity(0.7);
        m_nextButton->setGraphicsEffect(m_nextButtonOpacityEffect);
        m_nextButton->installEventFilter(this);
        lay_widget->addWidget(m_nextButton);

        m_timeSlider = new TimeSlider;
        m_timeSlider->setAccessibleName("kylin-video_TimeSlider_m_timeSlider");
        lay_widget->addWidget(m_timeSlider);

        m_posLabel = new QLabel;
        m_posLabel->setAccessibleName("kylin-video_QLabel_m_posLabel");
        lay_widget->addWidget(m_posLabel);

        {
            m_frame = new QFrame;
            m_frame->setAccessibleName("kylin-video_contralbar_QFrame_m_frame");
            QHBoxLayout *lay_frame = new QHBoxLayout(m_frame);
            lay_frame->setContentsMargins(0, 0, 6, 0);
            lay_frame->setSpacing(4);

            m_volumeButton = new QPushButton;
            m_volumeButton->setAccessibleName("kylin-video_QPushButton_m_volumeButton");
            connect(m_volumeButton, &QPushButton::clicked, [this](){
                // 点击之后弹出
                m_volumeWidget->show();
                m_toolWidget->hide();
                m_speedWidget->hide();
                m_timerVolumeWidgetHide->stop();
            });
            lay_frame->addWidget(m_volumeButton);

            m_speedButton = new QPushButton;
            m_speedButton->setAccessibleName("kylin-video_QPushButton_m_speedButton");
            // 平板点击弹出
            if (Global::isTablet) {
                connect(m_speedButton, &QPushButton::clicked, [this](){
                    m_volumeWidget->hide();
                    m_toolWidget->hide();
                    m_speedWidget->show();
                    m_speedWidget->setFocus(Qt::ActiveWindowFocusReason);
                    m_timerSpeedWidgetHide->stop();
                });
            }
            lay_frame->addWidget(m_speedButton);

            m_toolButton = new QPushButton;
            m_toolButton->setAccessibleName("kylin-video_QPushButton_m_toolButton");
            m_toolButton->setIcon(QIcon::fromTheme("ukui-full-backup-symbolic"));
            m_toolButton->setIconSize(QSize(16,16));
            connect(m_toolButton, &QPushButton::clicked, [this](){
                // 点击之后弹出
                m_volumeWidget->hide();
                m_toolWidget->show();
                m_speedWidget->hide();
                m_timerToolWidgetHide->stop();
            });
            lay_frame->addWidget(m_toolButton);

            m_fullscreenButton = new QPushButton;
            m_fullscreenButton->setAccessibleName("kylin-video_QPushButton_m_fullscreenButton");
            lay_frame->addWidget(m_fullscreenButton);
            if (m_isTabletMode)
                m_fullscreenButton->hide();
        }
        lay_widget->addWidget(m_frame);

        lay_widget->setStretchFactor(m_timeSlider, 1);
    }

    m_nextButton->setToolTip(tr("Next Page+Down"));
    m_preButton->setToolTip(tr("Previous Page+Up"));
    m_playPauseButton->setToolTip(tr("Play Space"));
    m_volumeButton->setToolTip(tr("Volume Up/Down"));
    m_speedButton->setToolTip(tr("Speed"));
    m_toolButton->setToolTip(tr("Tools"));
    m_fullscreenButton->setToolTip(tr("Full screen Enter"));

    m_nextButton->setFixedSize(16, 16);
    m_playPauseButton->setFixedSize(16, 22);
    m_preButton->setFixedSize(16, 16);

//    m_fullscreenButton->setFixedSize(16, 16);
    m_fullscreenButton->setFixedSize(THEME_BUTTON_SIZE, THEME_BUTTON_SIZE);
    m_speedButton->setFixedSize(THEME_BUTTON_SIZE, THEME_BUTTON_SIZE);
    m_toolButton->setFixedSize(THEME_BUTTON_SIZE, THEME_BUTTON_SIZE);
    m_volumeButton->setFixedSize(THEME_BUTTON_SIZE, THEME_BUTTON_SIZE);



    // 音量调节界面
    m_volumeWidget = new FilletWidget((QWidget*)parent());
    m_volumeWidget->setAccessibleName("kylin-video_contralbar_FilletWidget_m_volumeWidget");
    m_volumeWidget->setRadius(4);
    m_volumeWidget->installEventFilter(this);
    m_volumeWidget->setFixedSize(30, 90);
    QHBoxLayout *hl_volume = new QHBoxLayout(m_volumeWidget);
    hl_volume->setContentsMargins(5, 10, 5, 10);
    m_volumeSlider = new QSlider;
    m_volumeSlider->setAccessibleName("kylin-video_QSlider_m_volumeSlider");
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->installEventFilter(this);
    m_volumeSlider->setValue(g_settings->value("General/volume").toInt());
    // 屏蔽动效，否则MouseButtonPress事件会被过滤掉
    m_volumeSlider->setProperty("animation", false);
    hl_volume->addWidget(m_volumeSlider);
    m_volumeWidget->hide();

    // 音量调节隐藏逻辑
    m_timerVolumeWidgetHide = new QTimer;
    m_timerVolumeWidgetHide->setObjectName("kylin-video_QTimer_m_timerVolumeWidgetHide");
    m_timerVolumeWidgetHide->setSingleShot(true);
    connect(m_timerVolumeWidgetHide, &QTimer::timeout, [&](){m_volumeWidget->hide();});
    m_timerVolumeWidgetHide->setInterval(500);

    // 倍速调节界面
    m_speedWidget = new FilletWidget((QWidget*)parent());
    m_speedWidget->setAccessibleName("kylin-video_contralbar_FilletWidget_m_speedWidget");
    m_speedWidget->setRadius(4);
    m_speedWidget->setFixedSize(65, 150);
    m_speedWidget->installEventFilter(this);
    QHBoxLayout *hl_speed = new QHBoxLayout(m_speedWidget);
    hl_speed->setContentsMargins(0, 0, 0, 0);
    m_speedList = new QListWidget;
    m_speedList->setAccessibleName("kylin-video_QListWidget_m_speedList");
    connect(m_speedList, &QListWidget::itemClicked, [&](QListWidgetItem *item){
        QString test = item->text();
        m_speed = test.left(test.length()-1).toDouble();
        g_user_signal->setSpeed(m_speed);
    });
    hl_speed->addWidget(m_speedList);
    initSpeedList();
    m_speedWidget->hide();

    m_timerSpeedWidgetHide = new QTimer;
    m_timerSpeedWidgetHide->setObjectName("kylin-video_QTimer_m_timerSpeedWidgetHide");
    m_timerSpeedWidgetHide->setSingleShot(true);
    connect(m_timerSpeedWidgetHide, &QTimer::timeout, [&](){m_speedWidget->hide();});
    m_timerSpeedWidgetHide->setInterval(500);

    // 工具界面
    m_toolWidget = new FilletWidget((QWidget*)parent());
    m_toolWidget->setAccessibleName("kylin-video_contralbar_FilletWidget_m_toolWidget");
    m_toolWidget->setRadius(4);
    m_toolWidget->setFixedSize(105, 53);
    m_toolWidget->installEventFilter(this);
    QHBoxLayout *hl_tool = new QHBoxLayout(m_toolWidget);
    hl_tool->setContentsMargins(0, 0, 0, 0);
    m_toolList = new QListWidget;
    m_toolList->setAccessibleName("kylin-video_QListWidget_m_toolList");
    m_toolList->setAlternatingRowColors(false);
    connect(m_toolList, &QListWidget::itemClicked, [&](QListWidgetItem *item){
        item->setSelected(false);
        if (!(item->flags() | Qt::ItemIsEnabled))
            return;
        if (item->text() == QString(" ") + tr("Screen shot")) {
            // 截图
            g_user_signal->screenShot(false);
        }
        else {
            // 添加书签, 现在没有书签描述
            // 判断是否无痕浏览，如果无痕浏览不能添加书签
            if (!g_config->seamlessBrowsing.first)
                g_user_signal->addBookMark("");
        }
    });
    hl_tool->addWidget(m_toolList);
    initToolList();
    m_toolWidget->hide();

    m_timerToolWidgetHide = new QTimer;
    m_timerToolWidgetHide->setObjectName("kylin-video_QTimer_m_timerToolWidgetHide");
    m_timerToolWidgetHide->setSingleShot(true);
    connect(m_timerToolWidgetHide, &QTimer::timeout, [&](){m_toolWidget->hide();});
    m_timerToolWidgetHide->setInterval(500);

    m_volumeButton->installEventFilter(this);
    m_speedButton->installEventFilter(this);
    m_toolButton->installEventFilter(this);

    // 视频预览
//    m_videoPreview = new VideoPreview;
//    m_videoPreview->setAccessibleName("kylin-video_Video_Preview_m_videoPreview");
//    m_videoPreview->setFixedSize(192, 108);

    m_videoPreviewFf = new VideoPreviewFf(this);
    m_videoPreviewFf->setFixedSize(VIDEO_PREVIEW_SIZE);
    kdk::UkuiStyleHelper::self()->removeHeader(m_videoPreviewFf);

    setContralBarFont();

    raise();
}

/** **********************************************
 * 初始化控件槽函数
 *************************************************/
void ContralBar::initConnect()
{
#if 0
    connect(g_gsettings, &QGSettings::changed, [&](QString key){
        // 如果不是跟随主题的话直接返回
        if(key == "styleName")
            if(g_settings->value("General/follow_system_theme").toBool())
                if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
                    setLightTheme();
                else
                    setBlackTheme();
    });
#endif

    connect(m_volumeSlider, &QSlider::valueChanged, [&](int volume){
        if (m_volume == volume)
            return;
        m_volume = volume;
        g_user_signal->setVolume(m_volume);
    });

    // 使用double替换int，保证数据的准确性。
    connect(m_timeSlider, &TimeSlider::sliderMoved, [&](double value){
        if (m_seekTime == value / 1000 && QDateTime::currentMSecsSinceEpoch() - m_last_seek_time < 500) {
            m_last_seek_time = QDateTime::currentMSecsSinceEpoch();
            return;
        }

        // 最后一次的时间必须要更新
        m_seekTime = value / 1000;
        m_last_seek_time = QDateTime::currentMSecsSinceEpoch();
        if (m_isSeeking)
            return;
        m_isSeeking = true;
        m_seekTimer->stop();
        m_seekTimer->start();
        QTimer::singleShot(200, [=](){
            g_user_signal->seek(m_seekTime);
        });
    });

    connect(m_playPauseButton, &QPushButton::clicked, [&](){
        g_user_signal->play_pause();
    });

    connect(m_preButton, &QPushButton::clicked, [&](){
        g_user_signal->playPrev(true);
    });

    connect(m_nextButton, &QPushButton::clicked, [&](){
        g_user_signal->playNext(true);
    });

    connect(m_fullscreenButton, &QPushButton::clicked, this, &ContralBar::slotFullScreen);
    // 平板上面没有静音了，只有音量大小
    if (!Global::isTablet)
        connect(m_volumeButton, &QPushButton::clicked, this, &ContralBar::slotMute);
    connect(m_timeSlider, &TimeSlider::mousePosChange, this, &ContralBar::slotShowPreview);
    connect(m_timeSlider, &TimeSlider::mouseLeave, [this](){
        m_videoPreviewFf->hide();
        m_mouseOnSlider = false;
    });
    connect(g_gsettings, &QGSettings::changed, this, &ContralBar::gSettingChanged);
}

void ContralBar::initAnimation()
{
    // 透明度动画不生效
    m_showAnm = new QPropertyAnimation(this, "windowOpacity");
    m_showAnm->setObjectName("kylin-video_QPropertyAnimation_m_showAnm");
    m_showAnm->setDuration(200);

    m_hideAnm = new QPropertyAnimation(this, "windowOpacity");
    m_hideAnm->setObjectName("kylin-video_QPropertyAnimation_m_hideAnm");
    m_hideAnm->setDuration(200);
}

/** **********************************************
 * 速度选择列表，需要做主题适配
 *************************************************/
void ContralBar::initSpeedList()
{
    QFont f;
    f.setPixelSize(14);
    m_speedList->setFont(f);

    // 先设置背景，透明就行
    updateHightLightColor();

    m_speedList->clear();
    m_speedList->addItems(QStringList()
                          << QString(" ").append("2.0X")
                          << QString(" ").append("1.5X")
                          << QString(" ").append("1.25X")
                          << QString(" ").append("1.0X")
                          << QString(" ").append("0.75X")
                          << QString(" ").append("0.5X"));
    // 初始化的时候设置
    m_speed = g_settings->value("General/speed").toDouble();
    if(m_speed == 0)
    {
        m_speed = 1.0;
        g_settings->setValue("General/speed", m_speed);
    }

    for(int i=0; i<m_speedList->count(); i++) {
        QListWidgetItem *it = m_speedList->item(i);
        it->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        it->setSizeHint(QSize(52, 24));
        QString test = it->text();
        if(test.left(test.length()-1).toDouble() == m_speed)
            it->setSelected(true);
    }
    updateIcon();
}

void ContralBar::initToolList()
{
    QFont f;
    f.setPixelSize(14);
    m_toolList->setFont(f);

    // 先设置背景，透明就行
    m_toolList->setStyleSheet("QListWidget{background-color: rgba(255, 255, 255, 0);}"
                              "QListWidget::item::selected{background-color:rgba(1,1,1,0);color:rgb(55,144,250);}");

    m_toolList->clear();
    m_toolList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_toolList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_toolList->addItems(QStringList() << QString(" ").append(tr("Screen shot")) << QString(" ").append(tr("Add mark")));
    for (int i=0; i<m_toolList->count(); i++) {
        m_toolList->item(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_toolList->item(i)->setSizeHint(QSize(102, 24));
    }
}

void ContralBar::updateIcon()
{
    // 更新播放按钮
    QString ico_dir, ico;
    if(m_theme == STYLE_UKUI_LIGHT)
        ico_dir = "ico_light";
    else
        ico_dir = "ico";

    if(g_playstate == Mpv::Playing) {
        ico = "Pause";
        m_playPauseButton->setToolTip(tr("Pause Space"));
    }
    else {
        ico = "Play";
        m_playPauseButton->setToolTip(tr("Play Space"));
    }

    m_playPauseButton->setStyleSheet(QString("QPushButton{border-image: url(:/%0/%1-h.png);}")
                                      .arg(ico_dir).arg(ico));

    // 更新音量按钮
    m_volume = g_settings->value("General/volume").toInt();
    m_isMute = g_settings->value("General/mute").toBool();
    m_volumeSlider->setValue(m_volume);
    if(m_volume == 0 || m_isMute){
        m_volumeButton->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
    }
    else if(m_volume < 50){
        m_volumeButton->setIcon(QIcon::fromTheme("audio-volume-medium-symbolic"));
    }
    else{
        m_volumeButton->setIcon(QIcon::fromTheme("audio-volume-high-symbolic"));
    }
    m_volumeButton->setProperty("isWindowButton", 0x1);
    m_volumeButton->setProperty("useIconHighlightEffect", 0x2);
    m_volumeButton->setFlat(true);

    m_speedButton->setIcon(QIcon::fromTheme(QString("ukui-video-%1-symbolic").arg(m_speed == 1 ? "1.0" : m_speed == 2 ? "2.0" : QString::number(m_speed))));
    m_speedButton->setProperty("isWindowButton", 0x1);
    m_speedButton->setProperty("useIconHighlightEffect", 0x2);
    m_speedButton->setFlat(true);

    // 全屏按钮
    if(m_isFullScreen) {
        m_fullscreenButton->setToolTip(tr("Exit full screen Enter"));
        m_fullscreenButton->setIcon(QIcon::fromTheme("view-restore-symbolic"));

    }
    else {
        m_fullscreenButton->setToolTip(tr("Full screen Enter"));
        m_fullscreenButton->setIcon(QIcon::fromTheme("view-fullscreen-symbolic"));
    }
    m_fullscreenButton->setProperty("isWindowButton", 0x1);
    m_fullscreenButton->setProperty("useIconHighlightEffect", 0x2);
    m_fullscreenButton->setFlat(true);
}

bool ContralBar::eventFilter(QObject *target, QEvent *event)
{
    auto handleOpacity = [](QObject *target, QEvent *event, QGraphicsOpacityEffect *effect) {
        if (target == nullptr || effect == nullptr)
            return false;
        if (event->type() == QEvent::Enter) {
            effect->setOpacity(1.0);
        } else if (event->type() == QEvent::Leave) {
            effect->setOpacity(0.7);
        } else if (event->type() == QEvent::MouseButtonPress) {
            effect->setOpacity(0.86);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            effect->setOpacity(1.0);
        } else {
            return false;
        }
        return true;
    };

    if (target == m_preButton) {
        handleOpacity(target, event, m_preButtonOpacityEffect);
    } else if (target == m_playPauseButton) {
        handleOpacity(target, event, m_playPauseButtonOpacityEffect);
    } else if (target == m_nextButton) {
        handleOpacity(target, event, m_nextButtonOpacityEffect);
    }
    if (!Global::isTablet) {
        if (target == m_volumeWidget)
        {
            if (event->type() == QEvent::Leave)
            {
                m_timerVolumeWidgetHide->start();
                m_isMouseEnter = false;
                emit sigCanHide(true);
            }
            else if (event->type() == QEvent::Enter)
            {
                m_timerVolumeWidgetHide->stop();
                m_isMouseEnter = true;
                emit sigCanHide(false);
            }
            else if (event->type() == QEvent::Wheel && !m_volumeWidget->isHidden())
            {
                m_volumeSlider->event(event);
            }
        }
        else if (target == m_speedWidget)
        {
            if (event->type() == QEvent::Leave)
            {
                m_timerSpeedWidgetHide->start();
                m_isMouseEnter = false;
                emit sigCanHide(true);
            }
            else if (event->type() == QEvent::Enter)
            {
                m_timerSpeedWidgetHide->stop();
                m_isMouseEnter = true;
                emit sigCanHide(false);
            }
        }
        else if (target == m_toolWidget)
        {
            if (event->type() == QEvent::Leave)
            {
                m_timerToolWidgetHide->start();
                m_isMouseEnter = false;
                emit sigCanHide(true);
            }
            else if (event->type() == QEvent::Enter)
            {
                m_timerToolWidgetHide->stop();
                m_isMouseEnter = true;
                emit sigCanHide(false);
            }
        }
        else if (target == m_speedButton)
        {
            if (event->type() == QEvent::Leave)
            {
                m_timerSpeedWidgetHide->start();
            }
            else if (event->type() == QEvent::Enter)
            {
                m_volumeWidget->hide();
                m_toolWidget->hide();
                m_speedWidget->show();
                m_timerSpeedWidgetHide->stop();
            }
        }
        else if (target == m_volumeButton && g_playstate>=Mpv::Started && m_aid >= 0)
        {
            if (event->type() == QEvent::Leave)
            {
                m_timerVolumeWidgetHide->start();
            }
            else if (event->type() == QEvent::Enter)
            {
                m_speedWidget->hide();
                m_toolWidget->hide();
                m_volumeWidget->show();
                m_timerVolumeWidgetHide->stop();
            }
        }
        else if (target == m_toolButton) {
            if (event->type() == QEvent::Leave)
            {
                m_timerToolWidgetHide->start();
            }
            else if (event->type() == QEvent::Enter)
            {
                m_speedWidget->hide();
                m_volumeWidget->hide();
                m_toolWidget->show();
                m_timerToolWidgetHide->stop();
            }
        }
        else if (target == m_volumeSlider && event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                int sliderY = mouseEvent->pos().y();  // 鼠标点击相对滑块的 y 坐标
                double percentage = (m_volumeSlider->height() - static_cast<double>(sliderY))/ m_volumeSlider->height();
                int newValue = percentage * (m_volumeSlider->maximum() - m_volumeSlider->minimum()) + m_volumeSlider->minimum();
                m_volumeSlider->setValue(newValue);

                // 同步全局音量
                g_user_signal->setVolume(newValue);
                return false;
            }
    }
    return ThemeWidget::eventFilter(target, event);
}

void ContralBar::leaveEvent(QEvent *event)
{
    m_isMouseEnter = false;
    emit sigCanHide(true);
}

void ContralBar::showEvent(QShowEvent *event)
{
}

void ContralBar::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

bool ContralBar::event(QEvent *event)
{
    if (event->type() == QEvent::Wheel && !m_volumeWidget->isHidden())
    {
        m_volumeSlider->event(event);
    }
    if (event->type() == QEvent::Move || event->type() == QEvent::Resize)
    {
        int btn_fullscreen_offset = 28;
        if (Global::isTablet)
            btn_fullscreen_offset = 0;
        // 位置写为固定相对位置, 手动调试测出来的，可能不精确.
        if (Global::isTablet)
            m_volumeWidget->move(x()+width()-152-btn_fullscreen_offset, y()-2-m_volumeWidget->height());
        else
            m_volumeWidget->move(x()+width()-132-btn_fullscreen_offset, y()-2-m_volumeWidget->height());
        if (Global::isHaWeiKe){
            m_speedWidget->move(x()+113-btn_fullscreen_offset, y()-2-m_speedWidget->height());
        }else{
            m_speedWidget->move(x()+width()-113-btn_fullscreen_offset, y()-2-m_speedWidget->height());
        }
        m_toolWidget->move(x()+width()-92-btn_fullscreen_offset, y()-2-m_toolWidget->height());
    }
    return ThemeWidget::event(event);
}

void ContralBar::updateHightLightColor()
{
    m_speedList->setStyleSheet(
                QString("QListWidget{background-color: rgba(255, 255, 255, 0);}"
                        "QListWidget::item::selected{background-color:rgba(1,1,1,0);color:%1;}")
                        .arg(m_highlightColor.name()));

    if (g_gsettings->keys().contains("themeColor"))
    {
        m_highlight_color_name = g_gsettings->get("themeColor").toString();
    } else {
        m_highlight_color_name = "daybreakBlue";
    }

    if (m_theme == STYLE_UKUI_LIGHT)
    {
        m_preButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico_light/lastsong-h.png);}"));

        m_nextButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico_light/nextsong-h.png);}"));
    }
    else
    {
        m_preButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico/lastsong_h_3x.png);}"));

        m_nextButton->setStyleSheet(QString("QPushButton{border-image: url(:/ico/nextsong_h_3x.png);}"));
    }
    updateIcon();
}

void ContralBar::enterEvent(QEvent *event)
{
    m_isMouseEnter = true;
    emit sigCanHide(false);
}

/** **********************************
 * 全屏/非全屏
 *************************************/
void ContralBar::slotFullScreen()
{
    emit sigFullScreen(!m_isFullScreen);
}

void ContralBar::slotMute()
{
    g_user_signal->setMute(!m_isMute);
}

void ContralBar::slotShowPreview(qint64 _time)
{
    m_mouseOnSlider = true;
    m_mouseTime = _time;
    if (m_videoId < 0
            || !m_isVideo
            || g_config->hardwareType() == GlobalConfig::X100_GPU
            || m_isTabletMode){
//            || isWayland) {
        QString str_time;
        QTime time = QTime::fromMSecsSinceStartOfDay(_time);
        if(_time >= 3600)    // hours
            str_time = time.toString("h:mm:ss");
        else if(_time >= 60) // minutes
            str_time = time.toString("mm:ss");
        else
            str_time = time.toString("0:ss");
        m_timeSlider->setTimeTip(str_time);
    }
    else
    {
        m_timeSlider->setTimeTip("");
        QPoint gpos = mapToGlobal(m_timeSlider->pos());

        m_videoPreviewFf->setTime(qBound<qint64>(0,_time,(m_duration * 1000)));

        m_videoPreviewFf->getPreview();
        // 拖动预览窗口的时候防止滑动到范围之外
//        m_videoPreview->move(qBound(gpos.x(), QCursor::pos().x(), gpos.x() + m_timeSlider->width())
//                             - m_videoPreview->width()/2,
//                              gpos.y() - m_videoPreview->height() - 10);

        m_videoPreviewFf->move(qBound(gpos.x(), QCursor::pos().x(), gpos.x() + m_timeSlider->width())
                                                            - m_videoPreviewFf->width()/2,
                                                             gpos.y() - m_videoPreviewFf->height() - 10);

        if(Global::isUrl)
        {
//            m_videoPreview->hide();
            m_videoPreviewFf->hide();
        }else{
//            m_videoPreview->show();
//            qInfo() << "show";
            m_videoPreviewFf->show();
        }
//        kdk::WindowManager::keepWindowAbove(m_videoPreview->winId());
//        kdk::WindowManager::activateWindow(m_videoPreview->winId());

        kdk::WindowManager::keepWindowAbove(m_videoPreviewFf->winId());
        kdk::WindowManager::activateWindow(m_videoPreviewFf->winId());
    }
}

void ContralBar::slotPlayStateChange()
{
    switch (g_playstate) {
    case Mpv::Idle:
        m_toolList->item(1)->setFlags(m_toolList->item(1)->flags() & ~Qt::ItemIsEnabled);
        break;
    case Mpv::Started:
        // 如果是无痕浏览的话不能截图和添加书签
        if (g_config->seamlessBrowsing.first)
            m_toolList->setEnabled(false);
        else
            m_toolList->setEnabled(true);

        break;
    case Mpv::Loaded:
        m_canChangeDuration = true;
        break;
    case Mpv::Playing:
        updateIcon();
        break;
    case Mpv::Paused:
        updateIcon();
        break;
    case Mpv::Stopped:
        m_videoId = -1;
        m_duration = 0;
        m_timeSlider->clearMark();
        m_posLabel->setText("--:--:--/--:--:--");
        updateIcon();
        break;
    default:
        break;
    }

}

void ContralBar::gSettingChanged(QString key)
{
    if (key == "systemFontSize" || key == "systemFont") {
        setContralBarFont();
    }
}

void ContralBar::setContralBarFont()
{
    // 字体改变
    QString fontFamily = g_gsettings->get("systemFont").toString();
    double fontSize = g_gsettings->get("systemFontSize").toDouble();
    QFont f(fontFamily);
    f.setPointSizeF(fontSize);

    m_speedList->setFont(f);
    m_toolList->setFont(f);
//    m_videoPreview->setFont(f);
    m_posLabel->setFont(f);

    // 当字体过大会显示省略号，这时会设置弹窗，显示全部信息，目前的判断是基于多次测试尝试的方案，后续有更稳定的方案可以替换
    QFontMetrics fm(f);
    for(int i = 0; i < m_toolList->count(); ++i) {
        auto* p = m_toolList->item(i);
        if (fm.horizontalAdvance(p->text()) <= p->sizeHint().width()-20) {
            qInfo() << "m_toolList text:" << p->text() << ", width: " << fm.horizontalAdvance(p->text());
            qInfo() << "size:" << p->sizeHint().width();
            if (i == 0) {
                p->setToolTip(tr("Screen shot Alt+A"));
            }
            else {
                p->setToolTip(tr("Add mark B"));
            }
        } else {
            p->setToolTip("");
        }
    }

    for(int i = 0; i < m_speedList->count(); ++i) {
        auto* p = m_speedList->item(i);

        if (fm.horizontalAdvance(p->text()) >= p->sizeHint().width()-4) {
            qInfo() << "m_speedList text:" << p->text() << ", width: " << fm.horizontalAdvance(p->text());
            qInfo() << "size:" << p->sizeHint().width();
            p->setToolTip(p->text());
        } else {
            p->setToolTip("");
        }
    }
}
