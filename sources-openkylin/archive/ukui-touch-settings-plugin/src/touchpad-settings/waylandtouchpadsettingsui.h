/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef WAYLANDTOUCHPADSETTINGSUI_H
#define WAYLANDTOUCHPADSETTINGSUI_H

#include <QMovie>
#include <QWidget>
#include <kswitchbutton.h>
#include <ktabbar.h>
#include <QDBusArgument>
#include <QDBusVariant>

#include "touchpadsettingsui.h"
#include "waylandgesturemapper.h"
#include "waylandfuncmapper.h"

#include "kysdk/applications/klineframe.h"

//enum class TouchpadGestureType
//{
//    threeFingerClick,
//    threeFingerSwipeDown,
//    threeFingerSwipeUp,
//    threeFingersHorizontalSwipe,
//    fourFingerClick,
//    fourFingersHorizontalSwipe,
//    unknown
//};

//class TouchpadGestureInfo
//{
//public:
//    TouchpadGestureInfo() = default;
//    TouchpadGestureInfo(TouchpadGestureType gesture, QString descrip,
//                     QString actionDescrip, QString gifPath)
//        : gestureType(gesture)
//        , description(descrip)
//        , actionDescription(actionDescrip)
//        , gifPath(gifPath)
//    {}

//    TouchpadGestureInfo(const TouchpadGestureInfo&) = default;
//    TouchpadGestureInfo& operator=(const TouchpadGestureInfo&) = default;

//    TouchpadGestureType gestureType;
//    QString description;
//    QString actionDescription;
//    QString gifPath;
//};

bool getGestureStatus(quint32 gestureId, quint32 &outCurrentFuncId, bool &outEnabled);

enum class TouchpadGestureFingerCount
{
    threeFinger = 0,
    fourFinger
};

class QComboBox;
class QFrame;
class QGSettings;
class WaylandTouchpadUI : public QWidget
{
    Q_OBJECT

public:
    explicit WaylandTouchpadUI(QWidget *parent = nullptr);
    ~WaylandTouchpadUI() = default;

public slots:
    void playGif(uint gestureId, uint funcId);
//    void stopGif();

private:
    QWidget* m_mainWidget = nullptr;
    QWidget* m_baseWidget = nullptr;
    QWidget* m_trackPointWidget = nullptr;
    QWidget* m_gestureWidget = nullptr;

    QWidget* m_tapWidget = nullptr;
    QWidget* m_swipeUpWidget = nullptr;
    QWidget* m_swipeDownWidget = nullptr;
    QWidget* m_swipeLeftWidget = nullptr;
    QWidget* m_swipeRightWidget = nullptr;

    TouchpadGestureFingerCount m_currentFingerShowType = TouchpadGestureFingerCount::threeFinger;

    QMovie *m_gestureMovie;
    QMovie *m_functionMovie;
    QGSettings *m_touchpadGsetting;
    kdk::KSwitchButton* m_touchpadDisableBtn;
    QComboBox* m_scrollTypeComBox = nullptr;
    int m_previousScrollAreaIndex = -1;

    static const QList<QPair<QString, QString>> controlledItems;
    static const QList<QPair<QString, int>> tapGestureFuncItems;
    static const QList<QPair<QString, int>> swipeGestureUpLeftFuncItems;
    static const QList<QPair<QString, int>> swipeGestureDownRightFuncItems;

    QList<QWidget*> m_needUpdateVisibleWidgetList;

    ConfigMonitor* m_configMonitor;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:

    void createTouchPadSettingWidget();

    QWidget* createTitleWidget(QString titleText);

    //触摸板设置-层级1
    QWidget* createBaseSettingsWidget();
    //触摸板设置-层级2
    QWidget* createTouchpadDisableWidget();
    QWidget* createMouseDisableWidget();
    QWidget* createPointerSpeedWidget();
    QWidget* createTypingDisableWidget();

    //指点杆设置-层级1
    QWidget* createTrackPointSettingsWidget();
    //指点杆开关设置-层级2
    QWidget* createTrackPointEnabledWidget();

    //触摸板手势设置-层级1
    QWidget* createGestureSettingsWidget();
    //触摸板手势设置-层级2
    QWidget* createGestureBaseSettingsFrame();
    //触摸板手势设置-层级3
    QWidget* createTapToClickSettingWidget();
    QWidget* createTwoFingerClickSettingWidget();
    QWidget* createTapAndDragSettingWidget();
    QWidget* createBottomRightCornerClickMenuSettingWidget();

    //触摸板手势设置-层级2
    QWidget* createEdgeGestureSettingsFrame();
    //触摸板手势设置-层级3
    QWidget* createTopEdgeGestureSettingWidget();
    QWidget* createLeftEdgeGestureSettingWidget();
    QWidget* createRightEdgeGestureSettingWidget();

    //触摸板手势设置-层级2
    QWidget* createScrollSettingsFrame();
    //触摸板手势设置-层级3
    QWidget* createScrollSlideWidget();
    QWidget* createScrollAreaWidget();
    //预留捏合缩放-层级3
    QWidget* createScaleWidget();

    //触摸板手势设置-层级2
    QWidget* createGestureConfigWidget();
    //触摸板手势设置-层级3
    QWidget* createGestureConfigGuideWidget();
    QWidget* createGestureConfigTabWidget();
    QWidget* createGestureConfigTapSettingsWidget();
    QWidget* createGestureConfigSwipeUpSettingsWidget();
    QWidget* createGestureConfigSwipeDownSettingsWidget();
    QWidget* createGestureConfigSwipeLeftSettingsWidget();
    QWidget* createGestureConfigSwipeRightSettingsWidget();

    QWidget* createSingleWidget(const QString& text, QWidget* rightWidget);
    QWidget* createSingleSwitchWidget(const QString& settingKey, const QString& uiText, const bool needUpdateVisibilityKey = false);
    QWidget* createSwipeGestureWidget(
            int gestureId3Finger,    // 3指手势ID
            int gestureId4Finger,    // 4指手势ID
            const QString& displayText // 显示文本（如"Swipe Down"）
    );

//    QWidget* createMoreGestureWidget();

    QString currentScrollingType();

    bool hasTrackPointDevice() const;
    bool hasMouseDevice() const;
    void showWarningDialog();
    bool getSoftwareEdgeSlidingGestureCap();

    bool confirmGestureConflict(bool enablingEdgeScroll);
    void applyScrollingType(const QString& data);

    void pointerSpeedSlot(int value);

    void buriedSettings(QString settingsName, QString action, QString value) const;

    bool getTouchpadEnabled();

    kdk::KHLineFrame* createBaseLine();

    void playDefaultGif() {
        uint gestureId = m_currentFingerShowType == TouchpadGestureFingerCount::threeFinger ? GESTURE_TOUCHPAD_3FINGER_TAP : GESTURE_TOUCHPAD_4FINGER_TAP;
        quint32 funcId = 0;
        bool isEnable = false;
        getGestureStatus(gestureId, funcId, isEnable);
        playGif(gestureId, funcId);
    }

signals:
    void fingerShowTypeChange(int);

private slots:
    void updateVisible();
};
#endif // WAYLANDTOUCHPADSETTINGSUI_H
