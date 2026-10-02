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

#include "keyboardmain.h"

KeyboardMain::KeyboardMain() : mFirstLoad(true)
{
    pluginName = tr("Keyboard");
    pluginType = DEVICES;
}

KeyboardMain::~KeyboardMain()
{
}

QString KeyboardMain::plugini18nName()
{
    return pluginName;
}

int KeyboardMain::pluginTypes()
{
    return pluginType;
}

QWidget *KeyboardMain::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        keyBoardWidget = new KeyboardUi;
        keyBoardWidget->delayFrame()->slider()->installEventFilter(this);
        keyBoardWidget->speedFrame()->slider()->installEventFilter(this);
        keyBoardWidget->virtualKeyboardSizeFrame()->slider()->installEventFilter(this);
        this->installEventFilter(this);

        // new input method configtool
        inputMethodConfigTool = new InputMethodConfigTool(this);
        bool fcitxExists = inputMethodConfigTool->isFcitxServiceExists();
        keyBoardWidget->setFcitxServiceWarningVisible(!fcitxExists);
        keyBoardWidget->setCurrentInputMethodVisible(fcitxExists);

        keyBoardDbus = new QDBusInterface("org.ukui.ukcc.session",
                                           "/KeyBoard",
                                           "org.ukui.ukcc.session.KeyBoard",
                                           QDBusConnection::sessionBus(), this);

        if (!keyBoardDbus->isValid()) {
            qCritical() << "org.ukui.ukcc.session.KeyBoard DBus error:" << keyBoardDbus->lastError();
        } else {
            initKeyboardStatus();
            initConnection();
            QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                                  "/KeyBoard",
                                                  "org.ukui.ukcc.session.KeyBoard",
                                                  "changed",
                                                  this,
                                                  SLOT(dataChanged(QString)));

            if (isCloudDesktop()) {
                keyBoardWidget->inputMethodEditButton()->setChecked(false);
                keyBoardWidget->inputMethodFrame()->setEnabled(false);
            }
        }
    }

    return keyBoardWidget;
}

const QString KeyboardMain::name() const
{
    return QStringLiteral("Keyboard");
}

bool KeyboardMain::isShowOnHomePage() const
{
    return true;
}

QIcon KeyboardMain::icon() const
{
    return QIcon::fromTheme("input-keyboard-symbolic");
}

bool KeyboardMain::isEnable() const
{
    return true;
}

void KeyboardMain::initConnection()
{
    // 界面设置发生变化
    connect(keyBoardWidget->keyRepeatFrame(), &SwitchWidget::stateChanged, this, &KeyboardMain::keyRepeatSlot);

    connect(keyBoardWidget->delayFrame(), &SliderWidget::valueChanged, this, &KeyboardMain::keyDelaySlot);

    connect(keyBoardWidget->speedFrame(), &SliderWidget::valueChanged, this, &KeyboardMain::keySpeedSlot);

    connect(keyBoardWidget->keyTipsFrame(), &SwitchWidget::stateChanged, this, &KeyboardMain::keyTipsSlot);

    // yangtze 输入法设置
    connect(keyBoardWidget->inputMethodEditButton(), &QPushButton::clicked, this, &KeyboardMain::inputMethodSetSlot);

    connect(keyBoardWidget->showIconOnDesktopFrame(), &SwitchWidget::stateChanged, this, &KeyboardMain::showIconOnDesktopSlot);

    connect(keyBoardWidget->showIconOnTrayFrame(), &SwitchWidget::stateChanged, this, &KeyboardMain::showIconOnTraySlot);

    connect(keyBoardWidget->virtualKeyboardSizeFrame(), &SliderWidget::valueChanged, this, &KeyboardMain::keyboardSizeSlot);

    connect(keyBoardWidget->textSizeFrame(), &ComboxWidget::currentIndexChanged, this, &KeyboardMain::textSizeSlot);

    connect(keyBoardWidget->animationFrame(), &SwitchWidget::stateChanged, this, &KeyboardMain::animationSlot);

    connect(keyBoardWidget->soundEfectsFrame(), &SwitchWidget::stateChanged, this, &KeyboardMain::soundEfectsSlot);

    connect(inputMethodConfigTool, &InputMethodConfigTool::fcitxServiceStatusChanged, this, &KeyboardMain::fcitxServiceStatusChangedSlot);

    connect(inputMethodConfigTool, &InputMethodConfigTool::firstInputMethodChanged, this, &KeyboardMain::fcitxInputMethodChangedSlot);
}

bool KeyboardMain::isCloudDesktop()
{
    QFile file("/sys/class/dmi/id/sys_vendor");
    QString version;
    if(!file.exists()) {
        return false;
    } else {
        file.open(QIODevice::ReadOnly);
        version = file.readAll();
        qDebug() << "file /sys/class/dmi/id/sys_vendor:" << version;
        file.close();
        if(version.contains("sangfor")) {
            return true;
        }
    }

    return false;
}

void KeyboardMain::initKeyboardStatus()
{
    initKeyRepeat();

    initDelay();

    initSpeed();

    initKeyTips();

    initShowIconOnDesktop();

    initShowIconOnTray();

    initKeyboardSize();

    initTextSize();

    initAnimation();

    initSoundEfects();
}

void KeyboardMain::initKeyRepeat()
{
    //设置按键重复状态
    keyBoardWidget->keyRepeatFrame()->blockSignals(true);
    bool checked = keyBoardDbus->property("repeat").toBool();
    keyBoardWidget->keyRepeatFrame()->setChecked(checked);
    keyBoardWidget->keyRepeatFrame()->blockSignals(false);
    setKeyboardVisible(keyBoardWidget->keyRepeatFrame()->isChecked());
}

void KeyboardMain::initDelay()
{
    //设置按键重复的延时
    keyBoardWidget->delayFrame()->slider()->blockSignals(true);
    int value = keyBoardDbus->property("delay").toInt();
    keyBoardWidget->delayFrame()->setValue(value);
    keyBoardWidget->delayFrame()->slider()->blockSignals(false);
}

void KeyboardMain::initSpeed()
{
    //设置按键重复的速度
    keyBoardWidget->speedFrame()->slider()->blockSignals(true);
    int value = keyBoardDbus->property("rate").toInt();
    keyBoardWidget->speedFrame()->setValue(value);
    keyBoardWidget->speedFrame()->slider()->blockSignals(false);

}

void KeyboardMain::initKeyTips()
{
    //设置按键提示状态
    keyBoardWidget->keyTipsFrame()->blockSignals(true);
    bool checked = keyBoardDbus->property("showLockTip").toBool();
    keyBoardWidget->keyTipsFrame()->setChecked(checked);
    keyBoardWidget->keyTipsFrame()->blockSignals(false);
}

void KeyboardMain::initShowIconOnTray()
{
    // 在托盘显示虚拟键盘图标
    keyBoardWidget->showIconOnTrayFrame()->blockSignals(true);
    bool checked = keyBoardDbus->property("showOnTray").toBool();
    keyBoardWidget->showIconOnTrayFrame()->setChecked(checked);
    keyBoardWidget->showIconOnTrayFrame()->blockSignals(false);
}

void KeyboardMain::initShowIconOnDesktop()
{
    // 在桌面显示虚拟键盘图标
    keyBoardWidget->showIconOnDesktopFrame()->blockSignals(true);
    bool checked = keyBoardDbus->property("showOnDesktop").toBool();
    keyBoardWidget->showIconOnDesktopFrame()->setChecked(checked);
    keyBoardWidget->showIconOnDesktopFrame()->blockSignals(false);
}

void KeyboardMain::initKeyboardSize()
{
    // 键盘大小
    keyBoardWidget->virtualKeyboardSizeFrame()->blockSignals(true);
    int value = keyBoardDbus->property("keyboardSize").toInt();
    keyBoardWidget->virtualKeyboardSizeFrame()->setValue(value);
    keyBoardWidget->virtualKeyboardSizeFrame()->setRightText(QString::number(value)+"%");
    keyBoardWidget->virtualKeyboardSizeFrame()->blockSignals(false);
}

void KeyboardMain::initTextSize()
{
    //初始化按键文本大小
    keyBoardWidget->textSizeFrame()->comboBox()->blockSignals(true);
    int currentTextSize = keyBoardDbus->property("textSize").toInt();
    if (currentTextSize != SMALLTEXTSIZE && currentTextSize != MEDIUMTEXTSIZE && currentTextSize != LARGETEXTSIZE) {
        keyBoardDbus->call("resetKey", "");
        currentTextSize = keyBoardDbus->property("").toInt();
    }

    keyBoardWidget->textSizeFrame()->setCurrentIndex(keyBoardWidget->textSizeFrame()->comboBox()->findData(currentTextSize));

    keyBoardWidget->textSizeFrame()->comboBox()->blockSignals(false);
}

void KeyboardMain::initAnimation()
{
    // 动画效果
    keyBoardWidget->animationFrame()->blockSignals(true);
    bool checked = keyBoardDbus->property("animation").toBool();
    keyBoardWidget->animationFrame()->setChecked(checked);
    keyBoardWidget->animationFrame()->blockSignals(false);
}

void KeyboardMain::initSoundEfects()
{
    // 按键输入音效
    keyBoardWidget->soundEfectsFrame()->blockSignals(true);
    bool checked = keyBoardDbus->property("soundEffect").toBool();
    keyBoardWidget->soundEfectsFrame()->setChecked(checked);
    keyBoardWidget->soundEfectsFrame()->blockSignals(false);
}

void KeyboardMain::setKeyboardVisible(bool checked)
{
    keyBoardWidget->delayFrame()->setVisible(checked);
    keyBoardWidget->speedFrame()->setVisible(checked);
    keyBoardWidget->inputTestFrame()->setVisible(checked);
}

void KeyboardMain::dataChanged(QString key)
{
    if (key == QString("repeat")) {
        initKeyRepeat();
    } else if (key == QString("delay")) {
        initDelay();
    } else if (key == QString("rate")) {
        initSpeed();
    } else if (key == QString("show-lock-tip")) {
        initKeyTips();
    } else if (key == QString("showOnDesktop")) {
        initShowIconOnDesktop();
    } else if (key == QString("showOnTray")) {
        initShowIconOnTray();
    } else if (key == QString("keyboardSize")) {
        initKeyboardSize();
    } else if (key == QString("textSize")) {
        initTextSize();
    } else if (key == QString("animation")) {
        initAnimation();
    } else if (key == QString("soundEffect")) {
        initSoundEfects();
    }
}

/* Slot */
void KeyboardMain::keyRepeatSlot(bool checked)
{
    setKeyboardVisible(checked);
    keyBoardDbus->call("setKeyRepeat", checked);
    UkccCommon::buriedSettings(name(), keyBoardWidget->keyRepeatFrame()->objectName(), QString("settings"), checked ? "true":"false");
}

void KeyboardMain::keyDelaySlot(int value)
{
    keyBoardDbus->call("setDelay", value);
}

void KeyboardMain::keySpeedSlot(int value)
{
    keyBoardDbus->call("setSpeed", value);
}

void KeyboardMain::keyTipsSlot(bool checked)
{
    keyBoardDbus->call("setKeyTips", checked);
    UkccCommon::buriedSettings(name(), keyBoardWidget->keyTipsFrame()->objectName(), QString("settings"), checked ? "true":"false");
}

void KeyboardMain::inputMethodSetSlot()
{
#ifdef YANGTZE
    UkccCommon::buriedSettings(name(), keyBoardWidget->inputMethodPushButton()->objectName(), QString("settings"));
#else
    UkccCommon::buriedSettings(name(), keyBoardWidget->inputMethodFrame()->objectName(), QString("settings"));
#endif

    // show input method configtool window
    if (inputMethodConfigTool == nullptr) {
        return;
    }

    if (!inputMethodConfigTool->isFcitxServiceExists())
    {
        keyBoardWidget->showFcitxServiceWarningDialog();
        return;
    }

    inputMethodConfigTool->showWindow();
}

void KeyboardMain::fcitxServiceStatusChangedSlot(bool status)
{
    if (keyBoardWidget == nullptr) {
        return;
    }

    keyBoardWidget->setFcitxServiceWarningVisible(!status);
    keyBoardWidget->setCurrentInputMethodVisible(status);
}

void KeyboardMain::fcitxInputMethodChangedSlot(const QString &firstImName)
{
    if (keyBoardWidget == nullptr) {
        return;
    }

    keyBoardWidget->setCurrentInputMethodInfo(firstImName);
}

void KeyboardMain::showIconOnTraySlot(bool checked)
{
    keyBoardDbus->call("setIconOnTray", checked);
    UkccCommon::buriedSettings(name(), keyBoardWidget->showIconOnTrayFrame()->objectName(), QString("settings"), checked ? "true":"false");
}

void KeyboardMain::showIconOnDesktopSlot(bool checked)
{
    keyBoardDbus->call("setIconOnDesktop", checked);
    UkccCommon::buriedSettings(name(), keyBoardWidget->showIconOnDesktopFrame()->objectName(), QString("settings"), checked ? "true":"false");
}

void KeyboardMain::keyboardSizeSlot(int value)
{
    keyBoardDbus->call("setKeyboardSize", value);
    keyBoardWidget->virtualKeyboardSizeFrame()->setRightText(QString::number(value)+"%");
}

void KeyboardMain::textSizeSlot(int index)
{
    int value = keyBoardWidget->textSizeFrame()->comboBox()->itemData(index).toInt();
    keyBoardDbus->call("setTextSize", value);

    UkccCommon::buriedSettings(name(), keyBoardWidget->textSizeFrame()->objectName(), QString("settings"), QString::number(value));
}

void KeyboardMain::animationSlot(bool checked)
{
    keyBoardDbus->call("setAnimation", checked);
    UkccCommon::buriedSettings(name(), keyBoardWidget->animationFrame()->objectName(), QString("settings"), checked ? "true":"false");
}

void KeyboardMain::soundEfectsSlot(bool checked)
{
    keyBoardDbus->call("setSoundEffect", checked);
    UkccCommon::buriedSettings(name(), keyBoardWidget->soundEfectsFrame()->objectName(), QString("settings"), checked ? "true":"false");
}

bool KeyboardMain::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        if (watched == keyBoardWidget->delayFrame()->slider()) {
            int value = keyBoardWidget->delayFrame()->value();
            UkccCommon::buriedSettings(name(), keyBoardWidget->delayFrame()->objectName(), QString("settings"), QString::number(value));
        } else if (watched == keyBoardWidget->speedFrame()->slider()) {
            int value = keyBoardWidget->speedFrame()->value();
            UkccCommon::buriedSettings(name(), keyBoardWidget->speedFrame()->objectName(), QString("settings"), QString::number(value));
        } else if (watched == keyBoardWidget->virtualKeyboardSizeFrame()->slider()) {
            int value = keyBoardWidget->virtualKeyboardSizeFrame()->value();
            UkccCommon::buriedSettings(name(), keyBoardWidget->virtualKeyboardSizeFrame()->objectName(), QString("settings"), QString::number(value));
        }
    }

    if(event->type()== UkccArgEvent::TYPE) {
        UkccArgEvent *str = dynamic_cast<UkccArgEvent* >(event);
        if (str != nullptr) {
            qDebug() << "event string = " << str->str();
            if (str->str() == "inputmethod") {
                // show input method configtool window
                if (inputMethodConfigTool == nullptr) {
                    return QObject::eventFilter(watched, event);
                }
                if (!inputMethodConfigTool->isFcitxServiceExists())
                {
                    keyBoardWidget->showFcitxServiceWarningDialog();
                    return QObject::eventFilter(watched, event);
                }
                inputMethodConfigTool->showWindow();
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

