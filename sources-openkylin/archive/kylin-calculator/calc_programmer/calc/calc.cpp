#include "calc.h"

Calc *Calc::getInstance(void)
{
    static Calc *instance = NULL;
    if (instance == NULL) {
        instance = new Calc();
        instance->setObjectName("kylin-calculator_Calc_instance");
        return instance;
    }

    return instance;
}

QList<QString> Calc::cal(QStringList strList)
{
    QStringList infList;   // 中缀表达式
    QStringList postfList; // 后缀表达式
    QString result;        // 计算结果

    infList = strList;

    // 初始化符号优先级
    getPriority();

    // 中缀表达式转后缀表达式
    bool isInfToPostf = infToPostf(infList, postfList, result);

    if (!isInfToPostf) {
        qInfo() << "Expression processing error!";
        // 中缀转后缀错误直接返回错误信息
        m_resultVec[ERROR_FLAG] = setError(result).value(ERROR_FLAG);
        m_resultVec[ERROR_TYPE] = setError(result).value(ERROR_TYPE);
        m_resultVec[ERROR_INFO] = setError(result).value(ERROR_INFO);
        m_resultVec[RIGHT_RESULT] = "";

        return m_resultVec;
    }

    // 中缀转后缀正确，计算后缀
    result = calPostf(postfList);

    if ("ERROR" == setError(result).value(ERROR_FLAG)) {
        qInfo() << "Expression evaluation error!";
        // 计算错误，返回错误信息
        m_resultVec[ERROR_FLAG] = setError(result).value(ERROR_FLAG);
        m_resultVec[ERROR_TYPE] = setError(result).value(ERROR_TYPE);
        m_resultVec[ERROR_INFO] = setError(result).value(ERROR_INFO);
        m_resultVec[RIGHT_RESULT] = "";

        return m_resultVec;
    }

    // 计算正确，设置返回正确结果
    clear();
    m_resultVec[RIGHT_RESULT] = result;

    return m_resultVec;
}

void Calc::setBase(int base)
{
    m_base = base;
}

void Calc::getPriority()
{
    m_opPriority.clear();

    m_opPriority["+"] = 2;
    m_opPriority["s"] = 2;
    m_opPriority["*"] = 3;
    m_opPriority["/"] = 3;
    m_opPriority["&"] = 1;
    m_opPriority["|"] = 1;
    m_opPriority["^"] = 1;
    m_opPriority["N"] = 1;
    m_opPriority["<"] = 3;
    m_opPriority[">"] = 3;
    m_opPriority["("] = 0;
}

bool Calc::infToPostf(QStringList &infList, QStringList &postfList, QString &result)
{
    // 表达式为空
    if (infList.isEmpty()) {
        qWarning() << "The expression is empty!";
        result = QString("ERROR:TREU:" + tr("The expression is empty!"));
        return false;
    }

    postfList.clear();

    // 操作数与操作符的数量关系始终为1
    int difNumOp = 0;

    // 符号栈
    QStack<QString> opStack;

    for (int i = 0; i < infList.size(); i++) {
        QString str = infList.value(i);
        if (QString("(") == str) {
            if (opStack.empty() && postfList.empty()) {
                // 前面无字符，则直接入栈
                opStack.push(str);
                continue;
            }
            if (!m_unary.contains(infList.value(i - 1)) && !m_binary.contains(infList.value(i - 1))) {
                qWarning() << "Expression error!";
                result = QString("ERROR:FALSE:" + tr("Expression error!"));
                return false;
            }
            // 左括号前面是操作符，直接入符号栈
            opStack.push(str);
            continue;
        }
        if (QString(")") == str) {
            if (opStack.empty()) {
                // 栈为空，不能直接输入右括号
                qWarning() << "Missing left parenthesis!";
                result = QString("ERROR:FALSE:" + tr("Missing left parenthesis!"));
                return false;
            }
            if (QString("(") == infList.value(i - 1)) {
                // 左括号后面不能直接输入右括号，错误
                qWarning() << "The right bracket cannot be entered directly after the left bracket!";
                result = QString("ERROR:FALSE:" + tr("Expression error!"));
                return false;
            }
            if (m_binary.contains(infList.value(i - 1))) {
                // 双目运算符后面不能直接输入右括号，错误
                qWarning() << "You cannot directly enter a closing bracket after a binocular operator!";
                result = QString("ERROR:FALSE:" + tr("Expression error!"));
                return false;
            }
            // 右括号
            // 弹出所有元素直到遇到左括号
            while (QString("(") != opStack.top()) {
                QString top = opStack.top(); // 栈顶元素
                // 将栈顶元素放入后缀表达式
                postfList << top;
                // 弹出栈顶元素
                opStack.pop();

                if (opStack.empty()) {
                    // 栈为空，不能直接输入右括号
                    qWarning() << "Missing left parenthesis!";
                    result = QString("ERROR:FALSE:" + tr("Missing left parenthesis!"));
                    return false;
                }
            }
            // 循环结束，遇到左括号，弹出但不加入后缀表达式中
            opStack.pop();
            continue;
        }
        if (m_unary.contains(str)) {
            if (m_binary.contains(infList.value(i - 1))
                || QString("(") == infList.value(i - 1)) { //  || QString(")") == infList.value(i - 1)
                // 操作符过多
                qWarning() << "Too many operators!";
                result = QString("ERROR:FALSE:" + tr("Expression error!"));
                return false;
            }
            // 单目运算符，直接将栈中所有非“(”元素弹出放入后缀表达式中
            while (!opStack.empty()) {
                if (QString("(") == opStack.top()) {
                    break;
                }
                postfList << opStack.top();
                opStack.pop();
            }
            // 将单目运算符放入后缀
            postfList << str;
            continue;
        }
        if (m_binary.contains(str)) {
            if (("+" == str || QString("s") == str) && QString("(") == infList.value(i - 1)) {
                // 将减号当做负号使用
                postfList << 0;
                opStack.push(str);
                continue;
            }
            // 双目运算符，正常处理
            difNumOp--;

            if (difNumOp < 0) {
                // 操作符过多
                qWarning() << "Too many operators!";
                result = QString("ERROR:FALSE:" + tr("Expression error!"));
                return false;
            }
            if (opStack.empty()) {
                // 若为空栈，则直接入栈
                opStack.push(str);
                continue;
            }
            if (m_opPriority[str] > m_opPriority[opStack.top()]) {
                // 如果当前操作符优先级高于栈顶操作符优先级
                // 则直接入栈
                opStack.push(str);
                continue;
            }
            // 否则弹出栈中优先级大于等于当前操作符优先级的操作符，并最后将当前操作符压栈
            while (!opStack.empty() && m_opPriority[opStack.top()] >= m_opPriority[str]
                   && QString("(") != opStack.top()) {
                postfList << opStack.top();
                opStack.pop();
            }
            opStack.push(str);
            continue;
        }

        difNumOp++;
        // 操作数直接放入后缀表达式中
        if (!(Conversion::getInstance()->isLegitimate(str, m_base))) {
            // 操作数不合法
            qWarning() << "Illegal operand!";
            result = QString("ERROR:NUM_FALSE:" + tr("Input error!"));
            return false;
        }
        if (BASE_DEC != m_base) {
            // 将其他进制转换为十进制
            str = Conversion::getInstance()->otherToDec(str, m_base);
        }
        postfList << str;
    }

    if (0 == difNumOp) {
        // 操作数与操作符不匹配
        qWarning() << "Operand does not match operator!";
        result = QString("ERROR:TRUE:" + tr("Miss operand!"));
        return false;
    }

    // 符号栈不为空，将剩余符号依次放入后缀表达式中
    while (!opStack.empty()) {
        if (QString("(") != opStack.top()) {
            postfList << opStack.top();
        }
        opStack.pop();
    }

    return true;
}

QString Calc::calPostf(QStringList &postfList)
{
    QStack<QString> numStack; // 操作数栈
    numStack.clear();

    for (int i = 0; i < postfList.size(); i++) {
        // 从后缀取元素
        QString str = postfList.value(i);

        if (m_binary.contains(str)) {
            // 双目运算符
            if (numStack.isEmpty()) {
                // 表达式错误
                qWarning() << "Expression error!";
                return QString("ERROR:FALSE:" + tr("Expression error!"));
            }

            // 从操作数栈中取两个操作数
            QString num1 = numStack.top();
            numStack.pop();
            QString num2 = numStack.top();
            numStack.pop();

            // 获取计算结果
            QString value = calBinary(num2, num1, str);

            if (QString("ERROR") == setError(value).value(ERROR_FLAG)) {
                // 计算错误，返回错误信息
                qWarning() << "Calculation error!";
                return value;
            }
            // 计算正确结果放入操作数栈
            numStack.push(value);

            continue;
        }
        if (m_unary.contains(str)) {
            // 单目运算符
            if (numStack.isEmpty()) {
                // 表达式错误
                qWarning() << "Expression error!";
                return QString("ERROR:FALSE:" + tr("Expression error!"));
            }
            // 取一个操作数
            QString num = numStack.top();
            numStack.pop();

            QString value = calUnary(num, str);

            if (QString("ERROR") == setError(value).value(ERROR_FLAG)) {
                // 计算错误，返回错误信息
                qWarning() << "Calculation error!";
                return value;
            }
            // 计算正确结果放入操作数栈
            numStack.push(value);

            continue;
        }

        // 操作数，直接入栈
        numStack.push(str);
    }
    if (numStack.size() == 1) {
        // 计算正确
        return numStack.top();
    } else {
        // 检测后缀表达式的合法性：操作数是否有多余
        qWarning() << "Expression error!";
        return QString("ERROR:FALSE:" + tr("Expression error!"));
    }
}

QString Calc::calUnary(QString num, QString str)
{
    if (QString("~") == str) {
        // 取反
        return Conversion::getInstance()->calNot(num);
    }
    if (QString("L") == str) {
        // 循环左移
        return Conversion::getInstance()->calRoL(num);
    }
    if (QString("R") == str) {
        // 循环右移
        return Conversion::getInstance()->calRoR(num);
    }
    if (QString("q") == str) {
        // 左移1
        return Conversion::getInstance()->calLsh(num, QString("1"));
    }
    if (QString("p") == str) {
        // 右移1
        return Conversion::getInstance()->calRsh(num, QString("1"));
    }
    if (QString("v") == str) {
        // 按位取反
        return Conversion::getInstance()->calSub(QString("0"), num);
    }
    qWarning() << "Operator undefined!";
    return QString("ERROR:CAL_FALSE:" + tr("Operator undefined!"));
}

QString Calc::calBinary(QString num1, QString num2, QString str)
{
    // 根据操作符计算结果，现将结果进行合法性判断
    if (QString("+") == str) {
        // 加
        return Conversion::getInstance()->calAdd(num1, num2);
    }
    if (QString("s") == str) {
        // 减
        return Conversion::getInstance()->calSub(num1, num2);
    }
    if (QString("*") == str) {
        // 乘
        return Conversion::getInstance()->calMulit(num1, num2);
    }
    if (QString("/") == str) {
        // 除
        if (QString("0") == num2) {
            // 除数不能为0
            return QString("ERROR:CAL_FALSE:" + tr("Divisor cannot be 0!"));
        }
        return Conversion::getInstance()->calDiv(num1, num2);
    }
    if (QString("&") == str) {
        // 按位与
        return Conversion::getInstance()->calAnd(num1, num2);
    }
    if (QString("|") == str) {
        // 按位或
        return Conversion::getInstance()->calOr(num1, num2);
    }
    if (QString("^") == str) {
        // 按位异或
        return Conversion::getInstance()->calXor(num1, num2);
    }
    if (QString("<") == str) {
        // 左移
        if (num2.toLongLong() < 0 || Conversion::getInstance()->getDigit() <= num2.toLongLong()) {
            // 移位右操作数错误
            return QString("ERROR:CAL_FALSE:" + tr("Right operand error!"));
        }
        return Conversion::getInstance()->calLsh(num1, num2);
    }
    if (QString(">") == str) {
        // 右移
        if (num2.toLongLong() < 0 || Conversion::getInstance()->getDigit() <= num2.toLongLong()) {
            // 移位有操作数不能小于0
            return QString("ERROR:CAL_FALSE:" + tr("Right operand error!"));
        }
        return Conversion::getInstance()->calRsh(num1, num2);
    }
    if (QString("N") == str) {
        // 按位或非
        return Conversion::getInstance()->calNor(num1, num2);
    }
    qWarning() << "Operator undefined!";
    return QString("ERROR:CAL_FALSE:" + tr("Operator undefined!"));
}

QStringList Calc::setError(QString err)
{
    return err.split(QString(":"));
}

void Calc::clear()
{
    m_resultVec[ERROR_FLAG].clear();
    m_resultVec[ERROR_TYPE].clear();
    m_resultVec[ERROR_INFO].clear();
    m_resultVec[RIGHT_RESULT].clear();
}
