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
#include "touchpadsettingsui.h"

#include <QDBusInterface>
#include <QDebug>
#include <QMovie>
#include <QGSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QJsonValue>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QMouseEvent>

#include <unistd.h>
#include <kslider.h>
#include "kysdk/applications/klineframe.h"
#include <ukcc/interface/ukcccommon.h>

#include "gesturewidget.h"
#include "settingscommon.h"

#define KYLIN_USER_GUIDE_PATH              "/"
#define KYLIN_USER_GUIDE_SERVICE           "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE         "com.guide.hotel"

const QByteArray kTouchpadSchemas = "org.ukui.peripherals-touchpad";
const QString kMouseDisableKey = "disableOnExternalMouse";
const QString kTouchpadDisableKey = "touchpadEnabled";
const QString kTypingDisableKey = "disableWhileTyping";
const QString kClickKey = "tap-to-click";
const QString kScrollSlideKey = "natural-scroll";
const QString kPointerSpeedKey = "motion-acceleration";

TouchpadUI::TouchpadUI(QList<TouchpadGestureInfo> gestureInfos, QWidget *parent)
    : QWidget(parent)
    , m_gestureInfos(gestureInfos)
{
    QByteArray schema(kTouchpadSchemas);
    if (QGSettings::isSchemaInstalled(schema)) {
        m_touchpadGsetting = new QGSettings(kTouchpadSchemas, QByteArray(), this);
    }
    else {
        qCritical() << "GSetting org.ukui.peripherals-touchpad don't install";
        m_touchpadGsetting = nullptr;
    }

    m_configMonitor = new ConfigMonitor(CONFIG_FILE_NAME);

    initTouchpadEnabled();
    initUI();
}

void TouchpadUI::initTouchpadEnabled()
{
    if (m_configMonitor->getModuleControlledByName(CONFIG_KEY_TOUCHPAD_ENABLED)) {
        m_touchpadEnabled = m_configMonitor->getModuleConfigByName(CONFIG_KEY_TOUCHPAD_ENABLED).toBool();
    } else {
        m_touchpadEnabled = m_touchpadGsetting->get(kTouchpadDisableKey).toBool();
    }
}

void TouchpadUI::setTouchPadEnabled(bool enabled)
{
    if (m_configMonitor->getModuleControlledByName(CONFIG_KEY_TOUCHPAD_ENABLED)) {
        m_touchpadEnabled = m_configMonitor->getModuleConfigByName(CONFIG_KEY_TOUCHPAD_ENABLED).toBool();
    } else {
        m_touchpadEnabled = enabled;
        m_touchpadGsetting->set(kTouchpadDisableKey, enabled);
    }
    emit touchpadEnabled(m_touchpadEnabled);
}

void TouchpadUI::initUI()
{
    if (!m_touchpadGsetting) {
        qCritical() << "Don't show touchpad settings ui "
                       "because org.ukui.peripherals-touchpad don't install";
        return;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);

    QWidget* mainTitle = createTitleWidget(tr("Touchpad"));
    mainLayout->addWidget(mainTitle);

    QWidget* touchpadSettingWidget = createTouchPadSettingWidget();
    mainLayout->addWidget(touchpadSettingWidget);
    mainLayout->addStretch();

    //! \todo the "gesture preview" widget in touchpad and touchscreen plugin looks same,
    //! the different between them is just gif and text, make a class for this widget
    QWidget* gestureTitle = createTitleWidget(tr("Gesture"));
    mainLayout->addSpacing(42);
    mainLayout->addWidget(gestureTitle);

    QWidget* gesturePreviewWidget = createGesturePreviewWidget();
    mainLayout->addWidget(gesturePreviewWidget);

    QWidget* moreGestureWidget = createMoreGestureWidget();
    mainLayout->addSpacing(6);
    mainLayout->addWidget(moreGestureWidget);
    mainLayout->addStretch();
}

QWidget* TouchpadUI::createTitleWidget(QString titleText)
{
    QWidget* titleWidget = new QWidget(this);
    QLabel* titleLabel = new QLabel(this);
    titleLabel->setText(titleText);

    QHBoxLayout *titleLayout = new QHBoxLayout(this);
    titleLayout->setContentsMargins(16, 0, 0, 0);
    titleLayout->addWidget(titleLabel);
    titleWidget->setLayout(titleLayout);

    return titleWidget;
}

QWidget* TouchpadUI::createTouchPadSettingWidget()
{
    QFrame *touchpadFrame = new QFrame(this);
    touchpadFrame->setMaximumSize(16777215, 16777215);
    touchpadFrame->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *touchpadLyt = new QVBoxLayout(touchpadFrame);
    touchpadLyt->setMargin(0);
    touchpadLyt->setSpacing(0);

    QWidget* touchpadDisableWidget = createTouchpadDisableWidget();
    touchpadLyt->addWidget(touchpadDisableWidget);
    touchpadLyt->addWidget(new kdk::KHLineFrame(this));

    QWidget* mouseDisableWidget = createMouseDisableWidget();
    touchpadLyt->addWidget(mouseDisableWidget);
    touchpadLyt->addWidget(new kdk::KHLineFrame(this));

    QWidget* pointerSpeedWidget = createPointerSpeedWidget();
    touchpadLyt->addWidget(pointerSpeedWidget);
    touchpadLyt->addWidget(new kdk::KHLineFrame(this));

    QWidget* typingDisableWidget = createTypingDisableWidget();
    touchpadLyt->addWidget(typingDisableWidget);
    touchpadLyt->addWidget(new kdk::KHLineFrame(this));

    QWidget* clickDisableWidget = createClickDisableWidget();
    touchpadLyt->addWidget(clickDisableWidget);
    touchpadLyt->addWidget(new kdk::KHLineFrame(this));

    QWidget* scrollSlideWidget = createScrollSlideWidget();
    touchpadLyt->addWidget(scrollSlideWidget);
    touchpadLyt->addWidget(new kdk::KHLineFrame(this));

    QWidget* scrollAreaWidget = createScrollAreaWidget();
    touchpadLyt->addWidget(scrollAreaWidget);

    return touchpadFrame;
}

QWidget* TouchpadUI::createTouchpadDisableWidget()
{
    QWidget* touchpadDisableWidget = new QWidget(this);
    touchpadDisableWidget->setMaximumWidth(16777215);
    touchpadDisableWidget->setFixedHeight(60);

    QLabel* touchpadDisableLabel = new QLabel(tr("Touchpad"), this);

    m_touchpadDisableBtn = new kdk::KSwitchButton(this);
    m_touchpadDisableBtn->blockSignals(true);
    m_touchpadDisableBtn->setChecked(m_touchpadEnabled);
    m_touchpadDisableBtn->blockSignals(false);
    m_touchpadDisableBtn->installEventFilter(this);
    connect(m_touchpadDisableBtn, &kdk::KSwitchButton::stateChanged, this, &TouchpadUI::touchpadEnabledSlot);
    connect(this, &TouchpadUI::touchpadEnabled, this, [this](bool status){
        m_touchpadDisableBtn->setChecked(status);
    });
    onConfigChanged(m_touchpadDisableBtn, kTouchpadDisableKey, CONFIG_KEY_TOUCHPAD_ENABLED,
                    m_configMonitor->getModuleControlledByName(CONFIG_KEY_TOUCHPAD_ENABLED));
    connect(m_configMonitor, &ConfigMonitor::moduleConfigChanged, this,  [=]() {
        onConfigChanged(m_touchpadDisableBtn, kTouchpadDisableKey, CONFIG_KEY_TOUCHPAD_ENABLED,
                        m_configMonitor->getModuleControlledByName(CONFIG_KEY_TOUCHPAD_ENABLED));
    });

    QHBoxLayout *touchpadDisableHLayout = new QHBoxLayout(touchpadDisableWidget);
    touchpadDisableHLayout->addSpacing(6);
    touchpadDisableHLayout->addWidget(touchpadDisableLabel);
    touchpadDisableHLayout->addStretch();
    touchpadDisableHLayout->addWidget(m_touchpadDisableBtn);
    touchpadDisableHLayout->addSpacing(6);
    touchpadDisableWidget->setLayout(touchpadDisableHLayout);

    return touchpadDisableWidget;
}

QWidget* TouchpadUI::createMouseDisableWidget()
{
    QWidget* mouseDisableWidget = new QWidget(this);
    mouseDisableWidget->setMaximumWidth(16777215);
    mouseDisableWidget->setFixedHeight(60);

    //~ contents_path /Touchpad/Disable touchpad when using the mouse
    QLabel* mouseDisableLabel = new QLabel(tr("Disable touchpad when using the mouse"), this);

    kdk::KSwitchButton* mouseDisableBtn = new kdk::KSwitchButton(this);
    mouseDisableBtn->installEventFilter(this);
    mouseDisableBtn->blockSignals(true);
    mouseDisableBtn->setChecked(m_touchpadGsetting->get(kMouseDisableKey).toBool());
    mouseDisableBtn->blockSignals(false);
    mouseDisableBtn->setEnabled(m_touchpadEnabled);
    connect(mouseDisableBtn, &kdk::KSwitchButton::stateChanged, this, &TouchpadUI::mouseDisableSlot);
    connect(this, &TouchpadUI::touchpadEnabled, mouseDisableBtn, &kdk::KSwitchButton::setEnabled);


    onConfigChanged(mouseDisableBtn, kMouseDisableKey, CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED,
                    m_configMonitor->getModuleControlledByName(CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED));
    connect(m_configMonitor, &ConfigMonitor::moduleConfigChanged, this,  [=]() {
        onConfigChanged(mouseDisableBtn, kMouseDisableKey, CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED,
                        m_configMonitor->getModuleControlledByName(CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED));
    });

    QHBoxLayout *mouseDisableHLayout = new QHBoxLayout(mouseDisableWidget);
    mouseDisableHLayout->addSpacing(6);
    mouseDisableHLayout->addWidget(mouseDisableLabel);
    mouseDisableHLayout->addStretch();
    mouseDisableHLayout->addWidget(mouseDisableBtn);
    mouseDisableHLayout->addSpacing(6);
    mouseDisableWidget->setLayout(mouseDisableHLayout);

    return mouseDisableWidget;
}

//! \todo clean
QWidget* TouchpadUI::createPointerSpeedWidget()
{
    QWidget* pointerSpeedWidget = new QWidget(this);
    pointerSpeedWidget->setMaximumWidth(16777215);
    pointerSpeedWidget->setFixedHeight(60);


    //~ contents_path /Touchpad/Pointer Speed
    QLabel* pointerSpeedLabel = new QLabel(tr("Pointer Speed"), this);
    pointerSpeedLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QWidget *slideWidget = new QWidget(pointerSpeedWidget);
    QHBoxLayout *slideLayout = new QHBoxLayout(slideWidget);
    slideLayout->setContentsMargins(0,0,0,0);
    slideLayout->setSpacing(0);

    QLabel* pointerSpeedSlowLabel = new QLabel(tr("Slow"), this);
    QLabel* pointerSpeedFastLabel = new QLabel(tr("Fast"), this);
    kdk::KSlider* pointerSpeedSlider = new kdk::KSlider(Qt::Horizontal);
    slideLayout->addWidget(pointerSpeedSlowLabel);
    slideLayout->addWidget(pointerSpeedSlider);
    slideLayout->addWidget(pointerSpeedFastLabel);

    pointerSpeedSlider->setMinimum(11);
    pointerSpeedSlider->setMaximum(80);
    pointerSpeedSlider->setTracking(true);
    pointerSpeedSlider->blockSignals(true);
    pointerSpeedSlider->setValue(m_touchpadGsetting->get(kPointerSpeedKey).toInt()* 10);
    pointerSpeedSlider->blockSignals(false);
    pointerSpeedSlider->setEnabled(m_touchpadEnabled);
    connect(pointerSpeedSlider, &QSlider::valueChanged, this, &TouchpadUI::pointerSpeedSlot);
    connect(m_touchpadGsetting, &QGSettings::changed, this, [=](const QString &key) {
        if (key == kPointerSpeedKey) {
            int val = m_touchpadGsetting->get(kPointerSpeedKey).toInt();
            if (val < 1) {   //异常数据保护 避免手动更改gsetting配置文件导致鼠标过快或者过慢
                pointerSpeedSlider->blockSignals(true);
                pointerSpeedSlider->setValue(11);
                pointerSpeedSlider->blockSignals(false);
            } else if (val > 8) {
                pointerSpeedSlider->blockSignals(true);
                pointerSpeedSlider->setValue(8 *10);
                pointerSpeedSlider->blockSignals(false);
            }
            else {
                pointerSpeedSlider->blockSignals(true);
                pointerSpeedSlider->setValue(val *10);
                pointerSpeedSlider->blockSignals(false);
            }
        }

        if (key == kTouchpadDisableKey) {
            m_touchpadEnabled = m_touchpadGsetting->get(kTouchpadDisableKey).toBool();
            emit touchpadEnabled(m_touchpadEnabled);
        }
    });
    connect(this, &TouchpadUI::touchpadEnabled, pointerSpeedSlider, &kdk::KSwitchButton::setEnabled);

    QHBoxLayout *pointerSpeedHLayout = new QHBoxLayout(pointerSpeedWidget);
    pointerSpeedHLayout->setSpacing(0);
    pointerSpeedHLayout->setContentsMargins(16, 0, 16, 0);
    pointerSpeedHLayout->addWidget(pointerSpeedLabel, 1);
    pointerSpeedHLayout->addWidget(slideWidget, 1);
    pointerSpeedWidget->setLayout(pointerSpeedHLayout);

    return pointerSpeedWidget;
}

QWidget* TouchpadUI::createTypingDisableWidget()
{
    QWidget* typingDisableWidget = new QWidget(this);
    typingDisableWidget->setMaximumWidth(16777215);
    typingDisableWidget->setFixedHeight(60);

    //~ contents_path /Touchpad/Disable touchpad when typing
    QLabel* typingDisableLabel = new QLabel(tr("Disable touchpad when typing"), this);

    kdk::KSwitchButton* typingDisableBtn = new kdk::KSwitchButton(this);
    typingDisableBtn->blockSignals(true);
    typingDisableBtn->setChecked(m_touchpadGsetting->get(kTypingDisableKey).toBool());
    typingDisableBtn->blockSignals(false);
    typingDisableBtn->setEnabled(m_touchpadEnabled);
    connect(typingDisableBtn, &kdk::KSwitchButton::stateChanged, this, &TouchpadUI::typingDisableSlot);
    connect(this, &TouchpadUI::touchpadEnabled, typingDisableBtn, &kdk::KSwitchButton::setEnabled);

    onConfigChanged(typingDisableBtn, kTypingDisableKey, CONFIG_KEY_DISABLE_WHILE_TYPING,
                    m_configMonitor->getModuleControlledByName(CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED));
    connect(m_configMonitor, &ConfigMonitor::moduleConfigChanged, this,  [=]() {
        onConfigChanged(typingDisableBtn, kTypingDisableKey, CONFIG_KEY_DISABLE_WHILE_TYPING,
                        m_configMonitor->getModuleControlledByName(CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED));
    });

    QHBoxLayout *typingDisableHLayout = new QHBoxLayout(typingDisableWidget);
    typingDisableHLayout->addSpacing(6);
    typingDisableHLayout->addWidget(typingDisableLabel);
    typingDisableHLayout->addStretch();
    typingDisableHLayout->addWidget(typingDisableBtn);
    typingDisableHLayout->addSpacing(6);
    typingDisableWidget->setLayout(typingDisableHLayout);

    return typingDisableWidget;
}

QWidget* TouchpadUI::createClickDisableWidget()
{
    QWidget* clickWidget = new QWidget(this);
    clickWidget->setMaximumWidth(16777215);
    clickWidget->setFixedHeight(60);

    QHBoxLayout *clickHLayout = new QHBoxLayout(clickWidget);

    //~ contents_path /Touchpad/Touch and click on the touchpad
    QLabel* clickLabel = new QLabel(tr("Touch and click on the touchpad"), this);
    kdk::KSwitchButton* clickBtn = new kdk::KSwitchButton(this);
    clickBtn->blockSignals(true);
    clickBtn->setChecked(m_touchpadGsetting->get(kClickKey).toBool());
    clickBtn->blockSignals(false);
    clickBtn->setEnabled(m_touchpadEnabled);
    connect(clickBtn, &kdk::KSwitchButton::stateChanged, this, &TouchpadUI::clickSlot);
    connect(this, &TouchpadUI::touchpadEnabled, clickBtn, &kdk::KSwitchButton::setEnabled);

    clickHLayout->addSpacing(6);
    clickHLayout->addWidget(clickLabel);
    clickHLayout->addStretch();
    clickHLayout->addWidget(clickBtn);
    clickHLayout->addSpacing(6);
    clickWidget->setLayout(clickHLayout);

    return clickWidget;
}

QWidget* TouchpadUI::createScrollSlideWidget()
{
    QWidget* scrollSlideWidget = new QWidget(this);
    scrollSlideWidget->setMaximumWidth(16777215);
    scrollSlideWidget->setFixedHeight(60);

    //~ contents_path /Touchpad/Scroll bar slides with finger
    QLabel* scrollSlideLabel = new QLabel(tr("Scroll bar slides with finger"), this);
    kdk::KSwitchButton* scrollSlideBtn = new kdk::KSwitchButton(this);
    scrollSlideBtn->blockSignals(true);
    scrollSlideBtn->setChecked(!m_touchpadGsetting->get(kScrollSlideKey).toBool());
    scrollSlideBtn->blockSignals(false);
    scrollSlideBtn->setEnabled(m_touchpadEnabled);
    connect(scrollSlideBtn, &kdk::KSwitchButton::stateChanged, this, &TouchpadUI::scrollSlideSlot);
    connect(this, &TouchpadUI::touchpadEnabled, scrollSlideBtn, &kdk::KSwitchButton::setEnabled);

    QHBoxLayout *scrollSlideHLayout = new QHBoxLayout(scrollSlideWidget);
    scrollSlideHLayout->addSpacing(6);
    scrollSlideHLayout->addWidget(scrollSlideLabel);
    scrollSlideHLayout->addStretch();
    scrollSlideHLayout->addWidget(scrollSlideBtn);
    scrollSlideHLayout->addSpacing(6);
    scrollSlideWidget->setLayout(scrollSlideHLayout);

    return scrollSlideWidget;
}

//! \todo clean
QWidget* TouchpadUI::createScrollAreaWidget()
{
    QWidget* scrollAreaWidget = new QWidget(this);
    scrollAreaWidget->setMaximumWidth(16777215);
    scrollAreaWidget->setFixedHeight(60);

    //~ contents_path /Touchpad/Scrolling area
    QLabel* scrollAreaLabel = new QLabel(tr("Scrolling area"), this);
    scrollAreaLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QComboBox* scrollTypeComBox = new QComboBox(this);
    scrollTypeComBox->addItem(tr("Two-finger scrolling in the middle area"), V_FINGER_KEY);
    scrollTypeComBox->addItem(tr("Edge scrolling"), V_EDGE_KEY);
    scrollTypeComBox->addItem(tr("Disable scrolling"), N_SCROLLING);
    scrollTypeComBox->blockSignals(true);
    scrollTypeComBox->setCurrentIndex(scrollTypeComBox->findData(currentScrollingType()));
    scrollTypeComBox->blockSignals(false);
    scrollTypeComBox->setEnabled(m_touchpadEnabled);
    if (QString::compare(V_EDGE_KEY, scrollTypeComBox->currentData().toString()) == 0) {
        m_touchpadGsetting->set(H_EDGE_KEY, true);
        m_touchpadGsetting->set(H_FINGER_KEY, false);
    }
    if (QString::compare(V_FINGER_KEY, scrollTypeComBox->currentData().toString()) == 0) {
        m_touchpadGsetting->set(H_EDGE_KEY, false);
        m_touchpadGsetting->set(H_FINGER_KEY, true);
    }
    if (QString::compare(N_SCROLLING, scrollTypeComBox->currentData().toString()) == 0) {
        m_touchpadGsetting->set(H_EDGE_KEY, false);
        m_touchpadGsetting->set(H_FINGER_KEY, false);
    }
    connect(scrollTypeComBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this, [scrollTypeComBox, this](){
        //旧滚动类型设置为false，新滚动类型设置为true
        QString data = scrollTypeComBox->currentData().toString();
        // 禁用滚动
        if (QString::compare(data, N_SCROLLING) == 0) {
            m_touchpadGsetting->set(V_EDGE_KEY, false);
            m_touchpadGsetting->set(H_EDGE_KEY, false);
            m_touchpadGsetting->set(V_FINGER_KEY, false);
            m_touchpadGsetting->set(H_FINGER_KEY, false);
        }
        // 边界滚动:垂直边界+水平边界
        if (QString::compare(data, V_EDGE_KEY) == 0) {
            m_touchpadGsetting->set(V_EDGE_KEY, true);
            m_touchpadGsetting->set(H_EDGE_KEY, true);
            m_touchpadGsetting->set(V_FINGER_KEY, false);
            m_touchpadGsetting->set(H_FINGER_KEY, false);
        }
        // 中间双指:垂直中间+水平中间
        if (QString::compare(data, V_FINGER_KEY) == 0) {
            m_touchpadGsetting->set(V_EDGE_KEY, false);
            m_touchpadGsetting->set(H_EDGE_KEY, false);
            m_touchpadGsetting->set(V_FINGER_KEY, true);
            m_touchpadGsetting->set(H_FINGER_KEY, true);
        }
        buriedSettings("ScrollArea", "select", data);
    });
    connect(this, &TouchpadUI::touchpadEnabled, scrollTypeComBox, &kdk::KSwitchButton::setEnabled);

    QHBoxLayout *scrollAreaHLayout = new QHBoxLayout(scrollAreaWidget);
    scrollAreaHLayout->setSpacing(0);
    scrollAreaHLayout->setContentsMargins(16, 0, 16, 0);
    scrollAreaHLayout->addWidget(scrollAreaLabel, 1);
    scrollAreaHLayout->addWidget(scrollTypeComBox, 1);
    scrollAreaWidget->setLayout(scrollAreaHLayout);

    return scrollAreaWidget;
}

GestureWidget* TouchpadUI::createGestureWidget(const TouchpadGestureInfo &info)
{
    GestureWidget* gestureWidget = new GestureWidget(this);
    gestureWidget->setGestureDescription(info.description);
    gestureWidget->setGestureAction(info.actionDescription);
    connect(gestureWidget, &GestureWidget::loseFocus, this, &TouchpadUI::stopGif);
    connect(gestureWidget, &GestureWidget::clicked, this, [this, info]() {
        playGif(info.gifPath);
    });
    return gestureWidget;
}

QList<GestureWidget*> TouchpadUI::createGestureWidgets()
{
    QList<GestureWidget*> gestureWidgets;
    for(const auto& gestureInfo : qAsConst(m_gestureInfos)) {
        gestureWidgets << createGestureWidget(gestureInfo);
    }
    return gestureWidgets;
}

QWidget* TouchpadUI::createGesturePreviewWidget()
{
    QFrame *gesturePreviewWidget = new QFrame(this);
    gesturePreviewWidget->setMaximumSize(16777215, 16777215);
    gesturePreviewWidget->setFrameShape(QFrame::Shape::Box);
    QVBoxLayout *mainLayout = new QVBoxLayout(gesturePreviewWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *gestureFrame = new QFrame(gesturePreviewWidget);
    gestureFrame->setMaximumSize(16777215, 16777215);
    gestureFrame->setFrameShadow(QFrame::Plain);
    gestureFrame->setFrameShape(QFrame::Shape::Box);

    QHBoxLayout *gestureLayout = new QHBoxLayout(gestureFrame);
    gestureLayout->setContentsMargins(0, 0, 0, 0);
    gestureLayout->setMargin(16);

    QWidget *gestureSelectWidget = createGestureSelectWidget();
    gestureLayout->addWidget(gestureSelectWidget);
    gestureLayout->addSpacing(24);

    QWidget *videoWidget = createVideoWidget();
    gestureLayout->addWidget(videoWidget);

    mainLayout->addWidget(gestureFrame);
    gesturePreviewWidget->adjustSize();

    return gesturePreviewWidget;
}

QWidget* TouchpadUI::createGestureSelectWidget()
{
    QWidget *gestureSelectWidget = new QWidget(this);
    QVBoxLayout *gestureWidgetLayout = new QVBoxLayout(gestureSelectWidget);
    gestureWidgetLayout->setContentsMargins(0, 0, 0, 0);
    gestureWidgetLayout->setMargin(0);
    gestureWidgetLayout->setSpacing(4);

    QList<GestureWidget*> gestureWidgets = createGestureWidgets();
    for (auto* gestureWidget : qAsConst(gestureWidgets)) {
        gestureWidgetLayout->addWidget(gestureWidget);
    }

    return gestureSelectWidget;
}

QWidget* TouchpadUI::createVideoWidget()
{
    QWidget *videoWidget = new QWidget(this);
    videoWidget->setFixedSize(356, 402);
    QLabel *videoLabel = new QLabel(videoWidget);

    QVBoxLayout *videoLayout = new QVBoxLayout(videoWidget);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->addWidget(videoLabel);

    m_movie = new QMovie(this);
    m_movie->setFileName(":/gif/resources/"+projectCodeName+"三指轻点.gif");
    m_movie->setScaledSize(QSize(356, 402));
    m_movie->start();
    m_movie->stop();
    videoLabel->setMovie(m_movie);

    return videoWidget;
}

QWidget* TouchpadUI::createMoreGestureWidget()
{
    QWidget* moreGestureWidget = new QWidget(this);
    //~ contents_path /Touchpad/More gesture
    QPushButton *moreGestureBtn = new QPushButton(tr("More gesture"));
    connect(moreGestureBtn, &QPushButton::clicked, this, []() {
        QString pluginName = "ukui-control-center/Touchpad";
        QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
        QDBusInterface interface(service_name,
                                 KYLIN_USER_GUIDE_PATH,
                                 KYLIN_USER_GUIDE_INTERFACE,
                                 QDBusConnection::sessionBus());
        QDBusMessage msg = interface.call("showGuide", pluginName);

        qDebug() << "Plugin" << pluginName << "call method showGuide of" << KYLIN_USER_GUIDE_INTERFACE;
        if (msg.type() == QDBusMessage::ErrorMessage) {
            qWarning() << msg.errorName() << msg.errorMessage();
        }
    });

    QHBoxLayout *moreGestureButtonLayout = new QHBoxLayout;
    moreGestureButtonLayout->setContentsMargins(0, 0, 0, 0);
    moreGestureButtonLayout->addWidget(moreGestureBtn);
    moreGestureButtonLayout->addStretch();
    moreGestureWidget->setLayout(moreGestureButtonLayout);

    return moreGestureWidget;
}

QString TouchpadUI::currentScrollingType()
{
    if (m_touchpadGsetting->get(V_EDGE_KEY).toBool()) {
        return V_EDGE_KEY;
    }

    if (m_touchpadGsetting->get(V_FINGER_KEY).toBool()) {
        return V_FINGER_KEY;
    }

    return N_SCROLLING;
}

void TouchpadUI::buriedSettings(QString settingsName, QString action, QString value) const
{
    ukcc::UkccCommon::buriedSettings("TouchPad", settingsName, action, value);
}

/* slot functions */
void TouchpadUI::touchpadEnabledSlot(bool status)
{
    setTouchPadEnabled(status);
    buriedSettings(kTouchpadDisableKey, "settings", status ? "true":"false");
}

void TouchpadUI::mouseDisableSlot(bool status)
{
    if (!m_configMonitor->getModuleControlledByName(CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED))
        m_touchpadGsetting->set(kMouseDisableKey, status);
    buriedSettings(kMouseDisableKey, "settings", status ? "true":"false");
}

void TouchpadUI::pointerSpeedSlot(int value)
{
    m_touchpadGsetting->set(kPointerSpeedKey, value/10);
    buriedSettings(kPointerSpeedKey, "settings", QString::number(value));
}

void TouchpadUI::typingDisableSlot(bool status)
{
    if (!m_configMonitor->getModuleControlledByName(CONFIG_KEY_DISABLE_WHILE_TYPING))
        m_touchpadGsetting->set(kTypingDisableKey, status);
    buriedSettings(kTypingDisableKey, "settings", status ? "true":"false");
}

void TouchpadUI::clickSlot(bool status)
{
    m_touchpadGsetting->set(kClickKey, status);
    buriedSettings(kClickKey, "settings", status ? "true":"false");
}

void TouchpadUI::scrollSlideSlot(bool status)
{
    m_touchpadGsetting->set(kScrollSlideKey, !status);
    buriedSettings(kScrollSlideKey, "settings", status ? "true":"false");
}

void TouchpadUI::playGif(const QString& gifPath)
{
    m_movie->setFileName(gifPath);
    m_movie->stop();
    m_movie->start();
}

void TouchpadUI::stopGif()
{
    m_movie->stop();
//     停止播放后，跳转到第一页
    m_movie->jumpToFrame(0);
}

void TouchpadUI::showWarningDialog()
{
    int ret;
    QMessageBox msg(this->topLevelWidget());
    msg.setIcon(QMessageBox::Warning);
    msg.setText(tr("No functional mouse detected. Disabling the touchpad will make the computer inoperable. Do you want to disable the touchpad?"));
    QPushButton *cancelBtn = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *acceptBtn = msg.addButton(tr("Disable Touchpad"), QMessageBox::AcceptRole);
    acceptBtn->setProperty("useButtonPalette", true);
    msg.setDefaultButton(cancelBtn);
    ret = msg.exec();

    switch (ret) {
    case QDialog::DialogCode::Accepted:
        m_touchpadDisableBtn->setChecked(false);
        break;
    case QDialog::DialogCode::Rejected:
        m_touchpadDisableBtn->setChecked(true);
        break;
    }
}

bool TouchpadUI::hasMouseDevice() const
{
    QDBusInterface interface("org.ukui.SettingDaemon",
                             "/org/ukui/SettingDaemon/InputDeviceManger",
                             "org.ukui.SettingDaemon.InputDeviceManger",
                             QDBusConnection::sessionBus());
    QDBusReply<int> reply = interface.call("GetDevCountByType", "mouse");

    if (reply.isValid()) {
        return (reply.value() > 0);
    }
    qWarning() << reply.error().message();
    return true;
}

bool TouchpadUI::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_touchpadDisableBtn
            && m_touchpadDisableBtn->isChecked()
            && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton
                &&!hasMouseDevice()) {
            showWarningDialog();
        }
    }
    return false;
}
void TouchpadUI::onConfigChanged(kdk::KSwitchButton* button, QString gsettingKey, QString configKey, bool isControlled)
{
    disconnect(m_touchpadGsetting, &QGSettings::changed, button, nullptr);
    if(isControlled) {
        button->setChecked(m_configMonitor->getModuleConfigByName(configKey).toBool());
        button->setDisabled(true);
    } else {
        button->setDisabled(false);
        button->setChecked(m_touchpadGsetting->get(gsettingKey).toBool());

        connect(m_touchpadGsetting, &QGSettings::changed, button, [=](const QString &key){
            if (key == gsettingKey) {
                button->setChecked(m_touchpadGsetting->get(gsettingKey).toBool());
            }
        });
    }
}
