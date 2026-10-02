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
#ifndef UPLOAD_HELPER_H
#define UPLOAD_HELPER_H

#include <QTimer>
#include "device_information.h"
#include <QThread>

class UploadHelper : public QObject
{
    Q_OBJECT
public:
    UploadHelper(DeviceInformation info, int ppdRank, QObject *parent = nullptr);
    ~UploadHelper();
Q_SIGNALS:
    void finished();
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    void dowork();
    QThread *m_thread{nullptr};
    DeviceInformation m_info;
    int m_ppdRank;
};

#endif//UPLOAD_HELPER_H
