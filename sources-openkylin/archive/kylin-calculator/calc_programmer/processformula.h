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
#ifndef PROCESSFORMULA_H
#define PROCESSFORMULA_H

#include <QString>
#include <QVector>
#include <QMap>
#include "calc/calc.h"
#include "calc/conversion.h"

enum{
    MARK_CURRENT_EXPRESSION = 0,     // 0:用于计算的表达式
    MARK_DISPLY_EXPRESSION = 1,      // 1:用于显示的表达式
    MARK_BUDGET_RESULTS = 2,         // 2:预算显示
    MARK_HISTOR_RECORDS = 3,         // 3:历史记录显示
    MARK_CAL_RESULTS = 4,            // 4:计算结果
    MARK_BIN_RESULTS = 5,            // 5:二进制结果
    MARK_CODE_RESULTS = 6,           // 6:结果转换字符显示
    MARK_IS_RIGHT = 7,               // 7:计算是否正确
    MARK_IS_INPUT = 8,               // 8:是否可以继续输入 TRUE | FALSE ,TRUE 可以继续输入 FALSE 无法继续输入
};

class ProcessFormula : public QObject
{
    Q_OBJECT
public:
    static ProcessFormula *getInstance(void);

    /*************************************************
    **函数名称: process
    **函数功能: 将表达式处理为需要的各种形式，并且分析计算结果
    **输入参数: formula: 计算的表达式
    **输出参数: 无
    **返回值:   返回一个QList<QString>结构，返回处理后的各种信息
    **其它说明: 无
    *************************************************/
    QList<QString> process(QString formula);

    // 进制转换处理
    // value:需要转换的数值
    // befBase:转换前数值的进制
    // lastBase:需要转换的进制
    QString baseHandle(QString value, int lastBase);

    // 获取二进制
    // value:需要转换的数值
    QString bin(QString value);

    // 位数转换处理
    // digit:需要切换的位数
    void digitHandle(int digit);

    // 处理合法数
    // value:需要处理的数值
    QString legal(QString value);

    // 设置转换字符编码
    // code:需要转换字符编码
    void setCode(QString code);

    // 转换字符
    // value:需要转换的数值
    QString code(QString value);

    // 匹配双目操作符
    bool containsBinarry(QString binarry);

    // 匹配单目操作符
    bool containsUnary(QString unary);

    // 判断操作符个数
    int opNum(QString formula);

private:
    ProcessFormula(){}

    // 双目运算符计算符号和显示符号映射表
    QMap<QString, QString> m_binaryMap = {
        {"+", "+"},
        {"s", "ｰ"},
        {"*", "×"},
        {"/", "÷"},
        {"&", " AND "},
        {"|", " OR "},
        {"^", " XOR "},
        {"<", "<<"},
        {">", ">>"},
        {"N", " NOR "},
    };
    // 单目运算符和显示符号映射表
    QMap<QString, QString> m_unaryMap = {
        {"~", "~"},
        {"L", "RoL"},
        {"R", "RoR"},
        {"v", "ｰ"},
        {"p", "Rsh"},
        {"q", "Lsh"},
    };

    // 进制数
    QStringList m_digitList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};

    // 存储处理结果
    QList<QString> m_resultVec = {"0", "0", "", "0", "0", "0", "", "TRUE", "TRUE"};

    // 当前进制
    int m_base = BASE_DEC;

    // 当前字符编码
    QString m_code = QString("");

    // 处理用于计算的表达式
    QStringList handleExpression(QString formula);

    // 为表达式补齐括号
    QString setBrackets(QString formula);

    // 将操作符替换成需要显示的字符
    QString setSymbol(QString formula);

    // 替换单目运算符显示
    QString setUnary(QString formula, QString unary);

    // 设置历史记录的显示
    QString setHistory(QString formula, QString result);

    // 清空Vector
    void clearVec();
};

#endif // PROCESSFORMULA_H
