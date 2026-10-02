/* Copyright (C) 2011 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/


#include "utils/dll.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "display_control.h"

#include "DisplayControl/display_control_api_functions.h"

#define DISPLAY_CONTROL_LIB_NAME  "libDisplayControl"


// Define the DisplayControl API function pointers.
#define FUNCTION_(ret, name, sig, params) \
        static ret (*name) sig = NULL;
LIST_DISPLAY_CONTROL_API_FUNCTIONS(FUNCTION_)
#undef FUNCTION_

// Define a function that initializes the function pointers by looking up
// the symbols from the shared library.
static int initDisplayControlFuncs(ADynamicLibrary* displayControlLib) {
    void*  symbol;
    char*  error;

#define FUNCTION_(ret, name, sig, params) \
    symbol = adynamicLibrary_findSymbol(displayControlLib, #name, &error); \
    if (symbol != NULL) { \
        name = symbol; \
    } else { \
        derror("display control: Could not find required symbol (%s): %s", #name, error); \
        free(error); \
        return -1; \
    }
    LIST_DISPLAY_CONTROL_API_FUNCTIONS(FUNCTION_)
#undef FUNCTION_

    return 0;
}

/* Defined in android/hw-pipe-net.c */
//extern int android_init_opengles_pipes(void);

static ADynamicLibrary*  displayControlLib;
static int               displayControlStarted;
static char              displayControlAddress[4096] = {0};

int
android_initDisplayControl(void)
{
    char* error = NULL;

    if (displayControlLib != NULL)
        return 0;


    displayControlLib = adynamicLibrary_open(DISPLAY_CONTROL_LIB_NAME, &error);
    if (displayControlLib == NULL) {
        derror("Could not load display control library [%s]: %s",
               DISPLAY_CONTROL_LIB_NAME, error);
        return -1;
    }


    /* Resolve the functions */
    if (initDisplayControlFuncs(displayControlLib) < 0) {
        goto BAD_EXIT;
    }

    return 0;

BAD_EXIT:
    derror("Display control library could not be initialized!");
    adynamicLibrary_close(displayControlLib);
    displayControlLib = NULL;
    return -1;
}

int
android_startDisplayControl()
{
    if (!displayControlLib) {
        derror("Can't start display control without support libraries");
        return -1;
    }

    if (displayControlStarted) {
        return 0;
    }


    if (!initDisplayControl(displayControlAddress)) {
        derror("Can't start display control?");
        return -1;
    }

    displayControlStarted = 1;
    return 0;
}

static void strncpy_safe(char* dst, const char* src, size_t n)
{
    strncpy(dst, src, n);
    dst[n-1] = '\0';
}


void
android_stopDisplayControl(void)
{
    if (displayControlStarted) {
        //android_stop_opengl_logger();
        stopDisplayControl();
        displayControlStarted = 0;
    }
}

int android_display_control_set_path(const char* path)
{
    if(!path || (strlen(path) == 0))
        return -1;

    struct sockaddr_un local;

    if(strlen(path) >= sizeof(local.sun_path))
        return -E2BIG;

    memset(displayControlAddress, 0, sizeof(displayControlAddress));
    strcpy(displayControlAddress, path);

    return 0;
}

void
android_display_control_server_path(char* buff, size_t buffsize)
{
    strncpy_safe(buff, displayControlAddress, buffsize);
}
