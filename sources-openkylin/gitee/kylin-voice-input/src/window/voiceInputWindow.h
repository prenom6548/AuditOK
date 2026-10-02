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
#include <QString>
#include <QTimer>

class QScreen;
#include <ukuiwindowhelper.h>

class VoiceInputWindow : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString iconSource READ iconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(int windowSize READ windowSize CONSTANT)
    Q_PROPERTY(int hostSize READ hostSize CONSTANT)
    Q_PROPERTY(QColor themeColor READ themeColor NOTIFY themeColorChanged)
    Q_PROPERTY(bool isRecognizing READ isRecognizing NOTIFY isRecognizingChanged)

public:
    explicit VoiceInputWindow(QWindow *parent = nullptr);

    QString iconName() const;
    void setIconName(const QString &iconName);
    QString iconSource() const;

    int windowSize() const;
    int hostSize() const;
    bool isRecognizing() const;

    QColor themeColor() const;

public slots:
    void showWindow();
    void hideWindow();
    void updateIsRecognizing(bool isRecognizing);
    void onVolumeLevelChanged(qreal level);

signals:
    void iconNameChanged();
    void iconSourceChanged();
    void themeColorChanged();
    void isRecognizingChanged();

private:
    void initThemeColor();
    void updateThemeColorFromSettings();
    void updateIconSource();
    void setDisplayIcon(const QString &iconName);
    void restoreDefaultIcon();
    void onVolumeSampleTimeout();
    static QString iconNameForVolumeLevel(qreal level);
    void onStyleSettingsChanged(const QString &key);
    static QColor parseThemeColor(const QString &value);
    void updatePosition();
    void applyWindowGeometry(QScreen *targetScreen, int x, int y);
    void initWindowHelper();

    static constexpr int kWindowSize = 40;
    static constexpr int kBottomMargin = 50;
    static constexpr int kPulseExpand = 6;
    static constexpr int kVolumeSampleIntervalMs = 300;
    static constexpr const char *STYLE_SCHEMA_ID = "org.ukui.style";
    static constexpr const char *KEY_THEME_COLOR = "themeColor";

    QGSettings *m_styleSettings = nullptr;
    UkuiWindowHelper *m_windowHelper = nullptr;
    QString m_defaultIconName = QStringLiteral("audio-input-microphone-symbolic");
    QString m_iconName = QStringLiteral("audio-input-microphone-symbolic");
    QString m_iconSource;
    bool m_isRecognizing = false;
    qreal m_periodMaxVolumeLevel = 0.0;
    QTimer *m_volumeSampleTimer = nullptr;
    QColor m_themeColor = QColor(58, 140, 255);
};

#endif // VOICEINPUTWINDOW_H
