// Copyright (C) 2014 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __DISPLAY_CONTROL_DECODER_H_
#define __DISPLAY_CONTROL_DECODER_H_

#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>

typedef int32_t (*FBPost_proc_t) (uint32_t, int32_t, int32_t, int32_t, int32_t);
typedef void (*postLayer_proc_t) (const char*, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
typedef int32_t (*postAllLayersDone_proc_t) ();
typedef uint32_t (*getDisplayWidth_proc_t) ();
typedef uint32_t (*getDisplayHeight_proc_t) ();
typedef uint32_t (*getDisplayFPS_proc_t) ();
typedef uint32_t (*getDisplayXDpi_proc_t) ();
typedef uint32_t (*getDisplayYDpi_proc_t) ();
typedef void (*postVirtualDisplay_proc_t) (const char*, uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
typedef int32_t (*postAllVirtualDisplaysDone_proc_t) ();
typedef int32_t (*postBoForScreenRecord_proc_t) (uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t);
typedef void (*postCaptureScreenColorBuffer_proc_t) (uint32_t);
typedef int32_t (*authenticateMagic_proc_t) (uint32_t);
typedef int32_t (*updateSharingScreenDisplay_proc_t) (uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t);


namespace DisplayControl
{
class Mutex {

public:
    Mutex();
    ~Mutex();

    int lock();
    void unlock();

    class Autolock {
    public:
        inline Autolock(Mutex& mutex) : mLock(mutex)  { mLock.lock(); }
        inline Autolock(Mutex* mutex) : mLock(*mutex) { mLock.lock(); }
        inline ~Autolock() { mLock.unlock(); }
    private:
        Mutex& mLock;
    };

private:

    Mutex(const Mutex&);
    Mutex& operator = (const Mutex&);

    pthread_mutex_t mMutex;
};

inline Mutex::Mutex() {
    pthread_mutex_init(&mMutex, NULL);
}

inline Mutex::~Mutex() {
    pthread_mutex_destroy(&mMutex);
}

inline int Mutex::lock() {
    return -pthread_mutex_lock(&mMutex);
}

inline void Mutex::unlock() {
    pthread_mutex_unlock(&mMutex);
}

typedef Mutex::Autolock AutoMutex;

} // namespace DisplayControl

class DisplayControlDecoder
{
public:

    static DisplayControlDecoder *getInstance();
    FBPost_proc_t fbPost;
    postLayer_proc_t postLayer;
    postAllLayersDone_proc_t postAllLayersDone;
    getDisplayWidth_proc_t getDisplayWidth;
    getDisplayHeight_proc_t getDisplayHeight;
    getDisplayFPS_proc_t getDisplayFPS;
    getDisplayXDpi_proc_t getDisplayXDpi;
    getDisplayYDpi_proc_t getDisplayYDpi;
    postVirtualDisplay_proc_t postVirtualDisplay;
    postAllVirtualDisplaysDone_proc_t postAllVirtualDisplaysDone;
    postBoForScreenRecord_proc_t postBoForScreenRecord;
    postCaptureScreenColorBuffer_proc_t postCaptureScreenColorBuffer;
    authenticateMagic_proc_t authenticateMagic;
    updateSharingScreenDisplay_proc_t updateSharingScreenDisplay;

private:
    DisplayControlDecoder() {
        fbPost = NULL;
        postLayer = NULL;
        postAllLayersDone = NULL;
        getDisplayWidth = NULL;
        getDisplayHeight = NULL;
        getDisplayFPS = NULL;
        getDisplayXDpi = NULL;
        getDisplayYDpi = NULL;
        postVirtualDisplay = NULL;
        postAllVirtualDisplaysDone = NULL;
        postBoForScreenRecord = NULL;
        postCaptureScreenColorBuffer = NULL;
        authenticateMagic = NULL;
        updateSharingScreenDisplay = NULL;
    }
    ~DisplayControlDecoder() {}

    static DisplayControlDecoder *m_pInstance;


    class Garbo
    {
    public:
        ~Garbo()
        {
            if(DisplayControlDecoder::m_pInstance)
                delete m_pInstance;
        }
    };

    static Garbo garbo;
    static DisplayControl::Mutex lock;

};

#endif
