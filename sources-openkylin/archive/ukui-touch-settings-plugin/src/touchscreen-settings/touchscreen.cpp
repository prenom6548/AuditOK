/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "touchscreen.h"
#include "ui_touchscreen.h"

#include <unistd.h>

#include <kysdk/applications/kswitchbutton.h>
#include <ukcc/interface/ukcccommon.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

#include "kysdk/applications/klineframe.h"
#include <QUrl>
#include <QDebug>
#include <QResizeEvent>
#include <QPalette>
#include <QPushButton>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QVariant>
#include <QMovie>

#include "accessibilityhelper.h"
#include "gesturewidget.h"
#include "touchscreen-settings.h"
#include "settingscommon.h"
#include "tabletmodewidget.h"
#include "autotabletmodewidget.h"
#include "tabletmodedesktopwidget.h"

namespace {
const QString kWlcomEffectService = QStringLiteral("com.kylin.Wlcom");
const QString kWlcomEffectPath = QStringLiteral("/com/kylin/Wlcom/Effect");
const QString kWlcomEffectInterface = QStringLiteral("com.kylin.Wlcom.Effect");
const QString kTouchClickEffect = QStringLiteral("touch_click");
const QString kTouchTrailEffect = QStringLiteral("touch_trail");
const QString kEffectEnabledOption = QStringLiteral("force_enabled");

bool queryEffectFromListAllEffects(QDBusInterface& iface, const QString& effectName, bool* enabled = nullptr)
{
    QDBusMessage reply = iface.call(QStringLiteral("ListAllEffects"));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ListAllEffects failed:" << reply.errorName() << reply.errorMessage();
        return false;
    }
    if (reply.arguments().isEmpty()) {
        qWarning() << "ListAllEffects returned empty reply";
        return false;
    }

    const QVariant first = reply.arguments().constFirst();
    if (!first.canConvert<QDBusArgument>()) {
        qWarning() << "ListAllEffects unexpected reply type:" << first.typeName();
        return false;
    }

    const QDBusArgument &argument = qvariant_cast<QDBusArgument>(first);
    argument.beginArray();
    while (!argument.atEnd()) {
        argument.beginStructure();
        QString name;
        quint32 effectType = 0;
        bool effectEnabled = false;
        argument >> name >> effectType >> effectEnabled;
        argument.endStructure();

        if (name == effectName) {
            if (enabled) {
                *enabled = effectEnabled;
            }
            argument.endArray();
            return true;
        }
    }
    argument.endArray();

    qWarning() << "ListAllEffects: effect not found (unsupported):" << effectName;
    return false;
}
}

#define KYLIN_USER_GUIDE_PATH              "/"
#define KYLIN_USER_GUIDE_SERVICE           "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE         "com.guide.hotel"

TouchScreen::TouchScreen(QList<TouchGestureInfo> gestureInfos,
                         QString modeTypeText,
                         QString autoSwitchText,
                         QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TouchScreen)
    , m_cycleNum(0)
    , m_iconThemeGSettings(nullptr)
    , m_gestureInfos(gestureInfos)
    , m_modeTypeText(modeTypeText)
    , m_autoSwitchText(autoSwitchText)
    , m_autoTabletModeWidget(nullptr)
    , m_tabletModeDesktopWidget(nullptr)
{
    ui->setupUi(this);
    setupWidgetAccessibility(this, tr("Touchscreen settings"),
                             tr("Configure touchscreen gestures, tablet mode, and touch feedback"));
    initUI();
    initConnection();
    loadGif();
}

TouchScreen::~TouchScreen()
{
    delete ui;
    if(m_iconThemeGSettings) {
        m_iconThemeGSettings->deleteLater();
        m_iconThemeGSettings = nullptr;
    }
}

bool TouchScreen::eventFilter(QObject *watched, QEvent *event)
{
//    if (watched == videoWidget) {
//        qDebug() << __func__ << event;
//    }
    return eventFilter(watched, event);
}

GestureWidget* TouchScreen::createGestureWidget(const TouchGestureInfo& gestureInfo)
{
    GestureWidget *gesture = new GestureWidget(ui->gesture_widget);
    gesture->setGestureDescription(gestureInfo.description);
    gesture->setGestureAction(gestureInfo.actionDescription);
    setupWidgetAccessibility(gesture, gestureInfo.description, gestureInfo.actionDescription);
    connect(gesture, &GestureWidget::loseFocus, this, &TouchScreen::stopGif);
    connect(gesture, &GestureWidget::clicked, this, [this, gestureInfo]() {
        playGif(gestureInfo.gifPath);
    });
    return gesture;
}

QList<GestureWidget*> TouchScreen::createGestureWidgets()
{
    QList<GestureWidget*> gestureWidgets;
    for(auto gestureInfo : m_gestureInfos) {
        gestureWidgets << createGestureWidget(gestureInfo);
    }
    return gestureWidgets;
}

void TouchScreen::initUI()
{
    //~ contents_path /TouchScreen/more gesture
    ui->moreGestureButton->setText(tr("More gesture"));
    setupWidgetAccessibility(ui->moreGestureButton, tr("More gesture"),
                             tr("Open touchscreen gesture help"));
    ui->switchTablet_label->setText(m_modeTypeText);
    ui->touchscreen_label->setText(tr("Touchscreen gesture"));
    setupWidgetAccessibility(ui->touchscreen_label, tr("Touchscreen gesture"));
    initTouchFeedbackSettings();

    TabletModeWidget* tabletModeWidget = new TabletModeWidget;
    ui->verticalLayout->insertWidget(2, tabletModeWidget);

    if (getTabletModeEnabled()) {
        m_autoTabletModeWidget = new AutoTabletModeWidget(m_autoSwitchText);
        ui->verticalLayout->insertWidget(3, m_autoTabletModeWidget);
        m_tabletModeDesktopWidget = new TabletModeDesktopWidget(this);
        ui->verticalLayout->insertWidget(4, m_tabletModeDesktopWidget);
    }

    //! \note Do not show tablet mode if os without tablet mode
    if (!SettingsCommon::isTabletProductFeat() ||qgetenv("XDG_SESSION_DESKTOP") == "kylin-wlcom") {
        tabletModeWidget->deleteLater();
        m_autoTabletModeWidget->deleteLater();
        m_tabletModeDesktopWidget->deleteLater();
        ui->switchTablet_label->hide();
        ui->verticalSpacer_2->changeSize(1,0);
        ui->verticalSpacer_5->changeSize(1,0);
        ui->horizontalLayout_4->deleteLater();
    }

    QVBoxLayout *vlayout = new QVBoxLayout(ui->gesture_widget);
    vlayout->setMargin(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(4);
    ui->gesture_widget->setLayout(vlayout);

    QList<GestureWidget*> gestureWidgets = createGestureWidgets();
    for(auto* gestureWidget : gestureWidgets) {
         vlayout->addWidget(gestureWidget);
    }

    connect(ui->moreGestureButton, &QPushButton::clicked, this, [=]() {
        QString pluName = "ukui-control-center/TouchScreen";
        qDebug() << pluName;
        QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
        QDBusInterface interface(service_name,
                                 KYLIN_USER_GUIDE_PATH,
                                 KYLIN_USER_GUIDE_INTERFACE,
                                 QDBusConnection::sessionBus(),
                                 this);
        QDBusMessage msg = interface.call("showGuide", pluName);
    });
}

void TouchScreen::initConnection()
{
    QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                           "/",
                                           "com.kylin.statusmanager.interface",
                                           "isTabletModeEnabledChanged",
                                           this,
                                           SLOT(onTabletModeEnabledChanged(bool)));
}

void TouchScreen::onTabletModeEnabledChanged(bool enabled)
{
    resetAutoTabletModeWidgetEnabled(enabled);
    resetTabletModeDesktopWidgetEnabled(enabled);
}

void TouchScreen::resetAutoTabletModeWidgetEnabled(bool enabled)
{
    if (enabled && nullptr == m_autoTabletModeWidget) {
        m_autoTabletModeWidget = new AutoTabletModeWidget(m_autoSwitchText);
        ui->verticalLayout->insertWidget(3, m_autoTabletModeWidget);
    } else if (!enabled && m_autoTabletModeWidget != nullptr) {
         delete m_autoTabletModeWidget;
         m_autoTabletModeWidget = nullptr;
    }
}

void TouchScreen::resetTabletModeDesktopWidgetEnabled(bool enabled)
{
    if (enabled && nullptr == m_tabletModeDesktopWidget) {
        m_tabletModeDesktopWidget = new TabletModeDesktopWidget;
        ui->verticalLayout->insertWidget(4, m_tabletModeDesktopWidget);

    } else if (!enabled && m_tabletModeDesktopWidget != nullptr) {
         delete m_tabletModeDesktopWidget;
         m_tabletModeDesktopWidget = nullptr;
    }
}

bool TouchScreen::getTabletModeEnabled() const
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");
    QDBusReply<bool> reply = statusManager.call("isTabletModeEnabled");
    if (!reply.isValid()) {
        qWarning() << reply.error();
        return false;
    }

    return reply.value();
}

void TouchScreen::buriedSettings(QString settingsName, QString action, QString value) const
{
    ukcc::UkccCommon::buriedSettings("TouchScreen", settingsName, action, value);
}

bool TouchScreen::isWlcomEffectAvailable() const
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(kWlcomEffectService);
}

bool TouchScreen::isEffectSupported(const QString& effectName) const
{
    QDBusInterface iface(kWlcomEffectService,
                         kWlcomEffectPath,
                         kWlcomEffectInterface,
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        return false;
    }
    return queryEffectFromListAllEffects(iface, effectName);
}

bool TouchScreen::getEffectEnabled(const QString& effectName) const
{
    QDBusInterface iface(kWlcomEffectService,
                         kWlcomEffectPath,
                         kWlcomEffectInterface,
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        return false;
    }

    // 当前会话开关状态：(string, UINT32, Boolean)
    bool enabled = false;
    if (!queryEffectFromListAllEffects(iface, effectName, &enabled)) {
        return false;
    }
    return enabled;
}

void TouchScreen::setEffectEnabled(const QString& effectName, bool enabled)
{
    QDBusInterface iface(kWlcomEffectService,
                         kWlcomEffectPath,
                         kWlcomEffectInterface,
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qWarning() << "Wlcom Effect interface invalid";
        return;
    }

    // 立即生效（当前会话）
    QDBusMessage enableRes = iface.call(QStringLiteral("EnableEffect"), effectName, enabled);
    if (enableRes.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "EnableEffect failed for" << effectName
                   << enableRes.errorName() << enableRes.errorMessage();
    }

    // 持久化，下次启动生效
    QDBusMessage optionRes = iface.call(QStringLiteral("SetEffectOption"),
                                        effectName,
                                        kEffectEnabledOption,
                                        QVariant::fromValue(QDBusVariant(enabled?1:0)));
    if (optionRes.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "SetEffectOption failed for" << effectName
                   << optionRes.errorName() << optionRes.errorMessage();
    }
}

QWidget* TouchScreen::createEffectSwitchRow(const QString& text, const QString& effectName, QWidget* parent)
{
    QWidget* row = new QWidget(parent);
    row->setFixedHeight(60);
    row->setFocusPolicy(Qt::NoFocus);

    QLabel* label = new QLabel(text, row);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    kdk::KSwitchButton* btn = new kdk::KSwitchButton(row);
    btn->setFocusPolicy(Qt::StrongFocus);
    label->setBuddy(btn);
    btn->blockSignals(true);
    btn->setChecked(getEffectEnabled(effectName));
    btn->blockSignals(false);

    connect(btn, &kdk::KSwitchButton::stateChanged, this, [this, effectName](bool enabled) {
        setEffectEnabled(effectName, enabled);
        buriedSettings(effectName, "settings", enabled ? "true" : "false");
    });

    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(btn);

    const QString description = effectName == kTouchClickEffect
            ? tr("Show touch position when touching the screen")
            : tr("Show swipe trail when swiping on the screen");
    setupWidgetAccessibility(btn, text, description);

    return row;
}

QWidget* TouchScreen::createTouchFeedbackSettingsWidget()
{
    const QList<QPair<QString, QString>> effectItems = {
        //~ contents_path /TouchScreen/Touch position display
        {tr("Touch position display"), kTouchClickEffect},
        //~ contents_path /TouchScreen/Swipe trail display
        {tr("Swipe trail display"), kTouchTrailEffect},
    };

    QFrame* feedbackFrame = new QFrame(this);
    feedbackFrame->setMaximumSize(16777215, 16777215);
    feedbackFrame->setFrameShape(QFrame::Shape::Box);

    QVBoxLayout* vLyt = new QVBoxLayout(feedbackFrame);
    vLyt->setMargin(0);
    vLyt->setSpacing(0);

    int addedCount = 0;
    for (const auto& item : effectItems) {
        if (!isEffectSupported(item.second)) {
            continue;
        }
        if (addedCount > 0) {
            vLyt->addWidget(new kdk::KHLineFrame(this));
        }
        vLyt->addWidget(createEffectSwitchRow(item.first, item.second, feedbackFrame));
        ++addedCount;
    }

    if (addedCount == 0) {
        delete feedbackFrame;
        return nullptr;
    }

    return feedbackFrame;
}

void TouchScreen::initTouchFeedbackSettings()
{
    if (!isWlcomEffectAvailable()) {
        return;
    }

    QWidget* feedbackWidget = createTouchFeedbackSettingsWidget();
    if (!feedbackWidget) {
        return;
    }

    const int frameIndex = ui->verticalLayout->indexOf(ui->frame);
    ui->verticalLayout->insertWidget(frameIndex, feedbackWidget);
    ui->verticalLayout->insertSpacing(frameIndex + 1, 8);
}

void TouchScreen::loadGif()
{
    QLabel *videoLabel = new QLabel(ui->video_widget);
    setupWidgetAccessibility(videoLabel, tr("Gesture demonstration"),
                             tr("Preview the selected touchscreen gesture"));
    videoLabel->setFixedSize(400, 240);
    QVBoxLayout *videoLayout = new QVBoxLayout(ui->video_widget);
    videoLayout->setContentsMargins(0, 0, 16, 0);
    videoLayout->addWidget(videoLabel);
    m_movie = new QMovie;
    m_movie->setFileName(":/gif/resources/下边缘上划显示多任务视图.gif");
    m_movie->setScaledSize(QSize(385, 231));
    m_movie->start();
    m_movie->stop();
    videoLabel->setMovie(m_movie);
}

void TouchScreen::playGif(const QString& gifPath)
{
    m_movie->setFileName(gifPath);
    m_movie->stop();
    m_cycleNum = 0;
    m_movie->start();
}

void TouchScreen::stopGif()
{
    m_movie->stop();
//     停止播放后，跳转到第一页
    m_movie->jumpToFrame(0);
}

//    // 下面这种方式可以加载 mp4 文件，但是控制面板加载会报 QXcbConnection 的错误
//    // 用 demo 加载插件不会出现此问题，暂时没有思路
//void TouchScreen::loadVideo()
//{
//    ui->video_widget->setStyleSheet("border-image: none; background: green; border:none");
//    player = new QMediaPlayer;
//    medialist = new QMediaPlaylist;
//    medialist->addMedia(QUrl::fromLocalFile("/home/pangyi/touchgesture/touchgesture/单指边缘-多任务视图.mp4"));
//    medialist->addMedia(QUrl::fromLocalFile("/home/pangyi/touchgesture/touchgesture/单指向下-显示桌面.mp4"));
//    medialist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
//    player->setPlaylist(medialist);
//    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
//          [=](QMediaPlayer::Error error){
//        qDebug() << error;
//    });
//    connect(player, &QMediaPlayer::stateChanged, this, &TouchscreenSettings::stateChanged);
//    connect(player, &QMediaPlayer::mediaStatusChanged, this, &TouchscreenSettings::mediaStatusChanged);

//    QVBoxLayout *videoLayout = new QVBoxLayout(ui->video_widget);

//    videoWidget = new QVideoWidget;
//    videoWidget->installEventFilter(this);
//    videoWidget->setFixedSize(400, 400);
//    videoLayout->setContentsMargins(0, 0, 0, 0);
//    videoLayout->addWidget(videoWidget);
//    player->setVideoOutput(videoWidget);
//}

//void TouchScreen::startVideo1()
//{
//    medialist->setCurrentIndex(1);
//    videoWidget->show();
//    player->play();
//}

//void TouchScreen::mediaStatusChanged(QMediaPlayer::MediaStatus status)
//{
//    switch (status) {
//    case QMediaPlayer::UnknownMediaStatus:
//        qDebug() << "媒体未知状况！";
//        break;
//    case QMediaPlayer::NoMedia:
//        qDebug() << "没有媒体文件！";
//        break;
//    case QMediaPlayer::BufferingMedia:
//        qDebug() << "正在缓冲媒体文件！";
//        break;
//    case QMediaPlayer::BufferedMedia:
//        qDebug() << "媒体文件缓冲完成！" << videoWidget->contentsMargins() << videoWidget->contentsRect() << videoWidget->geometry() << videoWidget->size() << videoWidget->baseSize();
//        break;
//    case QMediaPlayer::LoadingMedia:
//        qDebug() << "正在加载媒体！";
//        break;
//    case QMediaPlayer::StalledMedia:
//        qDebug() << "播放停滞！";
//        break;
//    case QMediaPlayer::EndOfMedia:
//        qDebug() << "播放结束！";
//        break;
//    case QMediaPlayer::LoadedMedia:
//        qDebug() << "媒体加载完成！";
//        break;
//    case QMediaPlayer::InvalidMedia:
//        qDebug() << "不可用的媒体文件！";
//        break;
//    default: break;
//    }
//}

//void TouchScreen::stateChanged(QMediaPlayer::State state)
//{
//    switch (state) {
//    case QMediaPlayer::StoppedState:
//        qDebug() << "停止状态！";
//        break;
//    case QMediaPlayer::PlayingState:
//        qDebug() << "播放状态！";
//        break;
//    case QMediaPlayer::PausedState:
//        qDebug() << "暂停状态！";
//        break;
//    default: break;
//    }
//}
