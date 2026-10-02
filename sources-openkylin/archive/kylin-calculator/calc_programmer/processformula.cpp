#include "processformula.h"

ProcessFormula *ProcessFormula::getInstance(void)
{
    static ProcessFormula *instance = NULL;
    if (instance == NULL) {
        instance = new ProcessFormula();
        instance->setObjectName("kylin-calculator_ProcessFormula_instance");
        return instance;
    }

    return instance;
}

QList<QString> ProcessFormula::process(QString formula)
{
    // 记录用于记录计算的表达式
    m_resultVec[MARK_CURRENT_EXPRESSION] = formula;

    // 处理用于计算的表达式
    QStringList formulaList = handleExpression(formula);

    Calc::getInstance()->setBase(m_base);
    // 计算表达式
    QList<QString> resultList = Calc::getInstance()->cal(formulaList);

    if ("ERROR" == resultList[ERROR_FLAG]) {
        // 计算错误，处理错误信息
        m_resultVec[MARK_IS_RIGHT] = QString("FALSE");
        if ("CAL_FALSE" == resultList[ERROR_TYPE]) {
            m_resultVec[MARK_DISPLY_EXPRESSION] = setSymbol(formula);
            m_resultVec[MARK_BUDGET_RESULTS] = resultList[ERROR_INFO];
            m_resultVec[MARK_HISTOR_RECORDS].clear();
            m_resultVec[MARK_CAL_RESULTS] =  resultList[ERROR_INFO];
            m_resultVec[MARK_BIN_RESULTS] = QString("0");
            m_resultVec[MARK_CODE_RESULTS] = QString("");
            m_resultVec[MARK_IS_INPUT] = QString("FALSE");//resultList[ERROR_TYPE];
            return m_resultVec;
        }
        if ("NUM_FALSE" == resultList[ERROR_TYPE]) {
            // 设置返回结果
            m_resultVec[MARK_DISPLY_EXPRESSION] = setSymbol(formula.mid(0, formula.size() - 1));
            m_resultVec[MARK_BUDGET_RESULTS] = resultList[ERROR_INFO];
            m_resultVec[MARK_CURRENT_EXPRESSION].chop(1);
            m_resultVec[MARK_HISTOR_RECORDS].clear();
            //m_resultVec[MARK_CAL_RESULTS] =  m_resultVec[MARK_CURRENT_EXPRESSION];
            m_resultVec[MARK_BIN_RESULTS] = QString("0");
            m_resultVec[MARK_CODE_RESULTS] = QString("");
            m_resultVec[MARK_IS_INPUT] = QString("FALSE");
            return m_resultVec;
        }
        if ("FALSE" == resultList[ERROR_TYPE]) {
            // FALSE 表示该错误出现后，后面无法在继续输入， TRUE 表示该错误出现后，后面可以继续输入
            // 设置返回结果
            m_resultVec[MARK_DISPLY_EXPRESSION] = setSymbol(formula.mid(0, formula.size() - 1));
            m_resultVec[MARK_BUDGET_RESULTS] = resultList[ERROR_INFO];
            m_resultVec[MARK_CURRENT_EXPRESSION].chop(1);
            m_resultVec[MARK_HISTOR_RECORDS].clear();
            m_resultVec[MARK_CAL_RESULTS] =  resultList[ERROR_INFO];
            m_resultVec[MARK_BIN_RESULTS] = QString("0");
            m_resultVec[MARK_CODE_RESULTS] = QString("");
            m_resultVec[MARK_IS_INPUT] = QString("FALSE");
            return m_resultVec;
        }
        // 后续可以继续输入，设置返回结果
        m_resultVec[MARK_DISPLY_EXPRESSION] = setSymbol(formula);
        m_resultVec[MARK_BUDGET_RESULTS].clear();
        m_resultVec[MARK_HISTOR_RECORDS].clear();
        m_resultVec[MARK_CAL_RESULTS] = resultList[ERROR_INFO];
        m_resultVec[MARK_BIN_RESULTS] = QString("0");
        m_resultVec[MARK_CODE_RESULTS] = QString("");
        m_resultVec[MARK_IS_INPUT] = QString("TRUE");;

        return m_resultVec;
    } else {
        // 正确计算结果
        // 转换为原有进制
        QString result = Conversion::getInstance()->decTo(resultList[RIGHT_RESULT], m_base);
        // 设置返回结果
        m_resultVec[MARK_DISPLY_EXPRESSION] = setSymbol(formula);
        m_resultVec[MARK_BUDGET_RESULTS] = result;
        m_resultVec[MARK_HISTOR_RECORDS] = setHistory(setSymbol(formula), result);
        m_resultVec[MARK_CAL_RESULTS] = result;
        m_resultVec[MARK_BIN_RESULTS] = Conversion::getInstance()->decTo(resultList[RIGHT_RESULT],2);
        m_resultVec[MARK_CODE_RESULTS] = Conversion::getInstance()->setCode(result, m_code, m_base);
        m_resultVec[MARK_IS_INPUT] = QString("TRUE");
        m_resultVec[MARK_IS_RIGHT] = QString("TRUE");
    }
    return m_resultVec;
}

QString ProcessFormula::baseHandle(QString value, int lastBase)
{
    // 进制转换
    QString baseValue = Conversion::getInstance()->otherToDec(value, m_base);
    baseValue = Conversion::getInstance()->decTo(baseValue, lastBase);

    // 设置当前进制
    m_base = lastBase;
    Calc::getInstance()->setBase(lastBase);

    return baseValue;
}

QString ProcessFormula::bin(QString value)
{
    // 进制转换
    QString baseValue = Conversion::getInstance()->otherToDec(value, m_base);
    baseValue = Conversion::getInstance()->decTo(baseValue, BASE_BIN);

    return baseValue;
}

void ProcessFormula::digitHandle(int digit)
{
    // 设置当前位数
    Conversion::getInstance()->setDigit(digit);
}

QString ProcessFormula::legal(QString value)
{
    // 合法化
    return Conversion::getInstance()->setLegal(value, m_base);
}

void ProcessFormula::setCode(QString code)
{
    // 设置当前字符编码
    m_code = code;
}

QString ProcessFormula::code(QString value)
{
    // 返回当前数值对应字符
    return Conversion::getInstance()->setCode(value, m_code, m_base);
}

bool ProcessFormula::containsBinarry(QString binarry)
{
    return m_binaryMap.contains(binarry);
}

bool ProcessFormula::containsUnary(QString unary)
{
    return m_unaryMap.contains(unary);
}

int ProcessFormula::opNum(QString formula)
{
    int opNum = 0;
    for (int i = 0; i < formula.size(); i++) {
        if(m_unaryMap.contains(QString(formula.at(i))) ||
             m_binaryMap.contains(QString(formula.at(i))) ||
             QString("(") == QString(formula.at(i)) ||
             QString(")") == QString(formula.at(i))) {
            opNum++;
        }
    }

    return opNum;
}

QStringList ProcessFormula::handleExpression(QString formula)
{
    QString operand; // 操作数临时容器
    operand.clear();
    QStringList formulaList; // 处理后的表达式

    for (int i = 0; i < formula.size(); i++) {
        if(m_unaryMap.contains(QString(formula.at(i))) ||
             m_binaryMap.contains(QString(formula.at(i))) ||
             QString("(") == QString(formula.at(i)) ||
             QString(")") == QString(formula.at(i))) {
            // 操作符或者括号
            // 将前面的操作数放入formulaList
            if (!operand.isEmpty()) {
                // 放入表达式
                formulaList << operand;
            }

            // 然后放操作符
            formulaList << formula.at(i);

            // 需要清空operand用来存放下一个操作数
            operand.clear();
        } else {
            // 操作数
            operand.append(formula.at(i));
        }
    }

    if (!operand.isEmpty()) {
        // 循环结束，将最后的操作数放入表达式
        formulaList << operand;
    }

    return formulaList;
}

QString ProcessFormula::setBrackets(QString formula)
{
    // 左括号和右括号的差数
    int bracketDif = 0;
    for (int i = 0; i < formula.size(); i++) {
        if (QString("(") == QString(formula.at(i))) {
            bracketDif++;
        }
        if (QString(")") == QString(formula.at(i))) {
            bracketDif--;
        }
    }
    if (bracketDif > 0) {
        // 左括号多了，补齐右括号
        for (int i = bracketDif; i > 0; i--) {
            formula.append(QString(")"));
        }
    }

    return formula;
}

QString ProcessFormula::setSymbol(QString formula)
{
    QString str;
    for (int i = 0; i < formula.size(); i++) {
        if (m_binaryMap.contains(formula.at(i))) {
           // 双目运算符直接添加
           str.append(m_binaryMap.find(formula.at(i)).value());
           continue;
        }
        if (m_unaryMap.contains(formula.at(i))) {
            str = setUnary(str, m_unaryMap.find(formula.at(i)).value());
            continue;
        }
        str.append(formula.at(i));
    }

    return str;
}

QString ProcessFormula::setUnary(QString formula, QString unary)
{
    int braIndex = -1;
    bool isHaveRbra = false;
    for (int i = formula.size() - 1; i >= 0; i--) {
        QString str = formula.at(i);
        if (QString(")") == str) {
            isHaveRbra = true;
            continue;
        }
        if (QString("(") == str) {
            if (false == isHaveRbra) {
                braIndex = i;
                break;
            }
            braIndex = -1;
        }
    }
    return formula.mid(0, braIndex + 1) + unary + QString("(") + formula.mid(braIndex + 1, formula.size() - braIndex) + QString(")");
}

QString ProcessFormula::setHistory(QString formula, QString result)
{
    // 补齐括号
    formula = setBrackets(formula);

    return formula + QString("=") + result;
}

void ProcessFormula::clearVec()
{
    m_resultVec[MARK_CURRENT_EXPRESSION] = QString("0");
    m_resultVec[MARK_DISPLY_EXPRESSION] = QString("0");
    m_resultVec[MARK_BUDGET_RESULTS] = QString("0");
    m_resultVec[MARK_HISTOR_RECORDS] = QString("0");
    m_resultVec[MARK_CAL_RESULTS] = QString("0");
    m_resultVec[MARK_BIN_RESULTS] = QString("0");
    m_resultVec[MARK_CODE_RESULTS] = QString("");
    m_resultVec[MARK_IS_INPUT] = QString("TRUE");
    m_resultVec[MARK_IS_RIGHT] = QString("TRUE");
}
