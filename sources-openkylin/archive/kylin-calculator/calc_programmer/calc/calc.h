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
#ifndef CALC_H
#define CALC_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QStack>
#include <QList>
#include <QDebug>

#include "conversion.h"

// 返回信息
enum {
    ERROR_FLAG = 0,   // 1:错误信息标识 ERROR
    ERROR_TYPE = 1,   // 0:信息类别 TRUE | FALSE | CAL_FALSE | NUM_FALSE , FALSE表示表达式出错，后面无法在继续计算， TRUE 表示表达式出错，后面可以继续计算, CAL_FALSE表示计算出错
    ERROR_INFO = 2,   // 2:信息内容
    RIGHT_RESULT = 3, // 3:正确计算结果
};

class Calc : public QObject
{
    Q_OBJECT
public:
    static Calc *getInstance(void);

    /*************************************************
    **函数名称: cal
    **函数功能: 计算传入的表达式
    **输入参数: strList: 计算的表达式
    **输出参数: 无
    **返回值:   返回一个QList<QString>结构，
    **         {"错误信息标识", "信息类别", "信息内容", "正确计算结果"}，
    **         计算正确前三项为空，计算错误最以后一项为空
    **其它说明: 无
    *************************************************/
    QList<QString> cal(QStringList strList);

    // 设置进制数
    void setBase(int base);

private:
    Calc(){}
    // 返回结果
    QList<QString> m_resultVec = {"", "", "", ""};

    // 符号优先级表
    QMap<QString, int> m_opPriority;

    // 双目运算符 <：左移 >：右移 N:或非
    QStringList m_binary = {"+", "s", "*", "/", "&", "|", "^", "<", ">", "N"};

    // 单目运算符 L：循环左移 R：循环右移 q:左移1 p:右移1 v:相反数
    QStringList m_unary = {"~", "L", "R", "q", "p", "v"};

    // 括号
    QStringList m_brackets = {"(", ")"};

    // 进制数
    int m_base = BASE_DEC;

    // 初始化操作符优先级
    void getPriority();

    // 中缀转后缀
    // infList:中缀表达式
    // postList:后缀表达式
    // result:用于返回错误信息
    bool infToPostf(QStringList &infList, QStringList &postfList, QString& result);

    // 计算后缀表达式
    QString calPostf(QStringList& postfList);

    // 单目运算符处理
    // num:操作数
    // str:操作符
    QString calUnary(QString num, QString str);

    // 双目运算符处理
    // num1、num2:操作数
    // str:操作符
    QString calBinary(QString num1, QString num2, QString str);

    // 处理错误信息
    // 输入错误信息
    // 返回错误信息list
    // 0:错误信息标识
    // 1:信息类别
    // 2:信息内容
    QStringList setError(QString err);

    // 清空返回结果
    void clear();

};

#endif // LEGITIMACY_H
