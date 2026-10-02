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
#ifndef SINGLE_THREAD_WORKER_H
#define SINGLE_THREAD_WORKER_H

#include <QTimer>
#include <QThread>

class SingleThreadWorker : public QObject
{
    Q_OBJECT
public:
    SingleThreadWorker();
    ~SingleThreadWorker();
    void startWorker();
Q_SIGNALS:
    void finished();
protected:
    virtual void dowork() = 0;
private:
    QThread *m_thread{nullptr};
};

#endif//SINGLE_THREAD_WORKER_H
