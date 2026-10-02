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
#ifndef SETUPPLAYWIDGET_H
#define SETUPPLAYWIDGET_H

#include "../themewidget.h"

class CheckBox;

class SetupPlay : public ThemeWidget
{
    Q_OBJECT

public:
    explicit SetupPlay(QWidget *parent = nullptr);
    ~SetupPlay();

    void initData();
    void setBlackTheme();
    void setDefaultTheme();
    void setWidgetFont(QString family, int size);

private:
    void initConnect();
    void initLayout();

    CheckBox *m_autoFullScreenCheckBox;
    CheckBox *m_clearListExitCheckBox;
    CheckBox *m_findAssociatedPlayCheckBox;
    CheckBox *m_lastPosPlayCheckBox;
};

#endif // SETUPPLAYWIDGET_H
