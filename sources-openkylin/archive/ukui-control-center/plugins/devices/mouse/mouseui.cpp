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

#include <QFocusEvent>
#include <kysdk/applications/accessinfohelper.h>
#include "mouseui.h"
#define STYLE_FONT_SCHEMA  "org.ukui.style"

MouseUI::MouseUI(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    mVlayout = new QVBoxLayout(this);
    mVlayout->setMargin(0);
    mVlayout->setSpacing(40);
    initUI();
}

MouseUI::~MouseUI()
{
}

DoubleClickTestLabel::DoubleClickTestLabel()
{
    setAttribute(Qt::WA_DeleteOnClose);

    QSizePolicy pSizePolicy = this->sizePolicy();
    pSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    pSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    this->setSizePolicy(pSizePolicy);

    setFixedSize(QSize(54, 28));
    setScaledContents(true);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode) {
        picOff = QString(":/img/plugins/mouse/double-click-off-%1.png").arg("black");
        picOn = QString(":/img/plugins/mouse/double-click-on-%1.png").arg("black");
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode) {
                picOff = QString(":/img/plugins/mouse/double-click-off-%1.png").arg("black");
                picOn = QString(":/img/plugins/mouse/double-click-on-%1.png").arg("black");
            } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
                picOff = QString(":/img/plugins/mouse/double-click-off-%1.png").arg("white");
                picOn = QString(":/img/plugins/mouse/double-click-on-%1.png").arg("white");
            }
            setPixmap(QPixmap(picOff));
        }
    });

    setPixmap(QPixmap(picOff));

    mDoubleClickTestDbus = new QDBusInterface("org.ukui.ukcc.session",
                                              "/Mouse",
                                              "org.ukui.ukcc.session.Mouse",
                                              QDBusConnection::sessionBus(), this);

    if (!mDoubleClickTestDbus->isValid()) {
        qCritical() << "org.ukui.ukcc.session.KeyBoard DBus error:" << mDoubleClickTestDbus->lastError();
    }

    this->setToolTip(tr("double-click to test"));
}

DoubleClickTestLabel::~DoubleClickTestLabel()
{
}

void DoubleClickTestLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    int delay = mDoubleClickTestDbus->property("doubleClick").toInt();
    setPixmap(QPixmap(picOn));
    QTimer::singleShot(delay, this, [=]{
        setPixmap(QPixmap(picOff));
    });
}

void MyTextEdit::startCursor(void)
{
    QFocusEvent fe(QEvent::FocusIn, Qt::MouseFocusReason);
    focusInEvent(&fe);

    this->setProperty("setDisableBrush", QBrush(Qt::transparent));
    this->update();
}

void MouseUI::settingForIntel()
{
    mWheelSpeedWidget->hide();
    mDoubleClickIntervalWidget->hide();
    mScrollDirectionWidget->setFrameShape(QFrame::HLine);
    mMouseAccelerationWidget->hide();
}

void MouseUI::initUI()
{
    //鼠标
    SettingGroup *mouseFrame = new SettingGroup(this);
    mMouseTitleLabel = new TitleLabel(this);
    //~ contents_path /Mouse/Mouse
    mMouseTitleLabel->setText(tr("Mouse"));
    mMouseTitleLabel->setContentsMargins(16,0,0,0);
    setDominantHandFrame();
    setDoubleClickFrame();
    setWheelSpeedFrame();
    setScrollDirectionFrame();

    mouseFrame->addWidget(mDominantHandWidget);
    mouseFrame->addWidget(mDoubleClickIntervalWidget);
    mouseFrame->addWidget(mWheelSpeedWidget);
    mouseFrame->addWidget(mScrollDirectionWidget);

    QVBoxLayout *mouseVLayout = new QVBoxLayout(this);
    mouseVLayout->setSpacing(8);
    mouseVLayout->setMargin(0);
    mouseVLayout->addWidget(mMouseTitleLabel);
    mouseVLayout->addWidget(mouseFrame);

    //指针
    SettingGroup *pointerFrame = new SettingGroup(this);
    mPointerTitleLabel = new TitleLabel(this);
    //~ contents_path /Mouse/Pointer
    mPointerTitleLabel->setText(tr("Pointer"));
    mPointerTitleLabel->setContentsMargins(16,0,0,0);
    setPointerSpeedFrame();
    setMouseAccelerationFrame();
    setTailLightEffectFrame();
    setPointerPositionFrame();
    setPointerSizeFrame();

    pointerFrame->addWidget(mPointerSpeedWidget);
    pointerFrame->addWidget(mMouseAccelerationWidget);
    pointerFrame->addWidget(mMouseTailLightEffectWidget, true, UkccCommon::isSupportTailLight());
    pointerFrame->addWidget(mPointerPositionWidget);
    pointerFrame->addWidget(mPointerSizeWidget);

    QVBoxLayout *pointerVLayout = new QVBoxLayout(this);
    pointerVLayout->setSpacing(8);
    pointerVLayout->setMargin(0);
    pointerVLayout->addWidget(mPointerTitleLabel);
    pointerVLayout->addWidget(pointerFrame);

    //光标
    SettingGroup *cursorFrame = new SettingGroup(this);

    mCursorTitleLabel = new TitleLabel(this);
    //~ contents_path /Mouse/Text Cursor
    mCursorTitleLabel->setText(tr("Text Cursor"));
    mCursorTitleLabel->setContentsMargins(16,0,0,0);
    setBlinkCursorFrame();
    setCursorSpeedFrame();

    cursorFrame->addWidget(mBlinkingCursorOnTextWidget);
    cursorFrame->addWidget(mCursorSpeedWidget);

    QVBoxLayout *cursorVLayout = new QVBoxLayout(this);
    cursorVLayout->setSpacing(8);
    cursorVLayout->setMargin(0);
    cursorVLayout->addWidget(mCursorTitleLabel);
    cursorVLayout->addWidget(cursorFrame);

    mVlayout->addLayout(mouseVLayout);
    mVlayout->addLayout(pointerVLayout);
    mVlayout->addLayout(cursorVLayout);
    mVlayout->addStretch();

    KDK_EXTEND_ALL_INFO_FORMAT(mMouseTitleLabel, "Mouse", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mPointerTitleLabel, "Mouse", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mCursorTitleLabel, "Mouse", "", "");
}

void MouseUI::setDominantHandFrame()
{
    /* dominant hand Frame Setting */

    //~ contents_path /Mouse/Main mouse button
    mDominantHandWidget = new RadioButtonWidget(tr("Main mouse button"), this, UkccFrame::Top);
    mDominantHandWidget->setObjectName("Main mouse button");
    mDominantHandWidget->mainHLayout()->addStretch();
    mDominantHandLeftRadio = new QRadioButton(tr("Left key"), this);
    mDominantHandRightRadio = new QRadioButton(tr("Right key"), this);
    mDominantHandWidget->addButton(mDominantHandLeftRadio, 0);
    mDominantHandWidget->addButton(mDominantHandRightRadio, 1, true);
    KDK_EXTEND_ALL_INFO_FORMAT(mDominantHandLeftRadio, "Mouse", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mDominantHandRightRadio, "Mouse", "", "");
    mDominantHandLeftRadio->setAccessibleName(tr("Left key"));
    mDominantHandRightRadio->setAccessibleName(tr("Right key"));
}

void MouseUI::setScrollDirectionFrame()
{
    /* dominant hand Frame Setting */

    //~ contents_path /Mouse/Scroll direction
    mScrollDirectionWidget = new RadioButtonWidget(tr("Scroll direction"), this);
    mScrollDirectionWidget->setObjectName("Scroll direction");
    mScrollDirectionWidget->mainHLayout()->addStretch();
    mScrollDirectionForwardRadio = new QRadioButton(tr("Forward"), this);
    mScrollDirectionReverseRadio = new QRadioButton(tr("Reverse"), this);
    mScrollDirectionWidget->addButton(mScrollDirectionForwardRadio, 0);
    mScrollDirectionWidget->addButton(mScrollDirectionReverseRadio, 1, true);
    KDK_EXTEND_ALL_INFO_FORMAT(mScrollDirectionForwardRadio, "Mouse", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mScrollDirectionReverseRadio, "Mouse", "", "");
    mScrollDirectionForwardRadio->setAccessibleName(tr("Forward"));
    mScrollDirectionReverseRadio->setAccessibleName(tr("Reverse"));
}

void MouseUI::setWheelSpeedFrame()
{
    /* wheel speed Frame Setting */

    //~ contents_path /Mouse/Wheel speed
    mWheelSpeedWidget = new SliderWidget(tr("Wheel speed"), true, this);
    mWheelSpeedWidget->setObjectName("Wheel speed");
    mWheelSpeedWidget->setLeftText(tr("Slow"));
    mWheelSpeedWidget->setRightText(tr("Fast"));
    mWheelSpeedWidget->slider()->setSliderType(KSliderType::NodeSlider);
    mWheelSpeedWidget->slider()->setRange(1,10);
    mWheelSpeedWidget->slider()->setTickInterval(1);
    mWheelSpeedWidget->slider()->setPageStep(1);
    mWheelSpeedWidget->slider()->installEventFilter(this);
    QLabel *wheelSpeedSlowLabel = mWheelSpeedWidget->leftLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(wheelSpeedSlowLabel, "Mouse", "", "");
    KSlider *wheelSpeedSlider = mWheelSpeedWidget->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(wheelSpeedSlider, "Mouse", "", "");
    wheelSpeedSlider->setAccessibleName(tr("Wheel speed"));
    QLabel *wheelSpeedFastLabel = mWheelSpeedWidget->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(wheelSpeedFastLabel, "Mouse", "", "");
}

void MouseUI::setDoubleClickFrame()
{
    /* Double Click Interval time Frame Setting */

    //~ contents_path /Mouse/Double click speed
    mDoubleClickIntervalWidget = new SliderWidget(tr("Double click speed"), false, this, UkccFrame::Bottom);
    mDoubleClickIntervalWidget->setMinimumSize(QSize(550, 0));
    mDoubleClickIntervalWidget->setMaximumSize(QSize(16777215, 16777215));
    mDoubleClickIntervalWidget->setHeightAdaptive(false);
    KSlider *doubleClickIntervalSlider = mDoubleClickIntervalWidget->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(doubleClickIntervalSlider, "Mouse", "", "");
    doubleClickIntervalSlider->setAccessibleName(tr("Double click speed"));
    QLabel *doubleClickIntervalShortLabel = mDoubleClickIntervalWidget->leftLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(doubleClickIntervalShortLabel, "Mouse", "", "");
    QLabel *doubleClickIntervalLongLabel = mDoubleClickIntervalWidget->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(doubleClickIntervalLongLabel, "Mouse", "", "");

    mDoubleClickIntervalWidget->setObjectName("Double click speed");
    mDoubleClickIntervalWidget->addHintText(tr("Double click the image on the right to test the double-click effect"));
    mDoubleClickIntervalWidget->setLeftText(tr("Slow"));
    mDoubleClickIntervalWidget->setRightText(tr("Fast"));
    mDoubleClickIntervalWidget->slider()->setSliderType(KSliderType::SmoothSlider);
    mDoubleClickIntervalWidget->slider()->setMinimum(170);
    mDoubleClickIntervalWidget->slider()->setMaximum(1000);
    mDoubleClickIntervalWidget->slider()->setSingleStep(100);
    mDoubleClickIntervalWidget->slider()->setPageStep(100);
    mDoubleClickIntervalWidget->slider()->installEventFilter(this);
    DoubleClickTestLabel *doubleClickTestLabel = new DoubleClickTestLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(doubleClickTestLabel, "Mouse", "", "");
    mDoubleClickIntervalWidget->insertWidget(5, doubleClickTestLabel);
}

void MouseUI::setPointerSpeedFrame()
{
    /* Pointer Speed Frame Setting */

    //~ contents_path /Mouse/Pointer move speed
    mPointerSpeedWidget = new SliderWidget(tr("Pointer move speed"), false, this, UkccFrame::Top);
    QLabel *pointerSpeedSlowLabel = mPointerSpeedWidget->leftLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(pointerSpeedSlowLabel, "Mouse", "", "");
    KSlider *pointerSpeedSlider = mPointerSpeedWidget->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(pointerSpeedSlider, "Mouse", "", "");
    pointerSpeedSlider->setAccessibleName(tr("Pointer move speed"));
    QLabel *pointerSpeedFastLabel = mPointerSpeedWidget->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(pointerSpeedFastLabel, "Mouse", "", "");
    mPointerSpeedWidget->setObjectName("Pointer move speed");
    mPointerSpeedWidget->setLeftText(tr("Slow"));
    mPointerSpeedWidget->setRightText(tr("Fast"));
    mPointerSpeedWidget->slider()->setSliderType(KSliderType::SmoothSlider);
    mPointerSpeedWidget->slider()->setMinimum(100);
    mPointerSpeedWidget->slider()->setMaximum(1000);
    mPointerSpeedWidget->slider()->setSingleStep(50);
    mPointerSpeedWidget->slider()->setPageStep(50);
    mPointerSpeedWidget->slider()->installEventFilter(this);
}

void MouseUI::setMouseAccelerationFrame()
{
    /* Mouse Acceleration Frame Setting */
    //~ contents_path /Mouse/Enable mouse acceleration to improve pointer accuracy
    mMouseAccelerationWidget = new SwitchWidget(tr("Enable mouse acceleration to improve pointer accuracy"), this, UkccFrame::None);
    KDK_EXTEND_ALL_INFO_FORMAT(mMouseAccelerationWidget->switchButton(), "Mouse", "", "");
    mMouseAccelerationWidget->setObjectName("Enable mouse acceleration to improve pointer accuracy");
    mMouseAccelerationWidget->switchButton()->setAccessibleName(tr("Enable mouse acceleration to improve pointer accuracy"));
}

void MouseUI::setTailLightEffectFrame()
{
    mMouseTailLightEffectWidget = new SwitchWidget(tr("Drag the mouse to create a light badge with tail light effect"), this, UkccFrame::None);
    KDK_EXTEND_ALL_INFO_FORMAT(mMouseTailLightEffectWidget->switchButton(), "Mouse", "", "");
    mMouseTailLightEffectWidget->switchButton()->setAccessibleName(tr("Drag the mouse to create a light badge with tail light effect"));
}

void MouseUI::setPointerPositionFrame()
{
    /* Pointer Position Frame Setting */
    //~ contents_path /Mouse/Position the pointer position
    mPointerPositionWidget = new ComboxWidget(tr("Position the pointer position"), this, UkccFrame::None);
    QLabel *pointerPositionLabel = mPointerPositionWidget->label();
    KDK_EXTEND_ALL_INFO_FORMAT(pointerPositionLabel, "Mouse", "", "");
    QComboBox *pointerPositionCombox = mPointerPositionWidget->comboBox();
    KDK_EXTEND_ALL_INFO_FORMAT(pointerPositionCombox, "Mouse", "", "");
    mPointerPositionWidget->label()->setAccessibleName(tr("Position the pointer position"));
    mPointerPositionWidget->label()->setBuddy(pointerPositionCombox);
    pointerPositionCombox->setAccessibleName(tr("Position the pointer position"));

    mPointerPositionWidget->setObjectName("Position the pointer position");

    mPointerPositionWidget->comboBox()->addItem(tr("Not targeted"));
    mPointerPositionWidget->comboBox()->addItem(tr("Show pointer position when pressing Ctrl"));
    mPointerPositionWidget->comboBox()->addItem(tr("Shake pointer to display pointer position"));
    mPointerPositionWidget->comboBox()->setCurrentIndex(PositionType::NONE);
}

void MouseUI::setPointerSizeFrame()
{
    /* Pointer Size Frame Setting */

    //~ contents_path /Mouse/Pointer size
    mPointerSizeWidget = new UkccFrame();
    mPointerSizeWidget->setMinimumSize(550, 133);
    mPointerSizeWidget->setMaximumSize(16777215, 133);

    QHBoxLayout *pointerSizeHLayout = new QHBoxLayout();

    m_pPointerSizeLabel = new KLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPointerSizeLabel, "Mouse", "", "");
    //~ contents_path /Mouse/Pointer size
    m_pPointerSizeLabel->setText(tr("Pointer size"));
    m_pPointerSizeLabel->setObjectName("Pointer size");
    m_pPointerSizeLabel->setFixedWidth(214);
    QVBoxLayout *pointerSizeLabelVLayout =  new QVBoxLayout();
    pointerSizeLabelVLayout->setContentsMargins(0,0,0,0);
    pointerSizeLabelVLayout->addWidget(m_pPointerSizeLabel);
    pointerSizeLabelVLayout->addStretch();

    QPixmap pixmap;
    if (QIcon::fromTheme("dark-sense-cursor").isNull()) {
        pixmap = QPixmap(QString(":/img/plugins/mouse/dark-sense-cursor.svg"));
        m_pPointerSizeSmallBtn = new ThemeButton(tr("Small"), pixmap, QSize(SMALL_ICON_SIZE), mPointerSizeWidget);
        m_pPointerSizeMediumBtn = new ThemeButton(tr("Medium"), pixmap, QSize(MEDIUE_ICON_SIZE), mPointerSizeWidget);
        m_pPointerSizeLargeBtn = new ThemeButton(tr("Large"), pixmap, QSize(LARGE_ICON_SIZE), mPointerSizeWidget);
    } else {
        m_pPointerSizeSmallBtn = new ThemeButton(tr("Small"), QIcon::fromTheme("dark-sense-cursor"), QSize(SMALL_ICON_SIZE), mPointerSizeWidget);
        m_pPointerSizeMediumBtn = new ThemeButton(tr("Medium"), QIcon::fromTheme("dark-sense-cursor"), QSize(MEDIUE_ICON_SIZE), mPointerSizeWidget);
        m_pPointerSizeLargeBtn = new ThemeButton(tr("Large"), QIcon::fromTheme("dark-sense-cursor"), QSize(LARGE_ICON_SIZE), mPointerSizeWidget);
    }

    KDK_EXTEND_ALL_INFO_FORMAT(m_pPointerSizeSmallBtn, "Mouse", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPointerSizeMediumBtn, "Mouse", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPointerSizeLargeBtn, "Mouse", "", "");
    m_pPointerSizeLabel->setAccessibleName(tr("Pointer size"));
    m_pPointerSizeLabel->setBuddy(m_pPointerSizeSmallBtn);
    m_pPointerSizeSmallBtn->setAccessibleName(tr("Small"));
    m_pPointerSizeMediumBtn->setAccessibleName(tr("Medium"));
    m_pPointerSizeLargeBtn->setAccessibleName(tr("Large"));

    m_pPointerSizeSmallBtn->setProperty("pointer", "Small");
    m_pPointerSizeSmallBtn->setRect(QSize(POINTER_SIZE_WIDTH, POINTER_SIZE_HEIGHT));
    m_pPointerSizeSmallBtn->setIconCenter();
    m_pPointerSizeMediumBtn->setProperty("pointer", "Medium");
    m_pPointerSizeMediumBtn->setRect(QSize(POINTER_SIZE_WIDTH, POINTER_SIZE_HEIGHT));
    m_pPointerSizeMediumBtn->setIconCenter();
    m_pPointerSizeLargeBtn->setProperty("pointer", "Large");
    m_pPointerSizeLargeBtn->setRect(QSize(POINTER_SIZE_WIDTH, POINTER_SIZE_HEIGHT));
    m_pPointerSizeLargeBtn->setIconCenter();
    m_pPointerSizeGroup = new QButtonGroup(this);
    m_pPointerSizeGroup->setObjectName("Pointer size");
    m_pPointerSizeGroup->addButton(m_pPointerSizeSmallBtn);
    m_pPointerSizeGroup->addButton(m_pPointerSizeMediumBtn);
    m_pPointerSizeGroup->addButton(m_pPointerSizeLargeBtn);
    // Tab 落当前选中指针大小；方向键只移焦点（ThemeButton 内实现）
    m_pPointerSizeSmallBtn->refreshGroupKeyboardNav();
    connect(m_pPointerSizeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, [this](QAbstractButton *btn) {
        if (m_pPointerSizeLabel && btn) {
            m_pPointerSizeLabel->setBuddy(btn);
        }
    });
    QHBoxLayout *radioLayout = new QHBoxLayout();
    radioLayout->setSpacing(40);
    radioLayout->setContentsMargins(0, 0, 0, 0);
    radioLayout->addWidget(m_pPointerSizeSmallBtn);
    radioLayout->addWidget(m_pPointerSizeMediumBtn);
    radioLayout->addWidget(m_pPointerSizeLargeBtn);
    radioLayout->addStretch();
    pointerSizeHLayout->addLayout(pointerSizeLabelVLayout);
    pointerSizeHLayout->setAlignment(Qt::AlignTop);
    pointerSizeHLayout->addSpacing(8);
    pointerSizeHLayout->addLayout(radioLayout);
    pointerSizeHLayout->setContentsMargins(16, 16, 14, 0);

    mPointerSizeWidget->setLayout(pointerSizeHLayout);
}

void MouseUI::setBlinkCursorFrame()
{
    /* Blinking Cursor On Text Frame Setting */
    //~ contents_path /Mouse/Text cursor flashing
    mBlinkingCursorOnTextWidget = new SwitchWidget(tr("Text cursor flashing"), this, UkccFrame::Top);
    KDK_EXTEND_ALL_INFO_FORMAT(mBlinkingCursorOnTextWidget, "Mouse", "", "");
    KSwitchButton *blinkingCursorOnTextSBtn = mBlinkingCursorOnTextWidget->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(blinkingCursorOnTextSBtn, "Mouse", "", "");
    blinkingCursorOnTextSBtn->setAccessibleName(tr("Text cursor flashing"));
    mBlinkingCursorOnTextWidget->setObjectName("Text cursor flashing");
}

void MouseUI::setCursorSpeedFrame()
{
    /* Cursor Speed Frame Setting */

    //~ contents_path /Mouse/Cursor flashing frequency
    mCursorSpeedWidget = new SliderWidget(tr("Cursor flashing frequency"), false, this, UkccFrame::Bottom);
    QLabel *cursorSpeedSlowlabel = mCursorSpeedWidget->leftLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(cursorSpeedSlowlabel, "Mouse", "", "");
    KSlider *cursorSpeedSlider = mCursorSpeedWidget->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(cursorSpeedSlider, "Mouse", "", "");
    cursorSpeedSlider->setAccessibleName(tr("Cursor flashing frequency"));
    QLabel *cursorSpeedFastlabel = mCursorSpeedWidget->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(cursorSpeedFastlabel, "Mouse", "", "");

    mCursorSpeedWidget->setObjectName("Cursor flashing frequency");
    mCursorSpeedWidget->setLeftText(tr("Slow"));
    mCursorSpeedWidget->setRightText(tr("Fast"));
    mCursorSpeedWidget->slider()->setSliderType(KSliderType::SmoothSlider);
    mCursorSpeedWidget->slider()->setMinimum(100);
    mCursorSpeedWidget->slider()->setMaximum(2500);
    mCursorSpeedWidget->slider()->setSingleStep(200);
    mCursorSpeedWidget->slider()->setPageStep(200);

    MyTextEdit *cursorSpeedView = new MyTextEdit();
    KDK_EXTEND_ALL_INFO_FORMAT(cursorSpeedView, "Mouse", "", "");
    cursorSpeedView->setPlaceholderText("Abc");
    cursorSpeedView->setEnabled(false);
    cursorSpeedView->startCursor();
    cursorSpeedView->setFixedWidth(54);
    mCursorSpeedWidget->insertWidget(5, cursorSpeedView);

    mCursorSpeedWidget->installEventFilter(this);
}
