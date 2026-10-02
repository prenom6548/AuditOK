/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#ifndef MINIWINDOW_H
#define MINIWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QFrame>
#include <QTimer>
#include <QTime>

#include "mythread.h"
#include "daemondbus.h"
#include "custombutton.h"
#include "kpushbutton.h"
#include "kborderlessbutton.h"

class MiniWindow : public QMainWindow
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit MiniWindow(QWidget *parent = nullptr);
    ~MiniWindow();
    int m_WIDTH = 220;
    int m_HEIGHT = 36;

    QStackedWidget *m_recordStackedWidget = nullptr; //堆叠布局

    kdk::KBorderlessButton *m_miniPauseButton = nullptr; // mini模式的暂停按钮
    kdk::KBorderlessButton *m_miniStartButton = nullptr; // mini模式的开始按钮
    QPushButton *m_miniStopButton = nullptr;   //停止按钮

    bool m_miniStartOrPause = false;           // mini模式的判断开始或暂停是否按下

    QTime m_miniBaseTime; // 1
    //显示的时间
    QString m_timeStr;                 //时间串
    QLabel *m_miniTimeLabel = nullptr; //显示录制时间的标签
    QWidget *m_miniWidget = nullptr;   // mini主窗体的Wid
    QFrame *m_line = nullptr;
    QToolButton *m_miniCloseButton = nullptr;   //关闭按钮
    QPushButton *m_miniRecordButton = nullptr; //录制按钮


    void timeDisplay();
    void changeMiniContinuePauseState(bool isContinue);
    void changeMiniCoutinuePauseBtnThemeColor(QString themeColor);

private:
    QHBoxLayout *m_recordLayout = nullptr;
    QHBoxLayout *m_miniLayout = nullptr;
    QHBoxLayout *m_pageTwoLayout = nullptr;
    QHBoxLayout *m_restoreOrMiniAndCloseLayout = nullptr;

    QWidget *m_recordButtonWidget = nullptr;
    QWidget *m_pageTwoWidget = nullptr;            //包括停止、开始/暂停两个按钮的Wid
    QWidget *m_restoreOrMiniAndCloseWid = nullptr; //最大最小和关闭的Wid
    QToolButton *m_miniButton = nullptr;           //最大最小按钮

private:
    bool m_isPress;
    QPoint m_windowPos;
    QPoint m_dragPos;

    // 用户手册功能
    DaemonDbus *m_DaemonIpcDbus = nullptr;
    void resizeEvent(QResizeEvent *event);
    // 键盘响应事件
    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void initMiniWindow();
    void setMiniWindow();
    void initShortcutStyle();// 处理焦点

public slots:

    void switchPage();
    void normalShow();
    void closeWindow();
    void handleStartPauseSlot();
    void finishSlot(); //结束录音并保存
    void startOrPauseSlot();


signals:

    void startOrPauseSignal();
};

#endif // MINIWINDOW_H
