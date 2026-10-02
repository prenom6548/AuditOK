#include "touchpad_gesture_spy.h"
#include "gestures.h"
#include "main.h"
#include "platform.h"
#include "touch_gesture_action_control.h"
#include "xmlreader.h"

namespace KWin
{

/*
 *  TouchPad
 *
 */
KWIN_SINGLETON_FACTORY(TouchpadGestureSpy)
TouchpadGestureSpy::TouchpadGestureSpy(QObject *parent)
    : QObject(parent)
    , InputEventSpy()
    , m_touchpadReg(new TouchpadGestureRecognizer())
{
    // 辨別手勢
    QMap<GestureInfo, GestureConfig> gestureTouchpadConfig = XMLReader::getInstance()->getGestureConfig();
    auto gestureTouchpadSpecifics = gestureTouchpadConfig.keys();
    for (auto gestureTouchpadSpecific : gestureTouchpadSpecifics) {
        auto gestureAction = gestureTouchpadConfig[gestureTouchpadSpecific];
        m_gestureTouchpadAndAction = new GestureAndActionBase(gestureTouchpadSpecific, gestureAction);
        m_touchpadAction << m_gestureTouchpadAndAction;
        // 注册手势
        registerTouchpadGesture(m_gestureTouchpadAndAction);
    }
    // 将文件与更新手势关联  修改配置文件之后可进行手势更新
    QFile gestureTouchpadConfigFile(XMLReader::getInstance()->getConfigFile());
    if (gestureTouchpadConfigFile.exists()) {
        m_gestureWatcher = new QFileSystemWatcher();
        m_gestureWatcher->addPath(XMLReader::getInstance()->getConfigFile());
        connect(m_gestureWatcher, &QFileSystemWatcher::fileChanged, this, &TouchpadGestureSpy::updateTouchpadGesture);
    } else {
        // printf("this file is not exist");
    }
}
TouchpadGestureSpy::~TouchpadGestureSpy()
{
    delete m_touchpadReg;
    qDeleteAll(m_gesture.begin(), m_gesture.end());
    qDeleteAll(m_touchpadAction.begin(), m_touchpadAction.end());
    if (m_gestureWatcher) {
        delete m_gestureWatcher;
    }
    s_self = nullptr;
}

void TouchpadGestureSpy::registerTouchpadGesture(GestureAndActionBase *action)
{

    switch (action->getDevice()) {
    case TouchDevice::TouchPad: {
        switch (action->getGestureType()) {
        case GestureType::SWIPE: {
            touchpadSwipe = new TouchpadSwipeGesture();
            touchpadSwipe->setFingersCount(m_gestureTouchpadAndAction->getFingers());
            touchpadSwipe->setDirection(m_gestureTouchpadAndAction->getGestureDirection());
            m_touchpadReg->registerGesture(touchpadSwipe);
            m_gesture << touchpadSwipe;
            connect(touchpadSwipe, &GlobalGesture::started, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionStart);
            connect(touchpadSwipe, &GlobalGesture::triggered, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionTriggered);
            connect(touchpadSwipe, &GlobalGesture::cancelled, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionTriggered);
            break;
        }
        case GestureType::TAP: {
            touchpadTap = new TouchpadTapGesture();
            touchpadTap->setFingersCount(m_gestureTouchpadAndAction->getFingers());
            m_touchpadReg->registerGesture(touchpadTap);
            m_gesture << touchpadTap;
            /*
             *    三指四指的点击事件是只有在点击结束的时候才会触发 tap点击事件整个过程很短
             */
            //            connect(touchpadTap,&Gesture::started,m_gestureTouchpadAndAction,&GestureAndActionBase::onGestureActionStart);
            connect(touchpadTap, &GlobalGesture::triggered, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionTriggered);
            connect(touchpadTap, &GlobalGesture::cancelled, m_gestureTouchpadAndAction, []() {
                // printf("tap is cancel\n");
            });
            break;
        }
        case GestureType::SWIPESEQUENCE: {
            m_touchpadSwipe2 = new TouchpadSwipeSequenceGesture;
            m_touchpadSwipe2->setFingersCount(m_gestureTouchpadAndAction->getFingers());
            m_touchpadReg->registerGesture(m_touchpadSwipe2);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::started, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionStart);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::triggered, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionTriggered);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::cancelled, m_gestureTouchpadAndAction, &GestureAndActionBase::onGestureActionTriggered);
            connect(m_touchpadSwipe2, &TouchpadSwipeSequenceGesture::sigNotifyGestureDirection, [=](KWin::GlobalGesture::GestureDirection direction) {
                switch (direction) {
                case GlobalGesture::GestureDirection::Down:
                    m_gestureTouchpadAndAction->onAllDirectionKey(GlobalGesture::GestureDirection::Down);
                    break;
                case GlobalGesture::GestureDirection::Up:
                    m_gestureTouchpadAndAction->onAllDirectionKey(GlobalGesture::GestureDirection::Up);
                    break;
                case GlobalGesture::GestureDirection::Left:
                    m_gestureTouchpadAndAction->onAllDirectionKey(GlobalGesture::GestureDirection::Left);
                    break;
                case GlobalGesture::GestureDirection::Right:
                    m_gestureTouchpadAndAction->onAllDirectionKey(GlobalGesture::GestureDirection::Right);
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

void TouchpadGestureSpy::updateTouchpadGesture()
{
    // printf("====配置文件更新  触发更新手势功能==========updateTouchpadGesture----");
    m_gestureWatcher->addPath(XMLReader::getInstance()->getConfigFile());

    XMLReader::getInstance()->clearGestureConfig();
    XMLReader::getInstance()->phraseXml();
    QMap<GestureInfo, GestureConfig> touchpadGestureConfig = XMLReader::getInstance()->getGestureConfig();
    auto touchpadGestureSpecifics = touchpadGestureConfig.keys();

    for (int i = 0; i < m_gesture.size(); i++) {

        if (m_gesture[i]->gestureType() == GlobalGesture::GlobalGestureType::Swipe) {
            touchpadSwipe = static_cast<TouchpadSwipeGesture *>(m_gesture[i]);
            m_touchpadReg->unregisterGesture(touchpadSwipe);
            touchpadSwipe->disconnect();
        } else if (m_gesture[i]->gestureType() == GlobalGesture::GlobalGestureType::Tap) {
            touchpadTap = static_cast<TouchpadTapGesture *>(m_gesture[i]);
            m_touchpadReg->unregisterGesture(touchpadTap);
            touchpadTap->disconnect();
        } else if (m_gesture[i]->gestureType() == GlobalGesture::GlobalGestureType::SwipeSequence) {
            m_touchpadSwipe2 = static_cast<TouchpadSwipeSequenceGesture *>(m_gesture[i]);
            m_touchpadReg->unregisterGesture(m_touchpadSwipe2);
            m_touchpadSwipe2->disconnect();
        }
    }
    for (QVector<GlobalGesture *>::iterator it = m_gesture.begin(); it != m_gesture.end(); it++) {
        GlobalGesture *deleteTouchpadGesture = (*it);
        delete (deleteTouchpadGesture);
    }
    for (QList<GestureAndActionBase *>::iterator it = m_touchpadAction.begin(); it != m_touchpadAction.end(); it++) {
        GestureAndActionBase *deleteAction = (*it);
        delete (deleteAction);
    }
    m_gesture.clear();
    m_touchpadAction.clear();

    for (auto touchpadGestureSpecific : touchpadGestureSpecifics) {
        auto touchpadGestureAction = touchpadGestureConfig[touchpadGestureSpecific];
        m_gestureTouchpadAndAction = new GestureAndActionBase(touchpadGestureSpecific, touchpadGestureAction);
        m_touchpadAction << m_gestureTouchpadAndAction;
        registerTouchpadGesture(m_gestureTouchpadAndAction);
    }
    return;
}

void TouchpadGestureSpy::swipeGestureBegin(int fingerCount, quint32 time)
{

    m_touchpadReg->startRecognizer(GlobalGesture::GlobalGestureType::Swipe, fingerCount, time);
}
void TouchpadGestureSpy::swipeGestureUpdate(const QSizeF &delta, quint32 time)
{

    m_touchpadReg->updateRecognizer(GlobalGesture::GlobalGestureType::Swipe, delta, time);
}
void TouchpadGestureSpy::swipeGestureEnd(quint32 time)
{

    m_touchpadReg->endRecognizer(GlobalGesture::GlobalGestureType::Swipe, time);
}
void TouchpadGestureSpy::swipeGestureCancelled(quint32 time)
{

    m_touchpadReg->cancelRecognizer(GlobalGesture::GlobalGestureType::Swipe, time);
}

void TouchpadGestureSpy::tapGestureBegin(int fingerCount, quint32 time)
{

    m_touchpadReg->startRecognizer(GlobalGesture::GlobalGestureType::Tap, fingerCount, time);
}

}
