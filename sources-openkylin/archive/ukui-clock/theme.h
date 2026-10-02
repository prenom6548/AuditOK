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
#ifndef THEME_H
#define THEME_H
#include <QGSettings>
#include <QObject>
#include <QString>
#include <QColor>
#include "constant_class.h"
class theme : public QObject
{
    Q_OBJECT
public:
    explicit theme(QObject *parent = nullptr);
    QGSettings* qtSettings;
    friend class TestClass;  // 核心修改：声明测试类为友元

    static QString backcolor_str;
    static QColor backcolcr;
    static QColor dialogBackcolcr;
    static QColor selectDialogBackcolcr;
    static QColor itemBackColor;
    static QColor selectBtnBackColor;
    static QColor switchBtnBackColor;
    static QColor timeScrollBackColor;
    static QColor countdownRingBackColor;
    static QColor tinyBtnBackColor;
    static QColor boderColor;
    static int themetype;
    ~theme();
    static QPixmap changeIconColor(QPixmap pixmap, QColor color);
    static QString getColorStr(QColor color);
    static QColor highLight_Hover();
    static QColor button_Click();
    static QColor button_Hover();
    static QColor highLight_Click();
signals:
private:
    void setColorByTheme(QString currentThemeMode);
};
#define COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE 240,240,240,255
#define BORDERCOLOR_ALPHAF 0.15
#define SHADOWCOLOR_ALPHAF 0.16
const static int SHADOW_RANGE=8;
const static int VAGUE_RADIUS=15;
const static double BORDER_RANGE=0.5;
#endif // THEME_H
