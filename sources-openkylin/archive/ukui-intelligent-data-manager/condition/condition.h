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

#ifndef CONDITION_H
#define CONDITION_H

#include <QObject>
#include <QVariant>
#include <QSharedPointer>

class Condition : public QObject
{
    Q_OBJECT
public:
    enum Type {
        FileName,
        FileType,
        FileSize,
        FileContent,
        SearchLocation,
        DateRange,
        ConditionMatchRule
    };

    enum MatchRule {
        NameOrContent,
        NameAndContent
    };

    explicit Condition(Type type, QObject *parent = nullptr);
    static QSharedPointer<Condition> fromJsonObject(const QJsonObject &jsonObj);

    bool isValid();

    /*!
     * 设置条件的与或逻辑, 默认逻辑为与逻辑
     * \brief setIsAndCondition
     * \param isAnd
     */
    void setIsAndCondition(bool isAnd);
    bool isAndCondition() const;

    bool setFileNameCondition(bool contains, const QStringList &keywords);
    QStringList getFileNameKeywords();
    bool getFileNameIsContains();
    bool isFileNameMatchedWithKeywords(const QString &fileName);

    bool setFileTypeCondition(const QStringList &suffixes);
    bool setFileTypeCondition(int fileType);
    int getFileTypeSelectionType();
    QStringList getFileTypeSuffixes();
    bool isFileTypeMatchedWithSuffixes(const QString &fileName);
    bool isFileTypeMatchedWithSubType(int type, const QString &fileName, const QString &mimeType);

    bool setFileSizeCondition(bool contains, int type);
    bool getFileSizeIsContains();
    int getFileSizeType();
    bool isFileSizeMatchedWithSize(quint64 size);

    bool setFileContentCondition(const QString &content);
    QString getFileContentContent();

    bool setSearchLocationCondition(bool contains, const QString &path);
    bool getSearchLocationIsContains();
    QString getSearchLocationPath();
    bool isFileMatchedWithSearchLocation(const QString &filePath);

    bool setDateRangeCondition(int type, quint64 startMsec, quint64 endMsec);
    int getDateRangeType();
    quint64 getDateRangeStartMsec();
    quint64 getDateRangeEndMsec();
    bool isFileMatchedWithDate(int type, quint64 timeMsec);

    bool setConditionMatchRule(MatchRule rule, bool contains = true);
    MatchRule getConditionMatchRule();
    bool getConditionMatchRuleIsContains();

    Type type() const;

    QJsonObject conditionObject();

signals:

private:
    bool m_isAndCondition = true; // 设置是否为And类型条件
    Type m_type;
    QVariant m_conditionDescription; // json数据结构

};

#endif // CONDITION_H
