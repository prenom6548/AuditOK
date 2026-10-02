/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

// define in .cpp now & in .h later
#define DEBUG_MODE false

#include "InputProcess.h"

QVector<QString> InputProcess::res = QVector<QString> {ZERO, ZERO, EMPTY, EMPTY, EMPTY, EMPTY};

QString InputProcess::qstrNow = ZERO;
QString InputProcess::qstrPrepare = EMPTY;
QString InputProcess::labelNow = ZERO;
QString InputProcess::labelPrepare = EMPTY;
QString InputProcess::calAns = EMPTY;
QString InputProcess::lastAns = EMPTY;
QStringList InputProcess::calHistory = QStringList();

InputJudgmentGraph InputProcess::G = InputJudgmentGraph();

int InputProcess::i_couldBeCalFlag = -1;
int InputProcess::i_qstrAddFlag = -1;
int InputProcess::i_qstrNowUpdateFlag = -1;
int InputProcess::i_qstrUpdateFlag = -1;

void InputProcess::init()
{
    qstrNow = ZERO;
    qstrPrepare = EMPTY;
    labelNow = ZERO;
    labelPrepare = EMPTY;
    lastAns = EMPTY;
    res = QVector<QString> {ZERO, ZERO, EMPTY, EMPTY, EMPTY, EMPTY};
}

/* 判断该字符是否可以添加到表达式中 */
int InputProcess::qstrAdd(const QString &text)
{
    int formulaOrNum = formulaTypeCheck(qstrNow);

    QString ch = EMPTY;

    if (qstrNow.size()) {
        ch = qstrNow.right(1);
    }

    /* 括号判定 */
    if (BRACKET_LIST.contains(text)) {
        /* 左括号 */
        if (text == BRACKET_L) {
            /* 当前算式为空 或 0 , 直接添加 */
            if (qstrNow.size() == 0 || qstrNow.size() == 1 && qstrNow == ZERO) {
                qstrNow = text;
                return CORRECT_QSTR_ADD;
            }

            /* 末尾是 + - * / ^ 左括号才可以加 */
            if (isArithmeticOperator(ch) || ch == POWER_SYMBOL && text == BRACKET_L) {
                qstrNow += text;
                return CORRECT_QSTR_ADD;
            }

            return CANNOT_ADD_BRACKET_L;
        }

        /* 右括号 */
        if (text == BRACKET_R) {
            /* 当上个字符为 右括号 或 数字 时进行添加 */
            if (ch == BRACKET_R || ZERO_TO_NINE.contains(ch)) {
                qstrNow += BRACKET_R;
                if (findBracketPair(qstrNow, qstrNow.size() - 1) != -1) {
                    return CORRECT_QSTR_ADD;
                }
                qstrNow.chop(1);
            }
        }

        return CANNOT_ADD_BRACKET_R;
    }

    /* EQUAL */
    if (text == EQUAL) {
        if (ch == EQUAL || formulaOrNum == -2 || isCorrectFormula(qstrNow).first == 0) {
            return CANNOT_ADD_EQUAL;
        }

        qstrNow += EQUAL;

        return CORRECT_QSTR_ADD;
    }

    /* " + - * / " */
    if (ARITHMETIC_OPERATOR_LIST.contains(text)) {
        if (!qstrNow.size() || text == ch || (text == MUL || text == DIV) && ch == BRACKET_L) {
            return CANNOT_ADD_ARITHMETIC_OPERATOR;
        }

        qstrNow += text;

        return CORRECT_QSTR_ADD;
    }

    /* B C */
    if (text == CLEAN || text == BACKSPACE) {
        if (qstrNow.size() == 0 || qstrNow.size() == 1 && qstrNow == ZERO) {
            qstrNow = ZERO + text;
            return CORRECT_QSTR_ADD;
        }

        qstrNow += text;
        return CORRECT_QSTR_ADD;
    }

    /* 00 判断 */
    if (text == DOUBLE_ZERO) {
        if (qstrNow.size() == 0 || qstrNow.size() == 1 && qstrNow == ZERO || ch == ANS_END || ch == SCI_NUM_END) {
            return CANNOT_ADD_DOUBLE_ZERO;
        }

        QString tempQstr = getNumberFromBack(qstrNow);

        if (isCorrectInt(tempQstr).first != 1) {
            return CANNOT_ADD_DOUBLE_ZERO;
        }

        if (tempQstr == ZERO) {
            return CANNOT_ADD_DOUBLE_ZERO;
        }

        qstrNow += text;

        return CORRECT_QSTR_ADD;
    }

    /* . */
    if (text == POINT) {
        if (ch == text) {
            return CANNOT_ADD_POINT;
        }

        QString tempQstr = getNumberFromBack(qstrNow);

        if (tempQstr.contains(POINT) || tempQstr.contains(PERCENT)
            || isCorrectInt(getNumberFromBack(qstrNow)).first != 1) {
            return CANNOT_ADD_POINT;
        }

        qstrNow += text;

        return CORRECT_QSTR_ADD;
    }

    /* % */
    if (text == PERCENT) {
        if (ZERO_TO_NINE.contains(ch) || ch == ANS_END) {
            qstrNow += text;
            return CORRECT_QSTR_ADD;
        }

        return CANNOT_ADD_PERCENT;
    }

    /* 0~9 */
    if (ZERO_TO_NINE.contains(text)) {
        if (qstrNow.size() == 0 || qstrNow == ZERO || ch == ANS_END) {
            qstrNow = text;
            return CORRECT_QSTR_ADD;
        }

        if (ch == BRACKET_R || ch == PERCENT || ch == SCI_NUM_END) {
            return CANNOT_ADD_NUMBER;
        }

        /* 最后一个数为 0 时 , 如果输入为 0 错误 , 否则替换 0 */
        QString tempQstr = getNumberFromBack(qstrNow);

        if (tempQstr == ZERO) {
            if (text == ZERO) {
                return CANNOT_ADD_ZERO;
            }

            qstrNow.chop(1);
        }

        qstrNow += text;
        return CORRECT_QSTR_ADD;
    }

    /* function */
    if (FUNCTION_LIST.contains(text)) {
        if (formulaOrNum == 0 || formulaOrNum == 1 || ch == BRACKET_L) {
            qstrNow = qstrNow + text + BRACKET_L;
            return CORRECT_QSTR_ADD;
        }

        if (formulaOrNum == 2 || formulaOrNum == -2 || formulaOrNum == -1) {
            qstrNow = text + BRACKET_L + qstrNow + BRACKET_R;
            return CORRECT_QSTR_ADD;
        }

        return CANNOT_ADD_FUNCTION;
    }

    /* trans */
    if (TRANS_LIST.contains(text)) {
        if ((qstrNow.size() == 1 && qstrNow.left(1) == BRACKET_L)
            || qstrNow.size() >= 1 && qstrNow.right(1) == BRACKET_L) {
            return CANNOT_ADD_TRANS;
        }

        QStringList temp_trans = QString("x^(ｰ1),x^(2),x^(3),x^(,x^(1÷,x!").split(","); /*))*/
        QString temp = text;
        temp = temp_trans.at(TRANS_LIST.indexOf(temp));
        temp.remove(0, 1);

        if (formulaOrNum == 0) {
            qstrNow = ZERO + temp;
            return CORRECT_QSTR_ADD;
        }

        if (formulaOrNum == -1 || formulaOrNum == -2) {
            qstrNow = qstrNow + temp;
            return CORRECT_QSTR_ADD;
        }

        if (formulaOrNum == 2) {
            int size = qstrNow.size();
            if (qstrNow.right(4) == "^(2)" || qstrNow.right(4) == "^(3)") {
                if (BRACKET_R == qstrNow.at(size - 4 - 1)) {
                    int index = InputTools::findBracketPair(qstrNow, size - 4 - 1);
                    qstrNow = qstrNow.mid(0, index) + BRACKET_L + qstrNow.mid(index, size - index) + BRACKET_R + temp;
                    return CORRECT_QSTR_ADD;
                }
                qstrNow = BRACKET_L + qstrNow + BRACKET_R + temp;
                return CORRECT_QSTR_ADD;
            }
            if (qstrNow.right(1) == BRACKET_R) {
                int index = InputTools::findBracketPair(qstrNow, size - 1);
                if (QString("³√") == qstrNow.mid(index - 2, 2)) {
                    qstrNow = qstrNow.mid(0, index - 2) + BRACKET_L + QString("³√") + qstrNow.mid(index, size - index)
                              + BRACKET_R + temp;
                    return CORRECT_QSTR_ADD;
                }
                if (QString("√") == qstrNow.mid(index - 1, 1) && QString("³√") != qstrNow.mid(index - 2, 2)) {
                    qstrNow = qstrNow.mid(0, index - 1) + BRACKET_L + QString("√") + qstrNow.mid(index, size - index)
                              + BRACKET_R + temp;
                    return CORRECT_QSTR_ADD;
                }
                qstrNow = qstrNow + temp;
                return CORRECT_QSTR_ADD;
            }
            if (qstrNow.right(1) == SCI_NUM_END) {
                qstrNow = qstrNow + temp;
                return CORRECT_QSTR_ADD;
            }
            int index = InputTools::getNumLastIndex(qstrNow);

            qstrNow = qstrNow.mid(0, index) + BRACKET_L + qstrNow.mid(index, size - index) + BRACKET_R + temp;
            return CORRECT_QSTR_ADD;
        }

        return CANNOT_ADD_TRANS;
    }

    /* sci_num */
    if (SCI_NUM_LIST.contains(text)) {
        if (text == ANS && lastAns == EMPTY) {
            return ANS_IS_EMPTY;
        }

        if (qstrNow.size() == 0 || qstrNow.size() == 1 && qstrNow == ZERO || ch == ANS_END) {
            qstrNow.clear();
        }

        if (!qstrNow.size() || isArithmeticOperator(ch) || ch == BRACKET_L || ch == POWER_SYMBOL) {
            if (text == PI) {
                qstrNow = qstrNow + (BigFloat::PI).toQString() + SCI_NUM_END;
            } else if (text == E) {
                qstrNow = qstrNow + (BigFloat::E).toQString() + SCI_NUM_END;
            } else if (text == ANS) {
                qstrNow = qstrNow + lastAns + SCI_NUM_END;
            }

            return CORRECT_QSTR_ADD;
        }

        return CANNOT_ADD_SCI_NUM;
    }

    return INPUT_ERROR;
}

/* 对添加后的表达式进行处理 */
int InputProcess::qstrNowUpdate(const QString &text)
{
    QString qstrBack;
    QString qstrBack2;

    /* 最后一个字符 */
    if (qstrNow.size() >= 1) {
        qstrBack = QString(qstrNow.at(qstrNow.size() - 1));
    }

    /* 倒数第二个字符 */
    if (qstrNow.size() >= 2) {
        qstrBack2 = QString(qstrNow.at(qstrNow.size() - 2));
    }

    /* 最后一个字符为 清除 */
    if (qstrBack == CLEAN) {
        init();
        return CORRECT_QSTR_NOW_UPDATE;
    }

    /* 最后一个字符为 退格 */
    if (qstrBack == BACKSPACE) {
        if (qstrNow.size() <= 2) {
            init();
        } else {
            /* 移除退格符 */
            qstrNow.chop(1);

            /* 有末尾标识符的直接去掉整个数字 */
            if (QString(qstrNow.at(qstrNow.size() - 1)) == ANS_END) {
                qstrNow.chop(getAnsNumberLengthFromBack(qstrNow));
            } else if (QString(qstrNow.at(qstrNow.size() - 1)) == SCI_NUM_END) {
                qstrNow.chop(1);
                qstrNow.chop(getNumberLengthFromBack(qstrNow));
            } else {
                qstrNow.chop(1);

                /* 末尾是function */
                if (qstrNow.size() && FUNCTION_QSTR.contains(qstrNow.right(1))) {
                    while (qstrNow.size() && FUNCTION_QSTR.contains(qstrNow.right(1))) {
                        qstrNow.chop(1);
                    }
                }
            }

            if (qstrNow.size() && QString(qstrNow.right(1)) == POWER_SYMBOL) {
                qstrNow.chop(1);
            }
        }

        if (qstrNow.size() == 0) {
            init();
        }

        return CORRECT_QSTR_NOW_UPDATE;
    }

    /* 处理 " 3.+ " 这类情况 , 移除小数点*/
    if ((qstrNow.size() >= 2) && (isArithmeticOperator(qstrBack2) || qstrBack2 == POINT)
        && (isArithmeticOperator(qstrBack) || qstrBack == EQUAL)) {
        qstrNow.remove(qstrNow.size() - 2, 1);
        return CORRECT_QSTR_NOW_UPDATE;
    }

    // 4.
    if ((qstrNow.size() >= 2) && (isArithmeticOperator(qstrBack2) || isBracket(qstrBack2))
        && (qstrBack == POINT || qstrBack == PERCENT)) {
        qstrNow.insert(qstrNow.size() - 1, ZERO);
        return CORRECT_QSTR_NOW_UPDATE;
    }

    // 5.
    if (qstrNow.size() >= 2 && ZERO_TO_NINE.contains(qstrBack)) {
        if (qstrNow.size() == 2) {
            if (qstrBack2 == ZERO) {
                qstrNow.remove(0, 1);
                return qstrBack == ZERO ? CORRECT_QSTR_NOW_UPDATE : FAILED_QSTR_NOW_UPDATE;
            }

            return CORRECT_QSTR_NOW_UPDATE;
        }

        QString qstrBack3 = QString(qstrNow[qstrNow.size() - 3]);
        if (qstrBack2 == ZERO && (isArithmeticOperator(qstrBack3) || qstrBack3 == BRACKET_L)) {
            qstrNow.remove(qstrNow.size() - 2, 0);
            return qstrBack == ZERO ? CORRECT_QSTR_NOW_UPDATE : FAILED_QSTR_NOW_UPDATE;
        }

        return CORRECT_QSTR_NOW_UPDATE;
    }

    return CORRECT_QSTR_NOW_UPDATE;
}

int InputProcess::qstrUpdate(const QString &text)
{
    if (!calAns.size()) {
        return FAILED_QSTR_UPDATE;
    }

    if (text == EQUAL) {
        calHistory.append(clearANS_END_and_SCI_NUM_END(isCorrectFormula(qstrNow).second + EQUAL + calAns));
        qstrPrepare.clear();
        qstrNow = calAns + ANS_END;
        lastAns = calAns;
    }

    qstrPrepare = calAns;

    return CORRECT_QSTR_UPDATE;
}

void InputProcess::labelUpdate()
{
    // updatelabelNow
    QStringList qstrList;
    qstrList = formulaSplit(qstrNow);

    for (int i = 0; i < qstrList.size(); i++) {
        if (ZERO_TO_NINE.contains(QString(qstrList.at(i).at(0))))
            qstrList[i] = addComma(qstrList[i]);
    }
    labelNow = qstrList.join("");

    // updatelabelPrepare
    qstrList = formulaSplit(qstrPrepare);
    // qDebug () << qstrList;
    for (int i = 0; i < qstrList.size(); i++) {
        if (ZERO_TO_NINE.contains(QString(qstrList.at(i).at(0))))
            qstrList[i] = addComma(qstrList[i]);
    }
    labelPrepare = qstrList.join("");
}

void InputProcess::graphNodeUpdate(const QString &text)
{
    if (DEBUG_MODE) {
        qDebug() << "im in InputProcess::graphNodeUpdate";
    }

    // 1.
    if ((G.getStatus(STANDARD) && !G.getStatus(ERROR))
        || (G.getStatus(SCIENTIFIC) && (!G.getStatus(ERROR_INF) || !G.getStatus(ERROR_NAN)))) {

        // 2.
        if (text == CLEAN || text == BACKSPACE) {

            if (G.getStatus(STANDARD))
                G.updateAllSon(STANDARD, 1);
            else if (G.getStatus(SCIENTIFIC)) {
                G.updateAllSon(SCIENTIFIC, 1);
            }
        }
        return;
    }

    if (text == EQUAL) {
        if (G.getStatus(SCIENTIFIC)) {
            if (calAns.contains(INF_SYMBOL)) {
                G.update(ERROR_INF, false);
            } else if (calAns.contains(NAN_SYMBOL)) {
                G.update(ERROR_NAN, false);
            }
        }

        else if (G.getStatus(STANDARD)) {
            if (calAns.contains(INF_SYMBOL) || calAns.contains(NAN_SYMBOL)) {

                G.update(ERROR, false);
            }
        }
        return;
    }
    return;
}

QVector<QString> InputProcess::output(const QString &text)
{
    if ((i_qstrAddFlag != -1 && i_qstrAddFlag != CORRECT_QSTR_ADD)
        || (i_qstrNowUpdateFlag != -1 && i_qstrNowUpdateFlag != CORRECT_QSTR_NOW_UPDATE)) {
        QVector<QString> error_res = res;
        error_res[DISPLAY_ON_LABEL_PREPARE] = "input Error!";
        error_res[LATEST_HISTORY].clear();
        return error_res;
    }

    labelUpdate();
    graphNodeUpdate(text);

    if (calAns.size() == 0) {
        res[DISPLAY_ON_LABEL_PREPARE].clear();
        res[LABEL_PREPARE_CAL_QSTR].clear();
        res[LATEST_HISTORY].clear();
        res[CAL_ANS].clear();
    }


    res[DISPLAY_ON_LABEL_NOW] = clearANS_END_and_SCI_NUM_END(labelNow);
    res[LABEL_NOW_CAL_QSTR] = clearANS_END_and_SCI_NUM_END(qstrNow);
    res[CAL_ANS] = text == EQUAL ? clearANS_END_and_SCI_NUM_END(calAns) : EMPTY;
    res[LATEST_HISTORY] =
        (text == EQUAL && calHistory.size() && i_couldBeCalFlag) ? calHistory.at(calHistory.size() - 1) : EMPTY;
    res[DISPLAY_ON_LABEL_PREPARE] = clearANS_END_and_SCI_NUM_END(labelPrepare);
    res[LABEL_PREPARE_CAL_QSTR] = clearANS_END_and_SCI_NUM_END(qstrPrepare);

    return res;
}

void InputProcess::init_inputFromButton()
{
    qstrPrepare = EMPTY;
    labelNow = EMPTY;
    labelPrepare = EMPTY;
    calAns = EMPTY;

    i_couldBeCalFlag = -1;
    i_qstrAddFlag = -1;
    i_qstrNowUpdateFlag = -1;
    i_qstrUpdateFlag = -1;
}

/* 对输入数据进行处理 , 返回需要显示的数据 */
QVector<QString> InputProcess::inputFromButton(const QString &text)
{
    init_inputFromButton();

    if (!text.size())
        return res;

    /* 计算标志位处理 */
    if (text == STANDARD || text == SCIENTIFIC || text == DEG_SYMBOL || text == RAD_SYMBOL || text == EXCHANGE_RATE) {
        if (text == STANDARD || text == EXCHANGE_RATE) {
            G.updateAllSon(SCIENTIFIC, 0);
            init();
            G.updateAllSon(STANDARD, 1);
            calHistory.clear();
            return res;
        }

        if (text == SCIENTIFIC) {
            G.updateAllSon(STANDARD, 0);
            init();
            G.updateAllSon(SCIENTIFIC, 1);
            calHistory.clear();
            return res;
        }

        if (text == DEG_SYMBOL || text == RAD_SYMBOL) {
            QStringCalculator::cal(text);
            res[LATEST_HISTORY].clear();
            return res;
        }
    }

    if (!G.find(text)) {
        if (DEBUG_MODE) {
            qDebug() << "find text:" << text;
        }

        res[LATEST_HISTORY].clear();

        return res;
    }

    i_qstrAddFlag = qstrAdd(text);

    if (DEBUG_MODE) {
        qDebug() << "AfterQstrAddFlag:" << qstrNow;
        qDebug() << i_qstrAddFlag;
    }

    if (i_qstrAddFlag != CORRECT_QSTR_ADD) {
        return output(text);
    }

    /* 对添加后的表达式进行处理 */
    i_qstrNowUpdateFlag = qstrNowUpdate(text);

    if (DEBUG_MODE) {
        qDebug() << "AfterQstrNowUpdate:" << qstrNow;
        qDebug() << i_qstrNowUpdateFlag;
    }

    if (i_qstrNowUpdateFlag != CORRECT_QSTR_NOW_UPDATE) {
        return output(text);
    }

    if (qstrNow.right(1) == EQUAL) {
        qstrNow.chop(1);
    }

    /* 检测表达式是否可以进行计算 */
    i_couldBeCalFlag = couldBeCal(qstrNow);

    if (i_couldBeCalFlag == COULD_BE_CAL) {
        calAns = QStringCalculator::cal(qstrNow);
        i_qstrUpdateFlag = qstrUpdate(text);

        if (i_qstrUpdateFlag != CORRECT_QSTR_UPDATE) {
            return output(text);
        }
    }

    return output(text);
}

QVector<QString> InputProcess::inputFromButton(const char *text)
{
    return inputFromButton(QString(text));
}
