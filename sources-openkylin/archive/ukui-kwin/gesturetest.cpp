#include "gesturetest.h"

#include <iostream>
#include <QProcessEnvironment>

#include "screens.h"

namespace KWin {

GestureTest* GestureTest::_self = nullptr;

GestureTest* GestureTest::self(QObject *parent){
    if(!_self)
        _self = new GestureTest(parent);
    return _self;
}

GestureTest::GestureTest(QObject *parent):
    QObject(parent),
    m_gestureRecognizer(nullptr)
{
    QString kwin_touch_debug = QProcessEnvironment::systemEnvironment().value("KWIN_TOUCH_DEBUG");
    if(kwin_touch_debug == "ON"){
        init();
        std::cout << "开启kwin触摸的调试模式!" << std::endl;
    }
}

void GestureTest::init()
{
    m_gestureRecognizer = new GestureRecognizer(this);
    //! 初始化手势配置
    // 滑动手势
    for(int fingercount = 1; fingercount <= 5; ++fingercount){
        for(int dir = 0; dir < 4; ++dir){
            m_swipe[fingercount][dir] = new SwipeGesture(this);
            SwipeGesture* &g = m_swipe[fingercount][dir];
            g->setFingerCount(fingercount);
            g->setDirection(static_cast<Gesture::GestureDirection>(dir));
            g->setMinSwipeDistance(80);
            g->setStartGeometry(screens()->geometry(screens()->number(QPoint(0, 0))));
            connect(g, &SwipeGesture::started, this, [=](){
                //printf("[%s]:SwipeGesture-started: ", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
                printSwipeGesture(g);
            });
            connect(g, &SwipeGesture::triggered, this, [=](){
                //printf("[%s]:SwipeGesture-triggered\n\n", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
            });
            connect(g, &SwipeGesture::reach, this, [=](){
                //printf("\t[%s]:SwipeGesture-reach\n", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
            });
            connect(g, &SwipeGesture::cancelled, this, [=](){
                //printf("[%s]: SwipeGesture-cancelled\n\n", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
            });
            m_gestureRecognizer->registerGesture(g);
        }
    }

    // 捏合手势
    for(int fingercount = 2; fingercount <= 5; ++fingercount){
        for(int dir = 0; dir < 2; ++dir){
            m_pinch[fingercount][dir] = new PinchGesture(this);
            PinchGesture* &g = m_pinch[fingercount][dir];
            g->setFingerCount(fingercount);
            g->setDirection(static_cast<Gesture::GestureDirection>(dir + 4));
            connect(g, &PinchGesture::triggered, this, [=](){
                printPinchGesture(g);
                //printf("\n");
            });
            m_gestureRecognizer->registerGesture(g);
        }
    }

    // 长按手势
    for(int fingercount = 1; fingercount <= 5; ++fingercount){
        m_longPress[fingercount] = new LongPressGesture(this);
        LongPressGesture* &g = m_longPress[fingercount];
        g->setFingerCount(fingercount);
        g->setTimeout(1000);
        connect(g, &LongPressGesture::started, this, [=](){
            //printf("[%s]:LongPressGesture-started: ", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
            printLongPressGesture(g);
        });
        connect(g, &LongPressGesture::triggered, this, [=](){
            //printf("[%s]:LongPressGesture-triggered\n\n", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
        });
        connect(g, &LongPressGesture::cancelled, this, [=](){
            //printf("[%s]: LongPressGesture-cancelled\n\n", QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());
        });
        m_gestureRecognizer->registerGesture(g);
    }
}

void GestureTest::startGesture(int id, QPointF pos){
    if(!m_gestureRecognizer) return;
    m_gestureRecognizer->startGesture(id, pos);
}

void GestureTest::updateGesture(int id, QPointF pos){
    if(!m_gestureRecognizer) return;
    m_gestureRecognizer->updateGesture(id, pos);
}

void GestureTest::endGesture(int id, QPointF pos){
    if(!m_gestureRecognizer) return;
    m_gestureRecognizer->endGesture(id, pos);
}

void GestureTest::printSwipeGesture(SwipeGesture *gesture){
    QString info = QString("SwipeGesture 手势! \n"
                           "\t手指数量：%1\n"
                           "\t手势方向：%2\n"
                           "\t手势滑动的最小距离：%3\n"
                           "\t手势滑动的最大距离：%4\n"
                           "\t手势的起点的范围：(%5,%6,%7,%8)\n")
            .arg(gesture->fingerCount())
            .arg(dirToStr(gesture->direction()))
            .arg(gesture->minSwipeDistance())
            .arg(gesture->maxSwipeDistance())
            .arg(gesture->startGeometry().x())
            .arg(gesture->startGeometry().y())
            .arg(gesture->startGeometry().width())
            .arg(gesture->startGeometry().height());
    std::cout << info.toStdString();
}

void GestureTest::printPinchGesture(PinchGesture *gesture){
    QString info = QString("PinchGesture 手势！\n"
                           "\t手指数量：%1\n"
                           "\t手势方向：%2\n")
            .arg(gesture->fingerCount())
            .arg(dirToStr(gesture->direction()));
    std::cout << info.toStdString();
}

void GestureTest::printLongPressGesture(LongPressGesture *gesture){
    QString info = QString("LongPressGesture 手势！\n"
                           "\t手指数量：%1\n"
                           "\t触发时间：>%2ms\n")
            .arg(gesture->fingerCount())
            .arg(gesture->timeout());
    std::cout << info.toStdString();
}

QString GestureTest::dirToStr(Gesture::GestureDirection direction){
    switch (direction) {
    case Gesture::GestureDirection::Down: return "Down";
    case Gesture::GestureDirection::In: return "In";
    case Gesture::GestureDirection::NotSupport: return "NotSupport";
    case Gesture::GestureDirection::Left: return "Left";
    case Gesture::GestureDirection::Out: return "Out";
    case Gesture::GestureDirection::Right: return "Right";
    case Gesture::GestureDirection::Up: return "Up";
    case Gesture::GestureDirection::NoDirection: return "NoDirection";
    default: return "No Direction";
    }
}

}
