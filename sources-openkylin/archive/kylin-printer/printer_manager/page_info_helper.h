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
#ifndef PAGE_INFO_HELPER_H
#define PAGE_INFO_HELPER_H

#include <QString>
#include <QThread>
#include <QTimer>

class PageInfoHelper : public QObject
{
    Q_OBJECT
public:
    PageInfoHelper(const QString &uri, QObject *parent = nullptr);
    ~PageInfoHelper();
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    QString m_uri{QString()};
    QThread *m_thread{nullptr};
    QTimer *m_timer;
    int m_jobId{-1};
    int m_jobImpressionsCompleted{-1};
    void dowork();
    void getPageInfo();
Q_SIGNALS:
    void finished();
    void pageInfoUpdate(int jobId, int jobImpressionsCompleted);
};

#endif // PAGE_INFO_HELPER_H