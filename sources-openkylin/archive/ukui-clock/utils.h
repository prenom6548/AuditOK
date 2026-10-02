/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef UTILS_H
#define UTILS_H
#include <QObject>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>
#include <QFontDatabase>
#include <QPushButton>
#include <QLabel>
#include "ukuistylehelper/ukuistylehelper.h"
#include "windowmanager/windowmanager.h"

class Utils :public QObject
{
    Q_OBJECT

public:
    explicit Utils(QObject *parent = nullptr);
    void centerToScreen(QWidget* widget);
    static QString loadFontFamilyFromTTF();
    static QString loadFontHuaKangJinGangHeiRegularTTF();
    static QString getRandomId();
    static void setBtnBackgroundColorTransparent(QPushButton * btn);
    static void setBtnHighlightColorTransparent(QPushButton * btn);
    static QString changeNumToStr(int alarmHour);
    static QIcon getQIcon(QString themeUrl, QString localUrl);
    static QString getOmitStr(QString str,int sizeLimit);
    static QString getOmitStr(const QString& str, const QFont& font, int pixelWidthLimit);       // 根据字体和像素宽度限制截断字符串
    static bool checkLocalChina();
    static bool checkLocalUs();
    static bool checkLocalBo();
    static void resetText(QLabel* label , int pageIndex);  //根据字体大小重设文字样式
    static int handelColorRange(int value);
    static QString getClockVersion();
    QRect centerToScreenQRect(QWidget *widget);
};

#endif // UTILS_H
