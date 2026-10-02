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
#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <QWidget>

namespace Ui {
class TouchScreen;
}

namespace kdk {
class KSwitchButton;
}

enum class TouchGestureType
{
    bottomEdgeSwipe,
    topEdgeSwipe,
    rightEdgeSwipe,
    fourFingersSwipeDown,
    fourFingersHorizontalSwipe,
    unknown
};

class TouchGestureInfo
{
public:
    TouchGestureInfo() = default;
    TouchGestureInfo(TouchGestureType gesture, QString descrip,
                     QString actionDescrip, QString gifPath)
        : gestureType(gesture)
        , description(descrip)
        , actionDescription(actionDescrip)
        , gifPath(gifPath)
    {}

    TouchGestureInfo(const TouchGestureInfo&) = default;
    TouchGestureInfo& operator=(const TouchGestureInfo&) = default;

    TouchGestureType gestureType;
    QString description;
    QString actionDescription;
    QString gifPath;
};

class QMovie;
class QGSettings;
class GestureWidget;
class TouchGestureInfo;
class AutoTabletModeWidget;
class TabletModeDesktopWidget;
class TouchScreen : public QWidget
{
    Q_OBJECT
public:
    explicit TouchScreen(QList<TouchGestureInfo> gestureInfos, QString modeTypeText,
                         QString autoSwitchText,
                         QWidget* parent = nullptr);
    ~TouchScreen();

    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void stopGif();
signals:
    void iconThemeChangedSignal();

private slots:
    void onTabletModeEnabledChanged(bool enabled);
private:
    GestureWidget * createGestureWidget(const TouchGestureInfo& gestureInfo);
    QList<GestureWidget*> createGestureWidgets();
    void initUI();
    void initConnection();
    void loadGif();
    void playGif(const QString& gifPath);
    bool getTabletModeEnabled() const;
    void buriedSettings(QString settingsName, QString action, QString value) const;
    void resetAutoTabletModeWidgetEnabled(bool enabled);
    void resetTabletModeDesktopWidgetEnabled(bool enabled);
    void initTouchFeedbackSettings();
    QWidget* createTouchFeedbackSettingsWidget();
    QWidget* createEffectSwitchRow(const QString& text, const QString& effectName, QWidget* parent);
    bool isWlcomEffectAvailable() const;
    bool isEffectSupported(const QString& effectName) const;
    bool getEffectEnabled(const QString& effectName) const;
    void setEffectEnabled(const QString& effectName, bool enabled);

    Ui::TouchScreen *ui;
    quint32 m_cycleNum;
    QMovie *m_movie;
    QGSettings *m_iconThemeGSettings;
    QList<TouchGestureInfo> m_gestureInfos;

    const QString m_modeTypeText;
    const QString m_autoSwitchText;
    AutoTabletModeWidget* m_autoTabletModeWidget;
    TabletModeDesktopWidget* m_tabletModeDesktopWidget;

};

#endif // TOUCHSCREEN_H
