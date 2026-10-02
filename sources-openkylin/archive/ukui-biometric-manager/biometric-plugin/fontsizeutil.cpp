/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "fontsizeutil.h"
#include <QGSettings>
#include <QFont>
#include <QScreen>
#define ICON_TYPE_SCHENA "org.ukui.style"
#define FONT_SIZE "system-font-size"
#define DEFAULT_FONT_SIZE 10.0

double FontSizeUtil::getPtToPx()
{
    double ptToPx = 1.0;
    if (QApplication::primaryScreen()->logicalDotsPerInch() > 0) {
        ptToPx = 72.0 / (QApplication::primaryScreen()->logicalDotsPerInch());
    }
    return ptToPx;
}

void FontSizeUtil::setFontSize(QLabel *label, double baseFontSize)
{
    if (!label) {
        return;
    }
    
    QFont labelFont = qApp->font();
    double ptToPx = getPtToPx();
    double curFontSize = 0.0; // 字体大小差值
    
    // 获取系统字体大小
    if (QGSettings::isSchemaInstalled(ICON_TYPE_SCHENA)) {
        QGSettings *styleSettings = new QGSettings(ICON_TYPE_SCHENA, "");
        if (styleSettings) {
            double systemFontSize = styleSettings->get(FONT_SIZE).toDouble();
            if (systemFontSize > 0) {
                curFontSize = systemFontSize - DEFAULT_FONT_SIZE;
            }
            delete styleSettings;
        }
    }
    
    // 计算最终字体大小： (基础字体大小 + 字体大小差值) * ptToPx
    double targetPointSize = (baseFontSize + curFontSize) * ptToPx;
    labelFont.setPointSize(qRound(targetPointSize));
    label->setFont(labelFont);
}

