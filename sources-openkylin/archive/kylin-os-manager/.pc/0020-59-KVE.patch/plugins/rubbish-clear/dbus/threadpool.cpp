/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "threadpool.h"
#include <QDebug>
namespace KylinRubbishClear
{

ThreadPool::ThreadPool(QObject *parent) : QObject(parent) {}

ThreadPool::~ThreadPool()
{
    exitAllThreads();
}

ThreadPool *ThreadPool::Instance()
{
    static ThreadPool threadPool;
    return &threadPool;
}

QThread *ThreadPool::createNewThread()
{
    QThread *thread = new QThread;
    m_threadPool.push_back(thread);
    return thread;
}

void ThreadPool::moveObjectToThread(QObject *obj)
{
    QThread *work = createNewThread();
    obj->moveToThread(work);
    work->start();
}

void ThreadPool::exitAllThreads()
{
    for (QThread *thread: m_threadPool) {
        thread->quit();
        thread->wait(2000);
    }
}
} // namespace KylinRubbishClear
