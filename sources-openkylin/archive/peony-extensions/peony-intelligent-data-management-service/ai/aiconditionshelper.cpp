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

#include "aiconditionshelper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <unistd.h>

AIConditionsHelper::AIConditionsHelper(QObject *parent)
    : QObject{parent}
{
#ifdef USE_AI
    session = data_management_create_session();
    if (session) {
        inited = true;
    }
#endif
}

bool AIConditionsHelper::isValid()
{
    return inited;
}

AIConditionsHelper::~AIConditionsHelper()
{
#ifdef USE_AI
    data_management_destroy_session(session);
#endif
}

QJsonDocument AIConditionsHelper::pharseConditions(const QString &description, bool *ok)
{
    QJsonDocument doc;
#ifdef USE_AI
    if (!inited) {
        setOkResult(ok, false);
        return doc;
    }

    char *results = nullptr;
    QJsonObject obj;
    obj["description"] = description;
    QJsonDocument descriptionDoc(obj);
    DataManagementStatus ret = data_management_extract_search_conditions(session, descriptionDoc.toJson().constData(), &results);
    if (!ret.success) {
        setOkResult(ok, false);
        return doc;
    }

    doc = QJsonDocument::fromJson(results);

    setOkResult(ok, true);
    return doc;
#else
    setOkResult(ok, false);
    return doc;
#endif
}

bool AIConditionsHelper::isFileMatchedWithDescriptionCondition(const QString &filePath, const QString &descriptionCondition, bool *ok)
{
#ifdef USE_AI
    bool matched = false;
    if (!inited) {
        setOkResult(ok, false);
        return matched;
    }

    QJsonObject obj;
    obj["file"] = filePath;
    obj["description"] = descriptionCondition;
    QJsonDocument doc(obj);
    bool result = false;
    auto ret = data_management_is_file_relevant_to_description(session, doc.toJson().constData(), &result);
    if (!ret.success) {
        return matched;
    }

    if (result == 1) {
        matched = true;
    }

    setOkResult(ok, true);

    return matched;
#else
    setOkResult(ok, false);
    return false;
#endif
}

bool AIConditionsHelper::isFileMatchedWithLables(const QString &filePath, const QStringList &labels, bool *ok)
{
#ifdef USE_AI
    bool matched = false;
    if (!inited) {
        setOkResult(ok, false);
        return matched;
    }

    QJsonObject obj;
    obj["file"] = filePath;
    obj["tags"] = QJsonArray::fromStringList(labels);
    QJsonDocument doc(obj);
    bool result = false;
    DataManagementStatus ret = data_management_is_file_relevant_to_tags(session, doc.toJson().constData(), &result);
    if (!ret.success) {
        return matched;
    }

    if (result == 1) {
        matched = true;
    }

    setOkResult(ok, true);

    return matched;
#else
    setOkResult(ok, false);
    return false;
#endif
}

void AIConditionsHelper::setOkResult(bool *ok, bool result)
{
    if (ok) {
        *ok = result;
    }
}
