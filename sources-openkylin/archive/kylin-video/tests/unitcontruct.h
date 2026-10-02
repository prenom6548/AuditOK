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
#ifndef UNITCONTRUCT_H
#define UNITCONTRUCT_H

#include <QObject>
#include <QtTest/QtTest>
//#include "unitTest/unitcontruct.h"

class MainWindow;
class unitConstruct : public QObject
{
    Q_OBJECT
public:
    friend class unitConstruct;

    explicit unitConstruct(QObject *parent = nullptr);

    MainWindow* m_win { nullptr };
private slots:

    void unit_title();


    void unit_globalSignal();

    void unit_coreSignal();

    void unit_functions();
    
    void unit_mediaInfoDialog();
    
    void unit_playlistWidget();
    
    void unit_sliderBar();
    
    void unit_checkBox();
    
    void unit_cMessageBox();
//    -------
    void unit_eventPassWidget();
    void unit_playXWidget();
    void unit_setupCodec();
    void unit_urlEditWidget();
    void unit_setupSystem();
    void unit_setupPlay();
    void unit_setupScreenshot();
    void unit_setupSubtitle();
    void unit_setupVolume();
    void unit_setupDialog();
    void unit_sqliteHandle();
    
    void unit_textLabel();
    
    void unit_miniModeButton();

    // 新增的测试函数
    void unit_log();
    void unit_dbusAdapter();
////    ----------
    void unit_controlBar();
    void unit_setupShortcut();

    void unit_mediaInfo();

    void unit_mpvcore();

    void unit_playlist();

    void unit_globalConfig();

    void unit_util();

    void unit_slider();



//    void unit_exit();
};












#endif // UNITCONTRUCT_H
