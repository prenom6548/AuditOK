/*
 *
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
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "search-result-manager.h"

using namespace UkuiSearch;
SearchResultManager::SearchResultManager(const QString& plugin_id, QObject *parent) : QObject(parent)
{
    m_pluginId = plugin_id;
    m_resultQueue = new DataQueue<SearchPluginIface::ResultInfo>;
    m_getResultThread = new ReceiveResultThread(m_resultQueue, this);
    initConnections();
}

SearchResultManager::~SearchResultManager()
{
    if (m_getResultThread) {
        m_getResultThread->stop();
    }
}

size_t SearchResultManager::startSearch(const QString &keyword)
{
    qDebug()<<m_pluginId<<"started";
    m_getResultThread->startConsume();
    m_resultQueue->clear();
    SearchPluginIface *plugin = SearchPluginManager::getInstance()->getPlugin(m_pluginId);
    return plugin->KeywordSearch(keyword, m_resultQueue);
}

/**
 * @brief SearchResultManager::stopSearch 停止搜索，开始一次新搜索前或主界面退出时执行
 */
void SearchResultManager::stopSearch()
{
    m_getResultThread->stop();
    SearchPluginIface *plugin = SearchPluginManager::getInstance()->getPlugin(m_pluginId);
    plugin->stopSearch();
    qDebug() << m_pluginId << "stopped";
    m_resultQueue->clear();
}

void SearchResultManager::initConnections()
{
    connect(m_getResultThread, &ReceiveResultThread::gotResultInfo, this, &SearchResultManager::gotResultInfo);
}

ReceiveResultThread::ReceiveResultThread(DataQueue<SearchPluginIface::ResultInfo> * resultQueue, QObject *parent): QThread(parent)
{
    m_resultQueue = resultQueue;
    m_pollTimer = new QTimer;
    m_timeoutTimer = new QTimer;

    m_pollTimer->setInterval(50);
    m_timeoutTimer->setInterval(25000);
    m_timeoutTimer->setSingleShot(true);

    m_pollTimer->moveToThread(this);
    m_timeoutTimer->moveToThread(this);

    connect(this, SIGNAL(timerStart()), m_pollTimer, SLOT(start()));
    connect(this, SIGNAL(timerStart()), m_timeoutTimer, SLOT(start()));
    connect(this, &ReceiveResultThread::timerStop, m_pollTimer, &QTimer::stop);
    connect(this, &ReceiveResultThread::timerStop, m_timeoutTimer, &QTimer::stop);

    connect(m_pollTimer, &QTimer::timeout, this, &ReceiveResultThread::processQueue, Qt::DirectConnection);
    connect(m_timeoutTimer, &QTimer::timeout, this, [ this ] {
        qWarning() << "receive timeout result";
        Q_EMIT timerStop();
        m_resultQueue->clear();
    }, Qt::DirectConnection);
}

ReceiveResultThread::~ReceiveResultThread()
{
    stop();
    if (isRunning()) {
        quit();
        wait();
    }

    if (m_pollTimer) {
        delete m_pollTimer;
    }
    m_pollTimer = nullptr;

    if (m_timeoutTimer) {
        delete m_timeoutTimer;
    }
    m_timeoutTimer = nullptr;
}

void ReceiveResultThread::startConsume()
{
    if (!isRunning()) {
        start();
    }
    Q_EMIT timerStart();
}

void ReceiveResultThread::stop()
{
    Q_EMIT timerStop();
    m_resultQueue->clear();
}

void ReceiveResultThread::run()
{
    exec();
}

void ReceiveResultThread::processQueue()
{
    constexpr int maxProcessBatch = 100;
    int processed = 0;
    while (processed < maxProcessBatch) {
        SearchPluginIface::ResultInfo oneResult = m_resultQueue->tryDequeue();
        if (oneResult.name.isEmpty()) {
            break;
        }
        m_timeoutTimer->start();
        Q_EMIT gotResultInfo(oneResult);
        ++processed;
    }
}
