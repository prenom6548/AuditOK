/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "aipreviewhelper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <string>

#include <unistd.h>
#include "model/modelcontroller.h"
AIPreviewHelper::AIPreviewHelper(QObject *parent)
    : QObject{parent}
{
#ifdef USE_AI
    session = data_management_create_session();
    if (session) {
        inited = true;
    }
#endif
}

AIPreviewHelper::~AIPreviewHelper()
{
#ifdef USE_AI
    data_management_destroy_session(session);
#endif
}

bool AIPreviewHelper::isValid()
{
    return inited;
}

QStringList AIPreviewHelper::getFileLabels(const QString &filePath, bool *ok)
{
    QStringList list;
#ifdef USE_AI
    if (!inited) {
        setOkResult(ok, false);
        return list;
    }

    QJsonObject obj;
    QStringList files;
    files << filePath;
    obj["files"] = QJsonArray::fromStringList(files);
    QJsonDocument doc(obj);
    char *results = nullptr;
    auto ret = data_management_get_tags_of_files(session, doc.toJson().constData(), &results);
    if (!ret.success) {
        setOkResult(ok, false);
        return list;
    }

    doc = QJsonDocument::fromJson(results);
    if (doc.isArray()) {
        QJsonArray resultsArray = doc.array();
        for (auto result : resultsArray) {
            if (result.isObject()) {
                QJsonObject obj = result.toObject();
                QJsonValue tagsValue = obj.value("tags");
                if (tagsValue.isArray()) {
                    QJsonArray tagsArray = tagsValue.toArray();
                    for (auto tag : tagsArray) {
                        list << tag.toString();
                    }
                }
                break;
            }
        }
    }

    data_management_destroy_result(results);

    setOkResult(ok, true);

    return list;
#else
    setOkResult(ok, false);
    return list;
#endif
}

QString AIPreviewHelper::getFileSummary(const QString &filePath, bool *ok)
{
    QString summary;
#ifdef USE_AI
    if (!inited) {
        setOkResult(ok, false);
        return summary;
    }

    QJsonObject obj;
    QStringList files;
    files << filePath;
    obj["files"] = QJsonArray::fromStringList(files);
    QJsonDocument doc(obj);
    char *results = nullptr;
    auto ret = data_management_get_summarys_of_files(session, doc.toJson().constData(), &results);
    if (!ret.success) {
        setOkResult(ok, false);
        return summary;
    }

    doc = QJsonDocument::fromJson(results);
    if (doc.isArray()) {
        QJsonArray resultsArray = doc.array();
        for (auto result : resultsArray) {
            if (result.isObject()) {
                QJsonObject obj = result.toObject();
                summary = obj["summary"].toString();
                break;
            }
        }
    }

    data_management_destroy_result(results);

    setOkResult(ok, true);

    return summary;
#else
    setOkResult(ok, false);
    return summary;
#endif
}

void AIPreviewHelper::setOkResult(bool *ok, bool result)
{
    if (ok) {
        *ok = result;
    }
}
