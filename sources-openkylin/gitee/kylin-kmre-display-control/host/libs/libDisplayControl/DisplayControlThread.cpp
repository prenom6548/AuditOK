/*
* Copyright (C) 2011 The Android Open Source Project
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
#include "DisplayControlThread.h"

#include "ReadBuffer.h"
#include "DisplayControl.h"


#define STREAM_BUFFER_SIZE 4*1024*1024

DisplayControlThread::DisplayControlThread(kmre::DisplayControl::IOStream *stream, emugl::Mutex *lock) :
        emugl::Thread(),
        m_lock(lock),
        m_stream(stream) {}

DisplayControlThread::~DisplayControlThread() {
}

// static
DisplayControlThread* DisplayControlThread::create(kmre::DisplayControl::IOStream *stream, emugl::Mutex *lock) {
    return new DisplayControlThread(stream, lock);
}

void DisplayControlThread::forceStop() {
    m_stream->forceStop();
}

intptr_t DisplayControlThread::main() {

    displayControl_decoder_context_t dcDec;
    //
    // initialize decoders
    //
    initDisplayControlContext(&dcDec);

    kmre::DisplayControl::ReadBuffer readBuf(STREAM_BUFFER_SIZE);

    while (1) {

        int stat = readBuf.getData(m_stream);
        if (stat <= 0) {
            break;
        }


        bool progress;
        do {
            progress = false;

            m_lock->lock();

            //
            // try to process some of the command buffer using the
            // displayControl decoder
            //

            size_t last = dcDec.decode(readBuf.buf(), readBuf.validData(), m_stream);
            if (last > 0) {
                readBuf.consume(last);
                progress = true;
            }

            m_lock->unlock();

        } while( progress );

    }

    return 0;
}
