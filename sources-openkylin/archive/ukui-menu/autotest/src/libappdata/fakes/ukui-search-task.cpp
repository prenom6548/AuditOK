/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include "UkuiSearchTask"

namespace UkuiSearch {
namespace FakeSearchStore {

QStringList g_resultIds;
QString g_lastKeyword;
bool g_stopCalled = false;

void reset()
{
    g_resultIds.clear();
    g_lastKeyword.clear();
    g_stopCalled = false;
}

}

UkuiSearchTask::UkuiSearchTask(QObject *parent)
    : QObject(parent)
{
}

UkuiSearchTask::~UkuiSearchTask() = default;

DataQueue<ResultItem> *UkuiSearchTask::init()
{
    return &m_queue;
}

void UkuiSearchTask::initSearchPlugin(SearchProperty::SearchType searchType, const QString &customSearchType)
{
    Q_UNUSED(searchType);
    Q_UNUSED(customSearchType);
}

bool UkuiSearchTask::setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties)
{
    Q_UNUSED(searchType);
    Q_UNUSED(searchResultProperties);
    return true;
}

void UkuiSearchTask::setSearchOnlineApps(bool searchOnlineApps)
{
    Q_UNUSED(searchOnlineApps);
}

void UkuiSearchTask::clearKeyWords()
{
    FakeSearchStore::g_lastKeyword.clear();
}

void UkuiSearchTask::addKeyword(const QString &keyword)
{
    FakeSearchStore::g_lastKeyword = keyword;
}

size_t UkuiSearchTask::startSearch(SearchProperty::SearchType searchtype, QString customSearchType)
{
    Q_UNUSED(searchtype);
    Q_UNUSED(customSearchType);

    const size_t searchId = 1;
    for (const QString &id : FakeSearchStore::g_resultIds) {
        ResultItem item(searchId);
        item.setItemKey(id);
        item.setValue(SearchProperty::ApplicationDesktopPath, id);
        m_queue.enqueue(item);
    }
    return searchId;
}

void UkuiSearchTask::stop()
{
    FakeSearchStore::g_stopCalled = true;
}

}
