/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef SIZE_PARA_H
#define SIZE_PARA_H
#include <QString>
#include <QSize>
#include <QColor>
#define NET_CHECK_SCHEMA "org.kylin-nw-check.settings"
#define UKUI_STYLE "org.ukui.style"

const int MAIN_WIN_WIDTH = 900;
const int MAIN_WIN_HEIGHT = 600;
const int NAVIGATION_SECTOR_WIDTH = 300;
const int EXHBITION_SECTOR_WIDTH = 600;
const QSize DEVICE_LIST_SECTION = QSize(286, 500);
const QSize DEVICE_LIST_BTN_SIZE = QSize(268, 60);
const QSize DEVICE_PROPERTY_SECTION = QSize(392, 472);
const QSize TITLE_BTN_SIZE = QSize(24, 24);
const QSize TITLE_ICON_SIZE = QSize(24, 24);
const QSize BTN_ICON_SIZE = QSize(16, 16);
const QSize LARGE_BTN_ICON_SIZE = QSize(48, 48);
const QSize LARGE_BTN_SIZE = QSize(36, 36);
const QSize WIN_BTN_SIZE = QSize(30, 30);
const QSize NORMAL_BTN_SIZE = QSize(120, 36);
const QSize LOGO_ICON_SIZE = QSize(128, 128);
const QSize LOGO_BTN_SIZE = QSize(128, 128);

const QSize TITLE_MAIN_SIZE = QSize(64, 64);

const QString BTN_TO_LABEL_STYLE = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                                   "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                                   "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
const QString GRAY_TEXT_LABEL_STYLE = "QLabel{color:rgba(140, 140, 140, 1);}";

template <typename T> void elideLabelText(T &container, const QString &inputStr)
{
    QFontMetrics fontMetrics(container->font());
    int fontSize = fontMetrics.width(inputStr);

    if (fontSize > container->width()) {
        QString tmpStr = inputStr;
        tmpStr = fontMetrics.elidedText(inputStr, Qt::ElideRight, container->width());
        container->setText(tmpStr);
        container->setToolTip(inputStr);
    } else {
        container->setToolTip("");
        container->setText(inputStr);
    }
}


#endif // SIZE_PARA_H
