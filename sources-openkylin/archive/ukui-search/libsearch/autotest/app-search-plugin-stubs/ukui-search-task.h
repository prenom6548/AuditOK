#ifndef UKUI_SEARCH_TASK_STUB_H
#define UKUI_SEARCH_TASK_STUB_H

#include <QObject>

#include "plugininterface/data-queue.h"
#include "searchinterface/result-item.h"
#include "searchinterface/search-result-property.h"

namespace UkuiSearch {

class UkuiSearchTask : public QObject
{
public:
    explicit UkuiSearchTask(QObject *parent = nullptr) : QObject(parent) {}

    DataQueue<ResultItem> *init()
    {
        return &m_queue;
    }

    void clearKeyWords()
    {
        m_keywords.clear();
    }

    void addKeyword(const QString &keyword)
    {
        m_keywords << keyword;
    }

    void initSearchPlugin(SearchProperty::SearchType searchType, const QString &customSearchType = QString())
    {
        Q_UNUSED(searchType)
        Q_UNUSED(customSearchType)
    }

    void setSearchOnlineApps(bool searchOnlineApps)
    {
        m_searchOnlineApps = searchOnlineApps;
    }

    bool setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties)
    {
        Q_UNUSED(searchType)
        m_resultProperties = searchResultProperties;
        return true;
    }

    size_t startSearch(SearchProperty::SearchType searchType, QString customSearchType = QString())
    {
        Q_UNUSED(searchType)
        Q_UNUSED(customSearchType)
        return ++m_searchId;
    }

    void stop()
    {
        m_stopped = true;
    }

    DataQueue<ResultItem> m_queue;
    QStringList m_keywords;
    SearchResultProperties m_resultProperties;
    bool m_searchOnlineApps = false;
    bool m_stopped = false;
    size_t m_searchId = 0;
};

}

#endif
