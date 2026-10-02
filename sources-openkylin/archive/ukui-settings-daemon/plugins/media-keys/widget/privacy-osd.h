/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
#ifndef PRIVACYOSD_H
#define PRIVACYOSD_H

#include <QMap>
#include <QLabel>
#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QDBusInterface>
#include "QGSettings/qgsettings.h"
#include <ukuiwindowhelper/ukuiwindowhelper.h>


#define QT_THEME_SCHEMA                 "org.ukui.style"
#define DEFAULT_LOCALE_ICON_NAME        ":/ukui_res/ukui/"
#define PANEL_SCHEMA                    "org.ukui.panel.settings"
#define PANEL_SIZE_KEY                  "panelsize"
#define SYSTEM_FONT_SIZE                "system-font-size"
#define THEME_DARK_MODE                 "ukui-dark"
#define THEME_STYLE_NAME                "style-name"

#define SECOND_COLOR "kfont-secondary"

#define OSD_MINWIDTH 300

class PrivacyOSD : public QWidget
{
    Q_OBJECT
public:
    PrivacyOSD(QWidget *parent = nullptr);
    ~PrivacyOSD();
    void test(QString);
private:
    void initWindowsAttribute();
    void initAllControl();
    void setFixedSizeWithImageCount(int imagesCount);
    void initPromptFont();

    void initEmptyFont();

    void adjustPromptPositions();
    QString wrapText(const QString& text, const QFontMetrics& metrics, int maxWidth);
    void initIcons(QMap<QString, bool> &deviceStatus);
    inline double getGlobalOpacity() const;

    void paintEvent(QPaintEvent* e);

    int getPromptMaxWidth();
    int getSecondaryMaxWidth();

    int gerEmptyMaxWidth();

    void showSwitchWithImage(QMap<QString, bool> &deviceStatus, bool switchState);
    QPixmap drawLightColoredPixmap(const QPixmap &source, const QString &style);
    void callPrivacyBusShowOSD(bool state);

    void initWindowHelper();

    void showDeviceMsg(QMap<QString, bool> &deviceStatus, bool switchState);
    int getDevMaxWidth();

Q_SIGNALS:
    void osdHadShow();

private Q_SLOTS:
    void priScreenChanged(int x, int y, int width, int height);
    void repaintWidget();
    void onStyleChanged(const QString& key);
    void geometryChangedHandle();
    void doHideTimer();
    void doPrivacyBusSignal(bool state);
    QString getSvgIcon(QString device, bool state);

    void doPrepareForSleep(bool isSleep);
    void onUserActive(bool isActive);
    void onUnlock();
    bool initPrivacyState();
private:
    QTimer *m_hideTimer;
    QLabel *m_primaryPrompt;
    QLabel *m_secondaryPrompt;

    QLabel *m_emptyPrompt = nullptr;
    QLabel *m_deviceMsgLabel = nullptr;

    QGSettings *m_styleSettings;
    QDBusInterface *m_prvacyBus;
    QMap<QString,QLabel *> m_iconsMap;

    int m_inPrivacy = -1;
    bool m_isCallByDbus = false;//用于区分当前操作的源头是谁，如果是dbus，并且信息一致而且是show状态则隐藏。
    int m_primaryPromptIncrement;
    int m_secondaryPromptIncreament;

    UkuiWindowHelper* m_windowHelper = nullptr;
};

#endif // PRIVACYOSD_H
