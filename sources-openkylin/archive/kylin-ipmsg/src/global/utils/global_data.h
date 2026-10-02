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
 */

#ifndef __GLOBAL_DATA_H__
#define __GLOBAL_DATA_H__

#include "tcp_link.h"

#include <QString>
#include <QFont>
#include <QGSettings>
#include <QObject>
#include <QDBusInterface>

#include "rotatechangeinfo.h"

#define THEME_GSETTINGS "org.ukui.style"
#define APPLY_FONT_KEY "system-font"
#define APPLY_HIGHT_LIGHT_COLOR "theme-color"

#define KYLIN_STATUSMANAGER_SERVICE "com.kylin.statusmanager.interface"
#define KYLIN_STATUSMANAGER_PATH "/"
#define KYLIN_STATUSMANAGER_INTERFACE "com.kylin.statusmanager.interface"

#define ROTATION_NORMAL "normal"
#define ROTATION_UPSIDE_DOWN "upside-down"
#define ROTATION_LEFT "left"
#define ROTATION_RIGHT "right"

enum CurrentMode {
    PCMode = 0,
    HMode,
    VMode,
};

class GlobalData : public QObject
{
    Q_OBJECT
public:
    ~GlobalData();

    void getSystemTheme();
    void getSystemFontSize();
    void getSystemTransparency();

    QFont getFontSize12px();
    QFont getFontSize14px();
    QFont getFontSize16px();
    QFont getFontSize18px();

    /* 本机信息 */
    QString m_uuid;
    QString m_tcpListenIP;
    QString m_tcpListenPort;
    QString m_udpListenPort;

    /* tcp链接表 */
    TcpLink m_tcpLink;

    /* 字体监控 */
    double m_font14pxToPt;
    double m_font12pxToPt; /* 好友列表 信息字体 */
    double m_font16pxToPt; /* 标题栏设置弹窗描述字体 */
    double m_font18pxToPt; /* 聊天框 头像字体  , 主界面昵称*/

    QString m_fontName;
    QString m_colorName;
    // 获取当前模式
    CurrentMode getCurrentMode();
    CurrentMode m_currentMode = CurrentMode::PCMode;
    QPair<int, int> m_currentSize;

private:
    GlobalData();
    void getDbusData();  // xc平板上的dbus

    QDBusInterface *m_interface = nullptr;
    QGSettings *m_fontGsetting = nullptr;
    bool m_flatMode;
    QString m_rotation = ROTATION_NORMAL;

public:
    static GlobalData *getInstance();

private:
    void init(void);
    void getTcpListenIp(void);
    void getTcpListenPort(void);
    void getUdpListenPort(void);
    void getUuid(void);

    void getFontName();
    void changeFontName();

    void changeFontSize();

    // 接收xc平板上的信号
    void slotGetRotations(QString rotations); // 当前的旋转模式
    void slotGetMode(bool mode); // 根据当前翻折情况返回当前的机器模式

    RotateChangeInfo *m_rotateChange = nullptr; // 平板切换
    void slotChangeModel(bool mode, int width, int height);

signals:
    void signalChangeFont();
    void signalChangeFontSize();
    void signalChangeHightColor();
    void signalChangePC();
    void signalChangeFlatH();
    void signalChangeFlatV();
};

#endif
