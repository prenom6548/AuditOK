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
#ifndef MINIMODESHADE_H
#define MINIMODESHADE_H

#include <QWidget>
#include "widget/filletwidget.h"

#define BTN_LIGHT_BACKGROUND "rgba(249,249,249,77)"
#define BTN_BLACK_BACKGROUND "rgba(31,32,34,77)"

class QPushButton;
class MiniModeButton;

class MiniModeShade : public FilletWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit MiniModeShade(QWidget *parent = nullptr);

    void setBlackTheme();
    void setLightTheme();

signals:
    void sigShowNormal();
    void sigPlayPause();
    void sigClose();

private:
    void initLayout();
    void initConnect();

private:
    MiniModeButton  *btnClose,
                    *btnNormal,
                    *btnPlayPause;

protected:
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
};

#endif // MINIMODESHADE_H
