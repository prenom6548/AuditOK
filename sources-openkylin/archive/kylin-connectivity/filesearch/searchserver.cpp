#include "searchserver.h"
#include <QDebug>
#include <QMutexLocker>
#include "searchdatabase.h"

SearchServer::SearchServer(const QString &path, QObject *parent) : QObject(parent)
{
    m_searchDbPath = path;
    SearchDataBase::getInstance();
    connect(this, &SearchServer::sigInitSearch, this, &SearchServer::slotInitSearch);
    connect(this, &SearchServer::sigSearchFile, this, &SearchServer::slotSearchFile);
}

SearchServer::~SearchServer() {}

void SearchServer::slotInitSearch()
{
    QMutexLocker locker(&m_searchMutex);
    int ret = SearchDataBase::getInstance()->initDataBase(m_searchDbPath);
    if (ret < 0) {
        qWarning() << "load search server fail " << ret;
        Q_EMIT sigLoadFail();
        return;
    }
    ret = SearchDataBase::getInstance()->creatVirtualDBTable();
    if (ret < 0) {
        qWarning() << "search server init fail" << ret;
        Q_EMIT sigLoadFail();
        return;
    }
}

void SearchServer::slotSearchFile(const QString &keyword)
{
    QMutexLocker locker(&m_searchMutex);
    m_resultMap.clear();
    SearchDataBase::getInstance()->searchFromDB(m_resultMap, keyword);
    Q_EMIT sigSearchResult(m_resultMap);
}