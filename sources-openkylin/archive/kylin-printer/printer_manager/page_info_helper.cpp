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
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include "common.h"
#include "page_info_helper.h"
#include <QDebug>

PageInfoHelper::PageInfoHelper(const QString &uri, QObject *parent)
    : QObject(parent),
      m_uri(uri)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &PageInfoHelper::dowork);
    connect(this, &PageInfoHelper::finished, m_thread, &QThread::quit);
    connect(this, &PageInfoHelper::finished, this, &PageInfoHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

PageInfoHelper::~PageInfoHelper()
{

}

int extract_numbers(const QString &str)
{
    QRegularExpression re("\\b\\d+\\.?\\d*\\b");
    QRegularExpressionMatchIterator i = re.globalMatch(str);
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString numStr = match.captured();
        return numStr.toInt();
    }
    return -1;
}

void PageInfoHelper::getPageInfo()
{
    if (!m_uri.contains("ipp://") && !m_uri.contains("ipps://")
     && !m_uri.contains("http://") && !m_uri.contains("https://")) {
        return ;
     }
    QStringList command;
    command << "ipptool" << "-t" << "-v" << m_uri << "/usr/share/cups/ipptool/get-jobs.test";
    QString res;
    int msec = 1000;
    if (!execCommand(command, &res, nullptr, nullptr, &msec)) {
        return ;
    }
    auto list = res.split('\n');
    int jobId = -1;
    int jobImpressionsCompleted = -1;
    for (auto str : list) {
        str = str.trimmed();
        if (str.contains("job-id (integer) = ")) {
            jobId = extract_numbers(str);
        }

        if (str.contains("job-impressions-completed (integer)")) {
            jobImpressionsCompleted = extract_numbers(str);
        }
    }

    if (jobId == -1 || jobImpressionsCompleted == -1)
        return ;
    
    if (jobId == m_jobId && jobImpressionsCompleted == m_jobImpressionsCompleted) {
        return ;
    }
    m_jobId = jobId;
    m_jobImpressionsCompleted = jobImpressionsCompleted;
    emit pageInfoUpdate(m_jobId, m_jobImpressionsCompleted);
}

void PageInfoHelper::dowork()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PageInfoHelper::getPageInfo);
    getPageInfo();
    m_timer->start(2000);
}
