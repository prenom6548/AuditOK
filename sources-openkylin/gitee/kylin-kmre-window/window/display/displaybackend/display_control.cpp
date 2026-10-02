/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <syslog.h>

#include "libwrapper.h"
#include "display_control.h"
#include "display_control_api_functions.h" // defined LIST_DISPLAY_CONTROL_API_FUNCTIONS


static LibWrapper* displayControlLibWrapper = nullptr;
static bool displayControlStarted = false;
static char displayControlAddress[4096] = {0};

// Define the DisplayControl API function pointers.
#define FUNCTION_(ret, name, sig, params) \
        static ret (*func_##name) sig = nullptr;
LIST_DISPLAY_CONTROL_API_FUNCTIONS(FUNCTION_)
#undef FUNCTION_

// Define a function that initializes the function pointers by looking up
// the symbols from the shared library.
static int initDisplayControlFuncs(LibWrapper* dcLibWrapper) 
{
    if (!dcLibWrapper) {
        return -1;
    }
    
#define FUNCTION_(ret, name, sig, params) \
    { \
        auto [result, symbol] = dcLibWrapper->getSymbol(#name); \
        if (result && symbol) { \
            func_##name = symbol; \
        } \
        else { \
            return -1; \
        } \
    }

    LIST_DISPLAY_CONTROL_API_FUNCTIONS(FUNCTION_)
#undef FUNCTION_

    return 0;
}

int android_initDisplayControl(void)
{
    if (displayControlLibWrapper != nullptr) {
        return 0;
    }

    displayControlLibWrapper = new LibWrapper("libDisplayControl.so");

    /* Resolve the functions */
    if (initDisplayControlFuncs(displayControlLibWrapper) < 0) {
        syslog(LOG_ERR, "Display control library could not be initialized!");
        delete displayControlLibWrapper;
        displayControlLibWrapper = nullptr;
        
        return -1;
    }

    return 0;
}

int android_startDisplayControl()
{
    if (!displayControlLibWrapper) {
        syslog(LOG_ERR, "Can't start display control without support libraries");
        return -1;
    }

    if (displayControlStarted) {
        return 0;
    }


    if (!func_initDisplayControl(displayControlAddress)) {
        syslog(LOG_ERR, "Can't start display control?");
        return -1;
    }

    displayControlStarted = true;
    return 0;
}

void android_stopDisplayControl(void)
{
    if (displayControlStarted) {
        func_stopDisplayControl();
        displayControlStarted = false;
    }
}

int android_display_control_set_path(const char* path)
{
    if(!path || (strlen(path) == 0))
        return -1;

    struct sockaddr_un sun_addr;

    if(strlen(path) >= sizeof(sun_addr.sun_path))
        return -E2BIG;

    memset(displayControlAddress, 0, sizeof(displayControlAddress));
    strcpy(displayControlAddress, path);

    return 0;
}

void android_display_control_server_path(char* buff, size_t buffsize)
{
    if (buff && (buffsize > 0)) {
        strncpy(buff, displayControlAddress, buffsize);
        buff[buffsize - 1] = '\0';
    }
}
