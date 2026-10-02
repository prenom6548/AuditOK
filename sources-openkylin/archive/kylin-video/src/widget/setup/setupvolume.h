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
#ifndef SETUPVOLUME_H
#define SETUPVOLUME_H

#include "../themewidget.h"

class CheckBox;

class QLabel;
class QComboBox;
class QHBoxLayout;

class SetupVolume : public ThemeWidget
{
    Q_OBJECT

public:
    explicit SetupVolume(QWidget *parent = nullptr);
    ~SetupVolume();

    void initData();
    void setBlackTheme();
    void setDefaultTheme();
    void setWidgetFont(QString family, int size);

private:
    QLabel *m_title1Label;

    QHBoxLayout *m_hboxLayout;
    QComboBox *m_audioOutputDriver;

    QLabel *m_title2Label;
    CheckBox *m_globalVolumeCheckBox;
    CheckBox *m_standerdVolumeCheckBox;

    void initLayout();
    void initConnect();
};

#endif // SETUPVOLUME_H
