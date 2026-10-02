/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef ABNORMALWINDOW_H
#define ABNORMALWINDOW_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <ukuiwindowhelper.h>
#include "usdblockshortcut.h"

class QGSettings;

class AbnormalWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AbnormalWindow(QWidget *parent = nullptr);
    void showEmergencyWindow();
    void showAbnormalWindow();
    void hideAbnormalWindow();
    /**
     * @brief blockSomeShortCut 禁启用快捷键
     * @param isBlock 是否为禁用
     */
    void blockSomeShortCut(bool isBlock = false);

private Q_SLOTS:
    void onDesktopResized();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initBackgroundPix();

private:
    bool screensaverIsShow = false;
    QGSettings *saverGsettings;
    QString m_background;
    QString m_tipText;
    QString m_feedbackText;
    QPixmap m_backgroundPix;
    bool m_isEmergency = false;
    UkuiWindowHelper *m_windowHelper = nullptr;
    USDBlockShortCut *m_usdBlockShortCut = nullptr; // usd禁启用快捷键实例
};

#endif // ABNORMALWINDOW_H
