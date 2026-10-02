/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QMouseEvent>
#include <QThread>
#include <QX11Info>
#include <QMenu>
#include <QDebug>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QtDBus/QDBusReply>
#include <QInputMethodEvent>
#include <QInputMethod>
#include <QTransform>
#include <QPoint>
#include <QCursor>
#include <QClipboard>
#include <QTimer>
#include <QDateTime>
#include <QDropEvent>
#include <QMimeData>
#include <QGSettings>
#include <tuple>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syslog.h>

#include "displaywidget.h"
#include "dbusclient.h"
#include "keyboard/keyevent_handler.h"
#include "keyboard/keyevent_worker.h"
#include "common.h"
#include "utils.h"
#include "kmreenv.h"
#include "preferences.h"
#include "kmrewindow.h"
#include "displaymanager.h"
#include "keyboardgamekeymanager.h"
#include "gamekeymanager.h"
#include "displaybackend.h"
#include "eventmanager.h"
#include "app_control_manager.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

DisplayWidget::DisplayWidget(KmreWindow* window, int id, int width, int height, int orientation, QString displayName)
    : QOpenGLWidget(window)
    , mMainWindow(window)
    , mId(id)
    , mWidth(width)
    , mHeight(height)
    , mWidgetRealSize(width, height)
    , mWidgetMinSize(0, 0)
    , mDisplayName(displayName)
    , mIsMouseButtonPressed(false)
    , mTouchEventTrigged(false)
    , mImageRotation(-1)
    , mDisplaySizeScaling(1.f, 1.f)
    , m_inputX(0)
    , m_inputY(0)
{
    Q_UNUSED(orientation);

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setFocusPolicy(Qt::WheelFocus);//this->setFocusPolicy(Qt::StrongFocus);//解决该窗口无法响应按键事件，无该设置时，键盘事件由最外层的QWidget接收
    //this->setFocus(Qt::FocusReason::MouseFocusReason/*Qt::FocusReason::ActiveWindowFocusReason*/);
    this->setMouseTracking(true);
    this->setAcceptDrops(true);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    this->grabGesture(Qt::PinchGesture);
    installEventFilter(this);
    this->setAttribute(Qt::WA_InputMethodEnabled);//Why？ 需要初始化设置为true，并执行过一次changeInputBoxPos()后，才能保证窗口起来后需要切换输入法时不需要点击标题栏就能切换成功

    this->scrollTimer = new QTimer(this);
    this->connect(this->scrollTimer, SIGNAL(timeout()), this, SLOT(runAction()));

    initDisplayScalingFactor();
}

DisplayWidget::~DisplayWidget()
{
    qApp->removeEventFilter(this);
    DisplayBackend::getInstance()->unregisterDisplay(mId);
}

void DisplayWidget::initDisplayScalingFactor()
{
    //mDisplayScalingFactor = QApplication::primaryScreen()->devicePixelRatio();
    mDisplayScalingFactor = 1.0f;
    if (QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.xsettings")) {
        QGSettings scaleSettings("org.ukui.SettingsDaemon.plugins.xsettings", "/org/ukui/settings-daemon/plugins/xsettings/", this);
//        for (QString key : scaleSettings.keys()) {
//            QString gsettingsKey = QString(key).replace(".", "-").replace("_", "-");
//        }
        if (scaleSettings.keys().contains("scalingFactor")) {
            mDisplayScalingFactor = scaleSettings.get("scaling-factor").toDouble();//mDisplayScalingFactor = scaleSettings.get("scalingFactor").toDouble();
        }
        if (mDisplayScalingFactor <= 0.0f) {
            mDisplayScalingFactor = 1.0f;
        }
    }

    mWidgetRealSize = QSize(mWidth, mHeight) * mDisplayScalingFactor;
}

void DisplayWidget::initConnections()
{
    QDBusConnection::sessionBus().connect("org.ukui.KWin", "/TouchGestureManager", "org.ukui.KWin.TouchGestureManager", "edgeSwipeGesture", this, SLOT(onTouchGesture(QString, int)));
}

void DisplayWidget::setDisplaySize(uint32_t width, uint32_t height)
{
    setFixedSize(width, height);
    mWidgetRealSize = QSize(width, height) * mDisplayScalingFactor;

    if (mIsDDSSupported) {
        AppControlManager::getInstance()->updateAppWindowSize(mDisplayName, mId, width, height);
    }

    DisplayBackend::getInstance()->forceRedraw(mId);
}

void DisplayWidget::blurUpdate(uint32_t width, uint32_t height)
{
    if (mIsDDSSupported) {
        mWidgetMinSize = QSize(width, height);
        DisplayBackend::getInstance()->displayBlurUpdate(mId);
    }
}

void DisplayWidget::forceRedraw()
{
    DisplayBackend::getInstance()->forceRedraw(mId);
}

void DisplayWidget::enableDisplayUpdate(bool enable)
{
    DisplayBackend::getInstance()->enableDisplayUpdate(mId, enable);
}

void DisplayWidget::setImageRotation(int imageRotation)
{
    mImageRotation = imageRotation;
}

QPoint DisplayWidget::cursorPoint()
{
    QPoint point;
    if (mImageRotation == 1) {//90
        point = this->mapTo(this->parentWidget(), QPoint(m_inputX * this->width() / mHeight, m_inputY * this->height() / mWidth));
    } else if (mImageRotation == 2) {//180
        point = this->mapTo(this->parentWidget(), QPoint(m_inputX * this->width() / mWidth, m_inputY * this->height() / mHeight));
    } else if (mImageRotation == 3) {//270
        point = this->mapTo(this->parentWidget(), QPoint(m_inputX * this->width() / mHeight, m_inputY * this->height() / mWidth));
    } else {//0
        point = this->mapTo(this->parentWidget(), QPoint(m_inputX * this->width() / mWidth, m_inputY * this->height() / mHeight));
    }

    //如果输入坐标位置 + 50 > 屏幕分辨率端高度，此时输入法框会盖住输入框，需要进行如下处理，其中使用的50和150待优化
    const QDesktopWidget *desktop = QApplication::desktop();
    if (desktop) {
        QRect rect = desktop->screenGeometry();
        if (point.y() + 50 > rect.height()) {
            point.setY(point.y() - 150);
        }
    }

    return point;
}

void DisplayWidget::changeInputBoxPos()
{
    if (testAttribute(Qt::WA_InputMethodEnabled)) {
        QTransform t;
        //QPoint p = this->mapTo(this->parentWidget(), QPoint(m_inputX * this->width() / mWidth, m_inputY * this->height()/mHeight));//this->parentWidget()->parentWidget()
        QPoint p = this->cursorPoint();
        t.translate(p.x(), p.y());//平移x,y
        QGuiApplication::inputMethod()->setInputItemTransform(t);
        QGuiApplication::inputMethod()->setInputItemRectangle(this->rect());
        QGuiApplication::inputMethod()->update(Qt::ImQueryInput);
        //QGuiApplication::inputMethod()->commit();
        //syslog(LOG_DEBUG, "[DisplayWidget] changeInputBoxPos, x = %d, y = %d,"
        //        "rect.x = %d, rect.y = %d, rect.w = %d, rect.h = %d",
        //         p.x(), p.y(), rect().x(), rect().y(), rect().width(), rect().height());
        /*const auto keyboard_rect = QGuiApplication::inputMethod()->keyboardRectangle();
        const auto keyboard_visible = QGuiApplication::inputMethod()->isVisible();
        if (keyboard_visible) {//虚拟键盘是否可见，不是输入法框
        }*/
    }
}

void DisplayWidget::enableInputMethod(int id, bool ret, int x, int y)
{
    //syslog(LOG_DEBUG, "[DisplayWidget] enableInputMethod， id: %d, ret: %d, x: %d, y: %d", id, ret, x, y);
    //qDebug() << "########## enableInputMethod: " << ret << id << pkgName << x << y;
    if (!this->isActiveWindow()) {
        return;
    }
    //TODO:允许开启/关闭该窗口的输入法时，其他窗口的输入法需要关闭
    if (mId == id) {
        m_inputX = x;
        m_inputY = y;
        //syslog(LOG_DEBUG, "[DisplayWidget] %s input method!", ret ? "Enable" : "Disable");
        if (ret == false) {
            mIsIMEnabled = false;
            //当用户从一些输入框的检索列表选择文字时，此时模拟发送ESC事件去关闭输入法框
            KmreWindow *pw = nullptr;
            if (parentWidget()) {//if (parentWidget() && QWidget *pw = qobject_cast<QWidget *>(parentWidget()))
                pw = (KmreWindow*)parentWidget();
                if (pw && pw->isFullScreen()) {
                    pw->setEscShortBlockSignals(true);//阻塞快捷键ESC的信号
                }
            }

            Display *display = XOpenDisplay(NULL);
            if (display == NULL) {
                if (pw) {
                    pw->setEscShortBlockSignals(false);//取消阻塞快捷键ESC的信号
                }
                return;
            }
            unsigned int keycode;
            keycode = XKeysymToKeycode(display, XK_Escape);
            XTestFakeKeyEvent(display, keycode, True, 1);
            XTestFakeKeyEvent(display, keycode, False, 1);
            XFlush(display);
            XCloseDisplay(display);

            QTimer::singleShot(1000, this, [=] {//延时1s后停止接收输入法事件，太快停止会导致上面发送的ESC事件无效
                if (pw) {
                    pw->setEscShortBlockSignals(false);//取消阻塞快捷键ESC的信号
                }
                this->setAttribute(Qt::WA_InputMethodEnabled, ret);
                QGuiApplication::inputMethod()->reset();
            });
            this->setAttribute(Qt::WA_InputMethodEnabled, ret);
            QGuiApplication::inputMethod()->reset();
        }
        else {
            mIsIMEnabled = true;
            this->setAttribute(Qt::WA_InputMethodEnabled,ret);
            this->changeInputBoxPos();
        }
    }
}

void DisplayWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();
        emit doubleClicked();
    } else {
        event->ignore();
    }
}

void DisplayWidget::sendMouseEvent(MouseEventInfo mouseEvent)
{
    EventManager::getInstance()->sendMouseEvent(mId, mouseEvent);
}

void DisplayWidget::mouseEventHandler(int type, int slot, int x, int y)
{
    //syslog(LOG_DEBUG, "[DisplayWidget] mouseEventHandler, mId = %d, type = %d, slot = %d, x = %d, y = %d", mId, type, slot, x, y);
    if (mWidth == 0 || mHeight == 0 || this->width() == 0 || this->height() == 0) {
        return;
    }

    int xpos, ypos;

    if (mIsDDSSupported) {
        xpos = x;// * mDisplayScalingFactor;
        ypos = y;// * mDisplayScalingFactor;
    }
    else {
        float wScaling = 1.f, hScaling = 1.f;

        if (mWidth < mHeight) {
            // portrait
            if (this->width() < this->height()) {
                xpos = x * mWidth / this->width() * wScaling;
                ypos = y * mHeight / this->height() * hScaling;
            } else {
                xpos = x * mHeight / this->width() * wScaling;
                ypos = y * mWidth / this->height() * hScaling;
            }
        } else {
            // landscape
            if (this->width() < this->height()) {
                xpos = x * mHeight / this->width() * hScaling;
                ypos = y * mWidth / this->height() * wScaling;
            } else {
                xpos = x * mWidth / this->width() * hScaling;
                ypos = y * mHeight / this->height() * wScaling;
            }
        }
    }

    KeyboardGamekeyManager* keyboardGamekeyManager = mMainWindow->getKeyboardGamekeyManager();
    if ((!keyboardGamekeyManager) || keyboardGamekeyManager->isMouseClick()) {
        //syslog(LOG_DEBUG, "[%s] Send mouse event: mId = %d, type = %d, slot = %d, xpos = %d, ypos = %d", 
        //    __func__, mId, type, slot, xpos, ypos);
        sendMouseEvent(MouseEventInfo{type, slot, xpos, ypos});
    }
}

//Attention: 重写inputMethodQuery，设置一个合适的光标高度，否则光标高度为DisplayWidget的高度( QGuiApplication::inputMethod()->cursorRectangle() )
QVariant DisplayWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
    switch(query) {
    case Qt::ImCursorRectangle:
        return QRect(0, 0, 1, 24);
    default:
        return QWidget::inputMethodQuery(query);
    }
}

void DisplayWidget::onRequestActiveWindow()
{
    //syslog(LOG_DEBUG, "[DisplayWidget] onRequestActiveWindow, mId = %d", mId);
    this->setFocus();
    mIsMouseButtonPressed = false;
}

void DisplayWidget::mousePressEvent(QMouseEvent *event)
{
//    if (event->buttons() == Qt::RightButton) {
//        emit this->requestShowMainMenu(mIsIMEnabled);
//    }

    QWidget::mousePressEvent(event);
}

bool DisplayWidget::eventFilter(QObject *watched, QEvent *event)
{
    int xpos, ypos;
    int type = -1;
    int slot_id = -1;

    Q_UNUSED(watched);

    //syslog(LOG_DEBUG, "[DisplayWidget][%s] event type: %d", __func__, event->type());
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd) {
        const QTouchEvent *const touchEvent = static_cast<const QTouchEvent*>(event);
        const QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
        const int pointsCount = touchPoints.count();
        if (pointsCount >= 2) {
            for (int i = 0; i < pointsCount; i++) {
                const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.at(i);
                slot_id = i;
                switch (touchPoints.at(i).state()) {
                    case Qt::TouchPointPressed:
                    {
                        type = Button1_Press;
                    }
                    break;
                    case Qt::TouchPointMoved:
                    {
                        type = Mouse_Motion;
                    }
                    break;
                    case Qt::TouchPointReleased:
                    {
                        type = Button1_Release;
                    }
                    break;
                    default:
                        break;
                }
                if (type > 0) {
                    mouseEventHandler(type, slot_id, touchPoint0.pos().toPoint().x(), touchPoint0.pos().toPoint().y());
                }
            }
            return true;
        }
    }
    else if (event->type() == QEvent::InputMethod) {
        this->changeInputBoxPos();
        QInputMethodEvent* e = (QInputMethodEvent*) event;
        if (!hasFocus()) {//失去焦点时，输入法事件优先于FocusOut事件被处理
            e->setCommitString("");//将失去焦点后提交的字符串设置为空
        }
        else {
            QString commitText = e->commitString();
            if (!commitText.isEmpty()) {
                //syslog(LOG_DEBUG, "[DisplayWidget] commitText: %s", commitText.toStdString().c_str());
                EventManager::getInstance()->sendInputMethodData(mId, commitText);
            }
        }
    } 
    else if (event->type() == QEvent::KeyPress) {

        emit this->requestHideSubWindow();
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent) {
            if ((!keyEvent->isAutoRepeat()) || firstRepeat) {
                firstRepeat = false;
                emit keyboardPress(keyEvent);//lzh键盘信号
            }
            //qDebug() << keyEvent;
            if ((keyEvent->modifiers() & Qt::ControlModifier) != Qt::NoModifier) {
                mCTLKeyPressed = true;
            }

            //fix v10.1 key_meta close window
            if (keyEvent->key() == Qt::Key_Meta) {
                return false;
            }

            KeyEventHandler::KeyEvent inputEv;
            inputEv.scancode = keyEvent->nativeScanCode();//int
            inputEv.nativeModifiers = keyEvent->nativeModifiers();//int
            inputEv.modifiers = keyEvent->modifiers();//int
            inputEv.eventType = EventTextInput;//int

            if (inputEv.scancode == 49) {//键盘左上角的"`"按键
                if (keyEvent->modifiers() == Qt::ShiftModifier){
                    EventManager::getInstance()->sendInputMethodData(mId, "~");
                } else{
                    EventManager::getInstance()->sendInputMethodData(mId, "`");
                }
            }

            //syslog(LOG_DEBUG, "[DisplayWidget] key pressed, isAutoRepeat = %d", keyEvent->isAutoRepeat());
            GameKeyManager* gameKeyManager = mMainWindow->getGameKeyManager();
            if ((!keyEvent->isAutoRepeat()) && gameKeyManager) {
                //在walkwidget对象已经创建和显示的情况下
                if (gameKeyManager->hasSteelingWheel() && (keyEvent->modifiers() == Qt::NoModifier)) {
                    if (inputEv.scancode == 38) {//键盘"a"按键按下
                        if (isKeyWDown) {//键盘"w"按键和键盘"a"按键都处于按下状态，表示左上方向
                            emit walkKeyProcess(WALK_DIRECTION_LEFT_UP,true);
                        } else if (isKeySDown) {//键盘"s"按键和键盘"a"按键都处于按下状态，表示左下方向
                            emit walkKeyProcess(WALK_DIRECTION_LEFT_DOWN,true);
                        } else {//只有键盘"a"按键处于按下状态，表示左方向
                            emit walkKeyProcess(WALK_DIRECTION_LEFT,true);
                        }
                        isKeyADown = true;
                    } else if (inputEv.scancode == 25) {//键盘"w"按键按下
                        if (isKeyADown) {//键盘"a"按键和键盘"w"按键都处于按下状态，表示左上方向
                            emit walkKeyProcess(WALK_DIRECTION_LEFT_UP,true);
                        } else if (isKeyDDown) {//键盘"d"按键和键盘"w"按键都处于按下状态，表示右上方向
                            emit walkKeyProcess(WALK_DIRECTION_RIGHT_UP,true);
                        } else {//只有键盘"w"按键处于按下状态，表示上方向
                            emit walkKeyProcess(WALK_DIRECTION_UP,true);
                        }
                        isKeyWDown = true;
                    } else if (inputEv.scancode == 40) {//键盘"d"按键按下
                        if (isKeyWDown) {//键盘"w"按键和键盘"d"按键都处于按下状态，表示右上方向
                            emit walkKeyProcess(WALK_DIRECTION_RIGHT_UP,true);
                        } else if (isKeySDown) {//键盘"w"按键和键盘"d"按键都处于按下状态，表示右下方向
                            emit walkKeyProcess(WALK_DIRECTION_RIGHT_DOWN,true);
                        } else {//只有键盘"d"按键处于按下状态，表示右方向
                            emit walkKeyProcess(WALK_DIRECTION_RIGHT,true);
                        }
                        isKeyDDown = true;
                    } else if (inputEv.scancode == 39) {//键盘"s"按键按下
                        if (isKeyADown) {//键盘"a"按键和键盘"s"按键都处于按下状态，表示左下方向
                            emit walkKeyProcess(WALK_DIRECTION_LEFT_DOWN,true);
                        } else if (isKeyDDown) {//键盘"d"按键和键盘"s"按键都处于按下状态，表示左上方向
                            emit walkKeyProcess(WALK_DIRECTION_RIGHT_DOWN,true);
                        } else {//只有键盘"s"按键处于按下状态，表示下方向
                            emit walkKeyProcess(WALK_DIRECTION_DOWN,true);
                        }
                        isKeySDown = true;
                    } else {
                        emit oneKeyPressed(SingleKey::genKeyString(keyEvent));//根据当前键值找到对应的游戏按钮，发送按键信息给Android
                    }
                } else {
                    emit oneKeyPressed(SingleKey::genKeyString(keyEvent));//根据当前键值找到对应的游戏按钮，发送按键信息给Android
                }
            }

            KeyEventData *event = KeyEventHandler::getInstance()->handleKeyEvent(inputEv);
            if (event) {
                KmreKeyboard *kk = KeyEventWorker::getInstance()->manageKeyboardEvent(event);
                KeycodeBufferData *keycodeBuffer = kk->keycode_buffer;
                if (keycodeBuffer->keycode_count > 0) {
                    //syslog(LOG_DEBUG, "[DisplayWidget] writeKeyboardEventData, mId = %d, keycode = %d", mId, keycodeBuffer->keycodes);
                    EventManager::getInstance()->sendKeyBoardEvent(mId, keycodeBuffer);
                }
                keycodeBuffer->keycode_count = 0;
            }
        }
    } 
    else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *kev = static_cast<QKeyEvent*>(event);
        if (kev->isAutoRepeat() == false) {
            emit keyboardRelease(kev);//lzh键盘信号
        }
        //qDebug() << kev;
        if (kev->key() == Qt::Key_Meta) {
            //qDebug() << "KeyRelease Meta Key==========";
        }

        if ((kev->modifiers() & Qt::ControlModifier) == Qt::NoModifier) {
            mCTLKeyPressed = false;
            //qDebug() << "mCTLKeyPressed: " << mCTLKeyPressed;
        }

        GameKeyManager* gameKeyManager = mMainWindow->getGameKeyManager();
        if ((!kev->isAutoRepeat()) && gameKeyManager) {
            if (gameKeyManager->hasSteelingWheel()) {
                if (kev->nativeScanCode() == 38) {//键盘"a"按键抬起
                    emit walkKeyProcess(WALK_DIRECTION_LEFT,false);
                    if (isKeyWDown) {
                        emit walkKeyProcess(WALK_DIRECTION_UP,false);
                        emit walkKeyProcess(WALK_DIRECTION_UP,true);
                    } else if (isKeySDown) {
                        emit walkKeyProcess(WALK_DIRECTION_DOWN,false);
                        emit walkKeyProcess(WALK_DIRECTION_DOWN,true);
                    } else if (isKeyDDown) {
                        emit walkKeyProcess(WALK_DIRECTION_RIGHT,false);
                        emit walkKeyProcess(WALK_DIRECTION_RIGHT,true);
                    }
                    isKeyADown = false;
                } else if (kev->nativeScanCode() == 25) {//键盘"w"按键抬起
                    emit walkKeyProcess(WALK_DIRECTION_UP,false);
                    if (isKeyADown) {
                        emit walkKeyProcess(WALK_DIRECTION_LEFT,false);
                        emit walkKeyProcess(WALK_DIRECTION_LEFT,true);
                    } else if (isKeyDDown) {
                        emit walkKeyProcess(WALK_DIRECTION_RIGHT,false);
                        emit walkKeyProcess(WALK_DIRECTION_RIGHT,true);
                    } else if (isKeySDown) {
                        emit walkKeyProcess(WALK_DIRECTION_DOWN,false);
                        emit walkKeyProcess(WALK_DIRECTION_DOWN,true);
                    }
                    isKeyWDown = false;
                } else if (kev->nativeScanCode() == 40) {//键盘"d"按键抬起
                    emit walkKeyProcess(WALK_DIRECTION_RIGHT,false);
                    if (isKeyWDown) {
                        emit walkKeyProcess(WALK_DIRECTION_UP,false);
                        emit walkKeyProcess(WALK_DIRECTION_UP,true);
                    } else if (isKeySDown) {
                        emit walkKeyProcess(WALK_DIRECTION_DOWN,false);
                        emit walkKeyProcess(WALK_DIRECTION_DOWN,true);
                    } else if (isKeyADown) {
                        emit walkKeyProcess(WALK_DIRECTION_LEFT,false);
                        emit walkKeyProcess(WALK_DIRECTION_LEFT,true);
                    }
                    isKeyDDown = false;
                } else if (kev->nativeScanCode() == 39) {//键盘"s"按键抬起
                    emit walkKeyProcess(WALK_DIRECTION_DOWN,false);
                    if (isKeyADown) {
                        emit walkKeyProcess(WALK_DIRECTION_LEFT,false);
                        emit walkKeyProcess(WALK_DIRECTION_LEFT,true);
                    } else if (isKeyDDown) {
                        emit walkKeyProcess(WALK_DIRECTION_RIGHT,false);
                        emit walkKeyProcess(WALK_DIRECTION_RIGHT,true);
                    } else if (isKeyWDown) {
                        emit walkKeyProcess(WALK_DIRECTION_UP,false);
                        emit walkKeyProcess(WALK_DIRECTION_UP,true);
                    }
                    isKeySDown = false;
                } else {
                    emit oneKeyRelease(SingleKey::genKeyString(kev));
                }
            } else {
                emit oneKeyRelease(SingleKey::genKeyString(kev));
            }
        }
    }
    else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        focusDisplay();
        emit this->requestHideSubWindow();
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        emit mousePress(mouseEvent);//lzh鼠标信号

        KeyboardGamekeyManager* keyboardGamekeyManager = mMainWindow->getKeyboardGamekeyManager();
        if((!keyboardGamekeyManager) || keyboardGamekeyManager->isMouseClick()){
            QPoint pos = mouseEvent->globalPos() - this->parentWidget()->pos() - this->pos();
            if (mouseEvent->button() == Qt::LeftButton) {
                mIsMouseButtonPressed = true;
                type = Button1_Press;
                if (type > 0) {
                    xpos = pos.x();
                    ypos = pos.y();
                    if (!mCTLKeyPressed) {
                        mouseEventHandler(type, DEFAULT_SLOT, xpos, ypos);
                    }
                    else {//放大缩小
                        int zoom = utils::isZoomSupported(mMainWindow->getPackageName());
                        xZoom = xpos;
                        yZoom = ypos;
                        int space = 100;
                        if (zoom == 3) {
                            space = 140;
                        }
                        currentZoom = true;
                        mouseEventHandler(Button1_Press, 3, width()/2-space, height()/2-space);
                        mouseEventHandler(Button1_Press, DEFAULT_SLOT, width()/2+space, height()/2+space);
                    }
                }
            } 
            else if (mouseEvent->button() == Qt::RightButton) {
                if (!mIsMouseButtonPressed) {
                    type = Button3_Press;
                    if (type > 0) {
                        xpos = pos.x();
                        ypos = pos.y();
                        mouseEventHandler(type, DEFAULT_SLOT, xpos, ypos);
                    }
                }
            }
        }
    } 
    else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        emit mouseRelease(mouseEvent);//lzh鼠标信号

        KeyboardGamekeyManager* keyboardGamekeyManager = mMainWindow->getKeyboardGamekeyManager();
        if((!keyboardGamekeyManager) || keyboardGamekeyManager->isMouseClick()){
            QPoint pos = mouseEvent->globalPos() - this->parentWidget()->pos() - this->pos();
            if (mouseEvent->button() == Qt::LeftButton) {
                mIsMouseButtonPressed = false;
                type = Button1_Release;
                if (pos.x() < 0) {
                    xpos = 0;
                } else if (pos.x() >= this->width()) {
                    xpos = this->width() - 1;
                } else {
                    xpos = pos.x();
                }

                if (pos.y() < 0) {
                    ypos = 0;
                } else if (pos.y() >= this->height()) {
                    ypos = this->height() - 1;
                } else {
                    ypos = pos.y();
                }
                if (type > 0) {
                    if (!currentZoom) {
                        mouseEventHandler(type, DEFAULT_SLOT, xpos, ypos);
                    }
                    else {
                        int zoom = utils::isZoomSupported(mMainWindow->getPackageName());
                        int xSpace = xpos - xZoom;
                        int ySpace = ypos - yZoom;
                        int space = 100;
                        if (zoom == 3) {
                            space = 140;
                        }
                        mouseEventHandler(Button1_Release, 3, width()/2-space, height()/2-space);
                        mouseEventHandler(Button1_Release, DEFAULT_SLOT, width()/2+space + xSpace, height()/2+space + ySpace);
                        currentZoom = false;
                    }
                }
                mTouchEventTrigged = false;
            } 
            else if (mouseEvent->button() == Qt::RightButton) {
                if (!mIsMouseButtonPressed) {
                    type = Button3_Release;
                    if (pos.x() < 0) {
                        xpos = 0;
                    } else if (pos.x() >= this->width()) {
                        xpos = this->width() - 1;
                    } else {
                        xpos = pos.x();
                    }

                    if (pos.y() < 0) {
                        ypos = 0;
                    } else if (pos.y() >= this->height()) {
                        ypos = this->height() - 1;
                    } else {
                        ypos = pos.y();
                    }
                    if (type > 0) {
                        mouseEventHandler(type, DEFAULT_SLOT, xpos, ypos);
                    }
                }
            }
        }
    } 
    else if (event->type() == QEvent::MouseMove) {
        KeyboardGamekeyManager* keyboardGamekeyManager = mMainWindow->getKeyboardGamekeyManager();
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if(keyboardGamekeyManager && (!keyboardGamekeyManager->isMouseClick())){
            emit mouseMove(mouseEvent);//lzh鼠标信号
            settingMousePos(keyboardGamekeyManager->getViewLockPos());
        }
        if(keyboardGamekeyManager && keyboardGamekeyManager->isBrainGamekeyPress()){
            emit mouseLeave();
        }

        if((!keyboardGamekeyManager) || keyboardGamekeyManager->isMouseClick()){
            QPoint p = mouseEvent->globalPos() - this->parentWidget()->pos() - this->pos();
            if (mIsMouseButtonPressed) {
                type = Mouse_Motion;
            } else {
                type = -1;
            }

            if (p.x() < 0) {
                xpos = 0;
            } else if (p.x() >= this->width()) {
                xpos = this->width() - 1;
            } else {
                xpos = p.x();
            }

            if (p.y() < 0) {
                ypos = 0;
            } else if (p.y() >= this->height()) {
                ypos = this->height() - 1;
            } else {
                ypos = p.y();
            }

            if (type > 0) {
                if (!currentZoom) {
                    if (!mTouchEventTrigged) {
                        mouseEventHandler(type, DEFAULT_SLOT, xpos, ypos);
                    }
                }
                else {
                    int zoom = utils::isZoomSupported(mMainWindow->getPackageName());
                    int xSpace = xpos - xZoom;
                    int ySpace = ypos - yZoom;
                    int space = 100;
                    if (zoom == 3) {
                        space = 140;
                    }
                    mouseEventHandler(Mouse_Motion, 3, width()/2-space, height()/2-space);
                    mouseEventHandler(Mouse_Motion, DEFAULT_SLOT, width()/2+space + xSpace, height()/2+space + ySpace);
                }
            }
        }
    } 
    else if (event->type() == QEvent::Wheel) {
        focusDisplay();

        scrollCount++;
        QWheelEvent *we = static_cast<QWheelEvent*>(event);
        deltaAverage += we->delta();
        //syslog(LOG_DEBUG, "[DisplayWidget][%s] delta: %d", __func__, we->delta());
        long long currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        if (qAbs(currentTime - scrollTime) > 500) {
            if (this->scrollTimer->isActive()) {
                this->scrollTimer->stop();
            }
            this->scrollTimer->start(60);
            deltaAverage = we->delta();
            xPos = we->x();
            yPos = we->y();
            scrollTime = currentTime;
            scrollCount = 0;
        }
    } 
    else if (event->type() == QEvent::Leave) {
        mMainWindow->limitMouse();
        KeyboardGamekeyManager* keyboardGamekeyManager = mMainWindow->getKeyboardGamekeyManager();
        if(keyboardGamekeyManager && (!keyboardGamekeyManager->isMouseClick())){
            settingMousePos(keyboardGamekeyManager->getViewLockPos());
        }
        if(keyboardGamekeyManager && keyboardGamekeyManager->isBrainGamekeyPress()){
            emit mouseLeave();
        }
    } 
    else if (event->type() == QEvent::FocusOut){
        changeWidgetFocus();
    }
    return false;
}

void DisplayWidget::runAction()
{
    if (mCTLKeyPressed) {
        /* 缩放图片 */
        //onZoom();
    }
    else {
        /* 滚轮刷新 */
        onScroll();
    }

    if (this->scrollTimer->isActive()) {
        this->scrollTimer->stop();
    }
}

void DisplayWidget::onScroll()
{
    int x1 = xPos;
    int y1 = yPos;
    KmreConfig::Preferences *pref = KmreConfig::Preferences::getInstance();

    deltaAverage = deltaAverage / static_cast<double>(scrollCount + 1);
    int speed = qAbs((deltaAverage / 120.0) * scrollCount);
    //syslog(LOG_DEBUG, "deltaAverage = %d, scrollCount = %d, speed = %d", deltaAverage, scrollCount, speed);

    int distance = (speed < 4) ? pref->mScrollSensitivity : pref->mMaxScrollSensitivity;
    int space = (speed < 4) ? ((speed + 1) * 2) : 20;

    mouseEventHandler(Button1_Press, 2, x1, y1);
    long long now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    long long endTime = now + 300;
    while (now < endTime) {
        usleep(12*1000);
        y1 = (deltaAverage > 0)? (y1+space):(y1-space);
        mouseEventHandler(Mouse_Motion, 2, x1, y1);
        now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    usleep(40*1000);
    y1 = (deltaAverage > 0)? (y1+distance):(y1-distance);
    mouseEventHandler(Mouse_Motion, 2, x1, y1);
    usleep(10*1000);
    mouseEventHandler(Button1_Release, 2, x1, y1);
}

void DisplayWidget::onZoom()
{
    bool down = (deltaAverage > 0);
    int duration = 150;
    int space = 2;
    int x1 = xPos;
    int y1 = !down? (yPos - 75):(yPos - 40);
    int x2 = xPos;
    int y2 = !down? (yPos + 75):(yPos + 40);
    int zoom = utils::isZoomSupported(mMainWindow->getPackageName());
    if (zoom == 1) {
        duration = 150;
        space = 2;
        y1 = !down? (yPos - 75):(yPos - 40);
        y2 = !down? (yPos + 75):(yPos + 40);
    }
    else if (zoom == 2){
        duration = 300;
        space = 4;
        y1 = !down? (yPos - 150):(yPos - 20);
        y2 = !down? (yPos + 150):(yPos + 20);
    }
    else if (zoom == 3){
        if (scrollCount == 0 || scrollCount == 1) {
            duration = 90;
            space = 5;
            y1 = !down? (yPos - 100):(yPos - 30);
            y2 = !down? (yPos + 100):(yPos + 30);
        }
        else if (scrollCount == 2 || scrollCount == 3) {
            duration = 260;
            space = 6;
            y1 = !down? (yPos - 200):(yPos - 30);
            y2 = !down? (yPos + 200):(yPos + 30);
        }
        else {
            duration = 90;
            space = 5;
            y1 = !down? (yPos - 100):(yPos - 30);
            y2 = !down? (yPos + 100):(yPos + 30);
        }
    }
    else {
        return;
    }

    mouseEventHandler(Button1_Press, 3, x1, y1);
    usleep(5*1000);
    mouseEventHandler(Button1_Press, 4, x2, y2);

    long long now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    long long endTime = now + duration;
    while (now < endTime) {
        usleep(10*1000);
        y1 = !down? (y1 + space):(y1 - space);
        y2 = !down? (y2 - space):(y2 + space);
        mouseEventHandler(Mouse_Motion, 3, x1, y1);
        mouseEventHandler(Mouse_Motion, 4, x2, y2);
        now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    usleep(10*1000);
    mouseEventHandler(Button1_Release, 3, x1, y1);

    usleep(5*1000);
    mouseEventHandler(Button1_Release, 4, x2, y2);
}

// 模拟鼠标操作来强制画面刷新
void DisplayWidget::forceUpdating()
{
    // mouseEventHandler(Button1_Press, DEFAULT_SLOT, 100, 100);
    // mouseEventHandler(Mouse_Motion, DEFAULT_SLOT, 100, 101);
    // mouseEventHandler(Button1_Release, DEFAULT_SLOT, 100, 102);

    mouseEventHandler(Button1_Press, 2, 100, 100);
    usleep(10000);
    mouseEventHandler(Mouse_Motion, 2, 100, 101);
    usleep(10000);
    mouseEventHandler(Button1_Release, 2, 100, 102);
}

void DisplayWidget::focusDisplay()
{
    if (mMainWindow->getDisplayManager()->focusDisplayByDisplayId(mId)) {
        // waiting android to set focused display
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void DisplayWidget::resetControlData()
{
    mCTLKeyPressed = false;
}

void DisplayWidget::isShowMouse(bool status){
    if(status){
        this->setCursor(Qt::BlankCursor);
    }
    else{
        this->setCursor(Qt::ArrowCursor);
    }
}

void DisplayWidget::settingMousePos(QPoint pos){
    QPoint formatPos = this->mapToGlobal(pos);
    QCursor::setPos(formatPos);
}

void DisplayWidget::changeWidgetFocus(){
    firstRepeat = true;
    emit focusOut();
}

void DisplayWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void DisplayWidget::dragMoveEvent(QDragMoveEvent *ev)
{
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void DisplayWidget::dropEvent(QDropEvent *ev)
{
    qInfo() << ev->mimeData()->formats();

    if (!ev->mimeData()->hasUrls()) {
        return;
    }

    QList<QUrl> urls = ev->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    if (urls.count() == 1) {//urls.size() == 1
        if (urls.first().isLocalFile()) {//urls[0]
            QString path = urls.first().toLocalFile();
            //syslog(LOG_DEBUG, "[DisplayWidget] share file '%s' to '%s'", 
            //    path.toStdString().c_str(), mDisplayName.toStdString().c_str());
            emit shareFile(path, mDisplayName, mId, true);
        }
    }
    ev->acceptProposedAction();
}

bool DisplayWidget::registerDisplay(DisplayManager* displayManager) 
{
    return DisplayBackend::getInstance()->registerDisplay(displayManager, this);
}

void DisplayWidget::connectDisplay(int displayId, int width, int height) 
{
    mId = displayId;
    mWidth = width;
    mHeight = height;

    DisplayBackend::getInstance()->connectDisplay(this);
}
