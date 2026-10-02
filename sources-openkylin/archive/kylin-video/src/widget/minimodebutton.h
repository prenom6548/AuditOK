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
#ifndef MINIMODEBUTTON_H
#define MINIMODEBUTTON_H

#include <QPushButton>

class QLabel;

/** **************************
 * mini 模式下的按钮
 *****************************/

class MiniModeButton : public QPushButton
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    MiniModeButton(QString iconName, QSize size, QSize iconSize, QWidget *parent = nullptr);
    ~MiniModeButton();

    void resetName(QString iconName);

    void setLightTheme();
    void setBlackTheme();

private:
    QLabel *labIcon;
    QString iconName;
    bool isBlackTheme;

protected:
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
};

#endif // MINIMODEBUTTON_H
