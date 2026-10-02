#define DEBUG_MODE false

#include "QStringCalculator.h"

QString QStringCalculator::angelSymbol = DEG_SYMBOL;

/* 获得符号的优先性 */
int QStringCalculator::getOperatotPriority(const QString qstr)
{
    if (qstr == ADD || qstr == SUB)
        return 0;

    if (qstr == MUL || qstr == DIV)
        return 1;

    if (isBracket(qstr))
        return -1; /* 注意，这里将括号设为最低优先级，因此括号不会被弹出，除非遇到右括号 */
}

/* 判断符号的优先性 */
void QStringCalculator::postfixCheck(const QString &qstr, QStack<QString> &operatorList,
                                     QStringList &postfixExpressions)
{
    if (operatorList.empty()) {
        operatorList.push(qstr);
        return;
    }

    if (isBracket(qstr)) {
        if (qstr == BRACKET_L) {
            operatorList.push(qstr);
        } else {
            /* 弹出所有元素直到遇到左括号 */
            while (operatorList.top() != BRACKET_L) {
                QString t_qstr = operatorList.top();
                postfixExpressions.append(t_qstr);
                operatorList.pop();
            }

            /* 当遇到左括号时，弹出但不加入postfixExpressions(后缀表达式中) */
            operatorList.pop();
        }
    } else {
        /* 如果不是括号 */
        /* 取出栈顶元素 ，与当前符号进行优先性比较 */
        QString sym = operatorList.top();

        /* 比较两符号的优先性 */
        if (getOperatotPriority(qstr) <= getOperatotPriority(sym)) {
            /* 如果c的优先性比栈顶符号小或等于，弹出栈顶元素 */
            operatorList.pop();
            /* 并将其压入postfixExpressions（后缀表达式）中 */
            postfixExpressions.push_back(sym);
            /* 递归调用postfixCheck , 比较当前符号qstr与下一个栈顶符号的优先性 */
            postfixCheck(qstr, operatorList, postfixExpressions);
        } else {
            /* 如果qstr比栈顶符号优先级大，那将qstr压入operatorList(操作符栈)中 */
            operatorList.push(qstr);
        }
    }

    return;
}

/* 从infixExpression中取出元素，分配元素到operatorList和postfixExpressions中 */
void QStringCalculator::infixToPostfix(QStringList &infixExpression, QStringList &postfixExpressions)
{
    QStack<QString> operatorList;

    while (!infixExpression.empty()) {
        QString qstr = infixExpression.at(0);
        infixExpression.removeAt(0);

        if (isNumber(qstr)) {
            postfixExpressions.append(qstr);
        } else {
            /* 处理符号优先级 */
            postfixCheck(qstr, operatorList, postfixExpressions);
        }
    }

    /* 如果输入结束，将operatorList的元素全部弹出，加入后缀表达式中 */
    while (!operatorList.empty()) {
        QString c = operatorList.top();
        postfixExpressions.push_back(c);
        operatorList.pop();
    }
}

/* 计算后缀表达式 */
void QStringCalculator::calPostfix(QStringList &postfixExpressions, QStack<BigFloat> &calAns)
{
    while (!postfixExpressions.empty()) {
        QString qstr = postfixExpressions.at(0);
        postfixExpressions.removeAt(0);

        /* 如果是操作数 , 压入栈中 */
        if (ZERO_TO_NINE.contains(qstr.right(1))) {
            BigFloat op(qstr);
            calAns.push(op);
        } else {
            BigFloat op1("0");
            BigFloat op2("0");
            /* 如果是操作符，从栈中弹出元素进行计算 */
            if (0 < calAns.count()) {
                op1 = calAns.top();
                calAns.pop();
            }
            if (0 < calAns.count()) {
                op2 = calAns.top();
                calAns.pop();
            }

            if (qstr == ADD) {
                calAns.push(op2 + op1);
            } else if (qstr == SUB) {
                calAns.push(op2 - op1);
            } else if (qstr == MUL) {
                calAns.push(op2 * op1);
            } else if (qstr == DIV) {
                calAns.push(op2 / op1);
            }
        }
    }
}

/* 阶乘计算 */
/* 5^(3) */
QString QStringCalculator::transCalculator(const QStringList &expression, const int idx)
{
    BigFloat ans_l;
    BigFloat ans_r;

    ans_l = BigFloat(idx == 1 ? expression[0] : qstrListCalculator(expression.mid(1, idx - 2)));
    ans_r = BigFloat(idx == expression.size() - 2
                         ? expression[expression.size() - 1]
                         : qstrListCalculator(expression.mid(idx + 2, expression.size() - 3 - idx)));

    if (ans_l == BigFloat(0)) {
        if (ans_r == BigFloat(0)) {
            return ONE;
        } else if (ans_r > BigFloat(0)) {
            return ZERO;
        } else if (ans_r < BigFloat(0)) {
            return NAN_SYMBOL;
        }
    }

    return (ans_l ^ ans_r).toQString();
}

/* 计算函数 , log(2+3) */
QString QStringCalculator::functionCalculator(const QStringList &expression)
{
    QString func;
    BigFloat ans;
    ans = BigFloat(qstrListCalculator(expression.mid(2, expression.size() - 3)));
    func = expression[0];

    if (func == SIN) {
        ans = BigFloat::Sin(ans, angelSymbol);
    } else if (func == COS) {
        ans = BigFloat::Cos(ans, angelSymbol);
    } else if (func == TAN) {
        ans = BigFloat::Tan(ans, angelSymbol);
    } else if (func == ARCSIN) {
        ans = BigFloat::ASin(ans, angelSymbol);
    } else if (func == ARCCOS) {
        ans = BigFloat::ACos(ans, angelSymbol);
    } else if (func == ARCTAN) {
        ans = BigFloat::ATan(ans, angelSymbol);
    } else if (func == LOG) {
        ans = BigFloat::Lg(ans);
    } else if (func == LN) {
        ans = BigFloat::Ln(ans);
    } else if (func == SQUARE_ROOT) {
        BigFloat ans_l = ans;
        BigFloat ans_r = BigFloat(1) / BigFloat(2);
        ans = (ans_l ^ ans_r).toQString();
    } else if (func == CUBE_ROOT) {
        BigFloat ans_l = ans;
        BigFloat ans_r = BigFloat(1) / BigFloat(3);
        ans = (ans_l ^ ans_r).toQString();
    }

    return ans.toQString();
}

void QStringCalculator::formulaToInfixExpression(QStringList &infixExpression)
{
    for (int i = 0; i < infixExpression.size(); i++) {
        QString qstr = infixExpression.at(i);

        /* 处理% */
        if (isNumber(qstr)) {
            if (qstr.right(1) == PERCENT) {
                infixExpression[i] = (BigFloat(qstr.left(qstr.size() - 1)) / BigFloat(100)).toQString();
            }
        } else if (isFunction(qstr)) {
            /* 计算函数 */
            int idx = findBracketPair(infixExpression, i + 1);
            QString temp_qstr = functionCalculator(infixExpression.mid(i, idx - i + 1));

            /* 用计算结果代替函数 */
            for (int j = i + 1; j <= idx; j++) {
                infixExpression.removeAt(i + 1);
            }

            infixExpression.replace(i, temp_qstr);

            i--;
        } else if (qstr == POWER_SYMBOL) {
            /* 计算阶乘 */
            int idx_l;
            int idx_r;

            if (infixExpression.at(i - 1) == BRACKET_R) {
                idx_l = findBracketPair(infixExpression, i - 1);
            } else {
                idx_l = i - 1;
            }
            if (infixExpression.at(i + 1) == BRACKET_L) {
                idx_r = findBracketPair(infixExpression, i + 1);
            } else {
                idx_r = i + 1;
            }

            QString temp_qstr;
            temp_qstr = transCalculator(infixExpression.mid(idx_l, idx_r - idx_l + 1), i - idx_l);

            for (int j = idx_l + 1; j <= idx_r; j++) {
                infixExpression.removeAt(idx_l + 1);
            }

            infixExpression.replace(idx_l, temp_qstr);

            i = idx_l - 1;
        } else if (qstr == FACTORIAL_SYMBOL) {
            /* 计算阶乘 */
            if (infixExpression[i - 1] == BRACKET_R) {
                int idx = findBracketPair(infixExpression, i - 1);
                infixExpression[idx] =
                    BigFloat::Fact(BigFloat(qstrListCalculator(infixExpression.mid(idx + 1, i - 2 - idx)))).toQString();

                for (int j = idx + 1; j <= i; j++) {
                    infixExpression.removeAt(idx + 1);
                }
                i = idx;
            } else {
                infixExpression[i - 1] = BigFloat::Fact(BigFloat(infixExpression[i - 1])).toQString();
                infixExpression.removeAt(i);
                i--;
            }
        }
    }

    return;
}

QString QStringCalculator::qstrListCalculator(const QStringList &formulaList)
{
    QStringList infixExpression;    /* 盛放中缀表达式 */
    QStringList postfixExpressions; /* 盛放后缀表达式 */
    QStack<BigFloat> calAns;        /* 计算后缀表达式的辅助容器 */

    infixExpression = formulaList;

    /* 处理首字符为 - */
    if (infixExpression[0] == SUB) {
        infixExpression.insert(0, ZERO);
    }

    /* 处理 % , 函数 , 乘方 , 阶乘 */
    formulaToInfixExpression(infixExpression);

    foreach (QString ch, infixExpression) {
        if (ch.contains(INF_SYMBOL) || ch.contains(NAN_SYMBOL)) {
            return ch;
        }
    }

    infixToPostfix(infixExpression, postfixExpressions);

    /* 计算后缀表达式 */
    calPostfix(postfixExpressions, calAns);

    QString ans = calAns.top().toQString();

    return ans;
}

QString QStringCalculator::cal(const QString &qstr)
{
    QString formula = qstr;

    /* deg rad */
    if (formula == InputSymbols::DEG_SYMBOL || formula == InputSymbols::RAD_SYMBOL) {
        angelSymbol = formula;
        return formula;
    }

    /* 为空 */
    if (formula.size() == 0) {
        qWarning() << "The expression is empty!";
        return EMPTY;
    }

    /* 去掉ANS_END */
    formula = clearANS_END_and_SCI_NUM_END(formula);

    /* 去掉末尾符号 */
    while (isArithmeticOperator(formula.right(1))) {
        formula.chop(1);
    }

    /* 去除等号 */
    while (formula.right(1) == EQUAL) {
        formula.chop(1);
    }

    /* 括号检测 */
    if (!allBracketMatchCorrectly(formula)) {
        formula = bracketCompletion(formula);
    }

    /* 表达式检测 */
    formula = isCorrectFormula(formula).second;

    /* 表达式分割 , 检测 */
    QStringList formulaList = formulaSplit(formula);
    formulaList = isCorrectFormulaList(formulaList).second;

    /* 计算 */
    QString ans = qstrListCalculator(formulaList);

    /* 去掉末尾0 */
    if (ans.contains(POINT) && !ans.contains(SCIENTIFIC_NOTATION)) {
        while (ans[ans.size() - 1] == ZERO[0]) {
            ans.chop(1);
        }

        if (ans[ans.size() - 1] == POINT[0]) {
            ans.chop(1);
        }
    }

    /* 处理减号 */
    while (ans.contains(SUB)) {
        ans.replace(ans.indexOf(SUB), 1, '-');
    }

    /* 精度处理 */
    if (ans.contains(POINT)) {
        if ((ans.size() - ans.indexOf(POINT) > PRECISION) || (ans.indexOf(POINT) > PRECISION)) {
            ans = QString::number(ans.toDouble(), 'g', PRECISION);
        }
    } else {
        if (ans.size() > PRECISION) {
            ans = QString::number(ans.toDouble(), 'g', PRECISION);
        }
    }

    while (ans.contains('-')) {
        ans.replace(ans.indexOf('-'), 1, SUB[0]);
    }

    return ans;
}
