/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef TOUCHPADUI_H
#define TOUCHPADUI_H

#include <QWidget>
#include "../config-monitor/configmonitor.h"
#include <kswitchbutton.h>

//垂直边界
#define V_EDGE_KEY               "vertical-edge-scrolling"
//垂直中间
#define V_FINGER_KEY             "vertical-two-finger-scrolling"
#define N_SCROLLING              "none"
//水平中间
#define H_FINGER_KEY             "horizontal-two-finger-scrolling"
//水平边界
#define H_EDGE_KEY               "horizontal-edge-scrolling"

#define CONFIG_FILE_NAME         "org.ukui.peripherals-touchpad"
#define CONFIG_KEY_TOUCHPAD_ENABLED "touchpad-enabled"
#define CONFIG_KEY_DISABLE_ON_EXTERNAL_MOUSE_ENABLED "disable-on-external-mouse"
#define CONFIG_KEY_DISABLE_WHILE_TYPING "disable-while-typing"

/*
*motion-acceleration
* 0.1 --- 10
*  快 --- 慢
*/
const double kPointerSpeedMin = 0.1;
const double kPointerSpeedMax = 9;

enum class TouchpadGestureType
{
    threeFingerClick,
    threeFingerSwipeDown,
    threeFingerSwipeUp,
    threeFingersHorizontalSwipe,
    fourFingerClick,
    fourFingersHorizontalSwipe,
    unknown
};

class TouchpadGestureInfo
{
public:
    TouchpadGestureInfo() = default;
    TouchpadGestureInfo(TouchpadGestureType gesture, QString descrip,
                     QString actionDescrip, QString gifPath)
        : gestureType(gesture)
        , description(descrip)
        , actionDescription(actionDescrip)
        , gifPath(gifPath)
    {}

    TouchpadGestureInfo(const TouchpadGestureInfo&) = default;
    TouchpadGestureInfo& operator=(const TouchpadGestureInfo&) = default;

    TouchpadGestureType gestureType;
    QString description;
    QString actionDescription;
    QString gifPath;
};

class QFrame;
class QGSettings;
class GestureWidget;
class TouchpadUI : public QWidget
{
    Q_OBJECT

public:
    explicit TouchpadUI(QList<TouchpadGestureInfo> gestureInfos,
                        QWidget *parent = nullptr);
    ~TouchpadUI() = default;

public slots:
    void stopGif();

signals:
    void iconThemeChangedSignal();
    void touchpadEnabled(bool status);

private:
    QGSettings *m_touchpadGsetting;
    QMovie *m_movie;
    QVector<GestureWidget *> m_gestureArray;
    QString projectCodeName;
    QList<TouchpadGestureInfo> m_gestureInfos;
    bool m_touchpadEnabled;
    kdk::KSwitchButton* m_touchpadDisableBtn;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    ConfigMonitor* m_configMonitor;

    void initTouchpadEnabled();
    void initUI();
    void setTouchPadEnabled(bool enabled);

    QWidget* createTitleWidget(QString titleText);

    QWidget* createTouchPadSettingWidget();
    QWidget* createTouchpadDisableWidget();
    QWidget* createMouseDisableWidget();
    QWidget* createPointerSpeedWidget();
    QWidget* createTypingDisableWidget();
    QWidget* createClickDisableWidget();
    QWidget* createScrollSlideWidget();
    QWidget* createScrollAreaWidget();

    QWidget* createGesturePreviewWidget();
    QWidget* createGestureSelectWidget();
    GestureWidget* createGestureWidget(const TouchpadGestureInfo& info);
    QList<GestureWidget*> createGestureWidgets();
    QWidget* createVideoWidget();

    QWidget* createMoreGestureWidget();

    QString currentScrollingType();

    bool hasMouseDevice() const;
    void showWarningDialog();

    void touchpadEnabledSlot(bool status);
    void mouseDisableSlot(bool status);
    void pointerSpeedSlot(int value);
    void typingDisableSlot(bool status);
    void clickSlot(bool status);
    void scrollSlideSlot(bool status);
    void playGif(const QString& gifPath);
    void buriedSettings(QString settingsName, QString action, QString value) const;

private slots:
    void onConfigChanged(kdk::KSwitchButton*, QString, QString, bool);
};
#endif // TOUCHPAD_H
