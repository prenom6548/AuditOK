/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "single_thread_worker.h"

SingleThreadWorker::SingleThreadWorker()
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &SingleThreadWorker::dowork);

    connect(this, &SingleThreadWorker::finished, m_thread, &QThread::quit);
    connect(this, &SingleThreadWorker::finished, this, &SingleThreadWorker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

void SingleThreadWorker::startWorker()
{
    if(m_thread!=nullptr)
        m_thread->start();
}


SingleThreadWorker::~SingleThreadWorker()
{

}
