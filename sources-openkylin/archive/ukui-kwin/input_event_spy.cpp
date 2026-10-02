/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2016 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "input_event_spy.h"
#include "input.h"

#include <QPointF>
#include <QSizeF>
#include "touch_gesture_action_control.h"

namespace KWin
{

InputEventSpy::InputEventSpy() = default;

InputEventSpy::~InputEventSpy()
{
    if (input()) {
        input()->uninstallInputEventSpy(this);
    }
}

void InputEventSpy::pointerEvent(MouseEvent *event)
{
    Q_UNUSED(event)
}

void InputEventSpy::wheelEvent(WheelEvent *event)
{
    Q_UNUSED(event)
}

void InputEventSpy::keyEvent(KeyEvent *event)
{
    Q_UNUSED(event)
}

void InputEventSpy::touchDown(qint32 id, const QPointF &point, quint32 time)
{
    Q_UNUSED(id)
    Q_UNUSED(point)
    Q_UNUSED(time)
}

void InputEventSpy::touchMotion(qint32 id, const QPointF &point, quint32 time)
{
    Q_UNUSED(id)
    Q_UNUSED(point)
    Q_UNUSED(time)
}

void InputEventSpy::touchUp(qint32 id, quint32 time)
{
    Q_UNUSED(id)
    Q_UNUSED(time)
}

void InputEventSpy::pinchGestureBegin(int fingerCount, quint32 time)
{
    Q_UNUSED(fingerCount)
    Q_UNUSED(time)
}

void InputEventSpy::pinchGestureUpdate(qreal scale, qreal angleDelta, const QSizeF &delta, quint32 time)
{
    Q_UNUSED(scale)
    Q_UNUSED(angleDelta)
    Q_UNUSED(delta)
    Q_UNUSED(time)
}

void InputEventSpy::pinchGestureEnd(quint32 time)
{
    Q_UNUSED(time)
}

void InputEventSpy::pinchGestureCancelled(quint32 time)
{
    Q_UNUSED(time)
}

void InputEventSpy::swipeGestureBegin(int fingerCount, quint32 time)
{
    Q_UNUSED(fingerCount)
    Q_UNUSED(time)
}

void InputEventSpy::swipeGestureUpdate(const QSizeF &delta, quint32 time)
{
    Q_UNUSED(delta)
    Q_UNUSED(time)
}

void InputEventSpy::swipeGestureEnd(quint32 time)
{
    Q_UNUSED(time)
}

void InputEventSpy::swipeGestureCancelled(quint32 time)
{
    Q_UNUSED(time)

}

void InputEventSpy::tapGestureBegin(int fingerCount, quint32 time)
{
    Q_UNUSED(fingerCount)
    Q_UNUSED(time)

}

void InputEventSpy::tapGestureUpdate(const QSizeF &delta, quint32 time)
{
    Q_UNUSED(delta)
    Q_UNUSED(time)

}

void InputEventSpy::tapGestureEnd(quint32 time)
{
    Q_UNUSED(time)

}

void InputEventSpy::tapGestureCancelled(quint32 time)
{
    Q_UNUSED(time)

}

void InputEventSpy::switchEvent(SwitchEvent *event)
{
    Q_UNUSED(event)
}

void InputEventSpy::tabletToolEvent(QTabletEvent *event)
{
    Q_UNUSED(event)
}

void InputEventSpy::tabletToolButtonEvent(const QSet<uint> &pressedButtons)
{
    Q_UNUSED(pressedButtons)
}

void InputEventSpy::tabletPadButtonEvent(const QSet<uint> &pressedButtons)
{
    Q_UNUSED(pressedButtons)
}

void InputEventSpy::tabletPadStripEvent(int number, int position, bool isFinger)
{
    Q_UNUSED(number)
    Q_UNUSED(position)
    Q_UNUSED(isFinger)
}

void InputEventSpy::tabletPadRingEvent(int number, int position, bool isFinger)
{
    Q_UNUSED(number)
    Q_UNUSED(position)
    Q_UNUSED(isFinger)
}

/*
 *  TouchPad
 *
*/
KWIN_SINGLETON_FACTORY(TouchpadGestureManager)
TouchpadGestureManager::TouchpadGestureManager(QObject* parent) : QObject(parent),
    InputEventSpy(),
    m_touchpadReg(new TouchpadGestureRecognizer())
{
    //辨別手勢
    auto gestureTouchpadConfig = XMLReader::getInstance()->getGestureConfig();
    auto gestureTouchpadSpecifics = gestureTouchpadConfig.keys();
    for(QList<QString>& gestureTouchpadSpecific : gestureTouchpadSpecifics)
    {
        auto gestureAction = gestureTouchpadConfig[gestureTouchpadSpecific];
        m_gestureTouchpadAndAction = new GestureAndActionBase(gestureTouchpadSpecific,gestureAction);
        m_touchpadAction << m_gestureTouchpadAndAction;
        //注册手势
        registerTouchpadGesture(m_gestureTouchpadAndAction);
    }
    //将文件与更新手势关联  修改配置文件之后可进行手势更新
    QFile gestureTouchpadConfigFile(XMLReader::getConfigFile());
    if(gestureTouchpadConfigFile.exists()){
        m_gestureWatcher = new QFileSystemWatcher();
        m_gestureWatcher->addPath(XMLReader::getConfigFile());
        connect(m_gestureWatcher,&QFileSystemWatcher::fileChanged,this,&TouchpadGestureManager::updateTouchpadGesture);
    }else{
        //printf("this file is not exist");
    }
}
TouchpadGestureManager::~TouchpadGestureManager()
{
    delete m_touchpadReg;
    qDeleteAll(m_gesture.begin(),m_gesture.end());
    qDeleteAll(m_touchpadAction.begin(),m_touchpadAction.end());
    if(m_gestureWatcher){
        delete m_gestureWatcher;
    }
    s_self = nullptr;
}

void TouchpadGestureManager::registerTouchpadGesture(GestureAndActionBase *action)
{

    switch (action->getDevice()) {
    case TouchDevice::TouchPad:{
        switch (action->getGestureType()) {
        case GestureType::SWIPE:{
            touchpadSwipe = new TouchpadSwipeGesture();
            touchpadSwipe->setFingersCount(m_gestureTouchpadAndAction->getFingers());
            touchpadSwipe->setDirection(m_gestureTouchpadAndAction->getGestureDirection());
            m_touchpadReg->registerGesture(touchpadSwipe);
            m_gesture << touchpadSwipe;
            connect(touchpadSwipe,&Gesture::started,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionStart);
            connect(touchpadSwipe,&Gesture::triggered,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionTriggered);
            connect(touchpadSwipe,&Gesture::cancelled,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionTriggered);
            break;
        }
        case GestureType::TAP:{
            touchpadTap = new TouchpadTapGesture();
            touchpadTap->setFingersCount(m_gestureTouchpadAndAction->getFingers());
            m_touchpadReg->registerGesture(touchpadTap);
            m_gesture << touchpadTap;
            /*
            *    三指四指的点击事件是只有在点击结束的时候才会触发 tap点击事件整个过程很短
            */
//            connect(touchpadTap,&Gesture::started,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionStart);
            connect(touchpadTap,&Gesture::triggered,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionTriggered);
            connect(touchpadTap,&Gesture::cancelled,m_gestureTouchpadAndAction,[](){
                //printf("tap is cancel\n");
            });
            break;
        }
        case GestureType::SWIPESEQUENCE:{
            m_touchpadSwipe2 = new TouchpadSwipeSequenceGesture;
            m_touchpadSwipe2->setFingersCount(m_gestureTouchpadAndAction->getFingers());
            m_touchpadReg->registerGesture(m_touchpadSwipe2);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::started,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionStart);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::triggered, m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionTriggered);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::cancelled,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionTriggered);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::sigNotifyGestureDirection, [=](KWin::Gesture::GestureDirection direction){
                switch (direction) {
                case Gesture::GestureDirection::Down:
                    m_gestureTouchpadAndAction->onAllDirectionKey(Gesture::GestureDirection::Down);
                    break;
                case Gesture::GestureDirection::Up:
                    m_gestureTouchpadAndAction->onAllDirectionKey(Gesture::GestureDirection::Up);
                    break;
                case Gesture::GestureDirection::Left:
                    m_gestureTouchpadAndAction->onAllDirectionKey(Gesture::GestureDirection::Left);
                    break;
                case Gesture::GestureDirection::Right:
                    m_gestureTouchpadAndAction->onAllDirectionKey(Gesture::GestureDirection::Right);
                    break;
                default:
                    break;
                }
            });
            break;
        }
        default:
            break;
        }
        break;
    }
    case TouchDevice::TouchScreen:
        break;
    default:
        break;
    }
}

void TouchpadGestureManager::updateTouchpadGesture()
{
    //printf("====配置文件更新  触发更新手势功能==========updateTouchpadGesture----");
    m_gestureWatcher->addPath(XMLReader::getConfigFile());

    XMLReader::getInstance()->clearGestureConfig();
    XMLReader::getInstance()->phraseXml();
    auto touchpadGestureConfig = XMLReader::getInstance()->getGestureConfig();
    auto touchpadGestureSpecifics = touchpadGestureConfig.keys();

    for(int i=0;i<m_gesture.size();i++){

        if(m_gesture[i]->gestureType()== Gesture::GestureType::Swipe){
            touchpadSwipe = static_cast<TouchpadSwipeGesture*>(m_gesture[i]);
            m_touchpadReg->unregisterGesture(touchpadSwipe);
            touchpadSwipe->disconnect();
        }
        else if(m_gesture[i]->gestureType() == Gesture::GestureType::Tap){
            touchpadTap = static_cast<TouchpadTapGesture*>(m_gesture[i]);
            m_touchpadReg->unregisterGesture(touchpadTap);
            touchpadTap->disconnect();
        }
        else if(m_gesture[i]->gestureType() == Gesture::GestureType::SwipeSequence){
            m_touchpadSwipe2 = static_cast<TouchpadSwipeSequenceGesture*>(m_gesture[i]);
            m_touchpadReg->unregisterGesture(m_touchpadSwipe2);
            m_touchpadSwipe2->disconnect();
        }
    }
    for(QVector<Gesture*>::iterator it = m_gesture.begin();it != m_gesture.end();it++){
        Gesture *deleteTouchpadGesture = (*it);
        delete(deleteTouchpadGesture);
    }
    for(QList<GestureAndActionBase*>::iterator it = m_touchpadAction.begin();it != m_touchpadAction.end();it++){
        GestureAndActionBase *deleteAction = (*it);
        delete(deleteAction);
    }
    m_gesture.clear();
    m_touchpadAction.clear();

    for(QList<QString>& touchpadGestureSpecific : touchpadGestureSpecifics){
        auto touchpadGestureAction = touchpadGestureConfig[touchpadGestureSpecific];
        m_gestureTouchpadAndAction = new GestureAndActionBase(touchpadGestureSpecific,touchpadGestureAction);
        m_touchpadAction << m_gestureTouchpadAndAction;
        registerTouchpadGesture(m_gestureTouchpadAndAction);
    }
    return;
}

void TouchpadGestureManager::swipeGestureBegin(int fingerCount, quint32 time)
{
    Q_UNUSED(fingerCount)
    Q_UNUSED(time)
    m_touchpadReg->startRecognizer(Gesture::GestureType::Swipe,fingerCount,time);
}
void TouchpadGestureManager::swipeGestureUpdate(const QSizeF &delta, quint32 time)
{
    Q_UNUSED(delta)
    Q_UNUSED(time)
    m_touchpadReg->updateRecognizer(Gesture::GestureType::Swipe,delta,time);
}
void TouchpadGestureManager::swipeGestureEnd(quint32 time)
{
    Q_UNUSED(time)
    m_touchpadReg->endRecognizer(Gesture::GestureType::Swipe,time);

}
void TouchpadGestureManager::swipeGestureCancelled(quint32 time)
{
    Q_UNUSED(time)
    m_touchpadReg->cancelRecognizer(Gesture::GestureType::Swipe,time);
}


void TouchpadGestureManager::tapGestureBegin(int fingerCount, quint32 time)
{
    Q_UNUSED(fingerCount)
    Q_UNUSED(time)
    m_touchpadReg->startRecognizer(Gesture::GestureType::Tap,fingerCount,time);
}
void TouchpadGestureManager::tapGestureUpdate(const QSizeF &delta, quint32 time)
{
    Q_UNUSED(delta)
    Q_UNUSED(time)
    m_touchpadReg->updateRecognizer(Gesture::GestureType::Tap,delta,time);
}
void TouchpadGestureManager::tapGestureEnd(quint32 time)
{
    Q_UNUSED(time)
    m_touchpadReg->endRecognizer(Gesture::GestureType::Tap,time);
}
void TouchpadGestureManager::tapGestureCancelled(quint32 time)
{
    Q_UNUSED(time)
    m_touchpadReg->cancelRecognizer(Gesture::GestureType::Tap,time);
}

}
