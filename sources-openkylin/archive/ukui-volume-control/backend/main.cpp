/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <iostream>
//#include <glib.h>

#include "AudioContext.h"
#include "SingleApplication.h"
#include "VolumeControl.h"


int main()
{
    GMainLoop* mainloop;
    GMainContext* context = nullptr;

    mainloop = g_main_loop_new(context, FALSE);
    SingleApplication app("ukui-audio-service");

    if (app.isAlreadyRuning()) {
        std::cerr << "ukui-audio-service is already running. Exiting." << std::endl;
        return 1;
    }

    UkuiAudioFramwork::VolumeControl::getInstance().init();

    g_main_loop_run(mainloop);
    return 0;
}
