/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2022 KylinSoft Co., Ltd.

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "globalgestures.h"

#include "input.h"
#include "toplevel.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>

#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QRect>
#include <QTime>

namespace KWin
{

GlobalGesture::GlobalGesture(QObject *parent)
    : QObject(parent)
    , m_isStart(false)
{
    connect(this, &GlobalGesture::started, this, [=]() {
        m_isStart = true;
    });
    connect(this, &GlobalGesture::cancelled, this, [=]() {
        m_isStart = false;
    });
    connect(this, &GlobalGesture::triggered, this, [=]() {
        m_isStart = false;
    });
}

GlobalGesture::~GlobalGesture() = default;

/*! 滑动手势 */
GlobalSwipeGesture::GlobalSwipeGesture(QObject *parent)
    : GlobalGesture(parent)
{
    // 设置手势配置默认值
    m_fingerCount = 0;
    m_minimumR = MIN_SWIPE_DIST;
    m_maximumR = INT32_MAX;
    m_direction = GestureDirection::Down;
    m_startGeometry = QRect(0, 0, 0, 0);
}

GlobalSwipeGesture::~GlobalSwipeGesture() = default;

/*! Pinch手势 */
GlobalPinchGesture::GlobalPinchGesture(QObject *parent)
    : GlobalGesture(parent)
{
    m_direction = GestureDirection::In;
    m_fingerCount = 0;
}

GlobalPinchGesture::~GlobalPinchGesture() = default;

/*! LongPress手势 */
GlobalLongPressGesture::GlobalLongPressGesture(QObject *parent)
    : GlobalGesture(parent)
{
    m_timeout = LONGPRESS_DEFAULT_TIMEOUT;
}

GlobalLongPressGesture::~GlobalLongPressGesture() = default;

/*! Touchpad Swipe手势 */
TouchpadSwipeGesture::TouchpadSwipeGesture(QObject *parent)
    : GlobalGesture(parent)
{
    m_fingerCount = 0;
    m_minSwipeDistance = MIN_TOUCHPAD_SWIPE_DIST;
}

TouchpadSwipeGesture::~TouchpadSwipeGesture() = default;

/*! Touchpad Tap手势 */
TouchpadTapGesture::TouchpadTapGesture(QObject *parent)
    : GlobalGesture(parent)
{
    m_fingerCount = 0;
}

TouchpadTapGesture::~TouchpadTapGesture() = default;

/*! Touchpad Swipe Sequence手势 */
TouchpadSwipeSequenceGesture::TouchpadSwipeSequenceGesture(QObject *parent)
    : GlobalGesture(parent)
{
    m_fingerCount = 0;
}

TouchpadSwipeSequenceGesture::~TouchpadSwipeSequenceGesture() = default;

/*! 保存触摸状态 */
void GlobalTouchState::reset()
{
    beginTime = QTime::currentTime();
    // 手指数据清空
    QVector<QPointF> points;
    for (auto it = slotid.begin(); it != slotid.end(); ++it) {
        int &id = it->second;
        slotInit[id] = slotPoints[id];
        slotLast[id] = slotPoints[id];
        points.append(slotInit[id]);
    }
    initRect = calculateRect(points);
    curRect = lastRect = initRect;
}

void GlobalTouchState::insertSlot(const int &detail, const QPointF &pos)
{
    int id = 0;
    while (slotMask & (1 << id)) {
        id++;
    }
    if (id > MAX_SLOT_NUM - 1) {
        qDebug() << "Touching more than the maximum number of fingers, see MAX_SLOT_NUM";
        return;
    }
    slotMask |= 1 << id;
    slotPoints[id] = pos;
    slotInit[id] = pos;
    slotLast[id] = pos;
    slotid[detail] = id;
    // 计算多指下手指的抽象中心
    QVector<QPointF> points;
    for (auto it = slotid.begin(); it != slotid.end(); ++it) {
        points.append(slotInit[it->second]);
    }
    initRect = calculateRect(points);
    curRect = initRect;
}

void GlobalTouchState::updateSlot(const int &detail, const QPointF &pos)
{
    //! 屏蔽触摸坐标的抖动
    QPointF cp = getCurPos(detail);
    if (dist(cp.x(), cp.y(), pos.x(), pos.y()) > 4) {
        int id = slotid[detail];
        slotLast[id] = slotPoints[id];
        slotPoints[id] = pos;
    }

    //! 更新当前触摸状态的外接矩形
    QVector<QPointF> points;
    for (auto it = slotid.begin(); it != slotid.end(); ++it) {
        points.append(slotPoints[it->second]);
    }
    lastRect = curRect;
    curRect = calculateRect(points);
}

void GlobalTouchState::gestureDetector(const int &detail, const QPointF &pos)
{
}

void GlobalTouchState::deleteSlot(const int &detail)
{
    slotMask ^= 1 << slotid[detail];
    auto t = slotid.find(detail);
    slotid.erase(t);
}

int GlobalTouchState::getLastTime()
{
    int res = 0;
    QTime curTime = QTime::currentTime();
    res = beginTime.msecsTo(curTime);
    return res;
}

ExternalCircle GlobalTouchState::calculateEC(QPointF p1, QPointF p2, QPointF p3)
{
    ExternalCircle res;
    if (p1 == p2 && p1 == p3)
        return res;
    if (p1 == p2 || p2 == p3) {
        if (p1 == p2) {
            QPointF t = p2;
            p2 = p3;
            p3 = t;
        }
        res.r = dist(p1.x(), p1.y(), p2.x(), p2.y()) / 2;
        res.center.setX((p1.x() + p2.x()) / 2);
        res.center.setY((p1.y() + p2.y()) / 2);
        return res;
    }

    double a, b, c, p, s, sinc;
    double x1 = p1.x(), y1 = p1.y();
    double x2 = p2.x(), y2 = p2.y();
    double x3 = p3.x(), y3 = p3.y();
    a = dist(x1, y1, x2, y2); // 求出三角形三条边
    b = dist(x1, y1, x3, y3);
    c = dist(x2, y2, x3, y3);
    p = (a + b + c) / 2; // 半周长
    s = sqrt(p * (p - a) * (p - b) * (p - c)); // 海伦公式求面积
    std::cout << s << std::endl;
    sinc = (2 * s) / (a * b); // 求角c的正弦值，(1/2)*sinc*a*b = s;
    if (!sinc) { // 处理三点共线
        QPointF lb = QPointF(fmin(p1.x(), p2.x() < p3.x() ? p2.x() : p3.x()),
                             fmin(p1.y(), p2.y() < p3.y() ? p2.y() : p3.y()));
        QPointF rt = QPointF(fmax(p1.x(), p2.x() > p3.x() ? p2.x() : p3.x()),
                             fmax(p1.y(), p2.y() > p3.y() ? p2.y() : p3.y()));
        res.r = dist(lb.x(), lb.y(), rt.x(), rt.y()) / 2;
        res.center.setX((lb.x() + rt.x()) / 2);
        res.center.setY((lb.y() + rt.y()) / 2);
        return res;
    }
    res.r = c / (2 * sinc); // 2*r = c/sinc = a/sina = b/sinb
    // 求外接圆圆心
    double x = ((y2 - y1) * (y3 * y3 - y1 * y1 + x3 * x3 - x1 * x1) - (y3 - y1) * (y2 * y2 - y1 * y1 + x2 * x2 - x1 * x1)) / (2 * (x3 - x1) * (y2 - y1) - 2 * ((x2 - x1) * (y3 - y1)));
    double y = ((x2 - x1) * (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1) - (x3 - x1) * (x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1)) / (2 * (y3 - y1) * (x2 - x1) - 2 * ((y2 - y1) * (x3 - x1)));
    res.center = QPointF(x, y);
    return res;
}

ExternalRect GlobalTouchState::calculateRect(const QVector<QPointF> &points)
{
    ExternalRect rect;
    if (!points.size())
        return rect;
    double minx = points[0].x();
    double miny = points[0].y();
    double maxx = points[0].x();
    double maxy = points[0].y();
    for (const QPointF &p : points) {
        minx = fmin(minx, p.x());
        miny = fmin(miny, p.y());
        maxx = fmax(maxx, p.x());
        maxy = fmax(maxy, p.y());
    }
    rect.x = minx;
    rect.y = miny;
    rect.w = maxx - minx;
    rect.h = maxy - miny;
    rect.center.setX((minx + maxx) / 2);
    rect.center.setY((miny + maxy) / 2);
    rect.diagL = dist(minx, miny, maxx, maxy);
    return rect;
}

GlobalGestureRecognizer::GlobalGestureRecognizer(QObject *parent)
    : QObject(parent)
{
}

GlobalGestureRecognizer::~GlobalGestureRecognizer() = default;

void GlobalGestureRecognizer::registerGesture(KWin::GlobalGesture *gesture)
{
    Q_ASSERT(!m_gestures.contains(gesture));
    auto connection = connect(gesture, &QObject::destroyed, this, std::bind(&GlobalGestureRecognizer::unregisterGesture, this, gesture));
    m_destroyConnections.insert(gesture, connection);
    m_gestures << gesture;
}

void GlobalGestureRecognizer::unregisterGesture(KWin::GlobalGesture *gesture)
{
    auto it = m_destroyConnections.find(gesture);
    if (it != m_destroyConnections.end()) {
        disconnect(it.value());
        m_destroyConnections.erase(it);
    }
    m_gestures.removeAll(gesture);
    if (m_activeGestures.removeOne(gesture)) {
        if (gesture->isStart())
            Q_EMIT gesture->cancelled();
    }
}

/*!
 * \author Yunpeng Zhu.
 * \brief 这是^(*￣(oo)￣)^写的，牛牪犇不！
 */
int GlobalGestureRecognizer::startGesture(int detail, const QPointF &pos)
{
    // 当新触摸手指加入时，重置当前的检测状态
    reset();

    static int cnt = 0;

    m_touchState.insertSlot(detail, pos);
    int fingerCount = m_touchState.fingerCnt();

    QString processName = processNameByPos(m_touchState.curRect.center.toPoint());
    QString cmdline = cmdlineByPos(m_touchState.curRect.center.toPoint());

    qDebug() << "Current touch fingers: " << m_touchState.fingerCnt()
             << "Touch point: " << m_touchState.curRect.center.toPoint()
             << "process name: " << processName << "cmdline: " << cmdline;

    //! 判断手势类型 选择满足条件的手势进行激活
    for (GlobalGesture *gesture : qAsConst(m_gestures)) {

        bool isBlackListProc = false;

        // 手势是否发生在在 procname 黑名单的进程中中
        for (QString &proc : gesture->procBlacklist()) {
            //! \note 从 /proc/pid/status 中获取的 processName 只有15个字符，超过15个字符的进程会被截断
            if (proc.left(15) == processName) {
                isBlackListProc = true;
                break;
            }
        }
        if (isBlackListProc)
            continue;

        if (gesture->cmdBlacklist().indexOf(cmdline) != -1)
            continue;

        switch (gesture->gestureType()) {
        case GlobalGesture::GlobalGestureType::Swipe: {
            GlobalSwipeGesture *swipeGesture = qobject_cast<GlobalSwipeGesture *>(gesture);
            if (!detectSwpie(swipeGesture))
                continue;
            break;
        }
        case GlobalGesture::GlobalGestureType::Pinch: {
            GlobalPinchGesture *pg = dynamic_cast<GlobalPinchGesture *>(gesture);
            if (pg->fingerCount() != fingerCount)
                continue;
            break;
        }
        case GlobalGesture::GlobalGestureType::LongPress: {
            GlobalLongPressGesture *lg = dynamic_cast<GlobalLongPressGesture *>(gesture);
            if (lg->fingerCount() != fingerCount)
                continue;
            //! 对于long-press-gesture开启定时器
            QTimer *timer = new QTimer;
            timer->setInterval(lg->timeout());
            timer->start();
            m_longPressTimers.append(timer);
            connect(timer, &QTimer::timeout, this, [=]() {
                timer->stop();
                bool isCancel = false;
                double delta = calculateFingerDelta();
                if (delta > 5 * m_touchState.fingerCnt())
                    isCancel = true;
                if (!isCancel) {
                    Q_EMIT lg->started();
                    //! 清空当前除 long-press-gesture的所有激活的手势
                    for (auto it = m_activeGestures.begin(); it != m_activeGestures.end();) {
                        GlobalGesture *g = static_cast<GlobalGesture *>(*it);
                        if (g->gestureType() == GlobalGesture::GlobalGestureType::LongPress) {
                            ++it;
                        } else {
                            it = m_activeGestures.erase(it);
                        }
                    }
                }
            });
            break;
        }
        default:
            break;
        }
        m_activeGestures << gesture;
        ++cnt;
    }
    cnt = 0;

    return 0;
}

void GlobalGestureRecognizer::updateGesture(int detail, const QPointF &pos)
{
    //! 计算当前手势的移动方向
    m_touchState.updateSlot(detail, pos);

    double distance = 0;
    // 如果当前手势处于更新状态则去更新手势
    // not suppoort状态下，认为该手势已经不满足任何条件，不会触发任何动作
    if (m_direction != GlobalGesture::GestureDirection::NotSupport) {
        int fcnt = m_touchState.fingerCnt();
        if (fcnt == 1) {
            m_direction = calculateSingleDirection(detail);
            distance = calculateSingleDistance(detail);
            if (m_direction != GlobalGesture::GestureDirection::NoDirection
                && m_direction != GlobalGesture::GestureDirection::NotSupport)
                m_gestureType = GlobalGesture::GlobalGestureType::Swipe;
        } else {
            //! 滑动手势和捏合手势同时去竞争，谁先进入手势识别状态，则认为当前手势是谁
            //! 判断当前手势是否进入捏合状态
            if (m_gestureType == GlobalGesture::GlobalGestureType::Pinch || m_gestureType == GlobalGesture::GlobalGestureType::Waiting) {
                m_direction = calculatePinchDirection();
                if (m_direction != GlobalGesture::GestureDirection::NoDirection && m_direction != GlobalGesture::GestureDirection::NotSupport)
                    m_gestureType = GlobalGesture::GlobalGestureType::Pinch;
            }
            //! 判断是否进入滑动状态
            if (m_gestureType == GlobalGesture::GlobalGestureType::Swipe || m_gestureType == GlobalGesture::GlobalGestureType::Waiting) {
                m_direction = calculateSingleDirection();
                distance = calculateSingleDistance();
                if (m_direction != GlobalGesture::GestureDirection::NoDirection
                    && m_direction != GlobalGesture::GestureDirection::NotSupport)
                    m_gestureType = GlobalGesture::GlobalGestureType::Swipe;
            }
        }
    }

    //! 更新已经激活的手势的状态，对于已经不满足条件的手势进行剔除
    for (auto it = m_activeGestures.begin(); it != m_activeGestures.end();) {
        bool isDelete = false;
        GlobalGesture *g = static_cast<GlobalGesture *>(*it);

        if (m_gestureType != GlobalGesture::GlobalGestureType::Waiting && g->gestureType() != m_gestureType) {
            isDelete = true;
        } else {
            switch (g->gestureType()) {
            case GlobalGesture::GlobalGestureType::Swipe: {
                GlobalSwipeGesture *sg = dynamic_cast<GlobalSwipeGesture *>(g);

                if ((m_direction != GlobalGesture::GestureDirection::NoDirection
                     && sg->direction() != m_direction) //! 检测当前的滑动方向是否还满足该手势的要求
                    || distance > sg->maxSwipeDistance()) { //! 检测当前的滑动距离是否还满足该手势的要求

                    isDelete = true;
                    break;
                }

                //! 检测当前手势是否要开始更新
                if (distance > sg->minSwipeDistance()) {
                    if (g->isStart()) {
                        if (distance / (SWIPE_REACH_DELTA + 1) > m_swipeReachCount) {
                            ++m_swipeReachCount;
                            Q_EMIT sg->reach();
                        }
                    } else {
                        m_swipeReachCount = distance / (SWIPE_REACH_DELTA + 1);
                        Q_EMIT g->started();
                    }
                }
                break;
            }
            case GlobalGesture::GlobalGestureType::Pinch: {
                GlobalPinchGesture *pg = dynamic_cast<GlobalPinchGesture *>(g);
                if (m_direction != GlobalGesture::GestureDirection::NoDirection
                    && pg->direction() != m_direction) {
                    isDelete = true;
                }
                break;
            }
            case GlobalGesture::GlobalGestureType::LongPress: {
                /*! 中间手势类型一旦被确定，其余手势将被清空，
                 * 包括LongPress手势，
                 * 所以不需要对LongPress手势特殊判断，
                 * 只需在手势结束时，进行一次判断即可 */
                break;
            }
            default:
                break;
            }
        }

        if (isDelete) {
            if (g->isStart())
                Q_EMIT g->cancelled();
            it = m_activeGestures.erase(it);
        } else {
            ++it;
        }
    }
}

void GlobalGestureRecognizer::cancelActiveGesture()
{
    for (auto g : qAsConst(m_activeGestures)) {
        if (g->isStart())
            Q_EMIT g->cancelled();
    }
}

void GlobalGestureRecognizer::cancelGesture()
{
    cancelActiveGesture();
}

void GlobalGestureRecognizer::endGesture(int detail, const QPointF &pos)
{

    static int cnt = 0;
    //! 当前手势的滑动距离
    double distance = calculateSingleDistance(m_touchState.fingerCnt() == 1 ? detail : -1);

    // 对于还在激活状态的手势，将手势激活信号发出
    for (GlobalGesture *g : qAsConst(m_activeGestures)) {
        bool isDelete = false; // 判断该手势是否满足触发条件
        switch (g->gestureType()) {
        case GlobalGesture::GlobalGestureType::Swipe: {
            GlobalSwipeGesture *sg = dynamic_cast<GlobalSwipeGesture *>(g);
            QDesktopWidget *desktop = QApplication::desktop();
            double dpi = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;

            if (distance < sg->minSwipeDistance())
                isDelete = true;
            else if (m_gestureType != GlobalGesture::GlobalGestureType::Swipe)
                isDelete = true;
            else if (sg->timemore() != 0 && m_touchState.getLastTime() < sg->timemore()) {
                qDebug() << "GlobalSwipeGesture last time:" << m_touchState.getLastTime()
                         << "but gesture need time more:" << sg->timemore();
                isDelete = true;
            } else if (sg->timeless() != 0 && m_touchState.getLastTime() > sg->timeless()) {
                qDebug() << "GlobalSwipeGesture last time:" << m_touchState.getLastTime()
                         << "but gesture need time less:" << sg->timeless();
                isDelete = true;
            } else if (sg->swipemore() != 0 && distance < sg->swipemore() * (dpi / 160)) {
                qDebug() << "GlobalSwipeGesture swipe distance:" << distance
                         << "but gesture need swipe more:" << sg->swipemore() * (dpi / 160);
                isDelete = true;
            } else if (sg->swipeless() != 0 && distance > sg->swipeless() * (dpi / 160)) {
                qDebug() << "GlobalSwipeGesture swipe distance:" << distance
                         << "but gesture need swipe less:" << sg->swipeless() * (dpi / 160);
                isDelete = true;
            }
            break;
        }
        case GlobalGesture::GlobalGestureType::Pinch: {
            if (m_gestureType != GlobalGesture::GlobalGestureType::Pinch)
                isDelete = true;
            break;
        }
        case GlobalGesture::GlobalGestureType::LongPress: {
            GlobalLongPressGesture *lg = dynamic_cast<GlobalLongPressGesture *>(g);
            //! 判断手指有没有移动
            double delta = calculateFingerDelta();
            if (delta > 5 * lg->fingerCount()) {
                isDelete = true;
            }
            //! 判断是否到达手势要求的时间
            else if (m_touchState.getLastTime() < lg->timeout()) {
                isDelete = true;
            }
            break;
        }
        default: {
            isDelete = true;
            break;
        }
        }
        if (isDelete) {
            if (g->isStart())
                Q_EMIT g->cancelled();
        } else {
            ++cnt;
            Q_EMIT g->triggered();
        }
    }
    cnt = 0;

    m_touchState.deleteSlot(detail);
    m_activeGestures.clear();
    reset();
}

void GlobalGestureRecognizer::reset()
{
    m_swipeReachCount = 0;
    m_direction = GlobalGesture::GestureDirection::NoDirection;
    m_gestureType = GlobalGesture::GlobalGestureType::Waiting;
    m_touchState.reset();
    m_activeGestures.clear();
    // 清空定时器
    for (QTimer *timer : m_longPressTimers) {
        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
    }
    m_longPressTimers.clear();
}

GlobalGesture::GestureDirection GlobalGestureRecognizer::calculateSingleDirection(const int detail)
{
    GlobalGesture::GestureDirection res = GlobalGesture::GestureDirection::NoDirection;
    QPointF ori = detail == -1 ? m_touchState.initRect.center : m_touchState.getInitPos(detail);
    QPointF tar = detail == -1 ? m_touchState.curRect.center : m_touchState.getCurPos(detail);
    QPointF lastPoint = detail == -1 ? m_touchState.lastRect.center : m_touchState.getLastPos(detail);
    // 当前方向与滑动方向保持一致，方向变化后，手势要清空
    int dx = tar.x() - ori.x();
    int dy = tar.y() - ori.y(); // dx,dy  当前点相对与起始点的偏移量
    int lx = tar.x() - lastPoint.x();
    int ly = tar.y() - lastPoint.y(); // lx, ly 当前点相对于上一坐标的偏移量
    int dis = dist(tar.x(), tar.y(), ori.x(), ori.y());
    if (dis > MIN_SWIPE_DIST) {
        if (fabs(dx) > fabs(dy)) {
            double angle = 2 * acos(fabs(dx) / dis) * 180 / M_PI;
            // 确保手势移动在规定区域内 并且手势方向不发生改变
            if (angle < MAX_SWIPE_ANGLE && ((dx >= 0 && lx >= 0) || (dx <= 0 && lx <= 0))) {
                res = dx > 0 ? GlobalGesture::GestureDirection::Right : GlobalGesture::GestureDirection::Left;
            } else {
                res = GlobalGesture::GestureDirection::NotSupport;
            }
        } else {
            double angle = 2 * acos(fabs(dy) / dis) * 180 / M_PI;
            if (angle < MAX_SWIPE_ANGLE && ((dy >= 0 && ly >= 0) || (dy <= 0 && ly <= 0))) {
                res = dy > 0 ? GlobalGesture::GestureDirection::Down : GlobalGesture::GestureDirection::Up;
            } else {
                res = GlobalGesture::GestureDirection::NotSupport;
            }
        }
    }
    return res;
}

double GlobalGestureRecognizer::calculateSingleDistance(const int detail)
{
    QPointF ori = detail == -1 ? m_touchState.initRect.center : m_touchState.getInitPos(detail);
    QPointF tar = detail == -1 ? m_touchState.curRect.center : m_touchState.getCurPos(detail);
    return dist(ori.x(), ori.y(), tar.x(), tar.y());
}

//! 计算Pinch手势的方向
GlobalGesture::GestureDirection GlobalGestureRecognizer::calculatePinchDirection()
{
    int fcnt = m_touchState.fingerCnt();
    if (fcnt <= 1)
        return GlobalGesture::GestureDirection::NoDirection;
    ExternalRect crect = m_touchState.curRect;
    ExternalRect irect = m_touchState.initRect;
    ExternalRect lrect = m_touchState.lastRect;
    //! 判断pinch手势的方向
    double delta = fmax(fmax(fabs(crect.diagL - irect.diagL), fabs(crect.h - irect.h)), fabs(crect.w - irect.w));
    if (delta > MIN_PINCH_DIST) {
        double dci = crect.diagL - irect.diagL;
        double dcl = crect.diagL - lrect.diagL;
        if ((dci < 0 && dcl > 0) || (dci > 0 && dcl < 0))
            return GlobalGesture::GestureDirection::NotSupport;
        return dci < 0 ? GlobalGesture::GestureDirection::In : GlobalGesture::GestureDirection::Out;
    }
    return GlobalGesture::GestureDirection::NoDirection;
}

double GlobalGestureRecognizer::calculateFingerDelta()
{
    double delta = 0;
    for (auto it = m_touchState.slotid.begin(); it != m_touchState.slotid.end(); ++it) {
        QPointF ori = m_touchState.slotInit[it->second];
        QPointF tar = m_touchState.slotPoints[it->second];
        delta += dist(ori.x(), ori.y(), tar.x(), tar.y());
    }
    return delta;
}

//! 开始滑动时检测滑动手势是否满足当前的滑动状态
bool GlobalGestureRecognizer::detectSwpie(GlobalSwipeGesture *gesture)
{
    if (!gesture) {
        return false;
    }
    uint fingerCount = m_touchState.fingerCnt();
    if (gesture->fingerCount() != fingerCount) {
        return false;
    }

    // 检测起始点是否是开始范围内
    for (auto it = m_touchState.slotid.begin(); it != m_touchState.slotid.end(); ++it) {
        QPointF &p = m_touchState.slotInit[it->second];
        QRect rect = gesture->startGeometry();
        if (p.x() < rect.x() || p.x() > rect.x() + rect.width() || p.y() < rect.y() || p.y() > rect.y() + rect.height())
            return false;
    }

    // 检测滑动距离是否超出最大滑动距离
    //    double sdis = 0; // 滑动距离
    //    if(fingerCount == 1){
    //        auto it = m_touchState.slotid.begin();
    //        sdis = dist(m_touchState.slotInit[it->second].x(),
    //                    m_touchState.slotInit[it->second].y(),
    //                    m_touchState.slotPoints[it->second].x(),
    //                    m_touchState.slotPoints[it->second].y());
    //    }else{
    //        sdis = dist(m_touchState.initRect.center.x(),
    //                    m_touchState.initRect.center.y(),
    //                    m_touchState.curRect.center.x(),
    //                    m_touchState.curRect.center.y());
    //    }
    //    if(sdis > gesture->maximumR())
    //        return false;
    return true;
}

QString GlobalGestureRecognizer::directionToQString(GlobalGesture::GestureDirection direction)
{
    switch (direction) {
    case GlobalGesture::GestureDirection::Down:
        return QString("DOWN");
    case GlobalGesture::GestureDirection::In:
        return QString("IN");
    case GlobalGesture::GestureDirection::Left:
        return QString("LEFT");
    case GlobalGesture::GestureDirection::NotSupport:
        return QString("NOTSUPPORT");
    case GlobalGesture::GestureDirection::Out:
        return QString("OUT");
    case GlobalGesture::GestureDirection::Right:
        return QString("RIGHT");
    case GlobalGesture::GestureDirection::Up:
        return QString("UP");
    case GlobalGesture::GestureDirection::NoDirection:
        return QString("NoDirection");
    default:
        break;
    }
    return QString("ERROR");
}

QString GlobalGestureRecognizer::typeToQString(GlobalGesture::GlobalGestureType type)
{
    switch (type) {
    case GlobalGesture::GlobalGestureType::Swipe:
        return "Swipe";
    case GlobalGesture::GlobalGestureType::Pinch:
        return "Pinch";
    case GlobalGesture::GlobalGestureType::LongPress:
        return "LongPress";
    case GlobalGesture::GlobalGestureType::Waiting:
        return "Waiting";
    default:
        break;
    }
    return "Error";
}

QString GlobalGestureRecognizer::processNameByPos(QPoint pos)
{
    Toplevel *top = input()->findToplevel(pos);
    if (!top) {
        qWarning() << "Can not find toplevel from pos:" << pos;
        return QString();
    }
    int pid = top->pid();
    QString procName;
    QFile procFile(QString("/proc/%1/status").arg(pid));
    if (procFile.open(QFile::ReadOnly)) {
        procName = QString(procFile.readLine()).section(QRegExp("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
        procFile.close();
    } else {
        qWarning() << "Can not open file:" << procFile.fileName();
        return QString();
    }

    return procName;
}

QString GlobalGestureRecognizer::cmdlineByPos(QPoint pos)
{
    Toplevel *top = input()->findToplevel(pos);
    if (!top) {
        qWarning() << "Can not find toplevel from pos:" << pos;
        return QString();
    }
    int pid = top->pid();
    QString cmdline;
    QFile procFile(QString("/proc/%1/cmdline").arg(pid));
    if (procFile.open(QFile::ReadOnly)) {
        cmdline = QString(procFile.readLine());
        // 通过空格隔断参数
        if (cmdline.indexOf(" ") != -1) {
            cmdline = cmdline.left(cmdline.indexOf(" "));
        }
        procFile.close();
    } else {
        qWarning() << "Can not open file:" << procFile.fileName();
        return QString();
    }

    return cmdline;
}

TouchpadGestureRecognizer::TouchpadGestureRecognizer(QObject *parent)
    : QObject(parent)
{
    m_gestureState.init();
}

TouchpadGestureRecognizer::~TouchpadGestureRecognizer() = default;
void TouchpadGestureRecognizer::registerGesture(KWin::GlobalGesture *gesture)
{
    Q_ASSERT(!m_gesture.contains(gesture));
    auto connection = connect(gesture, &QObject::destroyed, this, std::bind(&TouchpadGestureRecognizer::unregisterGesture, this, gesture));
    m_destroyConnections.insert(gesture, connection);
    m_gesture << gesture;
}
void TouchpadGestureRecognizer::unregisterGesture(KWin::GlobalGesture *gesture)
{
    auto it = m_destroyConnections.find(gesture);
    if (it != m_destroyConnections.end()) {
        disconnect(it.value());
        m_destroyConnections.erase(it);
    }
    m_gesture.removeAll(gesture);
    if (m_activeGesture.removeOne(gesture)) {
        if (gesture->isStart()) {
            Q_EMIT gesture->cancelled();
        }
    }
}

void TouchpadGestureRecognizer::startRecognizer(GlobalGesture::GlobalGestureType flag, int fingerCount, quint32 time)
{

    clearDate();
    // 初始化手势状态
    m_gestureState.fingerCount = fingerCount;
    m_gestureState.gestureType = flag;

    for (GlobalGesture *gesture : qAsConst(m_gesture)) {
        switch (gesture->gestureType()) {
        case GlobalGesture::GlobalGestureType::Swipe: {
            TouchpadSwipeGesture *swp = dynamic_cast<TouchpadSwipeGesture *>(gesture);
            if (flag != GlobalGesture::GlobalGestureType::Swipe)
                continue;
            if (swp->fingerCount() != m_gestureState.fingerCount)
                continue;
            break;
        }
        case GlobalGesture::GlobalGestureType::Tap: {
            TouchpadTapGesture *tap = dynamic_cast<TouchpadTapGesture *>(gesture);
            if (flag != GlobalGesture::GlobalGestureType::Tap)
                continue;
            if (tap->fingerCount() != m_gestureState.fingerCount)
                continue;
            // if(!tap->isStart())
            Q_EMIT tap->triggered();
            break;
        }
        case GlobalGesture::GlobalGestureType::SwipeSequence: {
            TouchpadSwipeSequenceGesture *swipe2 = dynamic_cast<TouchpadSwipeSequenceGesture *>(gesture);
            if (swipe2->fingerCount() != m_gestureState.fingerCount)
                continue;
            if (flag != GlobalGesture::GlobalGestureType::Swipe)
                continue;
            break;
        }
        default:
            break;
        }
        m_activeGesture << gesture;
    }
}
void TouchpadGestureRecognizer::updateRecognizer(GlobalGesture::GlobalGestureType flag, const QSizeF &delta, quint32 time)
{

    // 更新手势的状态
    m_gestureState.sumDeltaX += delta.width();
    m_gestureState.sumDeltaY += delta.height();
    m_gestureState.curDeltaX += delta.width();
    m_gestureState.curDeltaY += delta.height();

    // 防抖处理
    if ((fabs(delta.width()) < 0.4 || fabs(delta.height()) < 0.4)
        && m_gestureState.curDeltaX < STATE_UPDATE_SPAN / 2.0
        && m_gestureState.curDeltaY < STATE_UPDATE_SPAN / 2.0)
        return;

    // 计算当前手势滑动的方向
    GlobalGesture::GestureDirection direction = GlobalGesture::GestureDirection::NoDirection;

    // 计算x、y轴单位（STATE_UPDATE_SPAN）方向的变化
    GlobalGesture::GestureDirection horDirection = GlobalGesture::GestureDirection::NoDirection;
    GlobalGesture::GestureDirection verDirection = GlobalGesture::GestureDirection::NoDirection;

    //! 注意这里的flag是触摸板上报的事件类型
    switch (flag) {
    case GlobalGesture::GlobalGestureType::Swipe: {
        direction = calculateDirection(delta);
        if (fabs(m_gestureState.curDeltaX) > STATE_UPDATE_SPAN) {
            horDirection = m_gestureState.curDeltaX > 0 ? GlobalGesture::GestureDirection::Right : GlobalGesture::GestureDirection::Left;
            m_gestureState.lastDeltaX += m_gestureState.curDeltaX;
            m_gestureState.curDeltaX = 0;
        }
        if (fabs(m_gestureState.curDeltaY) > STATE_UPDATE_SPAN) {
            verDirection = m_gestureState.curDeltaY > 0 ? GlobalGesture::GestureDirection::Down : GlobalGesture::GestureDirection::Up;
            m_gestureState.lastDeltaY += m_gestureState.curDeltaY;
            m_gestureState.curDeltaY = 0;
        }
        break;
    }
    case GlobalGesture::GlobalGestureType::Tap: {
        break;
    }
    default:
        break;
    }

    if (m_gestureState.gestureDirection == GlobalGesture::GestureDirection::NoDirection) {
        m_gestureState.gestureDirection = direction;
    } else if (m_gestureState.gestureDirection != direction) {
        m_gestureState.gestureDirection = GlobalGesture::GestureDirection::NotSupport;
    }

    //! 决策当前激活手势的去留
    for (auto it = m_activeGesture.begin(); it != m_activeGesture.end();) {
        GlobalGesture *gesture = static_cast<GlobalGesture *>(*it);
        bool notSupport = false;
        switch (gesture->gestureType()) {
        case GlobalGesture::GlobalGestureType::Swipe: {
            TouchpadSwipeGesture *swp = dynamic_cast<TouchpadSwipeGesture *>(gesture);
            if ((swp->direction() != m_gestureState.gestureDirection)) {
                notSupport = true;
                break;
            }
            if (!swp->isStart()) {
                //! 判断手势的滑动距离是否满足开始的条件
                // 滑动距离
                bool isSendStart = false;
                if (GlobalGesture::GestureDirection::Left == swp->direction() || GlobalGesture::GestureDirection::Right == swp->direction()) {
                    isSendStart = fabs(m_gestureState.sumDeltaX) > swp->minSwipeDistance();
                } else {
                    isSendStart = fabs(m_gestureState.sumDeltaY) > swp->minSwipeDistance();
                }
                if (isSendStart)
                    Q_EMIT swp->started();
            }
            break;
        }
        case GlobalGesture::GlobalGestureType::Tap: {
            break;
        }
        case GlobalGesture::GlobalGestureType::SwipeSequence: {
            TouchpadSwipeSequenceGesture *swipe2 = dynamic_cast<TouchpadSwipeSequenceGesture *>(gesture);
            // 当左右滑动时，开启该手势
            if (!swipe2->isStart() && (m_gestureState.gestureDirection == GlobalGesture::GestureDirection::Left || m_gestureState.gestureDirection == GlobalGesture::GestureDirection::Right) && fabs(m_gestureState.sumDeltaX) > 15) {
                Q_EMIT swipe2->started();
            }

            // 手势开始后，持续通知当前的手势方向
            if (swipe2->isStart()) {
                if (horDirection != GlobalGesture::GestureDirection::NoDirection)
                    Q_EMIT swipe2->sigNotifyGestureDirection(horDirection);
                if (verDirection != GlobalGesture::GestureDirection::NoDirection)
                    Q_EMIT swipe2->sigNotifyGestureDirection(verDirection);
            }
            break;
        }
        default: {
            notSupport = true;
            break;
        }
        }

        if (notSupport) {
            if (gesture->isStart()) {
                gesture->cancelled();
            }
            it = m_activeGesture.erase(it);
        } else {
            ++it;
        }
    }
}
void TouchpadGestureRecognizer::endRecognizer(GlobalGesture::GlobalGestureType flag, quint32 time)
{

    for (GlobalGesture *gesture : qAsConst(m_activeGesture)) {
        bool notSupport = false;
        switch (gesture->gestureType()) {
        case GlobalGesture::GlobalGestureType::Swipe: {
            break;
        }
        case GlobalGesture::GlobalGestureType::Tap: {
            break;
        }
        case GlobalGesture::GlobalGestureType::SwipeSequence: {
            break;
        }
        default: {
            notSupport = true;
            break;
        }
        }
        if (gesture->isStart()) {
            if (notSupport) {
                Q_EMIT gesture->cancelled();
            } else {
                Q_EMIT gesture->triggered();
            }
        }
    }
    clearDate();
}
void TouchpadGestureRecognizer::cancelRecognizer(GlobalGesture::GlobalGestureType flag, quint32 time)
{

    /*
     * 收到cancel事件之后  把当前发送的信号取消掉
     */
    //            notSupport = true;

    for (GlobalGesture *gesture : qAsConst(m_activeGesture)) {
        if (gesture->isStart()) {
            Q_EMIT gesture->cancelled();
        }
    }
}

/*
 * 只有当向一个方向滑动超过一定的距离的时候才会认定是在该方向进行了滑动
 * 需要计算width和height
 * 向左滑动：width<0 width > height
 * 向右滑动：width>0 width > height
 * 向上滑动：height > 0 height > width
 * 向下滑动：height < 0 height > width
 */
GlobalGesture::GestureDirection TouchpadGestureRecognizer::calculateDirection(const QSizeF &delta)
{
    GlobalGesture::GestureDirection direction = GlobalGesture::GestureDirection::NoDirection;
    if (std::abs(delta.height()) > std::abs(delta.width())) {
        if (delta.height() > 0) {
            direction = GlobalGesture::GestureDirection::Down;
        } else {
            direction = GlobalGesture::GestureDirection::Up;
        }
    } else if (std::abs(delta.width()) > std::abs(delta.height())) {
        if (delta.width() > 0) {
            direction = GlobalGesture::GestureDirection::Right;
        } else {
            direction = GlobalGesture::GestureDirection::Left;
        }
    }
    return direction;
}

void TouchpadGestureRecognizer::clearDate()
{
    m_gestureState.init();
    m_activeGesture.clear();
}

// TouchpadGestureState
void TouchpadGestureState::init()
{
    gestureType = GlobalGesture::GlobalGestureType::Waiting;
    sumDeltaX = sumDeltaY = 0;
    curDeltaX = curDeltaY = 0;
    lastDeltaX = lastDeltaY = 0;
    fingerCount = 0;
    isGestureDirectionChange = false;
    gestureDirection = GlobalGesture::GestureDirection::NoDirection;
}

}
