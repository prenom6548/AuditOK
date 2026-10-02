/********************************************************************
 UKUI-KWin - the UKUI3.0 window manager
 This file is part of the UKUI project
 The ukui-kwin is forked from kwin

Copyright (C) 2014-2020 kylinos.cn

 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2017 Martin Gräßlin <mgraesslin@kde.org>

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

/*
 * KWin UKUI Global Gesture
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * Authors: Yunpeng Zhu <zhuyunpeng@kylinos.cn>
 *
 */
#ifndef KWIN_GESTURES_H
#define KWIN_GESTURES_H

#include <ukui-kwin_export.h>

#include <QObject>
#include <QPointF>
#include <QSizeF>
#include <QMap>
#include <QVector>
#include <QString>
#include <QRect>
#include <QTime>
#include <QTimer>

#include <unordered_map>

// 触摸
#define MAX_SWIPE_ANGLE (90)                // 滑动过程中的容错角度
#define MIN_SWIPE_DIST (20)                 // 最小的滑动距离px
#define SWIPE_REACH_DELTA (40)              // 滑动过程中触发
#define MIN_PINCH_DIST (40)                 // 进入滑动状态的最小变化量px
#define LONGPRESS_DEFAULT_TIMEOUT (200)     // 长按手势的默认触发时间

#define dist(x1,y1,x2,y2) (double)(sqrt(pow(x1-x2,2)+pow(y1-y2,2)))

// 触摸板
#define MIN_TOUCHPAD_SWIPE_DIST  (20)         // 触摸板 Swipe 手势开始的执行的最小滑动距离
#define STATE_UPDATE_SPAN (40)                // 进行cur、last状态切换的间隔

namespace KWin
{

class Gesture : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief 手势枚举类型
     */
    enum class GestureType{
        Swipe
        ,Pinch
        ,LongPress
        ,Tap
        ,SwipeSequence
        ,Waiting // 还未明确下来该手势
    };

    //! 标记手势是哪个设备的手势
    enum class TouchDevice{
        TouchScreen,        // 触摸屏
        TouchPad            // 触摸板
    };

    enum class GestureDirection{
        // Swipe
        Down = 0
        ,Left
        ,Up
        ,Right
        // Pinch
        ,In = 4
        ,Out
        // other
        ,NoDirection    // 手势还不明确
        ,NotSupport     // 不支持该手势
    };
    virtual ~Gesture() = 0;

    virtual GestureType gestureType() = 0;
    virtual TouchDevice touchDevice() = 0;
    bool isStart(){ return m_isStart; }

protected:
    explicit Gesture(QObject *parent);
    bool m_isStart;                     // 标记手势是否开始

Q_SIGNALS:
    /**
     * Matching of a gesture started and this Gesture might match.
     * On further evaluation either the signal @ref triggered or
     * @ref cancelled will get emitted.
     */
    void started();
    /**
     * Gesture matching ended and this Gesture matched.
     */
    void triggered();
    /**
     * This Gesture no longer matches.
     */
    void cancelled();
};

class SwipeGesture : public Gesture
{
    Q_OBJECT
public:

    GestureType gestureType() override { return GestureType::Swipe; }
    TouchDevice touchDevice() override { return TouchDevice::TouchScreen; }

    explicit SwipeGesture(QObject *parent = nullptr);
    ~SwipeGesture() override;

    void setFingerCount(uint count) {
        if(count < 1 || count > 5) count = 0;
        m_fingerCount = count;
    }
    uint fingerCount() const {
        return m_fingerCount;
    }

    GestureDirection direction() const {
        return m_direction;
    }
    void setDirection(GestureDirection direction) {
        if((int)direction < 0 || (int)direction > 3) direction = GestureDirection::Down;
        m_direction = direction;
    }

    void setMinSwipeDistance(int diatance) {
        if(diatance < MIN_SWIPE_DIST) diatance = MIN_SWIPE_DIST;
        m_minimumR = diatance;
    }
    int minSwipeDistance() const {
        return m_minimumR;
    }

    void setMaxSwipeDistance(int diatance) {
        if(diatance < MIN_SWIPE_DIST) diatance = MIN_SWIPE_DIST;
        m_maximumR = diatance;
    }
    int maxSwipeDistance() const {
        return m_maximumR;
    }

    void setStartGeometry(const QRect &geometry){
        m_startGeometry = geometry;
    }

    QRect startGeometry(){
        return m_startGeometry;
    }

Q_SIGNALS:
    /*!
     * 目前的手势支持中，并不会发出该信号
     * 接口的保留，为了edge的特效
     */
    void progress(qreal);

    /*!
     * 每当手势的滑动距离到达 SWIPE_REACH_DELTA 时触发
     */
    void reach();

private:
    uint m_fingerCount = 0;
    GestureDirection m_direction;
    int m_minimumR = 0;
    int m_maximumR = 0;
    QRect m_startGeometry;
};
/*!
 * \brief 捏合手势
 */
class PinchGesture : public Gesture{
    Q_OBJECT
public:

    GestureType gestureType() override { return GestureType::Pinch; }
    TouchDevice touchDevice() override { return TouchDevice::TouchScreen; }

    explicit PinchGesture(QObject *parent = nullptr);
    ~PinchGesture() override;

    void setDirection(GestureDirection direction){
        if((int)direction < 4 || (int)direction > 5) direction = GestureDirection::In;
        m_direction = direction;
    }

    GestureDirection direction(){
        return m_direction;
    }

    void setFingerCount(int count){
        if(count < 2 || count > 5) count = 0;
        m_fingerCount = count;
    }

    int fingerCount(){
        return m_fingerCount;
    }

private:
    int m_fingerCount;
    GestureDirection m_direction;
};

/*!
 * \brief 长按手势
 */
class LongPressGesture : public Gesture{
    Q_OBJECT
public:
    GestureType gestureType() override { return GestureType::LongPress; }
    TouchDevice touchDevice() override { return TouchDevice::TouchScreen; }

    explicit LongPressGesture(QObject *parent = nullptr);
    ~LongPressGesture() override;

    void setTimeout(int timeout){
        if(timeout < 0) timeout = 0;
        m_timeout = timeout;
    }

    int timeout(){
        return m_timeout;
    }

    void setFingerCount(int count){
        if(count < 1 || count > 5) count = 0;
        m_fingerCnt = count;
    }

    int fingerCount(){
        return m_fingerCnt;
    }

private:
    int m_timeout;      // 长按手势的触发时间 ms
    int m_fingerCnt;    // 手势触发要求的手指数量
};

/*!
 * \author Yunpeng Zhu.
 * \brief 记录外接圆参数
 */
typedef struct _external_circle{
    QPointF center;
    double r;
}ExternalCircle;
/*!
 * \brief 记录外接矩形参数
 */
typedef struct _external_rect{
    double x, y, w, h;          // 外接矩形的范围
    double diagL;               // 对角线长度
    QPointF center;             // 外接矩形的中心

    QString toQString(){
        QString res = QString("QRect(%1, %2, %3, %4);"
                              "DiagL=%5;"
                              "Center(%6, %7)").arg(x).arg(y).arg(w).arg(h)
                .arg(diagL)
                .arg(center.x()).arg(center.y());
        return res;
    }

}ExternalRect;

/*!
 *  \author Yunpeng Zhu.
 *  \brief 记录当前触摸手势状态
 */
class TouchState{
public:
    static const int MAX_SLOT_NUM = 10;

    ExternalRect initRect, lastRect, curRect;   // 多指抽象成单指的位置，目前采用外接矩形做匹配
    QPointF slotInit[MAX_SLOT_NUM];             // 开始各触摸点的位置
    QPointF slotLast[MAX_SLOT_NUM];             // 上次各触摸点的位置
    QPointF slotPoints[MAX_SLOT_NUM];           // 当前各触摸点的位置
    int slotMask;                               // 记录(0 ~ 9)的使用情况
    std::unordered_map<int, int> slotid;        // detail -> (0 ~ 9)
    QTime beginTime;                            // 记录手势开始的时间

    TouchState(): slotMask(0){}
    //! 处理驱动上报的事件异常，事件不成对时的异常
    void insertSlot(const int &detail, const QPointF &pos);
    void updateSlot(const int &detail, const QPointF &pos);
    void deleteSlot(const int &detail);
    void gestureDetector(const int &detail, const QPointF &pos);
    int fingerCnt() { return slotid.size(); }
    QPointF getInitPos(const int &detail){ return slotInit[slotid[detail]]; }   // 得到触摸点的初始位置
    QPointF getCurPos(const int &detail){ return slotPoints[slotid[detail]]; }  // 得到当前点的位置
    QPointF getLastPos(const int &detail){ return slotLast[slotid[detail]]; }   // 得到上次的触摸位置
    int getLastTime();                  // 获取手势的持续时间
    void reset();                       // 重置触摸检测的状态

    /*!
     * \brief 计算三点的外接圆
     * \param p1, p2, p3
     * \return
     */
    ExternalCircle calculateEC(QPointF p1, QPointF p2, QPointF p3);
    /*!
     * \brief 获取点集的外接矩形
     */
    ExternalRect calculateRect(const QVector<QPointF> &points);
};

class UKUI_KWIN_EXPORT GestureRecognizer : public QObject
{
    Q_OBJECT
public:
    GestureRecognizer(QObject *parent = nullptr);
    ~GestureRecognizer() override;

    void registerGesture(Gesture *gesture);
    void unregisterGesture(Gesture *gesture);

    /*!
     * \author Yunpeng Zhu.
     */
    int startGesture(int detail, const QPointF &pos);
    void updateGesture(int detail, const QPointF &pos);
    void cancelGesture();
    void cancelActiveGesture();
    void endGesture(int detail, const QPointF &pos);

    bool detectSwpie(SwipeGesture *gesture);

private:
    /*!
     * \brief 重置手势检测状态
     */
    void reset();
    /*!
     * \brief 计算单指的滑动方向
     * \param detail = -1 表示计算多指外接矩形的滑动方向
     */
    Gesture::GestureDirection calculateSingleDirection(const int detail = -1);
    /*!
     * \brief 计算detail表示的手指的滑动距离
     * \param detail = -1 时 表示计算多指外接矩形的滑动距离
     * \return
     */
    double calculateSingleDistance(int detail = -1);
    /*!
     * \brief 计算多指捏合方向
     * \return 手势方向
     */
    Gesture::GestureDirection calculatePinchDirection();
    /*!
     * \brief 计算手指间的偏移量
     * \return
     */
    double calculateFingerDelta();

    QString directionToQString(Gesture::GestureDirection direction);
    QString typeToQString(Gesture::GestureType type);


    QVector<Gesture*> m_gestures;           // 所有已经
    QVector<Gesture*> m_activeGestures;     // 记录当前已经激活的手势
    QVector<QTimer*>  m_longPressTimers;    // 对于长按手势，开启定时器去发送started信号
    bool gestureDectorState;                // 是否开始手势检测
    TouchState m_touchState;                // 保存触摸点数据
    Gesture::GestureDirection m_direction;  // 记录当前手势方向
    Gesture::GestureType m_gestureType;     // 记录当前手势类型
    QMap<Gesture*, QMetaObject::Connection>  m_destroyConnections;

    int m_swipeReachCount;                  // 保存当前滑动手势触发的进度
};


class TouchpadSwipeGesture : public Gesture
{
public:
    GestureType gestureType() override{ return GestureType::Swipe;}
    TouchDevice touchDevice() override { return TouchDevice::TouchPad; }

    explicit TouchpadSwipeGesture(QObject *parent = nullptr);
    ~TouchpadSwipeGesture();

    void setFingersCount(int fingerCount){
        if(fingerCount<=2 || fingerCount >= 5){
            fingerCount = 0;
        }
        m_fingerCount= fingerCount;
    }
    int fingerCount(){
        return m_fingerCount;
    }

    GestureDirection direction() const {
        return m_direction;
    }
    void setDirection(GestureDirection direction) {
        m_direction = direction;
    }

    qreal minSwipeDistance(){
        return m_minSwipeDistance;
    }

    void setMinSwipeDistance(qreal minSwipeDistance){
        m_minSwipeDistance = minSwipeDistance;
    }

private:
    int m_fingerCount = 0;
    GestureDirection m_direction = KWin::Gesture::GestureDirection::Down;
    //! 手势触发的最小滑动距离
    qreal m_minSwipeDistance;
};

class TouchpadTapGesture : public Gesture
{
public:
    GestureType gestureType() override { return GestureType::Tap; }
    TouchDevice touchDevice() override { return TouchDevice::TouchPad; }

    explicit TouchpadTapGesture(QObject *parent = nullptr);
    ~TouchpadTapGesture() override;

    void setFingersCount(int fingerCount){
        if(fingerCount != 3 && fingerCount != 4){
            fingerCount = 0;
        }
        m_fingerCount = fingerCount;
    }
    int fingerCount(){
        return m_fingerCount;
    }
    void setIcount(int icount){
        m_icount = icount;
    }
    int icount(){
        return m_icount;
    }

private:
    int m_fingerCount;
    int m_icount;
};

/* 该手势只关心滑动的手指数量，当方向改变时，会发出Direction-Change的信号 */
class TouchpadSwipeSequenceGesture : public Gesture{
    Q_OBJECT
public:
    GestureType gestureType() override{ return GestureType::SwipeSequence; }
    TouchDevice touchDevice() override { return TouchDevice::TouchPad; }

    explicit TouchpadSwipeSequenceGesture(QObject *parent = nullptr);
    ~TouchpadSwipeSequenceGesture();

    void setFingersCount(int fingerCount){
        if(fingerCount <= 2 || fingerCount >= 5){
            fingerCount = 0;
        }
        m_fingerCount = fingerCount;
    }
    int fingerCount(){
        return m_fingerCount;
    }

Q_SIGNALS:
    void sigNotifyGestureDirection(KWin::Gesture::GestureDirection direction);

private:
    const int StepMinSpan = 5; // 触发一次direction changed的最小间隔
    int m_fingerCount = 0;
    GestureDirection m_direction = KWin::Gesture::GestureDirection::Down;
};


struct TouchpadGestureState{
    Gesture::GestureType gestureType;
    //! X,Y 不要理解为二维坐标，按照各自一维坐标去理解
    //! 将sum，last，cur，理解为三种状态向量，则状态切换公式为 last + cur = sum
    //! last 与 cur 发生状态切换的阈值为 STATE_UPDATE_SPAN
    // 自手势开始，到当前状态的手势偏移量
    qreal sumDeltaX;
    qreal sumDeltaY;
    // 上次确定手势方向时，手势的总偏移量
    // 可以将手势状态确定理解为关键点，该值为当前值和关键点相对的偏移量
    qreal lastDeltaX;
    qreal lastDeltaY;
    // current delta 相对与last delta 的偏移量
    qreal curDeltaX;
    qreal curDeltaY;

    // 手势的手指数量
    int fingerCount;

    // 手势的方向是否发生变化
    bool isGestureDirectionChange;

    // 手势的方向
    Gesture::GestureDirection gestureDirection;

    TouchpadGestureState(){ init(); }
    // 初始化参数
    void init();
};

class TouchpadGestureRecognizer : public QObject
{
    Q_OBJECT
public:
    TouchpadGestureRecognizer(QObject *parent=nullptr);

    ~TouchpadGestureRecognizer() override;
    bool threeFigerCount = false;
    bool fourFigerCount = false;

    void registerGesture(Gesture *gesture);
    void unregisterGesture(Gesture *gesture);

    void startRecognizer(Gesture::GestureType flag, int fingerCount,quint32 time);
    void updateRecognizer(Gesture::GestureType flag,const QSizeF &delta,quint32 time);
    void endRecognizer(Gesture::GestureType flag,quint32 time);
    void cancelRecognizer(Gesture::GestureType flag,quint32 time);
private:

    QVector<Gesture*> m_gesture;
    QVector<Gesture*> m_activeGesture;
    QMap<Gesture*,QMetaObject::Connection> m_destroyConnections;

    TouchpadGestureState m_gestureState;
    Gesture::GestureDirection calculateDirection(const QSizeF &delta);
    void clearDate();

};

}

Q_DECLARE_METATYPE(KWin::Gesture::GestureDirection)
Q_DECLARE_METATYPE(KWin::Gesture::GestureType)
Q_DECLARE_METATYPE(KWin::Gesture::TouchDevice)

#endif
