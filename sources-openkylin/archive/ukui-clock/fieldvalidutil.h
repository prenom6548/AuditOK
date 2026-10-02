/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 */
#ifndef FIELDVALIDUTIL_H
#define FIELDVALIDUTIL_H

#include <QObject>
#include <QRegExp>
#include <QDebug>
#include <string>
#include <regex>
class FieldValidUtil : public QObject
{
    Q_OBJECT
public:
    explicit FieldValidUtil(QObject *parent = nullptr);
    static bool isNull(int value) ;
    static bool isNull(QString str) ;
    static bool isNull(std::string str) ;
    static bool isNull(long value) ;
    static bool isNull(float value) ;
    static bool isNull(double value) ;
    static bool isNotNull(int value) ;
    static bool isValueBetweenRange(int value,int small,int big) ;
    static bool isNotNull(QString value) ;
    static bool isNotNull(long value) ;
    static bool isUrl(QString str);
    static bool isPwd(QString str);
    static bool QStringCheck(QString str);
    static bool isEmail(QString str);
    static bool isInteger(QString str);
    static bool isNumeric(QString str);
    static bool isDigits(QString str);
    static bool isFloat(QString str);
    static bool isTel(QString text);
    static bool isPhone(QString text);
    static bool isMobile(QString text);
    static bool isIdCardNo(QString text);
    static bool isZipCode(QString text);
    static bool isIntEqZero(int num);
    static bool isIntGtZero(int num);
    static bool isIntGteZero(int num);
    static bool isFloatEqZero(float num);
    static bool isFloatGtZero(float num);
    static bool isFloatGteZero(float num);
    static bool isRightfulQString(QString text);
    static bool isEnglish(QString text);
    static bool isChineseChar(QString text);
    static bool isChinese(QString text);
    static bool isContainsSpecialChar(QString text);
    static QString QStringFilter(QString text);
    static QString QStringFilter(std::string text);
    static QString htmlFilter(QString inputQString);
    static bool match(QString text, QString reg);
    static QString replaceAll(QString text, QString reg);
    static QString replaceAll(std::string text, QString reg);
    static void createEntityCode(QString fields);
    static QString makeFirstUpper(QString str);



signals:

};
#define NO_SPECIAL_CHAR  "^[\u4E00-\u9FA5A-Za-z0-9_]+$"

#endif // FIELDVALIDUTIL_H
