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

#include "condition.h"

#include <QJsonObject>
#include <QJsonArray>

const QString Folder_Type = "inode/directory";
const QString Image_Type = "image/";
const QString Video_Type = "video/";
const QString Text_Type = "text/";
const QString Wps_Type = "application/wps-office";
const QString Audio_Type = "audio/";
const QStringList wpsSuffix = {"doc", "docx", "dot", "wps", "wpt", "xlx", "xlxs", "et", "ppt", "pptx", "wpp"};

Condition::Condition(Type type, QObject *parent) : QObject(parent)
{
    m_type = type;
}

QSharedPointer<Condition> Condition::fromJsonObject(const QJsonObject &jsonObj)
{
    int type = jsonObj["condition type"].toInt();
    auto condition = QSharedPointer<Condition>(new Condition(Type(type)));
    condition->m_conditionDescription = jsonObj;
    if (jsonObj["orCondition"].toBool()) {
        condition->m_isAndCondition = false;
    }
    return condition;
}

bool Condition::isValid()
{
    return !m_conditionDescription.isNull();
}

void Condition::setIsAndCondition(bool isAnd)
{
    m_isAndCondition = isAnd;

    QJsonObject obj = m_conditionDescription.toJsonObject();
    obj["orCondition"] = !isAnd;
    m_conditionDescription = QVariant(obj);
}

bool Condition::setFileNameCondition(bool contains, const QStringList &keywords)
{
    if (m_type == FileName && !keywords.isEmpty()) {
        QJsonObject fileNameObj;
        fileNameObj["condition type"] = int(m_type);
        fileNameObj["contains"] = contains;
        fileNameObj["keywords"] = QJsonArray::fromStringList(keywords);
        fileNameObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(fileNameObj);
        return true;
    }
    return false;
}

bool Condition::getFileNameIsContains()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["contains"].toBool();
}

bool Condition::isFileNameMatchedWithKeywords(const QString &fileName)
{
    bool match = false;
    QStringList keywords = this->getFileNameKeywords();
    if (this->getFileNameIsContains()) {
        for (auto keyword : keywords) {
            if (fileName.contains(keyword)) {
                match = true;
                break;
            }
        }
    } else {
        // 当条件为不包含时, 如果存在包含的关键词则不匹配, 此时应该跳出循环
        match = true;
        for (auto keyword : keywords) {
            if (fileName.contains(keyword)) {
                match = false;
                break;
            }
        }
    }

    return match;
}

QStringList Condition::getFileNameKeywords()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    auto jsonArray = jsonObj["keywords"].toArray();
    QStringList keywords;
    for (auto value : jsonArray) {
        keywords << value.toString();
    }
    return keywords;
}

bool Condition::setFileTypeCondition(const QStringList &suffixes)
{
    if (m_type == FileType && !suffixes.isEmpty()) {
        QJsonObject fileTypeObj;
        fileTypeObj["condition type"] = int(m_type);
        fileTypeObj["type"] = -1;
        fileTypeObj["suffixes"] = QJsonArray::fromStringList(suffixes);
        fileTypeObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(fileTypeObj);
        return true;
    }
    return false;
}

bool Condition::setFileTypeCondition(int fileType)
{
    if (m_type == FileType) {
        QJsonObject fileTypeObj;
        fileTypeObj["condition type"] = int(m_type);
        fileTypeObj["type"] = fileType;
        fileTypeObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(fileTypeObj);
        return true;
    }
    return false;
}

int Condition::getFileTypeSelectionType()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["type"].toInt();
}

QStringList Condition::getFileTypeSuffixes()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    auto jsonArray = jsonObj["suffixes"].toArray();
    QStringList suffixes;
    for (auto value : jsonArray) {
        suffixes << value.toString();
    }
    return suffixes;
}

bool Condition::isFileTypeMatchedWithSuffixes(const QString &fileName)
{
    auto suffixes = getFileTypeSuffixes();
    bool match = false;

    for (auto suffix : suffixes) {
        if (fileName.endsWith("." + suffix)) {
            match = true;
            break;
        }
    }

    return match;
}

bool Condition::isFileTypeMatchedWithSubType(int type, const QString &fileName, const QString &mimeType)
{
    bool match = false;

    // todo: 同步peony的判断依据
    // tr("Folder"), tr("Plain Text"), tr("WPS Document"), tr("PDF"), tr("Image"), tr("Video"), tr("Audio")
    if (type > 0) {
        switch (type) {
        case 1: {
            match = mimeType == Folder_Type;
            break;
        }
        case 2: {
            match = mimeType.startsWith(Text_Type);
            break;
        }
        case 3: {
            match = mimeType.startsWith(Wps_Type);
            if (!match) {
                QString tmp = fileName;
                int index = tmp.lastIndexOf(".");
                if (index >= 0) {
                    tmp = tmp.chopped(index);
                    if (!tmp.isEmpty()) {
                        match = wpsSuffix.contains(tmp);
                    }
                }
            }
            break;
        }
        case 4: {
            match = fileName.endsWith("pdf");
            break;
        }
        case 5: {
            match = mimeType.startsWith(Image_Type);
            break;
        }
        case 6: {
            match = mimeType.startsWith(Video_Type);
            break;
        }
        case 7: {
            match = mimeType.startsWith(Audio_Type);
            break;
        }
        default:
            break;
        }
    }

    return match;
}

bool Condition::setFileSizeCondition(bool contains, int type)
{
    if (m_type == FileSize) {
        QJsonObject fileSizeObj;
        fileSizeObj["condition type"] = int(m_type);
        fileSizeObj["contains"] = contains;
        fileSizeObj["type"] = type;
        fileSizeObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(fileSizeObj);
        return true;
    }
    return false;
}

bool Condition::getFileSizeIsContains()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["contains"].toBool();
}

int Condition::getFileSizeType()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["type"].toInt();
}

bool Condition::isFileSizeMatchedWithSize(quint64 size)
{
    bool match = false;
    int fileSizeType = this->getFileSizeType();
    bool contains = this->getFileSizeIsContains();
    bool sizeMatch = false;

    switch (fileSizeType) {
    case 0: {
        // "Empty(0K)"
        if (size == 0) {
            sizeMatch = true;
        }
        break;
    }
    case 1: {
        // "Tiny(0-16K)"
        if (size <= 16 * 1024 && size > 0) {
            sizeMatch = true;
        }
        break;
    }
    case 2: {
        // "Small(16K-1M)"
        if (size <= 1024 * 1024 && size > 16 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 3: {
        // "Medium(1M-128M)"
        if (size <= 128 * 1024 * 1024 && size > 1024 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 4: {
        // "Big(128M-1G)"
        if (size <= 1024 * 1024 * 1024 && size > 128 * 1024 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 5: {
        // "Large(1-4G)"
        quint64 fourGiB = 1024 * 1024 * 1024;
        fourGiB *= 4;
        if (size <= fourGiB && size > 1024 * 1024 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 6: {
        // "Great(>4G)"
        quint64 fourGiB = 1024 * 1024 * 1024;
        fourGiB *= 4;
        if (size > fourGiB) {
            sizeMatch = true;
        }
        break;
    }
    default:
        break;
    }

    match = contains? sizeMatch: !sizeMatch;
    return match;
}

bool Condition::setFileContentCondition(const QString &content)
{
    if (m_type == FileContent && !content.isEmpty()) {
        QJsonObject fileContentObj;
        fileContentObj["condition type"] = int(m_type);
        fileContentObj["content"] = content;
        fileContentObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(fileContentObj);
        return true;
    }
    return false;
}

QString Condition::getFileContentContent()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["content"].toString();
}

bool Condition::setSearchLocationCondition(bool contains, const QString &path)
{
    if (m_type == SearchLocation && !path.isEmpty()) {
        QJsonObject searchLocationObj;
        searchLocationObj["condition type"] = int(m_type);
        searchLocationObj["contains"] = contains;
        searchLocationObj["path"] = path;
        searchLocationObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(searchLocationObj);
        return true;
    }
    return false;
}

bool Condition::getSearchLocationIsContains()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["contains"].toBool();
}

QString Condition::getSearchLocationPath()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["path"].toString();
}

bool Condition::isFileMatchedWithSearchLocation(const QString &filePath)
{
    bool match = false;
    bool contains = getSearchLocationIsContains();
    auto searchPath = getSearchLocationPath();
    if (!searchPath.endsWith("/")) {
        searchPath.append("/");
    }
    if (filePath.contains(searchPath)) {
        match = contains? true: false;
    } else {
        // 如果条件为不包含则认为匹配
        match = !contains;
    }

    return match;
}

bool Condition::setDateRangeCondition(int type, quint64 startMsec, quint64 endMsec)
{
    if (m_type == DateRange && (startMsec <= endMsec)) {
        QJsonObject dateRangObj;
        dateRangObj["condition type"] = int(m_type);
        dateRangObj["type"] = type;
        dateRangObj["start"] = QString::number(qulonglong(startMsec));
        dateRangObj["end"] = QString::number(qulonglong(endMsec));
        dateRangObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(dateRangObj);
        return true;
    }
    return false;
}

int Condition::getDateRangeType()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["type"].toInt();
}

quint64 Condition::getDateRangeStartMsec()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["start"].toString().toULongLong();
}

quint64 Condition::getDateRangeEndMsec()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["end"].toString().toULongLong();
}

bool Condition::isFileMatchedWithDate(int type, quint64 timeMsec)
{
    bool match = false;

    quint64 start = getDateRangeStartMsec();
    quint64 end = getDateRangeEndMsec();

    match = timeMsec >= start && timeMsec <= end;

    return match;
}

bool Condition::setConditionMatchRule(MatchRule rule, bool contains)
{
    if (m_type == ConditionMatchRule) {
        QJsonObject conditionMatchRuleObj;
        conditionMatchRuleObj["condition type"] = int(m_type);
        conditionMatchRuleObj["contains"] = contains;
        conditionMatchRuleObj["type"] = int(rule);
        conditionMatchRuleObj["orCondition"] = !m_isAndCondition;
        m_conditionDescription = QVariant(conditionMatchRuleObj);
        return true;
    }
    return false;
}

Condition::MatchRule Condition::getConditionMatchRule()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    int result = jsonObj["type"].toInt();
    return MatchRule(result);
}

bool Condition::getConditionMatchRuleIsContains()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["contains"].toBool();
}

Condition::Type Condition::type() const
{
    return m_type;
}

QJsonObject Condition::conditionObject()
{
    return m_conditionDescription.toJsonObject();
}

bool Condition::isAndCondition() const
{
    return m_isAndCondition;
}
