/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VOICEINPUTWINDOW_H
#define VOICEINPUTWINDOW_H

#include <QColor>
#include <QGSettings>
#include <QQuickView>
#include <QTimer>
#include <ukuiwindowhelper.h>

class VoiceInputWindow : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString iconSource READ iconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(int windowSize READ windowSize WRITE setWindowSize NOTIFY windowSizeChanged)
    Q_PROPERTY(int hostSize READ hostSize NOTIFY windowSizeChanged)
    Q_PROPERTY(QColor themeColor READ themeColor NOTIFY themeColorChanged)
    Q_PROPERTY(int pulseToken READ pulseToken NOTIFY pulseTokenChanged)

public:
    explicit VoiceInputWindow(QWindow *parent = nullptr);

    QString iconName() const;
    void setIconName(const QString &iconName);
    QString iconSource() const;

    int windowSize() const;
    void setWindowSize(int size);
    int hostSize() const;
    int pulseToken() const;

    QColor themeColor() const;

public slots:
    void showWindow();
    void hideWindow();
    void playRecognizingPulse();
    void onTextRecognizing();
    void onTextRecognized();
    void onVolumeLevelChanged(qreal level);

signals:
    void iconNameChanged();
    void iconSourceChanged();
    void windowSizeChanged();
    void themeColorChanged();
    void pulseTokenChanged();

private:
    void initThemeColor();
    void updateThemeColorFromSettings();
    void updateIconSource();
    void setDisplayIcon(const QString &iconName);
    void restoreDefaultIcon();
    void startLoadingAnimation();
    void stopLoadingAnimation();
    void advanceLoadingIcon();
    void onVolumeSampleTimeout();
    static QString iconNameForVolumeLevel(qreal level);
    void onStyleSettingsChanged(const QString &key);
    static QColor parseThemeColor(const QString &value);
    void updatePosition();
    void initWindowHelper();

    static constexpr bool kEnableLoadingIcon = false;
    static constexpr int kBottomMargin = 50;
    static constexpr int kPulseExpand = 6;
    static constexpr int kLoadingDelayMs = 800;
    static constexpr int kLoadingIntervalMs = 100;
    static constexpr int kLoadingIconCount = 8;
    static constexpr int kVolumeSampleIntervalMs = 300;
    static constexpr const char *STYLE_SCHEMA_ID = "org.ukui.style";
    static constexpr const char *KEY_THEME_COLOR = "themeColor";

    QGSettings *m_styleSettings = nullptr;
    UkuiWindowHelper *m_windowHelper = nullptr;
    QString m_defaultIconName = QStringLiteral("audio-input-microphone-symbolic");
    QString m_iconName = QStringLiteral("audio-input-microphone-symbolic");
    QString m_iconSource;
    int m_windowSize = 40;
    int m_pulseToken = 0;
    int m_loadingIndex = 0;
    qreal m_periodMaxVolumeLevel = 0.0;
    bool m_isLoadingIcon = false;
    QTimer *m_loadingDelayTimer = nullptr;
    QTimer *m_loadingCycleTimer = nullptr;
    QTimer *m_volumeSampleTimer = nullptr;
    QColor m_themeColor = QColor(58, 140, 255);
};

#endif // VOICEINPUTWINDOW_H
