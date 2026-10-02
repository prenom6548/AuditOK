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

#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>
#include <QMutex>
#include <QBitArray>
#include <QKeyEvent>

#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdint.h>
#include "common.h"
#include "dbusclient.h"
#include "eventdata.h"

using namespace kmre;

#define __ALIGN_MASK(x, mask) (((x) + (mask))&~(mask))
#define ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a) - 1)


class KmreWindow;
class DisplayManager;

class DisplayWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    DisplayWidget(KmreWindow* window, int id, int width, int height, int orientation, QString displayName);
    virtual ~DisplayWidget();

    void initConnections();
    void enableInputMethod(int id, bool ret, int x, int y);
    void setImageRotation(int imageRotation);
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const Q_DECL_OVERRIDE;
    QPoint cursorPoint();
    void changeInputBoxPos();
    void resetControlData();
    void updateDDSSupportState(bool support) {mIsDDSSupported = support;}
    bool isDDSSupported() {return mIsDDSSupported;}
    void setDisplaySize(uint32_t width, uint32_t height);
    void forceRedraw();
    void enableDisplayUpdate(bool enable);
    void blurUpdate(uint32_t width, uint32_t height);
    //void enableResponseEvents(bool b) {m_responseEvents = b;}
    bool getInputFocus() {return mIsIMEnabled;}
    int getDisplayId() {return mId;}
    bool registerDisplay(DisplayManager* displayManager);
    void connectDisplay(int displayId, int width, int height);
    QSize getWidgetRealSize() {return mWidgetRealSize;}
    QSize getWidgetMinSize() {return mWidgetMinSize;}
    void changeWidgetFocus();
    void sendMouseEvent(MouseEventInfo mouseEvent);
    void setDisplaySizeScaling(std::tuple<float, float> &scaling) {mDisplaySizeScaling = scaling;}
    void forceUpdating();
    virtual void setAutoUpdate(bool enable) {}

public slots:
    void onRequestActiveWindow();
    void runAction();
    void isShowMouse(bool status);
    void settingMousePos(QPoint pos);

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    
signals:
    void doubleClicked();
    void sigBack();
    void mouseMoved(int deltaX, int deltaY);
    void shareFile(QString path, QString displayName, int displayId, bool check);

    void walkKeyProcess(WalkDirectionType type, bool isPressed);
    void oneKeyPressed(QString strCode);
    void oneKeyRelease(QString strCode);
//    void requestShowCustomCursor(bool b);
    void requestHideSubWindow();

//    void requestShowMainMenu(bool inputMethodEnabled);
    void keyboardPress(QKeyEvent *pKey);
    void keyboardRelease(QKeyEvent *pKey);
    void mousePress(QMouseEvent *pKey);
    void mouseRelease(QMouseEvent *pKey);
    void mouseMove(QMouseEvent *pKey);
    void focusOut();
    void mouseLeave();

protected:
    int mWidth;
    int mHeight;
    bool firstRepeat = false;
    KmreWindow* mMainWindow = nullptr;

private:
    void initDisplayScalingFactor();
    void mouseEventHandler(int type, int slot, int x, int y);
    void onScroll();
    void onZoom();
    void focusDisplay();

    int mId;
    QString mDisplayName;
    float mDisplayScalingFactor;
    std::tuple<float, float> mDisplaySizeScaling;
    QSize mWidgetRealSize;
    QSize mWidgetMinSize;
    bool mIsMouseButtonPressed;
    bool mTouchEventTrigged;
    int mImageRotation;//图像的旋转角度0(0)    1(90)     2(180)   3(270)
    int m_inputX;
    int m_inputY;
    bool isKeyADown = false;
    bool isKeyWDown = false;
    bool isKeyDDown = false;
    bool isKeySDown = false;
    bool mCTLKeyPressed = false;
    bool mIsDDSSupported = false;
    bool mIsIMEnabled = false;

    QTimer *scrollTimer = nullptr;
    int scrollCount = 0;
    long long scrollTime = 0;
    int deltaAverage = 0;
    int xPos = 0;
    int yPos = 0;
    int xZoom = 0;
    int yZoom = 0;
    bool currentZoom = false;
};

#endif // DISPLAYWIDGET_H
