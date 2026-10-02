/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "themeController.h"
#include <QVariant>

QPixmap ThemeController::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QColor gray(128,128,128);
    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else if(qAbs(color.red()-standard.red())<20 && qAbs(color.green()-standard.green())<20 && qAbs(color.blue()-standard.blue())<20)
                {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else
                {
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap ThemeController::drawColoredPixmap(const QPixmap &source, const QColor &sampleColor)
{
//    QColor gray(128,128,128);
//    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++)
    {
        for (int y = 0; y < img.height(); y++)
        {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0)
            {
                color.setRed(sampleColor.red());
                color.setGreen(sampleColor.green());
                color.setBlue(sampleColor.blue());
                img.setPixelColor(x, y, color);
             }
        }
    }
    return QPixmap::fromImage(img);
}

QColor ThemeController::getCurrentIconColor()
{
    QPixmap pixmap = QIcon::fromTheme("open-menu-symbolic").pixmap(16,16);
    QImage img = pixmap.toImage();
    for (int x = 0; x < img.width(); x++)
    {
        for (int y = 0; y < img.height(); y++)
        {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0)
            {
                return color;
            }
        }
    }
}

ThemeController::ThemeController() :
    m_gsetting(nullptr),
    m_themeFlag(LightTheme),
    m_iconFlag(ClassicStyle)
{
    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOW))
    {
        m_gsetting = new QGSettings(FITTHEMEWINDOW);
        if(m_gsetting->get("style-name").toString() == "ukui-dark" || m_gsetting->get("style-name").toString() == "ukui-black")
        {
            m_themeFlag = DarkTheme;
        }
        else
        {
           m_themeFlag = LightTheme;
        }
        if(m_gsetting->get("style-name").toString() == "ukui-icon-theme-default")
        {
            m_iconFlag = DefaultStyle;
        }
        else
        {
           m_iconFlag = ClassicStyle;
        }
    }
}

ThemeController::~ThemeController()
{
    if (m_gsetting) {
        delete m_gsetting;
    }
}

void ThemeController::initThemeStyle()
{
    m_themeFlag = LightTheme;
    m_iconFlag = ClassicStyle;

    if (m_gsetting) {
        if (m_gsetting->get("style-name").toString() == "ukui-dark" || m_gsetting->get("style-name").toString() == "ukui-black")
        {
            m_themeFlag = DarkTheme;
        }
        if(m_gsetting->get("style-name").toString() == "ukui-icon-theme-default")
        {
            m_iconFlag = DefaultStyle;
        }
    }
}
