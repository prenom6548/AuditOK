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

#include "ailabelhelper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QDebug>

#include <string>

#include <unistd.h>

AILabelHelper::AILabelHelper(QObject *parent)
    : QObject{parent}
{
#ifdef USE_AI
    session = data_management_create_session();
    if (session) {
        inited = true;
    }
#endif
}

AILabelHelper::~AILabelHelper()
{
#ifdef USE_AI
    data_management_destroy_session(session);
#endif
}

bool AILabelHelper::isValid()
{
    return inited;
}

QStringList AILabelHelper::getTopLabels(int maxCount, bool *ok)
{
    QStringList list;
#ifdef USE_AI
    if (!inited) {
        setOkResult(ok, false);
        return list;
    }

    char *results = nullptr;
    auto ret = data_management_get_all_tags(session, &results);
    if (!ret.success) {
        setOkResult(ok, false);
        return list;
    }
    QJsonDocument doc = QJsonDocument::fromJson(results);
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QJsonValue tagsValue = obj.value("tags");
        if (tagsValue.isArray()) {
            QJsonArray tagsArray = tagsValue.toArray();
            for (auto tag : tagsArray) {
                list << tag.toString();
                if (maxCount > 0 && list.count() == maxCount)
                    break;
            }
        }
    } else if (doc.isArray()) {
        QJsonArray tagsArray = doc.array();
        for (auto tag : tagsArray) {
            list << tag.toString();
            if (maxCount > 0 && list.count() == maxCount)
                break;
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

QStringList AILabelHelper::getSimilarLabels(const QString &label, bool *ok)
{
    QStringList list;
#ifdef USE_AI
    if (!inited) {
        setOkResult(ok, false);
        return list;
    }

    QJsonObject obj;
    QStringList labels;
    labels << label;
    obj["tags"] = QJsonArray::fromStringList(labels);
    QJsonDocument tagsDoc(obj);
    char *results = nullptr;
    auto ret = data_management_search_similar_tags(session, tagsDoc.toJson().constData(), &results);
    if (!ret.success) {
        setOkResult(ok, false);
        return list;
    }

    QJsonDocument doc = QJsonDocument::fromJson(results);
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QJsonValue tagsValue = obj.value("tags");
        if (tagsValue.isArray()) {
            QJsonArray tagsArray = tagsValue.toArray();
            for (auto tag : tagsArray) {
                list << tag.toString();
            }
        }
    } else if (doc.isArray()) {
        QJsonArray tagsArray = doc.array();
        for (auto tag : tagsArray) {
            list << tag.toString();
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

QStringList AILabelHelper::getFileLabels(const QString &filePath, bool *ok)
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

QStringList AILabelHelper::getFilesByLabels(const QStringList &labels, bool *ok)
{
    QStringList list;
#ifdef USE_AI
    if (!inited) {
        setOkResult(ok, false);
        return list;
    }

    qDebug() << labels;

    QJsonArray tagsArray = QJsonArray::fromStringList(labels);
    QJsonObject obj;
    obj["tags"] = tagsArray;
    QJsonDocument doc(obj);
    char *results = nullptr;
    qDebug() << doc;
    auto ret = data_management_search_files_about_tags(session, doc.toJson().constData(), &results);
    if (!ret.success) {
        setOkResult(ok, false);
        return list;
    }
    doc = QJsonDocument::fromJson(results);
    if (doc.isObject()) {
        obj = doc.object();
        QJsonValue value = obj["files"];
        if (value.isArray()) {
            QJsonArray filesArray = value.toArray();
            for (auto file : filesArray) {
                list << file.toString();
            }
        }
    } else if (doc.isArray()) {
        QJsonArray filesArray = doc.array();
        for (auto file : filesArray) {
            list << file.toString();
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

void AILabelHelper::setOkResult(bool *ok, bool result)
{
    if (ok) {
        *ok = result;
    }
}
