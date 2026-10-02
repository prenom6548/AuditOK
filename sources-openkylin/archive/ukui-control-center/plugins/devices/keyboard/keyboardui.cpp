/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "keyboardui.h"

#include <kysdk/applications/accessinfohelper.h>
#include "ukcccommon.h"
using namespace ukcc;

KeyboardUi::KeyboardUi(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 0, 0);
    mVlayout->setSpacing(8);
    initUI();
}

KeyboardUi::~KeyboardUi()
{

}

void KeyboardUi::setFcitxServiceWarningVisible(bool isVisible)
{
    mInputMethodWidget->setVisible(isVisible);
}

void KeyboardUi::showFcitxServiceWarningDialog()
{
  QMessageBox inputMethodWarningDialog(this);
  inputMethodWarningDialog.setWindowTitle("");
  QString warningTitle =
      tr("The input method framework is not installed in the system currently, "
         "so the input function cannot be used normally.");
  QString warningMessage = tr("Please install the input method framework Fcitx5 "
                             "and then restart the system.");

  inputMethodWarningDialog.setText(warningTitle);
  inputMethodWarningDialog.setInformativeText(warningMessage);

  inputMethodWarningDialog.setIcon(QMessageBox::Warning);
  inputMethodWarningDialog.setStandardButtons(QMessageBox::Ok);
  inputMethodWarningDialog.setButtonText(QMessageBox::Ok, QString(tr("OK")));
  inputMethodWarningDialog.setDefaultButton(QMessageBox::Ok);

  inputMethodWarningDialog.exec();
  return;
}

void KeyboardUi::initUI()
{
    mKeyboardFrame = new SettingGroup(this);
    mKeyboardSetTitleLabel = new TitleLabel(this);
    //~ contents_path /Keyboard/Keyboard
    mKeyboardSetTitleLabel->setText(tr("Keyboard"));
    mKeyboardSetTitleLabel->setContentsMargins(16, 0, 0, 0);

    setKeyRepeatFrame();
    setDelayFrame();
    setSpeedFrame();
    setInputTestFrame();
    setKeyTipsFrame();

    /* add widget */
    mKeyboardFrame->addWidget(mKeyRepeatFrame);
    mKeyboardFrame->addWidget(mDelayFrame);
    mKeyboardFrame->addWidget(mSpeedFrame);
    mKeyboardFrame->addWidget(mInputTestFrame);
#ifdef YANGTZE
    mKeyboardFrame->insertWidget(4, mKeyTipsFrame);
#endif

    mVirtualKeyboardFrame = new SettingGroup(this);
    mVirtualKeyboardSetTitleLabel = new TitleLabel(this);
    //~ contents_path /Keyboard/Virtual Keyboard
    mVirtualKeyboardSetTitleLabel->setText(tr("Virtual Keyboard"));
    mVirtualKeyboardSetTitleLabel->setContentsMargins(16, 0, 0, 0);

    setShowIconOnDesktopFrame();
    setShowIconOnTrayFrame();
    setVirtualKeyboardSizeFrame();
    setTextSizeFrame();
    setAnimationFrame();
    setSoundEfectsFrame();

    mVirtualKeyboardFrame->addWidget(mShowIconOnTrayFrame);
    mVirtualKeyboardFrame->addWidget(mShowIconOnDesktopFrame);
    mVirtualKeyboardFrame->addWidget(mVirtualKeyboardSizeFrame);
    mVirtualKeyboardFrame->addWidget(mTextSizeFrame);
    mVirtualKeyboardFrame->addWidget(mAnimationFrame);
    mVirtualKeyboardFrame->addWidget(mSoundEfectsFrame);

    mInputTitleLabel = new TitleLabel(this);
    //~ contents_path /Keyboard/Input Method
    mInputTitleLabel->setText(tr("Input Method"));
    mInputTitleLabel->setContentsMargins(16, 0, 0, 0);
    setInputMethodFrame();

    mVlayout->addWidget(mKeyboardSetTitleLabel);
    mVlayout->addWidget(mKeyboardFrame);
#ifndef YANGTZE
    mVlayout->addWidget(mKeyTipsFrame);
    mVlayout->addSpacing(40);
#endif

    mVlayout->addWidget(mVirtualKeyboardSetTitleLabel);
    mVlayout->addWidget(mVirtualKeyboardFrame);

#ifndef YANGTZE
    mVlayout->addSpacing(40);
#endif

    mVlayout->addWidget(mInputTitleLabel);
    mVlayout->addWidget(mInputMethodEditFrame);

    mVlayout->addStretch();

    // 部分功能暂时隐藏，等虚拟键盘提供接口支持后显示
    mShowIconOnTrayFrame->setVisible(false);
    mTextSizeFrame->setVisible(false);
    mSoundEfectsFrame->setVisible(false);

#ifdef YANGTZE
    mVirtualKeyboardSetTitleLabel->setVisible(false);
    mVirtualKeyboardFrame->setVisible(false);
    mInputTitleLabel->setVisible(false);
    mInputMethodSetFrame->setVisible(false);
#else
#endif

    KDK_EXTEND_ALL_INFO_FORMAT(mKeyboardSetTitleLabel, "Keyboard", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mInputTitleLabel, "Keyboard", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mVirtualKeyboardSetTitleLabel, "Keyboard", "", "");
}

void KeyboardUi::setKeyRepeatFrame()
{
    /* Key Repeat */
    //~ contents_path /Keyboard/Repeatedly inputing when pressing keys
    mKeyRepeatFrame = new SwitchWidget(tr("Repeatedly inputing when pressing keys"), this, UkccFrame::None);
    mKeyRepeatFrame->setObjectName("Repeatedly inputing when pressing keys");

    KSwitchButton *keyRepeatSBtn = mKeyRepeatFrame->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(keyRepeatSBtn, "Keyboard", "", "");
    keyRepeatSBtn->setAccessibleName(tr("Repeatedly inputing when pressing keys"));
}

void KeyboardUi::setDelayFrame()
{
    /* delay */
    //~ contents_path /Keyboard/Response repetition interval duration
    mDelayFrame = new SliderWidget(tr("Response repetition interval duration"), false, this);
    mDelayFrame->slider()->setSliderType(KSliderType::StepSlider);
    mDelayFrame->setObjectName("Delay");
    mDelayFrame->setLeftText(tr("Short"));
    mDelayFrame->setRightText(tr("Long"));
    mDelayFrame->slider()->setMinimum(200);
    mDelayFrame->slider()->setMaximum(2100);
    mDelayFrame->slider()->setPageStep(20);
    mDelayFrame->slider()->setSingleStep(20);
    mDelayFrame->slider()->installEventFilter(this);

    KSlider *delaySlider = mDelayFrame->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(delaySlider, "Keyboard", "", "");
    delaySlider->setAccessibleName(tr("Response repetition interval duration"));
    QLabel *delayShortLabel = mDelayFrame->leftLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(delayShortLabel, "Keyboard", "", "");
    QLabel *delayLongLabel = mDelayFrame->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(delayLongLabel, "Keyboard", "", "");
}

void KeyboardUi::setSpeedFrame()
{
    /* Speed */
    //~ contents_path /Keyboard/Repetitive input speed
    mSpeedFrame = new SliderWidget(tr("Repetitive input speed"), false, this);
    mSpeedFrame->setObjectName("Repetitive input speed");
    mSpeedFrame->setLeftText(tr("Slow"));
    mSpeedFrame->setRightText(tr("Fast"));
    mSpeedFrame->slider()->setMinimum(10);
    mSpeedFrame->slider()->setMaximum(110);
    mSpeedFrame->slider()->setPageStep(1);
    mSpeedFrame->slider()->installEventFilter(this);
    KSlider *speedSlider = mSpeedFrame->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(speedSlider, "Keyboard", "", "");
    speedSlider->setAccessibleName(tr("Repetitive input speed"));
    QLabel *speedShortLabel = mSpeedFrame->leftLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(speedShortLabel, "Keyboard", "", "");
    QLabel *speedLongLabel = mSpeedFrame->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(speedLongLabel, "Keyboard", "", "");
}

void KeyboardUi::setInputTestFrame()
{
    /* Input test */
    //~ contents_path /Keyboard/Repeated input test
    mInputTestFrame = new LineEditWidget(tr("Repeated input test"), this);
    mInputTestFrame->lineEdit()->setPlaceholderText(tr("Test the repetition speed of input content"));

    QLineEdit *inputTestLineedit = mInputTestFrame->lineEdit();
    KDK_EXTEND_ALL_INFO_FORMAT(inputTestLineedit, "Keyboard", "", "");
    inputTestLineedit->setAccessibleName(tr("Test the repetition speed of input content"));
}

void KeyboardUi::setKeyTipsFrame()
{
    /* key tips */
    //~ contents_path /Keyboard/Prompt when pressing the Caps key
    mKeyTipsFrame = new SwitchWidget(tr("Prompt when pressing the Caps key"), this, UkccFrame::Around);
    mKeyTipsFrame->setObjectName("Prompt when pressing the Caps key");
    KSwitchButton *keyTipsSBtn = mKeyTipsFrame->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(keyTipsSBtn, "Keyboard", "", "");
    keyTipsSBtn->setAccessibleName(tr("Prompt when pressing the Caps key"));
}

void KeyboardUi::setInputMethodFrame()
{
    mInputMethodEditFrame = new UkccFrame(this);

    // 输入法label
    mInputMethodLabel = new KLabel(this);
    mInputMethodLabel->setText(tr("Input Method"));

    // 编辑按钮
    mInputMethodEditBtn = new QPushButton(this);
    mInputMethodEditBtn->setText(tr("Edit"));
    mInputMethodEditBtn->setObjectName("Edit");
    mInputMethodEditBtn->setMinimumWidth(96);
    mInputMethodEditBtn->setMaximumWidth(96);

    // 警告提示布局
    mInputMethodWidget = new QWidget();
    mInputMethodWarningLayout = new QHBoxLayout(mInputMethodWidget);
    mInputMethodWarningLayout->setSpacing(4);
    mInputMethodWarningLayout->setContentsMargins(0, 0, 0, 0);
    mInputMethodWarningLayout->setAlignment(Qt::AlignVCenter);

    // 警告icon
    mInputMethodServiceWarningIcon = new KLabel(mInputMethodWidget);
    QIcon warningIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
    mInputMethodServiceWarningIcon->setFixedSize(16, 16);
    mInputMethodServiceWarningIcon->setPixmap(warningIcon.pixmap(16, 16));

    // 警告label
    mInputMethodServiceWarning = new LightLabel();
    mInputMethodServiceWarning->setText(tr("No input method framework was detected"));

    mInputMethodWarningLayout->addWidget(mInputMethodServiceWarningIcon);
    mInputMethodWarningLayout->addWidget(mInputMethodServiceWarning);

    // 当前输入法显示布局（与警告布局互斥）
    mCurrentInputMethodWidget = new QWidget();
    mCurrentInputMethodLayout = new QHBoxLayout(mCurrentInputMethodWidget);
    mCurrentInputMethodLayout->setSpacing(4);
    mCurrentInputMethodLayout->setContentsMargins(0, 0, 0, 0);

    // 当前输入法名称label
    mFirstInputMethodInfo = new LightLabel();
    mCurrentInputMethodLayout->addWidget(mFirstInputMethodInfo);

    // 默认隐藏当前输入法显示，显示警告
    mCurrentInputMethodWidget->setVisible(false);

    // 整体布局
    mInputMethodHLayout = new QHBoxLayout();
    mInputMethodHLayout->setSpacing(16);
    mInputMethodHLayout->setContentsMargins(16, 0, 16, 0);
    mInputMethodHLayout->addWidget(mInputMethodLabel);
    mInputMethodHLayout->addStretch();
    mInputMethodHLayout->addWidget(mInputMethodWidget);
    mInputMethodHLayout->addWidget(mCurrentInputMethodWidget);
    mInputMethodHLayout->addWidget(mInputMethodEditBtn);

    mInputMethodEditFrame->setMinimumSize(QSize(550, 60));
    mInputMethodEditFrame->setMaximumSize(QSize(16777215, 60));
    mInputMethodEditFrame->setFrameShape(QFrame::WinPanel);
    mInputMethodEditFrame->setLayout(mInputMethodHLayout);

    KDK_EXTEND_ALL_INFO_FORMAT(mInputMethodLabel, "Keyboard", "", "");
    // KDK 之后覆盖可读名；buddy 提供「输入法」，按钮名只放「编辑」，避免读两遍
    mInputMethodLabel->setAccessibleName(tr("Input Method"));
    mInputMethodLabel->setBuddy(mInputMethodEditBtn);
    KDK_EXTEND_ALL_INFO_FORMAT(mInputMethodEditBtn, "Keyboard", "", "");
    mInputMethodEditBtn->setAccessibleName(tr("Edit"));
    mInputMethodEditBtn->setAccessibleDescription(QString());
    KDK_EXTEND_ALL_INFO_FORMAT(mInputMethodServiceWarningIcon, "Keyboard", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mInputMethodServiceWarning, "Keyboard", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(mFirstInputMethodInfo, "Keyboard", "", "");
}

void KeyboardUi::setShowIconOnTrayFrame()
{
    mShowIconOnTrayFrame = new SwitchWidget(tr("Show icon on tray"), this, UkccFrame::None);
    mShowIconOnTrayFrame->setObjectName("Show icon on tray");
    KSwitchButton *showIconOnTraySBtn = mShowIconOnTrayFrame->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(showIconOnTraySBtn, "Keyboard", "", "");
}

void KeyboardUi::setShowIconOnDesktopFrame()
{
    //~ contents_path /Keyboard/Show floating ball on the desktop
    mShowIconOnDesktopFrame = new SwitchWidget(tr("Show floating ball on the desktop"), this, UkccFrame::None);
    mShowIconOnDesktopFrame->setObjectName("Show floating ball on the desktop");
    KSwitchButton *showIconOnDesktopSBtn = mShowIconOnDesktopFrame->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(showIconOnDesktopSBtn, "Keyboard", "", "");
    showIconOnDesktopSBtn->setAccessibleName(tr("Show floating ball on the desktop"));
}

void KeyboardUi::setVirtualKeyboardSizeFrame()
{
    //~ contents_path /Keyboard/Keyboard Size
    mVirtualKeyboardSizeFrame = new SliderWidget(tr("Keyboard Size"), false, this);
    mVirtualKeyboardSizeFrame->setObjectName("Keyboard Size");
    mVirtualKeyboardSizeFrame->slider()->setRange(20,100);
    mVirtualKeyboardSizeFrame->leftLabel()->setFixedWidth(24);
    mVirtualKeyboardSizeFrame->leftLabel()->setPixmap(QIcon::fromTheme("preferences-desktop-keyboard-symbolic"), 16, 16);
    KSlider *virtualKeyboardSizeSlider = mVirtualKeyboardSizeFrame->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(virtualKeyboardSizeSlider, "Keyboard", "", "");
    virtualKeyboardSizeSlider->setAccessibleName(tr("Keyboard Size"));
    QLabel *virtualKeyboardSizeLabel = mVirtualKeyboardSizeFrame->rightLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(virtualKeyboardSizeLabel, "Keyboard", "", "");
}

void KeyboardUi::setTextSizeFrame()
{
    mTextSizeFrame = new ComboxWidget(tr("Text size"), this);
    mTextSizeFrame->setObjectName("Text size");
    mTextSizeFrame->comboBox()->addItem(tr("Small"), SMALLTEXTSIZE);
    mTextSizeFrame->comboBox()->addItem(tr("Medium"), MEDIUMTEXTSIZE);
    mTextSizeFrame->comboBox()->addItem(tr("Large"), LARGETEXTSIZE);
    QLabel *textSizeLabel = mTextSizeFrame->label();
    KDK_EXTEND_ALL_INFO_FORMAT(textSizeLabel, "Keyboard", "", "");
    QComboBox *textSizeCombox = mTextSizeFrame->comboBox();
    KDK_EXTEND_ALL_INFO_FORMAT(textSizeCombox, "Keyboard", "", "");
    textSizeCombox->setAccessibleName(tr("Text size"));
}

void KeyboardUi::setAnimationFrame()
{
    //~ contents_path /Keyboard/Animation
    mAnimationFrame = new SwitchWidget(tr("Animation"), this, UkccFrame::None);
    mAnimationFrame->setObjectName("Animation");
    KSwitchButton *animationSBtn = mAnimationFrame->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(animationSBtn, "Keyboard", "", "");
    animationSBtn->setAccessibleName(tr("Animation"));
}

void KeyboardUi::setSoundEfectsFrame()
{
    mSoundEfectsFrame = new SwitchWidget(tr("Input sound effect"), this, UkccFrame::None);
    mSoundEfectsFrame->setObjectName("Input sound effect");
    KSwitchButton *soundEfectsSBtn = mSoundEfectsFrame->switchButton();
    KDK_EXTEND_ALL_INFO_FORMAT(soundEfectsSBtn, "Keyboard", "", "");
    soundEfectsSBtn->setAccessibleName(tr("Input sound effect"));
}

void KeyboardUi::setCurrentInputMethodInfo(const QString &name)
{
    QString text = name + tr("etc.");
    mFirstInputMethodInfo->setText(text);
    QFontMetrics fontMetrics(mFirstInputMethodInfo->font());
    int textWidth = fontMetrics.horizontalAdvance(text);
    mFirstInputMethodInfo->setMinimumWidth(textWidth);
}

void KeyboardUi::setCurrentInputMethodVisible(bool isVisible)
{
    mCurrentInputMethodWidget->setVisible(isVisible);
}
