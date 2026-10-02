/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "search-vfs-uri-parser.h"
#include "file-utils.h"
#include "file-info-job.h"
#include "file-info.h"

#include <QStringList>
#include <QRegularExpression>
#include <QDebug>
using namespace Peony;

SearchVFSUriParser::SearchVFSUriParser()
{

}

const QString SearchVFSUriParser:: parseSearchKey(const QString &uri, const QString &key, const bool &search_file_name,
        const bool &search_content, const QString &extend_key, const bool &recursive)
{
    //When searching the computer directory, switch to search under the root directory. fix bug97220
    QString tmp = uri;
    if(uri.startsWith("computer:///")){
        tmp = "file:///";
    }
    QString search_str = "search:///search_uris="+tmp;
    if (search_file_name)
        search_str += "&name_regexp="+key;
    if (search_content)
        search_str += "&content_regexp="+key;
    //multiple name key search
    if (extend_key != "")
        search_str += "&extend_regexp="+extend_key;
    else if (! search_file_name && ! search_content && extend_key=="")
    {
        qWarning()<<"Search content or file name at least one be true!";
        //use default search file name
        search_str += "&name_regexp="+key;
    }

    if (recursive)
        return QString(search_str+"&recursive=1");

    return QString(search_str+"&recursive=0");
}

const QString SearchVFSUriParser::getSearchUriNameRegexp(const QString &searchUri)
{
    auto string = searchUri;
    string.remove("search:///");
    auto list = string.split("&", QString::SkipEmptyParts);
    if (checkForSpecialInStringLists(list) || string.contains("&&")) {
        list = handleAmpersandSplitStringList(&string, list);
    }
    qDebug() << "list length:" <<list.length();
    QString ret = "";
    for (auto arg : list) {
        if (arg.startsWith("name_regexp=")) {
            qDebug()<<"name_regexp: "<<arg;
            auto tmp = arg.remove("name_regexp=");
            if (ret == "")
                ret = tmp;
            else
                ret += "," + tmp;
        }

        if (arg.startsWith("extend_regexp"))
        {
            qDebug()<<"extend_regexp: "<<arg;
            auto tmp = arg.remove("extend_regexp=");
            if (ret == "")
                ret = tmp;
            else
                ret += "," + tmp;
        }
    }
    return ret;
}

const QString SearchVFSUriParser::getSearchUriTargetDirectory(const QString &searchUri)
{
    auto string = searchUri;
    string.remove("search:///");
    QStringList list = string.split("&");
    if (Peony::SearchVFSUriParser::checkForSpecialInStringLists(list)) {
        list = Peony::SearchVFSUriParser::handleAmpersandSplitStringList(&string, list);
    }else{
        list = string.split("&", QString::SkipEmptyParts);
    }
    for (auto arg : list) {
        if (arg.startsWith("search_uris=")) {
            qDebug()<<"arg:"<<arg;
            auto tmp = arg.remove("search_uris=");
            auto uris = tmp.split(",");
            if (uris.count() == 1) {
                //FIXME: replace BLOCKING api in ui thread.
                //When searching the computer directory, switch to search under the root directory. fix bug97220
                auto displayName = FileUtils::getFileDisplayName(tmp);
                if(displayName.isEmpty()){
                    auto info = Peony::FileInfo::fromUri(tmp);
                    if (info.get()->isEmptyInfo()) {
                        Peony::FileInfoJob j(info);
                        j.querySync();
                    }
                    displayName = FileUtils::getFileDisplayName(tmp);
                    displayName = displayName.isEmpty()? QObject::tr("Computer") : displayName;
                }
                return displayName;
            }
            tmp = nullptr;
            for (auto uri : uris) {
                //FIXME: replace BLOCKING api in ui thread.
                auto displayName = FileUtils::getFileDisplayName(uri);
                if (tmp == nullptr)
                    tmp = displayName;
                else
                    tmp = tmp + ", " + displayName;
            }
            return tmp;
        }
    }
    return nullptr;
}

const QString SearchVFSUriParser::addSearchKey(const QString &uri, const bool &search_engine)
{
    QString search_str = uri;
    if (search_engine) {
        search_str += "&search_engine=1";
    } else {
        search_str += "&search_engine=0";
    }
    return search_str;
}

const QString SearchVFSUriParser::getSearchUriPath(const QString &searchUri)
{
    auto string = searchUri;
    string.remove("search:///");
    auto list = string.split("&");
    if (checkForSpecialInStringLists(list)) {
        list = handleAmpersandSplitStringList(&string, list);
    }
    for (auto arg : list) {
        if (arg.startsWith("search_uris=")) {
            qDebug()<<"arg:"<<arg;
            auto tmp = arg.remove("search_uris=");
            return tmp;
        }
    }
    return QString();
}

bool SearchVFSUriParser::checkForSpecialInStringLists(const QStringList &args)
{
    bool isSpecial = false;
    for (auto arg : args) {
        if (arg == "name_regexp=" || !arg.contains("=")) {
            isSpecial = true;
            break;
        }
    }
    return isSpecial;
}

QStringList SearchVFSUriParser::handleAmpersandSplitStringList(const QString *uri, const QStringList &args)
{
    QStringList lists = args;
    if (args.size() <= 1 || uri->contains(QRegularExpression("search_hidden=|use_regexp=|case_sensitive=|extend_regexp=|save="))) {
        return lists;
    }

    // 解析 search_uris 参数
    if (uri->contains("search_uris=")) {
        QString searchUris = extractParam(*uri, "search_uris=", "&name_regexp=");
        if (!searchUris.isEmpty()) {
            lists.replace(0, "search_uris=" + searchUris);
        }
    }

    // 解析 name_regexp 参数
    if (uri->contains("name_regexp=")) {
        QString nameRegExp = extractParam(*uri, "name_regexp=", "&recursive=");
        if (!nameRegExp.isEmpty()) {
            for (int i = 0; i < args.size(); ++i) {
                if (args[i].startsWith("name_regexp=")) {
                    lists.replace(i, "name_regexp=" + nameRegExp);
                    break;
                }
            }
        }
    }

    return lists;
}

QString SearchVFSUriParser::extractParam(const QString &source, const QString &key, const QString &delimiter)
{
    // 检查 key 的起始位置
    int startIndex = source.indexOf(key);
    if (startIndex == -1) {
        return QString();
    }

    // 从起始位置后开始查找分隔符
    startIndex += key.length();
    int endIndex = source.indexOf(delimiter, startIndex);
    if (endIndex == -1) {
        return QString();
    }

    return source.mid(startIndex, endIndex - startIndex);
}
