/*
* Copyright (C) 2011-2015 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "DisplayControl/display_control_api.h"

#include "IOStream.h"
#include "DisplayControlServer.h"

#include "UnixStream.h"


#include <string.h>

static DisplayControlServer* s_displayControlThread = NULL;
static char s_displayControlAddr[256];


static kmre::DisplayControl::IOStream *createDisplayControlThread(int p_stream_buffer_size,
                                                             unsigned int clientFlags);

int initDisplayControl(const char* addr) {
    //
    // Fail if display control is already initialized
    //
    if (s_displayControlThread) {
        return false;
    }

    //
    // initialize the display control and listen to connections
    // on a thread in the current process.
    //

    s_displayControlThread = DisplayControlServer::create(addr);
    if (!s_displayControlThread) {
        return false;
    }
    strncpy(s_displayControlAddr, addr, sizeof(s_displayControlAddr));

    s_displayControlThread->start();

    return true;
}

int stopDisplayControl(void)
{
    bool ret = false;

    // open a dummy connection to the display control to make it
    // realize the exit request.
    // (send the exit request in clientFlags)
    kmre::DisplayControl::IOStream *dummy = createDisplayControlThread(8, IOSTREAM_CLIENT_EXIT_SERVER);
    if (!dummy) return false;

    if (s_displayControlThread) {
        // wait for the thread to exit
        ret = s_displayControlThread->wait(NULL);

        delete s_displayControlThread;
        s_displayControlThread = NULL;
    }

    delete dummy;

    return ret;
}

kmre::DisplayControl::IOStream *createDisplayControlThread(int p_stream_buffer_size, unsigned int clientFlags)
{
    kmre::DisplayControl::SocketStream*  stream = NULL;

    stream = new kmre::DisplayControl::UnixStream(p_stream_buffer_size);

    if (!stream) {
        return NULL;
    }
    if (stream->connect(s_displayControlAddr) < 0) {
        delete stream;
        return NULL;
    }

    //
    // send clientFlags to the displayControl
    //
    unsigned int *pClientFlags =
                (unsigned int *)stream->allocBuffer(sizeof(unsigned int));
    *pClientFlags = clientFlags;
    stream->commitBuffer(sizeof(unsigned int));

    return stream;
}
