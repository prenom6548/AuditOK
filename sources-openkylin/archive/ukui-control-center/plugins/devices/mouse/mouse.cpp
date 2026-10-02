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
#include "mouse.h"

Mouse::Mouse() : mFirstLoad(true)
{
    pluginName = tr("Mouse");
    pluginType = DEVICES;
}

Mouse::~Mouse()
{
}

QString Mouse::plugini18nName()
{
    return pluginName;
}

int Mouse::pluginTypes()
{
    return pluginType;
}

QWidget *Mouse::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        mouseWidget = new MouseUI;
        mouseWidget->wheelSpeedWidget()->slider()->installEventFilter(this);
        mouseWidget->doubleClickIntervalWidget()->slider()->installEventFilter(this);
        mouseWidget->pointerSpeedWidget()->slider()->installEventFilter(this);
        mouseWidget->cursorSpeedWidget()->slider()->installEventFilter(this);
        mouseWidget->setAttribute(Qt::WA_DeleteOnClose);
        mouseDbus = new QDBusInterface("org.ukui.ukcc.session",
                                       "/Mouse",
                                       "org.ukui.ukcc.session.Mouse",
                                       QDBusConnection::sessionBus(), this);

        if (!mouseDbus->isValid()) {
            qCritical() << "org.ukui.ukcc.session.KeyBoard DBus error:" << mouseDbus->lastError();
        } else {
            initMouseStatus();
            initConnection();
            QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                                  "/Mouse",
                                                  "org.ukui.ukcc.session.Mouse",
                                                  "changed",
                                                  this,
                                                  SLOT(dataChanged(QString)));

        }
    }
    return mouseWidget;
}

const QString Mouse::name() const
{
    return QStringLiteral("Mouse");
}

bool Mouse::isShowOnHomePage() const
{
    return true;
}

QIcon Mouse::icon() const
{
    return QIcon::fromTheme("input-mouse-symbolic");
}

bool Mouse::isEnable() const
{
    return true;
}

void Mouse::initMouseStatus()
{
    initDominantHand();
    initScrollDirection();
    initWheelSpeed();
    initDoubleClickInterval();
    initPointerSpeed();
    initMouseAcceleration();
    initTailLightEffect();
    initPointerPosition();
    initPointerSize();
    initBlinkingCursor();
    initCursorSpeed();
}

void Mouse::initDominantHand()
{
    //初始化惯用手, 左手：右键为主键，右手：左键为主键
    mouseWidget->dominantHandWidget()->buttonGroup()->blockSignals(true);
    bool currentDominantHand = mouseDbus->property("dominantHand").toBool();
    if (currentDominantHand == true) {
        mouseWidget->dominantHandRightRadio()->setChecked(true);
    } else {
        mouseWidget->dominantHandLeftRadio()->setChecked(true);
    }
    mouseWidget->dominantHandWidget()->buttonGroup()->blockSignals(false);
}

void Mouse::initScrollDirection()
{
    // 初始化滚动方向 true:界面与鼠标滚轮滚动方向相反(反向),false:界面与滚轮滚动方向相同(正向)
    mouseWidget->scrollDirectionWdiget()->buttonGroup()->blockSignals(true);

    bool currentScrollDirection = mouseDbus->property("scrollDirection").toBool();
    if (currentScrollDirection) {
        mouseWidget->scrollDirectionReverseRadio()->setChecked(true);
    } else {
        mouseWidget->scrollDirectionForwardRadio()->setChecked(true);
    }
    mouseWidget->scrollDirectionWdiget()->buttonGroup()->blockSignals(false);

}
void Mouse::initWheelSpeed()
{
    //初始化滚轮速度
    mouseWidget->wheelSpeedWidget()->slider()->blockSignals(true);
    int value = mouseDbus->property("wheelSpeed").toInt();
    if (value < 1 || value >10) {
        mouseDbus->call("resetKey", "wheelSpeed");
    }
    mouseWidget->wheelSpeedWidget()->setValue(mouseDbus->property("wheelSpeed").toInt());
    mouseWidget->wheelSpeedWidget()->slider()->blockSignals(false);

}
void Mouse::initDoubleClickInterval()
{
    //初始化双击间隔时长
    mouseWidget->doubleClickIntervalWidget()->slider()->blockSignals(true);
    mouseWidget->doubleClickIntervalWidget()->setValue(mouseWidget->doubleClickIntervalWidget()->slider()->maximum()
                                                       - mouseDbus->property("doubleClick").toInt()
                                                       + mouseWidget->doubleClickIntervalWidget()->slider()->minimum());
    mouseWidget->doubleClickIntervalWidget()->slider()->blockSignals(false);
}
void Mouse::initPointerSpeed()
{
    //初始化指针速度
    mouseWidget->pointerSpeedWidget()->slider()->blockSignals(true);
    mouseWidget->pointerSpeedWidget()->setValue(static_cast<int>(mouseDbus->property("pointerSpeed").toDouble()*100));
    mouseWidget->pointerSpeedWidget()->slider()->blockSignals(false);
}
void Mouse::initMouseAcceleration()
{
    //初始化鼠标加速
    mouseWidget->mouseAccelerationWidget()->blockSignals(true);
    mouseWidget->mouseAccelerationWidget()->setChecked(mouseDbus->property("mouseAcceleration").toBool());
    mouseWidget->mouseAccelerationWidget()->blockSignals(false);
}

void Mouse::initTailLightEffect()
{
    QDBusReply<bool> effectReply = mouseDbus->call("isEffectSupported");
    bool m_isSupportEffect = effectReply.value();
    qDebug() << Q_FUNC_INFO << "isEffectSupported valid" << effectReply.isValid() << effectReply.value();
    mouseWidget->tailLightEffectWidget()->setVisible(m_isSupportEffect);

    mouseWidget->tailLightEffectWidget()->blockSignals(true);
    mouseWidget->tailLightEffectWidget()->setChecked(mouseDbus->property("tailLightEffect").toBool());
    mouseWidget->tailLightEffectWidget()->blockSignals(false);
}

void Mouse::initPointerPosition()
{
    //初始化按 ctrl 键时显示指针位置
    mouseWidget->pointerPositionWidget()->blockSignals(true);

    bool ctrlLocalePointer = mouseDbus->property("pointerPosition").toBool();
    if (ctrlLocalePointer) {
        mouseWidget->pointerPositionWidget()->setCurrentIndex(PositionType::PRESSCTRL);
    }

    bool shakeCursor = mouseDbus->property("shakeCursor").toBool();
    if (shakeCursor) {
        mouseWidget->pointerPositionWidget()->setCurrentIndex(PositionType::SHARKMOUSE);
    }

    mouseWidget->pointerPositionWidget()->blockSignals(false);
}

void Mouse::initPointerSize()
{
    //初始化指针大小
    mouseWidget->pointerSizeButtonGroup()->blockSignals(true);
    int currentPointerSize = mouseDbus->property("pointerSize").toInt();
    if (currentPointerSize != SMALLPOINTER && currentPointerSize != MEDIUMPOINTER && currentPointerSize != LARGEPOINTER) {
        mouseDbus->call("resetKey", "pointerSize");
        currentPointerSize = mouseDbus->property("pointerSize").toInt();
    }

    qDebug() << "current pointer size:" << currentPointerSize;
    if (currentPointerSize == SMALLPOINTER) {
        mouseWidget->pointerSizeSmallRadio()->setBtnClicked(true);
        mouseWidget->pointerSizeSmallRadio()->clicked();
        mouseWidget->pointerSizeButtonGroup()->buttonClicked(mouseWidget->pointerSizeSmallRadio());
    } else if (currentPointerSize == MEDIUMPOINTER) {
        mouseWidget->pointerSizeMediumRadio()->setBtnClicked(true);
        mouseWidget->pointerSizeMediumRadio()->clicked();
        mouseWidget->pointerSizeButtonGroup()->buttonClicked(mouseWidget->pointerSizeMediumRadio());
    } else if (currentPointerSize == LARGEPOINTER) {
        mouseWidget->pointerSizeLargeRadio()->setBtnClicked(true);
        mouseWidget->pointerSizeLargeRadio()->clicked();
        mouseWidget->pointerSizeButtonGroup()->buttonClicked(mouseWidget->pointerSizeLargeRadio());
    }
    mouseWidget->pointerSizeButtonGroup()->blockSignals(false);
}
void Mouse::initBlinkingCursor()
{
    //初始化文本区域光标闪烁
    mouseWidget->blinkingCursorOnTextWidget()->blockSignals(true);
    mouseWidget->blinkingCursorOnTextWidget()->setChecked(mouseDbus->property("cursorBlink").toBool());
    mouseWidget->blinkingCursorOnTextWidget()->blockSignals(false);
    mouseWidget->cursorSpeedWidget()->setVisible(mouseDbus->property("cursorBlink").toBool());
}
void Mouse::initCursorSpeed()
{
    //初始化光标速度
    mouseWidget->cursorSpeedWidget()->slider()->blockSignals(true);
    mouseWidget->cursorSpeedWidget()->setValue(mouseWidget->cursorSpeedWidget()->slider()->maximum() - mouseDbus->property("cursorSpeed").toInt()
                                 + mouseWidget->cursorSpeedWidget()->slider()->minimum());
    mouseWidget->cursorSpeedWidget()->slider()->blockSignals(false);
}

void Mouse::initConnection()
{

    connect(mouseWidget->pointerSizeButtonGroup(), QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        emit mouseWidget->pointerSizeMediumRadio()->btnCheckedSignals(eBtn == mouseWidget->pointerSizeMediumRadio() ? true : false);
    });

    connect(mouseWidget->pointerSizeButtonGroup(), QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        emit mouseWidget->pointerSizeSmallRadio()->btnCheckedSignals(eBtn == mouseWidget->pointerSizeSmallRadio() ? true : false);
    });

    connect(mouseWidget->pointerSizeButtonGroup(), QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        emit mouseWidget->pointerSizeLargeRadio()->btnCheckedSignals(eBtn == mouseWidget->pointerSizeLargeRadio() ? true : false);
    });


    connect(mouseWidget->dominantHandWidget(), QOverload<int>::of(&RadioButtonWidget::buttonClicked),
            this, &Mouse::dominantHandSlot);

    connect(mouseWidget->scrollDirectionWdiget(), QOverload<int>::of(&RadioButtonWidget::buttonClicked),
            this, &Mouse::scrollDirectionSlot);

    connect(mouseWidget->wheelSpeedWidget(), &SliderWidget::valueChanged, this, &Mouse::wheelSpeedSlot);

    connect(mouseWidget->doubleClickIntervalWidget(), &SliderWidget::valueChanged , this, &Mouse::doubleClickIntervalSlot);

    connect(mouseWidget->pointerSpeedWidget(), &SliderWidget::valueChanged, this, &Mouse::pointerSpeedSlot);

    connect(mouseWidget->tailLightEffectWidget(), &SwitchWidget::stateChanged, this, &Mouse::tailLightEffectSlot);

    connect(mouseWidget->mouseAccelerationWidget(), &SwitchWidget::stateChanged, this, &Mouse::mouseAccelerationSlot);

    connect(mouseWidget->pointerPositionWidget()->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Mouse::pointerPositionSlot);

    connect(mouseWidget->pointerSizeButtonGroup(), QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &Mouse::pointerSizeSlot);

    connect(mouseWidget->blinkingCursorOnTextWidget(), &SwitchWidget::stateChanged, this, &Mouse::blinkCursorOnTextSlot);

    connect(mouseWidget->cursorSpeedWidget()->slider(), &KSlider::sliderReleased, this, &Mouse::cursorSpeedSlot);

}


void Mouse::dataChanged(QString key)
{
    if (key == QString("dominantHand")) {
        initDominantHand();
    } else if (key == QString("scrollDirection")) {
        initScrollDirection();
    } else if (key == QString("wheelSpeed")) {
        initWheelSpeed();
    } else if (key == QString("doubleClick")) {
        initDoubleClickInterval();
    } else if (key == QString("pointerSpeed")) {
        initPointerSpeed();
    } else if (key == QString("mouseAcceleration")) {
        initMouseAcceleration();
    } else if (key == QString("pointerPosition") || key == QString("shakeCursor")) {
        initPointerPosition();
    } else if (key == QString("pointerSize")) {
        initPointerSize();
    }else if (key == QString("cursorBlink")) {
        initBlinkingCursor();
    } else if (key == QString("cursorSpeed")) {
        initCursorSpeed();
    }
}

/* Slot */
void Mouse::dominantHandSlot(int id)
{
    bool tmpLeftHand;

    // 左键主键：右手、left-handed=false，右键主键：左手、left-handed=true
    if (id == 0) {
        tmpLeftHand = false;
    } else {
        tmpLeftHand = true;
    }

    mouseDbus->call("setDominantHand", tmpLeftHand);

    UkccCommon::buriedSettings(name(), mouseWidget->dominantHandWidget()->objectName(), QString("settings"), id == 0 ? "left-key" : "right-key");
}

void Mouse::scrollDirectionSlot(int id)
{
    bool tmpScrollDirection;

    // true:界面与鼠标滚轮滚动方向相反(反向),false:界面与滚轮滚动方向相同(正向)
    if (id == 0) {
        tmpScrollDirection = false;
    } else {
        tmpScrollDirection = true;
    }

    mouseDbus->call("setScrollDirection", tmpScrollDirection);

    UkccCommon::buriedSettings(name(), mouseWidget->scrollDirectionWdiget()->objectName(), QString("settings"), id == 0 ? "Forward" : "Reverse");
}

void Mouse::wheelSpeedSlot()
{
    mouseDbus->call("setWheelSpeed", mouseWidget->wheelSpeedWidget()->value());
}

void Mouse::doubleClickIntervalSlot()
{

    mouseDbus->call("setDoubleClick", mouseWidget->doubleClickIntervalWidget()->slider()->maximum()
                    - mouseWidget->doubleClickIntervalWidget()->value()
                    + mouseWidget->doubleClickIntervalWidget()->slider()->minimum());
    qApp->setDoubleClickInterval(mouseWidget->doubleClickIntervalWidget()->value());
}

void Mouse::pointerSpeedSlot(int value)
{
    mouseDbus->call("setPointerSpeed", static_cast<double>(value)/mouseWidget->pointerSpeedWidget()->slider()->maximum() * 10);
}

void Mouse::tailLightEffectSlot(bool checked)
{
    mouseDbus->call("setTailLightEffect", checked);
}

void Mouse::mouseAccelerationSlot(bool checked)
{
    mouseDbus->call("setMouseAcceleration", checked);

    UkccCommon::buriedSettings(name(), mouseWidget->mouseAccelerationWidget()->objectName(), QString("settings"), checked ? "true":"false");
}

void Mouse::pointerPositionSlot(int index)
{
    switch (index) {
    case PositionType::NONE:
        mouseDbus->call("setPointerPosition", false);
        mouseDbus->call("setShakeCursor", false);
        break;
    case PositionType::PRESSCTRL:
        mouseDbus->call("setPointerPosition", true);
        mouseDbus->call("setShakeCursor", false);
        break;
    case PositionType::SHARKMOUSE:
        mouseDbus->call("setPointerPosition", false);
        mouseDbus->call("setShakeCursor", true);
        break;
    }

    UkccCommon::buriedSettings(name(), mouseWidget->pointerPositionWidget()->objectName(), QString("selected"), mouseWidget->pointerPositionWidget()->comboBox()->currentText());
}

void Mouse::pointerSizeSlot(QAbstractButton *button)
{
    QString pointerSize = button->property("pointer").toString();
    int tmpPointerSize;

    if (QString::compare(pointerSize, "Small") == 0) {
        tmpPointerSize = SMALLPOINTER;
    } else if (QString::compare(pointerSize, "Medium") == 0) {
        tmpPointerSize = MEDIUMPOINTER;
    } else if (QString::compare(pointerSize, "Large") == 0) {
        tmpPointerSize = LARGEPOINTER;
    }

    mouseDbus->call("setPointerSize", tmpPointerSize);

    UkccCommon::buriedSettings(name(), mouseWidget->pointerSizeButtonGroup()->objectName(), QString("settings"), pointerSize);
}

void Mouse::blinkCursorOnTextSlot(bool checked)
{
    mouseWidget->cursorSpeedWidget()->setVisible(checked);
    mouseDbus->call("setCursorBlink", checked);

    if (!checked) {
        mouseDbus->call("setStyleCursorSpeed", 0);
    } else {
        int mValue = mouseWidget->cursorSpeedWidget()->slider()->maximum() - mouseWidget->cursorSpeedWidget()->slider()->value() + mouseWidget->cursorSpeedWidget()->slider()->minimum();
        mouseDbus->call("setStyleCursorSpeed", mValue);
    }

    UkccCommon::buriedSettings(name(), mouseWidget->cursorSpeedWidget()->objectName(), QString("settings"), checked ? "true":"false");
}

void Mouse::cursorSpeedSlot()
{
    mouseDbus->call("setCursorSpeed", mouseWidget->cursorSpeedWidget()->slider()->maximum() - mouseWidget->cursorSpeedWidget()->value()
                    + mouseWidget->cursorSpeedWidget()->slider()->minimum());
}

bool Mouse::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        if (watched == mouseWidget->wheelSpeedWidget()->slider()) {
            int value = mouseWidget->wheelSpeedWidget()->value();
            UkccCommon::buriedSettings(name(), mouseWidget->wheelSpeedWidget()->objectName(), QString("settings"), QString::number(value));
        } else if (watched == mouseWidget->doubleClickIntervalWidget()->slider()) {
            int value = mouseWidget->doubleClickIntervalWidget()->value();
            UkccCommon::buriedSettings(name(), mouseWidget->doubleClickIntervalWidget()->objectName(), QString("settings"), QString::number(value));
        } else if (watched == mouseWidget->pointerSpeedWidget()->slider()) {
            int value = mouseWidget->pointerSpeedWidget()->value();
            UkccCommon::buriedSettings(name(), mouseWidget->pointerSpeedWidget()->objectName(), QString("settings"), QString::number(value));
        } else if (watched == mouseWidget->cursorSpeedWidget()->slider()) {
            int value = mouseWidget->cursorSpeedWidget()->value();
            UkccCommon::buriedSettings(name(), mouseWidget->cursorSpeedWidget()->objectName(), QString("settings"), QString::number(value));

        }
    }
    return QObject::eventFilter(watched, event);
}

