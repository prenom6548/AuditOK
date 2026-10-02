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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "minimodebutton.h"

#include <QLabel>
#include <QHBoxLayout>

MiniModeButton::MiniModeButton(QString iconName, QSize size, QSize iconSize, QWidget *parent):
    QPushButton(parent),
    iconName(iconName)
{
    labIcon = new QLabel(this);
    labIcon->setAccessibleName("kylin-video_QLabel_labIcon");
    setFixedSize(size);
    labIcon->setFixedSize(iconSize);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(labIcon);
}

MiniModeButton::~MiniModeButton()
{

}

void MiniModeButton::resetName(QString iconName)
{
    this->iconName = iconName;
    if(isBlackTheme)
        setBlackTheme();
    else
        setLightTheme();
}

//60% 透明度（153 = 255×60%）
void MiniModeButton::setLightTheme()
{
    isBlackTheme = false;
    if(iconName == "")
        return;
    setStyleSheet(QString("QPushButton{border-radius:%0px;background-color:rgba(249,249,249,153);}").arg(width()/2));
    labIcon->setStyleSheet(QString("border-image:url(:/%0/%1-d.png);").arg("ico_light").arg(iconName));
}

//60% 透明度（153 = 255×60%）
void MiniModeButton::setBlackTheme()
{
    isBlackTheme = true;
    if(iconName == "")
        return;
    setStyleSheet(QString("QPushButton{border-radius:%0px;background-color:rgba(31,32,34,153);}").arg(width()/2));
    labIcon->setStyleSheet(QString("border-image:url(:/%0/%1-d.png);").arg("ico").arg(iconName));
}

// 鼠标进入：透明度提升至 70%（180 = 255×70%）
void MiniModeButton::enterEvent(QEvent *e)
{
    QString icodir;
    if(isBlackTheme)
    {
        setStyleSheet(QString("QPushButton{border-radius:%0px;background-color:rgba(31,32,34,180);}").arg(width()/2));
        icodir = "ico";
    }
    else
    {
        setStyleSheet(QString("QPushButton{border-radius:%0px;background-color:rgba(249,249,249,180);}").arg(width()/2));
        icodir = "ico_light";
    }
    labIcon->setStyleSheet(QString("border-image:url(:/%0/%1-h.png);").arg(icodir).arg(iconName));
}

// 鼠标离开：恢复 60% 透明度
void MiniModeButton::leaveEvent(QEvent *e)
{
    QString icodir;
    if(isBlackTheme)
    {
        setStyleSheet(QString("QPushButton{border-radius:%0px;background-color:rgba(31,32,34,153);}").arg(width()/2));
        icodir = "ico";
    }
    else
    {
        setStyleSheet(QString("QPushButton{border-radius:%0px;background-color:rgba(249,249,249,153);}").arg(width()/2));
        icodir = "ico_light";
    }
    labIcon->setStyleSheet(QString("border-image:url(:/%0/%1-d.png);").arg(icodir).arg(iconName));
}
