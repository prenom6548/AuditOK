/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2022 KylinSoft Co., Ltd.

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef GESTURE_FOLLOW_H
#define GESTURE_FOLLOW_H
#include <QObject>
#include <iostream>
#include <kwin_export.h>
#include <kwinglobals.h>

#include "input_event_spy.h"

namespace KWin
{
class AbstractClient;

class KWIN_EXPORT GestureFollowInputFilter : public QObject, public InputEventSpy
{
    Q_OBJECT
public:
    void touchDown(qint32 id, const QPointF &pos, quint32 time) override;
    void touchMotion(qint32 id, const QPointF &pos, quint32 time) override;
    void touchUp(qint32 id, quint32 time) override;
    void pointerEvent(MouseEvent *event) override;

private Q_SLOTS:
    /**
     * 监听平板模式变化，进行隐藏显示任务栏
     */
    void monitorModeChanged();

private:
    // 记录要操作的窗口,并存储窗口信息
    AbstractClient *m_gestureFollowWindow = nullptr;

    // 记录任务栏是否显示
    bool m_tabletPanelhide = true;

    // 记录多任务视图是否显示
    bool m_multitaskViewDisplay = false;

    // 标志是否开始手势跟随
    bool m_startGestureFollow = false;

    // 标志是否点击在任务栏内部
    bool m_clickPositionInPanel = false;

    // 记录任务栏初始化时始位置X坐标
    int m_panelOriginXCoordinate = 0;

    // 记录任务栏初始化时始位置Y坐标
    int m_panelOriginYCoordinate = 0;

    // 记录任务栏是显示还是隐藏时的Y轴坐标临界值, 超过该坐标抬起手指，任务栏显示，否则任务栏隐藏
    int m_panelDisplayYCoordinate = 0;

    // panel的高度
    int m_panelHeight = 0;

    // 记录屏幕分辨率，屏幕的高度
    int m_screenHeight = 0;

    // 记录唤起多任务视图的Y轴临界值
    int m_YcriticalValue = 0;

    // 记录手指id
    int m_gestureId = -1;

    // 记录手势开始的时间
    QTime m_beginTime;

    QStringList blackList = {
        "kylin-nm",
        "ukui-bluetooth",
        "ukui-search",
    };

    QStringList unmanageWindowList = {
        "ukui-session-tools",
    };

    // 前台类型
    enum FrontWindowType {
        DesktopWindow,
        UtilityWindow,
        NormalWindow,
    };

    // 存储手势id和起始位置
    std::unordered_map<int, QPointF> gestureInfo;

private:
    /**
     * 当窗口激活信号发出时，进行client处理，确定是更新任务栏窗口信息，还是隐藏任务栏
     */
    void handleActiveClient(AbstractClient *client = nullptr);

    /**
     * 遍历stackingorder获取窗口数据
     */
    void getTabletTaskbarWininfo();

    /**
     * 获取屏幕高度
     */
    int getScreenHeight();

    /**
     * 获取屏幕宽度
     */
    int getScreenWidth();

    /**
     * 判断当前显示的是不是桌面
     */
    bool isDisplayingDesktop();

    bool haveFullscreenUnmanagedWindowShowing();

    /**
     * 返回前台应用类型
     */
    FrontWindowType frontWindowType();

    /**
     * 桌面透明度动画
     */
    void desktopOpacityAnimation(double deltaY);

    /**
     * 由于分辨率会改变，以及平板桌面任务栏宽度会不断的变化，所以每次需要在手指点下的瞬间
     * 重新获取屏幕分辨率的高度，确定显示多任务视图的临界值
     * 重新获取任务栏的宽度，确定任务栏显示的x坐标
     * 重新获取任务栏从屏幕底部画出一半后的Y轴坐标
     */
    void resetReferenceValue();

    /**
     * 获取当前press位置是否在任务栏上，如果在不作处理，如果不在，则隐藏任务栏
     */
    void handlePressEvent(int x, int y);

    /**
     * 处理唤起多任务视图手势的准确性：针对手势方向和速度进行处理判断
     * 确定是唤起多任务视图的手势
     */
    bool isAwakenViewGesture();

    void sendTriggerMultitaskView();
    void sendMultitaskViewDbusResult(quint32 type);

    KWIN_SINGLETON(GestureFollowInputFilter)
};
}

#endif
