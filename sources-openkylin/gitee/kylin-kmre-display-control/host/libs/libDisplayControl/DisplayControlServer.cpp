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
#include "DisplayControlServer.h"

#include "DisplayControlThread.h"
#include "UnixStream.h"
#include <signal.h>
#include <pthread.h>

#include <map>

#include <string.h>

typedef std::map<DisplayControlThread*, kmre::DisplayControl::SocketStream*> DisplayControlThreadsMap;

DisplayControlServer::DisplayControlServer() :
    m_lock(),
    m_listenSock(NULL),
    m_exiting(false)
{
}

DisplayControlServer::~DisplayControlServer()
{
    delete m_listenSock;
}



DisplayControlServer *DisplayControlServer::create(const char* addr)
{
    if(!addr || (strlen(addr) == 0))
    {
        return NULL;
    }

    DisplayControlServer *server = new DisplayControlServer();
    if (!server) {
        return NULL;
    }

    server->m_listenSock = new kmre::DisplayControl::UnixStream();


    if (server->m_listenSock->listen(addr) < 0) {
        delete server;
        return NULL;
    }

    return server;
}

intptr_t DisplayControlServer::main()
{
    DisplayControlThreadsMap threads;

    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);

    while(1) {
        kmre::DisplayControl::SocketStream *stream = m_listenSock->accept();
        if (!stream) {
            fprintf(stderr,"Error accepting gles connection, ignoring.\n");
            continue;
        }
        unsigned int clientFlags;
        if (!stream->readFully(&clientFlags, sizeof(unsigned int))) {
            fprintf(stderr,"Error reading clientFlags\n");
            delete stream;
            continue;
        }

                

        // check if we have been requested to exit while waiting on accept
        if ((clientFlags & IOSTREAM_CLIENT_EXIT_SERVER) != 0) {
            m_exiting = true;
            delete stream;
            break;
        }


        DisplayControlThread *rt = DisplayControlThread::create(stream, &m_lock);
        if (!rt) {
            fprintf(stderr,"Failed to create DisplayControlThread\n");
            delete stream;
        } else if (!rt->start()) {
            fprintf(stderr,"Failed to start DisplayControlThread\n");
            delete rt;
            delete stream;
        }

        //
        // remove from the threads list threads which are
        // no longer running
        //
        for (DisplayControlThreadsMap::iterator n,t = threads.begin();
             t != threads.end();
             t = n) {
            // first find next iterator
            n = t;
            n++;

            // delete and erase the current iterator
            // if thread is no longer running
            if (t->first->isFinished()) {
                delete t->second;
                delete t->first;
                threads.erase(t);
            }
        }

        // if the thread has been created and started, insert it to the list
        if (rt) {
            threads.insert({rt, stream});
        }
    }

    //
    // Wait for all threads to finish
    //
    for (DisplayControlThreadsMap::iterator t = threads.begin();
         t != threads.end();
         t++) {
        t->first->forceStop();
        t->first->wait(NULL);
        delete t->second;
        delete t->first;
    }
    threads.clear();

    return 0;
}
