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
#ifndef _DISPLAY_CONTROL_THREAD_H
#define _DISPLAY_CONTROL_THREAD_H

#include "IOStream.h"

#include "mutex.h"
#include "thread.h"

// A class used to model a thread of the DisplayControlServer. Each one of them
// handles a single guest client / protocol byte stream.
class DisplayControlThread : public emugl::Thread {
public:
    // Create a new DisplayControlThread instance.
    // |stream| is an input stream that will be read from the thread,
    // and deleted by it when it exits.
    // |mutex| is a pointer to a shared mutex used to serialize
    // decoding operations between all threads.
    // TODO(digit): Why is this needed here? Shouldn't this be handled
    //              by the decoders themselves or at a lower-level?
    static DisplayControlThread* create(kmre::DisplayControl::IOStream* stream, emugl::Mutex* mutex);

    // Destructor.
    virtual ~DisplayControlThread();

    // Returns true iff the thread has finished.
    // Note that this also means that the thread's stack has been
    bool isFinished() { return tryWait(NULL); }

    // Force a thread to stop.
    void forceStop();

private:
    DisplayControlThread();  // No default constructor

    DisplayControlThread(kmre::DisplayControl::IOStream* stream, emugl::Mutex* mutex);

    virtual intptr_t main();

    emugl::Mutex* m_lock;
    kmre::DisplayControl::IOStream* m_stream;
};

#endif
