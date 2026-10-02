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
#include <QJsonDocument>
#include <QFileInfo>
#include <QDateTime>
#include <kysdk/kysdk-system/libkydate.h>

const QString Folder_Type = "inode/directory";
const QString Image_Type = "image/";
const QString Video_Type = "video/";
const QString Text_Type = "text/";
const QString Wps_Type = "application/wps-office";
const QString Audio_Type = "audio/";
const QStringList wpsSuffix = {"doc", "docx", "dot", "wps", "wpt", "xlx", "xlxs", "et", "ppt", "pptx", "wpp"};

QString getRelativeTime(QDateTime dateTime)
{
    QString displayString;
#ifdef COMPLIE_AS_PROJECT
    time_t lt;
    lt = time(NULL);
    struct tm m_tm = *localtime(&lt);

    QDate date = dateTime.date();
    QTime qtime = dateTime.time();

    m_tm.tm_year = date.year();
    m_tm.tm_mon = date.month();
    m_tm.tm_mday = date.day();

    m_tm.tm_hour = qtime.hour();
    m_tm.tm_min = qtime.minute();
    m_tm.tm_sec = qtime.second();

    auto ret = kdk_system_timeformat_transform(&m_tm);
    char* formatDate = kdk_system_tran_absolute_date(&m_tm);
    displayString = QObject::tr("%1 %2").arg(formatDate).arg(ret->timesec);
    kdk_free_timeinfo(ret);
    free(formatDate);
#endif
    return displayString;
}

Condition::Condition(Type type, QObject *parent) : QObject(parent)
{
    m_type = type;
}

std::shared_ptr<Condition> Condition::fromJsonObject(const QJsonObject &jsonObj)
{
    int type = jsonObj["condition type"].toInt();
    auto condition = std::shared_ptr<Condition>(new Condition(Type(type)));
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
        if(!value.toString().isEmpty())
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

bool Condition::getFileTypeIsContains()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["contains"].toBool();
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
    bool contains = getFileTypeIsContains();
    if (type == ALL_TYPE)
        return contains;

    switch (type)
    {
    case FILE_FOLDER:
    {
        match = mimeType == Folder_Type;
        break;
    }
    case PICTURE:
    {
        match = mimeType.startsWith(Image_Type);
        break;
    }
    case VIDEO:
    {
        match = mimeType.startsWith(Video_Type);
        break;
    }
    case TXT_FILE:
    {
        match = mimeType.startsWith(Text_Type);
        break;
    }
    case WPS_FILE:
    {
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
    case AUDIO:
    {
        match = (mimeType.startsWith(Audio_Type)||mimeType.contains("application/x-smaf"));
        break;
    }
    case OTHERS:
    {
        //exclude classfied types, show the rest other types
        if ((mimeType != Folder_Type) && ! mimeType.startsWith(Image_Type) && !mimeType.startsWith(Video_Type)
                && ! mimeType.startsWith(Text_Type) && ! mimeType.startsWith(Wps_Type) && !mimeType.startsWith(Audio_Type) && !mimeType.contains("application/x-smaf"))
            return true;
        break;
    }
    default:
        break;
    }
    match = contains? match: !match;
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
        // "Empty(all)"
        sizeMatch = true;
        break;
    }
    case 1: {
        // "Empty(0K)"
        if (size == 0) {
            sizeMatch = true;
        }
        break;
    }
    case 2: {
        // "Tiny(0-16K)"
        if (size <= 16 * 1024 && size > 0) {
            sizeMatch = true;
        }
        break;
    }
    case 3: {
        // "Small(16K-1M)"
        if (size <= 1024 * 1024 && size > 16 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 4: {
        // "Medium(1M-128M)"
        if (size <= 128 * 1024 * 1024 && size > 1024 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 5: {
        // "Big(128M-1G)"
        if (size <= 1024 * 1024 * 1024 && size > 128 * 1024 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 6: {
        // "Large(1-4G)"
        quint64 fourGiB = 1024 * 1024 * 1024;
        fourGiB *= 4;
        if (size <= fourGiB && size > 1024 * 1024 * 1024) {
            sizeMatch = true;
        }
        break;
    }
    case 7: {
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

bool Condition::getDateRangeIsContains()
{
    auto jsonObj = m_conditionDescription.toJsonObject();
    return jsonObj["contains"].toBool();
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
    bool contain = getDateRangeIsContains();
    auto Datetype = FilterFileModifyTime(getDateRangeType());
    if (Datetype == ALL_TIME)
        return contain;
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto dateTime = QDateTime::fromMSecsSinceEpoch(time);
    //qDebug() << "cur QDateTime:" <<dateTime.toString(Qt::SystemLocaleShortDate);
    QDate date = dateTime.date();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    QDate md_date = QDateTime::fromMSecsSinceEpoch(timeMsec).date();
    //qDebug() << "modify time:" <<QDateTime::fromMSecsSinceEpoch(modifiedTime *1000).toString(Qt::SystemLocaleShortDate);
    int md_year= md_date.year();
    int md_month = md_date.month();
    int md_day = md_date.day();
    switch(Datetype)
    {
    case TODAY:
    {
        if (year == md_year && month == md_month && day == md_day)
            return contain;
        break;
    }
    case YESTERDAY:
    {
        QDate yesterday = date.addDays(-1);
        if(yesterday == md_date)/* 判断给定日期是否是昨天 */
            return contain;
        break;
    }
    case THIS_WEEK:
    {
        QDate md_date(md_year, md_month, md_day);
        int week, md_week,week_year, md_week_year;
        week = date.weekNumber(&week_year);
        md_week = md_date.weekNumber(&md_week_year);
        if (week == md_week && week_year == md_week_year)
            return contain;
        break;
    }
    case LAST_WEEK:
    {
        QDate monDate = date.addDays(-date.dayOfWeek() + 1);
        if(monDate.addDays(-7) <= md_date &&  md_date <= monDate.addDays(-1))/* 判断给定日期是否在上周 */
            return contain;
        break;
    }
    case THIS_MONTH:
    {
        if (year == md_year && month == md_month)
            return contain;
        break;
    }
    case LAST_MONTH:
    {
        int lastMonth = (month == 1) ? 12 : month - 1;
        if(md_month == lastMonth){/*  判断给定日期是否在上个月 */
            if((month == 1 && md_year == (year - 1)) || (month !=1 && year == md_year)){
                return contain;
            }
        }
        break;

    }
    case THIS_YEAR:
    {
        if (year == md_year)
            return contain;
        break;
    }
    case LAST_YEAR:
    {
        int lastYear = year - 1;
        if(md_year == lastYear)/* 判断给定日期是否在去年 */
            return contain;
        break;
    }
    case BEFORE_LAST_YEAR:
    {
        int lastYear = year - 1;
        if(md_year < lastYear)/* 判断给定日期是否在去年以前 */
            return contain;
        break;
    }
    default:
        break;
    }
    return !contain;

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

QStringList Condition::getConditionLabels()
{
    QStringList labels;
    switch (m_type) {
    case FileName: {
        auto keywords = getFileNameKeywords();
        if (!keywords.isEmpty()) {
            if (getFileNameIsContains()) {
                for (auto keyword : keywords) {
                    if(!keyword.isEmpty())
                        labels << tr("File name contains \"%1\"").arg(keyword);
                }
            } else {
                for (auto keyword : keywords) {
                    labels << tr("File name without \"%1\"").arg(keyword);
                }
            }
        }
        break;
    }
    case FileType: {
        QStringList suffixes = getFileTypeSuffixes();
        if (!suffixes.isEmpty()) {
            QString joinSuffixes;
            if (suffixes.count() > 1) {
                joinSuffixes  = suffixes.join(", ");
            } else {
                joinSuffixes = suffixes.first();
            }
            if(!joinSuffixes.isEmpty())
                labels << tr("File type contains \"%1\"").arg(joinSuffixes);
        } else {
            QStringList selections = {tr("By suffix"), tr("Folder"), tr("Plain Text"), tr("WPS Document"), tr("PDF"), tr("Image"), tr("Video"), tr("Audio")};
            int type = getFileTypeSelectionType();
            if (type >= 0 && type < selections.count()) {
                QString fileType = selections.at(type);
                labels << tr("File type contains %1").arg(fileType);
            }
        }
        break;
    }
    case FileSize: {
        bool contains = getFileSizeIsContains();
        int sizeType = getFileSizeType();

        QStringList sizes = {tr("All"),tr("Empty(0K)"), tr("Tiny(0-16K)"), tr("Small(16K-1M)"), tr("Medium(1M-128M)"), tr("Big(128M-1G)"), tr("Large(1-4G)"), tr("Great(>4G)")};
        if (sizeType >= 0 && sizeType < sizes.count()) {
            if (contains) {
                labels << tr("File size contains %1").arg(sizes.at(sizeType));
            } else {
                labels << tr("File size without %1").arg(sizes.at(sizeType));
            }
        }
        break;
    }
    case FileContent: {
        labels << tr("File content match %1").arg(getFileContentContent());
        break;
    }
    case SearchLocation: {
        bool contains = getSearchLocationIsContains();
        QFileInfo fileInfo(getSearchLocationPath());
        QString fileName = fileInfo.fileName();
        if (contains) {
            labels << tr("File location contains \"%1\"").arg(fileName);
        } else {
            labels << tr("File location without \"%1\"").arg(fileName);
        }
        break;
    }
    case DateRange: {
        qint64 time1 = getDateRangeStartMsec();
        qint64 time2 = getDateRangeEndMsec();
        QDateTime dateTime1 = QDateTime::fromMSecsSinceEpoch(time1);
        QDateTime dateTime2 = QDateTime::fromMSecsSinceEpoch(time2);
        QString startTime = tr("Later than \"%1\"").arg(getRelativeTime(dateTime1));
        QString endTime = tr("Earlier than \"%1\"").arg(getRelativeTime(dateTime2));
        labels << startTime << endTime;
        break;
    }
    default:
        break;
    }
    return labels;
}

bool Condition::isAndCondition() const
{
    return m_isAndCondition;
}

QList<std::shared_ptr<Condition>> Condition::generateContitionsFromDescription(const QVariant &description)
{
    QList<std::shared_ptr<Condition>> result;
    QJsonObject jsonObj = description.toJsonObject();
    int count = jsonObj["conditions count"].toInt();
    for (int i = 0; i < count; i++) {
        auto conditionObj = jsonObj[QString::number(i)].toObject();
        auto condition = Condition::fromJsonObject(conditionObj);
        result << condition;
    }
    return result;
}

QStringList Condition::getLabelsFromDescription(const QVariant &description)
{
    QStringList labels;
    QJsonObject jsonObj = description.toJsonObject();
    QJsonValue labelsValue = jsonObj["labels"];
    if (labelsValue.isArray()) {
        QJsonArray labelsArray = labelsValue.toArray();
        for (auto label : labelsArray) {
            labels << label.toString();
        }
    }
    labels.sort();
    return labels;
}

QVariant Condition:: pharseConditions(const QList<std::shared_ptr<Condition> > &conditions, const QStringList &labels){
    QJsonObject configObj;
    configObj["conditions count"] = conditions.count();
    for (int i = 0; i < conditions.count(); i++) {
        configObj[QString::number(i)] = conditions[i]->conditionObject();
    }
    if (!labels.isEmpty()) {
        configObj["labels"] = QJsonArray::fromStringList(labels);
    }
    return configObj;
}
