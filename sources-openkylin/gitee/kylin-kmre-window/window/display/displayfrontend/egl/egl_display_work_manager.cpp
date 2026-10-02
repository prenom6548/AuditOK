/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
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

#include "egl_display_work_manager.h"
#include "egl_display_helper.h"

#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

EglDisplayWorkManager::EglDisplayWorkManager(DisplayWidget *widget, QObject *parent)
    : DisplayWorkManager(widget, parent)
{
    initialize();
}

EglDisplayWorkManager::~EglDisplayWorkManager()
{
    exitRender();
    mWorkerThread->quit();
    mWorkerThread->wait();
}

void EglDisplayWorkManager::initialize()
{
    mDisplayHelper = new EglDisplayHelper(mWidget);
    connect(mDisplayHelper, SIGNAL(orientationChanged(int, int)), this, SIGNAL(orientationChanged(int, int)));
    connect(this, SIGNAL(exitRender()), mDisplayHelper, SLOT(destroyDisplay()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(update(int, int, int, int, int, int)),
        mDisplayHelper, SLOT(update(int, int, int, int, int, int)), Qt::QueuedConnection);
    connect(this, SIGNAL(redraw()), mDisplayHelper, SLOT(redraw()), Qt::QueuedConnection);

    mWorkerThread = new QThread();
    mDisplayHelper->moveToThread(mWorkerThread);
    connect(mWorkerThread, &QThread::finished, mDisplayHelper, &EglDisplayHelper::deleteLater);
    connect(mWorkerThread, &QThread::finished, mWorkerThread, &QThread::deleteLater);
    mWorkerThread->start();
}

void EglDisplayWorkManager::enableDisplayUpdate(bool enable)
{
    mDisplayHelper->enableUpdate(enable);
}

EglDisplayHelper* EglDisplayWorkManager::getDisplayHelper()
{
    return static_cast<EglDisplayHelper*>(mDisplayHelper);
}

void EglDisplayWorkManager::blurUpdate(int msecond)
{
    mDisplayHelper->enableBlur(true);

    QTimer::singleShot(msecond, [&] {
        mDisplayHelper->enableBlur(false);
        emit redraw();
    });

    emit redraw();
}
