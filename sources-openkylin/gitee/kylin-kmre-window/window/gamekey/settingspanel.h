/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#ifndef SETTINGS_PANEL_H
#define SETTINGS_PANEL_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QSlider>
#include <QGridLayout>

#define DEFAULT_SETTINGS_PANEL_WIDTH  450
#define DEFAULT_SETTINGS_PANEL_HEIGHT 80

class KmreWindow;

class SettingsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPanel(KmreWindow* window);
    ~SettingsPanel();
    
    void enableAddSteelingWheelBtn(bool enable);
    
signals:
    void addSingleKey();
    void addSteelingWheel();
    void saveGameKeys();
    void hideGameKeys();
    void clearGameKeys();
    void updateTransparency(double ratio);

private:
    
    QGridLayout* mLayout = nullptr;

    QToolButton* mAddSingleKeyBtn = nullptr;
    QToolButton* mAddSteelingWheelBtn = nullptr;
    QPushButton* mSaveBtn = nullptr;
    QPushButton* mHideBtn = nullptr;
    QPushButton* mClearBtn = nullptr;
    QSlider* mTransparencySlider = nullptr;
    QLabel* mTransparencyLabel = nullptr;

    double m_width_scale_ratio;
    double m_height_scale_ratio;
};

#endif // SETTINGS_PANEL_H
