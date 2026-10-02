/*
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
#ifndef CONVERSION_H
#define CONVERSION_H

#define DIGIT_64 64
#define DIGIT_32 32
#define DIGIT_16 16
#define DIGIT_8 8
#define DIGIT_OCT_MAX 22
#define DIGIT_HEX_MAX 16
#define BASE_HEX 16
#define BASE_DEC 10
#define BASE_OCT 8
#define BASE_BIN 2
#define UNSIGNED_CHAR_MAX 255
#define UNSIGNED_INT_MAX 65535
#define UNSIGNED_LONG_MAX 4294967295
#define UNSIGNED_LLONG_MAX 18446744073709551615

#include <QList>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <cmath>
#include <iostream>
#include <bitset>
#include <string>
#include <climits>
using namespace std;

class Conversion
{
public:
    static Conversion *getInstance(void);

    // 设置当前位数
    void setDigit(int digit);

    /*************************************************
    **函数名称: decTo
    **函数功能: 将十进制数值型字符串转换为其他进制数值型字符串
    **输入参数: dec: 十进制数值型字符串
    **         base: 将传入的十进制数值型字符串转换为base进制数值型字符串
    **输出参数: 无
    **返回值:   转换后的base进制数数值型字符串
    **其它说明: 无
    *************************************************/
    QString decTo(QString dec, int base);

    /*************************************************
    **函数名称: otherToDec
    **函数功能: 将其他进制数值型字符串转换为十进制进制数值型字符串
    **输入参数: other: 其他进制数值型字符串
    **         base: 传入的数值型字符串的进制
    **输出参数: 无
    **返回值:   转换后的十进制数数值型字符串
    **其它说明: 无
    *************************************************/
    QString otherToDec(QString other, int base);

    /*************************************************
    **函数名称: isLegitimate
    **函数功能: 判断传入的数是否属于当前进制范围
    **输入参数: value: 任意进制数值型字符串
    **         base: 传入的数值型字符串的进制，缺省值为10
    **输出参数: 无
    **返回值:   true符合，false不符合
    **其它说明: 无
    *************************************************/
    bool isLegitimate(QString value, int base = BASE_DEC);

    /*************************************************
    **函数名称: setLegal
    **函数功能: 根据当前的位数将传入的数进行合法话，转换到对应范围
    **输入参数: value: 任意进制数值型字符串
    **         base: 传入的数值型字符串的进制，缺省值为2
    **输出参数: 无
    **返回值:   转换后的数值型字符串
    **其它说明: 无
    *************************************************/
    QString setLegal(QString value, int base = BASE_BIN);

    /*************************************************
    **函数名称: setCode
    **函数功能: 将数值型字符串转换为对应的编码字符串
    **输入参数: value: 任意进制数值型字符串
    **         code: 编码格式
    **         base: 传入的数值型字符串的进制
    **输出参数: 无
    **返回值:   转换后的编码字符串
    **其它说明: 无
    *************************************************/
    QString setCode(QString value, QString code, int base);

    // 加
    // num1+num2
    QString calAdd(QString num1, QString num2);

    // 减
    // num1-num2
    QString calSub(QString num1, QString num2);

    // 乘
    // num1*num2
    QString calMulit(QString num1, QString num2);

    // 除
    // num1/num2
    QString calDiv(QString num1, QString num2);

    // 按位取反
    // ~num
    QString calNot(QString num);

    // 按位与
    // num1&num2
    QString calAnd(QString num1, QString num2);

    // 按位或
    // num1|num2
    QString calOr(QString num1, QString num2);

    // 按位异或
    // num1^num2
    QString calXor(QString num1, QString num2);

    // 按位或非
    // ~(num1|num2)
    QString calNor(QString num1, QString num2);

    // 左移计算
    // num1<<num2
    QString calLsh(QString num1, QString num2);

    // 右移计算
    // num1>>num2
    QString calRsh(QString num1, QString num2);

    // 循环左移
    QString calRoL(QString num);

    // 循环右移
    QString calRoR(QString num);

    // 当前二进制位数
    int getDigit() const;

private:
    Conversion(){}

    // 当前二进制位数
    int m_digit = DIGIT_64;

    // 当前八进制位数
    int m_otcDigit = DIGIT_OCT_MAX;

    // 当前十六进制位数
    int m_hexDigit = DIGIT_HEX_MAX;

    // 最大范围
    quint64 m_max = UNSIGNED_LLONG_MAX;

    // 进制数
    QStringList m_digitList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};

    // 十进制转换为2进制处理
    QString DecToBin(qint64 decInt);

    // 二进制转八进制、十六进制
    // bin:二进制
    // num:八进制和十六进制的一位对应二进制的位数
    QString BinToOctHex(QString bin, int num);

    // 根据指定位置获取QString
    // str:源字符串
    // start:开始位置
    // size:获取长度
    QString mySplit(QString str, int start, int size);


    // 转换位当前位数下合法的数
    // num:需要转换的十进制数
    QString setLegalNum(qint64 num);

    // N进制转换为十进制递归转换
    // other:N进制
    // dec:转换后的十进制
    // base:几进制
    // digitNum:位指针
    // count:索引
    void resOtherToDec(QString other, quint64& dec, int base, int digitNum, int count);

    // 计算阶乘
    // x的y次方
    quint64 factorial(int x, int y);
};

#endif // CONVERSION_H
