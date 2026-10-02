/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "datacollect.h"
#include <kysdk/diagnosetest/libkydatacollect.h>
#include <QtConcurrent>

DataCollect::DataCollect(QObject *parent) : QObject(parent)
{

}

void DataCollect::sendMainWindowKdkDataAsync(WindowButton theme)
{
    int themeType = theme;
    switch(themeType)
    {
        case m_GlobalTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"mainwindow_globaltheme_click","mainwindow");
            //释放内存
            kdk_dia_data_free(node);
        });
        break;
        case m_IconTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"mainwindow_icontheme_click","mainwindow");
            kdk_dia_data_free(node);
        });
        break;
        case m_CursorTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"mainwindow_cursortheme_click","mainwindow");
            kdk_dia_data_free(node);
        });
        break;
        case m_PlymouthTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"mainwindow_plymouththeme_click","mainwindow");
            kdk_dia_data_free(node);
        });
        break;
        case m_GrubTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"mainwindow_grubtheme_click","mainwindow");
            kdk_dia_data_free(node);
        });
        break;
            default:
               break;
    }
}

void DataCollect::sendNavigationKdkDataAsync(DataCollect::NavigationButton theme)
{
    int themeType = theme;
    switch(themeType)
    {
        case n_GlobalTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"navigationbar_globaltheme_click","navigationbar");
            kdk_dia_data_free(node);
        });
        break;
        case n_IconTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"navigationbar_icontheme_click","navigationbar");
            kdk_dia_data_free(node);
        });
        break;
        case n_CursorTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"navigationbar_cursortheme_click","navigationbar");
            kdk_dia_data_free(node);
        });
        break;
        case n_PlymouthTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"navigationbar_plymouththeme_click","navigationbar");
            kdk_dia_data_free(node);
        });
        break;
        case n_GrubTheme:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"navigationbar_grubtheme_click","navigationbar");
            kdk_dia_data_free(node);
        });
        break;
        default:
           break;
    }
}

void DataCollect::sendGlobalThemeKdkDataAsync(DataCollect::GlobalThemeChangeWidget widget)
{
    int changewidget = widget;
    switch(changewidget)
    {
        case g_CoverWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"cover_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        case g_WallPaperWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"wall_paper_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        case g_ExteriorWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"exterior_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        case g_FilletWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"fillet_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        case g_AccentColorWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"accent_color_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        case g_TransparencyWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"transparency_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        case g_WindowFilletWidget:
        QtConcurrent::run([=]() {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
            kdk_dia_upload_default(node,"window_fillet_widget_click","globaltheme");
            kdk_dia_data_free(node);
        });
        break;
        default:
           break;
    }
}
