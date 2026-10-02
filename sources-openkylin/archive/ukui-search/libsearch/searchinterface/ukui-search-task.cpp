/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "ukui-search-task.h"
#include "search-task-plugin-manager.h"
#include <QDebug>
using namespace UkuiSearch;
namespace UkuiSearch {

class UkuiSearchTaskPrivate
{
    friend class UkuiSearchTask;
private:
    SearchController m_searchCotroller;
    size_t m_searchId = 0;
    QUuid m_uuid;
};
}

UkuiSearchTask::UkuiSearchTask(QObject *parent) : QObject(parent), d(new UkuiSearchTaskPrivate())
{
    d->m_uuid = QUuid::createUuid();
}

UkuiSearchTask::~UkuiSearchTask()
{
    stop();
    SearchTaskPluginManager::getInstance()->destroyPlugins(d->m_uuid);
}

DataQueue<ResultItem> *UkuiSearchTask::init()
{
    return d->m_searchCotroller.initDataQueue();
}

void UkuiSearchTask::addSearchDir(const QString &path)
{
    d->m_searchCotroller.addSearchDir(path);
}

void UkuiSearchTask::setRecurse(bool recurse)
{
    d->m_searchCotroller.setRecurse(recurse);
}

void UkuiSearchTask::addKeyword(const QString &keyword)
{
    d->m_searchCotroller.addKeyword(keyword);
}

void UkuiSearchTask::setMatchAllIfNoKeyword(bool matchAll)
{
    d->m_searchCotroller.setMatchAllIfNoKeyword(matchAll);
}

void UkuiSearchTask::addFileLabel(const QString &label)
{
    d->m_searchCotroller.addFileLabel(label);
}

void UkuiSearchTask::setOnlySearchFile(bool onlySearchFile)
{
    d->m_searchCotroller.setOnlySearchFile(onlySearchFile);
}

void UkuiSearchTask::setOnlySearchDir(bool onlySearchDir)
{
    d->m_searchCotroller.setOnlySearchDir(onlySearchDir);
}

void UkuiSearchTask::setSearchOnlineApps(bool searchOnlineApps)
{
    d->m_searchCotroller.setSearchOnlineApps(searchOnlineApps);
}

void UkuiSearchTask::setSearchHiddenFiles(bool searchHiddenFiles)
{
    d->m_searchCotroller.setSearchHiddenFiles(searchHiddenFiles);
}

void UkuiSearchTask::initSearchPlugin(SearchProperty::SearchType searchType, const QString &customSearchType)
{
    SearchTaskPluginIface *plugin = SearchTaskPluginManager::getInstance()->initPlugins(d->m_uuid, searchType, customSearchType);
    if (plugin) {
        plugin->setController(d->m_searchCotroller);
        connect(plugin, &SearchTaskPluginIface::searchFinished,this, &UkuiSearchTask::searchFinished);
        connect(plugin, &SearchTaskPluginIface::searchError,this, &UkuiSearchTask::searchError);
        connect(plugin, &SearchTaskPluginIface::reachInformNum,this, &UkuiSearchTask::reachInformNum);
    } else {
        qWarning() << "The plugin has been initialized or the plugin failed to load.";
    }
}

bool UkuiSearchTask::setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties)
{
    return d->m_searchCotroller.setResultProperties(searchType, searchResultProperties);
}

void UkuiSearchTask::setCustomResultDataType(QString customSearchType, QStringList dataType)
{
    return d->m_searchCotroller.setCustomResultDataType(customSearchType, dataType);
}

size_t UkuiSearchTask::startSearch(SearchProperty::SearchType searchtype, QString customSearchType)
{
    d->m_searchId = d->m_searchCotroller.refreshSearchId();
    if(d->m_searchCotroller.getDataQueue() == nullptr) {
        qWarning() << "the date queue has not been initialized, you need run init first!";
    }

    d->m_searchCotroller.refreshDataqueue();
    //plugin manager do async search here
    if (!SearchTaskPluginManager::getInstance()->startSearch(d->m_uuid, searchtype, customSearchType)) {
        Q_EMIT searchError(d->m_searchCotroller.getCurrentSearchId(), tr("Current task uuid error or an unregistered plugin is used!"));
    }

    return d->m_searchId;
}

void UkuiSearchTask::stop()
{
    d->m_searchCotroller.stop();
}

bool UkuiSearchTask::isSearching(SearchProperty::SearchType searchtype, QString customSearchType)
{
    return SearchTaskPluginManager::getInstance()->isSearching(d->m_uuid, searchtype, customSearchType);
}

void UkuiSearchTask::clearAllConditions()
{
    d->m_searchCotroller.clearAllConditions();
}

void UkuiSearchTask::clearKeyWords()
{
    d->m_searchCotroller.clearKeyWords();
}

void UkuiSearchTask::clearSearchDir()
{
    d->m_searchCotroller.clearSearchDir();
}

void UkuiSearchTask::clearFileLabel()
{
    d->m_searchCotroller.clearFileLabel();
}

void UkuiSearchTask::setMaxResultNum(unsigned int maxResults)
{
    d->m_searchCotroller.setMaxResultNum(maxResults);
}

void UkuiSearchTask::setInformNum(int num)
{
    d->m_searchCotroller.setInformNum(num);
}
