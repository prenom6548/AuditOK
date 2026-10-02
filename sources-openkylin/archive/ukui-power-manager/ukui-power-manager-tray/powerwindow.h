/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POWERWINDOW_H
#define POWERWINDOW_H

#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QGSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QSlider>
#include <QMouseEvent>
#include <QProcess>
#include <QFont>
#include <QPushButton>
#include <QToolButton>

#include "kylable.h"
#include "kslider.h"
#include "divider.h"
#include "kwindowsystem.h"
#include "customstyle.h"
#include "ukuiwindowhelper.h"

#include "upm-gsettings/upm_gsettings.h"
#include "upower-dbus/upm_upowerdbus.h"

using namespace kdk;

class powerwindow : public QWidget
{
    Q_OBJECT

public:
    explicit powerwindow(QWidget *parent = nullptr);
    ~powerwindow();

    void setIconName(QString iconName);
    void setAcOnlineState(bool acOnlineState);
    void setBatteryState(int iBatteryState);
    void setBatteryPercentage(int iBatteryPercentage);
    void setShowPowerLeftTime(int iShowPowerLeftTime);
    void setBatteryLeftTime(qlonglong timeToFull, qlonglong timeToEmpty);
    void setPowerStateLabelForAcOnlineChanged();
    void setPowerStateLabelForBatteryInfoChanged();
    void setPercentageLabelFontsize();
    void showPowerWindow();
    void slideWindowByPanelPosition(int panelPosition);

private :
    int           m_deviceNum = 0;
    double        m_sysFontSize;

    bool          m_isWayland = false;
    int           m_panelPosition;
    int           m_batteryState = battery_state_last;
    bool          m_batteryStateChanged = false;
    int           m_batteryPercentage;
    bool          m_acOnlineState = true;
    bool          m_acOnlineChange;
    int           m_showBatteryLeftTime;
    qint64        m_timeToFull;
    qint64        m_timeToEmpty;
    QString       m_iconName;

    double        m_transparency = 0.75;
    QGSettings    *m_powerManagerGsettings;
    QGSettings    *m_styleSettings;

    QVBoxLayout  *m_pmainlayout    = nullptr;
    QHBoxLayout  *m_toplayout      = nullptr;
    QHBoxLayout  *m_firstlayout    = nullptr;
    QHBoxLayout  *m_lastlayout     = nullptr;
    QHBoxLayout  *m_statelayout    = nullptr;

    QWidget      *m_firstwidget;
    QWidget      *m_lastWidget;
    QWidget      *m_topwidget;
    QWidget      *m_statewidget;

    QPushButton  *m_iconButton;
    QLabel       *m_percentageLabel;
    QLabel       *m_powerStateLabel;
    QLabel       *m_toplabel;
    QToolButton  *m_enduranceIconButtun;
    QToolButton  *m_performanceIconButtun;

    Divider      *m_segmentationLine_1;
    Divider      *m_segmentationLine_2;

    KSlider      *m_StateSlider;
    KyLable      *m_settingLabel;

    int          m_powerPolicyAc;
    int          m_powerPolicyBattery;

    QRect        m_windowPositionRect;

    UkuiWindowHelper *m_windowHelper = nullptr;

    enum powerPolicy
    {
        Performance = 0,
        Balance,
        EnergySaving,
    };
    enum PanelPosition
    {
        panel_bottom = 0,
        panel_top,
        panel_left,
        panel_right
    };

    void initUI();
    void initPlatformType();
    void resetWindowPosition();
    void setWindowProperty();
    void setSliderValue();
    void updateBatteryIcon();

    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

private Q_SLOTS:
    void sliderValueChanged(int);

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // POWERWINDOW_H
