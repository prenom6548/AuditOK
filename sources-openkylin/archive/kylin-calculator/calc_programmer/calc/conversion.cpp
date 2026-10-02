#include "conversion.h"


Conversion *Conversion::getInstance(void)
{
    static Conversion *instance = NULL;
    if (instance == NULL) {
        instance = new Conversion();
        return instance;
    }

    return instance;
}

void Conversion::setDigit(int digit)
{
    // 设置当前位数
    this->m_digit = digit;

    // 根据当前位数，设置当前无符号数最大值
    if (DIGIT_8 == digit) {
        this->m_max = UNSIGNED_CHAR_MAX;
    } else if (DIGIT_16 == digit) {
        this->m_max = UNSIGNED_INT_MAX;
    } else if (DIGIT_32 == digit) {
        this->m_max = UNSIGNED_LONG_MAX;
    } else if (DIGIT_64 == digit) {
        this->m_max = UNSIGNED_LLONG_MAX;
    }
}

QString Conversion::decTo(QString dec, int base)
{
    // 判断传入的十进制数是否合法
    bool ok = true;
    quint64 decInt = dec.toLongLong(&ok);
    if (!ok) {
        qCritical() << "Parameter conversion -- toLongLong Error!";
        decInt = -1;
    }

    if (!isLegitimate(dec)) {
        // 不合法转换为合法数
        decInt = setLegalNum(decInt).toLongLong();
    }

    // 将十进制转换为对应的进制
    QString str = "";
    switch (base) {
    case BASE_BIN:
        // 二进制
        str = DecToBin(decInt);
        break;
    case BASE_OCT:
        // 先转二进制再转由二进制转八进制
        str = BinToOctHex(DecToBin(decInt), 3);
        break;
    case BASE_HEX:
        // 先转二进制再转由二进制转十六进制
        str = BinToOctHex(DecToBin(decInt), 4);
        break;
    default:
        str = dec;
        break;
    }

    // 去除前面多余的0
    while (str.left(1) == QString("0") && 1 != str.size()) {
        str = str.mid(1, str.size() - 1);
    }

    return str;
}

bool Conversion::isLegitimate(QString value, int base)
{
    // 盛放转换后的十进制
    qint64 decInt = 0;

    // 判断当前是否超出qint64的最大范围
    bool quintIsOk = true;
    bool qintIsOk = true;
    value.toULongLong(&quintIsOk, base);
    value.toLongLong(&qintIsOk, base);

    if (!quintIsOk && !qintIsOk) {
        // 越界
        qCritical() << "Illegal operand!";
        return false;
    }

    // 将不是十进制的进制转换为十进制
    if (BASE_DEC != base) {
        // 不是十进制，转换十进制
        decInt = otherToDec(value, base).toLongLong(&qintIsOk);
    } else {
        // 是十进制，直接转
        decInt = value.toLongLong(&qintIsOk);
    }
    if (!qintIsOk) {
        // 越界
        qCritical() << "Illegal operand!";
        return false;
    }

    if (decInt > 0) {
        // “0在这里也是正数”，需要加上
        decInt++;
    }

    // 输入的数不能大于当前字节数下的最大范围 2^(位数)
    if (abs(decInt) > factorial(BASE_BIN, m_digit - 1)) {
        // 越界
        qCritical() << "Illegal operand!";
        return false;
    }
    return true;
}

QString Conversion::setLegal(QString value, int base)
{
    // 将传入的数转换为十进制数
    value = otherToDec(value, base);
    // 将十进制数合法化
    value = setLegalNum(value.toLongLong());

    // 转换会原来的进制
    switch (base) {
    case BASE_BIN:
        return decTo(value, base);
    case BASE_OCT:
        return decTo(value, base);
    case BASE_DEC:
        return value;
    case BASE_HEX:
        return decTo(value, base);
    default:
        return value;
    }
}

QString Conversion::DecToBin(qint64 decInt)
{
    // 使用C++bitset模板类进行二进制转换
    string str = "";
    if (DIGIT_8 == m_digit) {
        // template的存在是为了告诉编译器，其后的第一个<不是小于号，而是模板参数列表的格式。
        bitset<DIGIT_8> bit8 = bitset<DIGIT_8>(decInt);
        str = bit8.template to_string<char, char_traits<char>, allocator<char>>();
    } else if (DIGIT_16 == m_digit) {
        bitset<DIGIT_16> bit16 = bitset<DIGIT_16>(decInt);
        str = bit16.template to_string<char, char_traits<char>, allocator<char>>();
    } else if (DIGIT_32 == m_digit) {
        bitset<DIGIT_32> bit32 = bitset<DIGIT_32>(decInt);
        str = bit32.template to_string<char, char_traits<char>, allocator<char>>();
    } else if (DIGIT_64 == m_digit) {
        bitset<DIGIT_64> bit64 = bitset<DIGIT_64>(decInt);
        str = bit64.template to_string<char, char_traits<char>, allocator<char>>();
    }
    // 将string转换为QString
    QString result = QString::fromStdString(str);

    return result;
}

QString Conversion::BinToOctHex(QString bin, int num)
{
    // 放置转换后的进制list容器
    QString result = "";

    // 临时容器，用于二进制转换成八进制和十六进制
    QString str = "";

    // 从最后一位开始，每num位合并成一位
    int size = bin.size();
    while (size >= num) {
        str.clear();

        // 按照传入的num位数分割字符，从size - 1位开始，从右往左分割
        str = mySplit(bin, size - 1, num);

        quint64 index = 0;
        // 将分割出来的list进行合并
        resOtherToDec(str, index, BASE_BIN, str.size() - 1, 0);

        // 合并的数就是对应的进制数
        result.push_front(m_digitList.at(index));

        // 移动起始点
        size = size - num;
    }

    str.clear();

    // 最后可能会剩下小于num个数，将剩下的数合并，放到最前面
    if (size > 0) {
        str = mySplit(bin, size - 1, size);
        quint64 index = 0;
        resOtherToDec(str, index, BASE_BIN, str.size() - 1, 0);
        result.push_front(m_digitList.at(index));
    }

    return result;
}

QString Conversion::mySplit(QString str, int start, int size)
{
    if (str.isEmpty() || start < 0 || size < 0) {
        qCritical() << "Parameter error!";
        return "";
    }
    // 放置分割后的字符list
    QString result = "";

    for (int i = 0; i < size; i++) {
        // 将list中索引位start - i对应的字符放入result中
        result.push_front(str.at(start - i));
    }

    return result;
}

QString Conversion::otherToDec(QString other, int base)
{
    if (BASE_DEC == base) {
        // 传入的是十进制不作处理
        qWarning() << "The number passed in is already decimal!";
        return other;
    }

    quint64 dec = 0;
    // 递归转换十进制
    resOtherToDec(other, dec, base, other.size() - 1, 0);
    qint64 result = 0;
    // 判断正负数
    if (dec >= factorial(BASE_BIN, m_digit - 1)) {
        result = dec - m_max - 1;
    } else {
        result = qint64(dec);
    }

    return QString::number(result);
}

QString Conversion::setLegalNum(qint64 num)
{
    // 将该数转换为二进制
    QString result = DecToBin(num);
    // 根据当前位数移出多余的位数再转换成十进制
    result = result.mid(result.size() - m_digit, m_digit);
    result = otherToDec(result, BASE_BIN);

    return result;
}

QString Conversion::calAdd(QString num1, QString num2)
{
    return setLegalNum(num1.toLongLong() + num2.toLongLong());
}

QString Conversion::calSub(QString num1, QString num2)
{
    qint64 result = num1.toLongLong() - num2.toLongLong();
    if (result == factorial(BASE_BIN, m_digit - 1)) {
        return setLegalNum(~result);
    } else {
        return setLegalNum(result);
    }
}

QString Conversion::calMulit(QString num1, QString num2)
{
    return setLegalNum(num1.toLongLong() * num2.toLongLong());
}

QString Conversion::calNot(QString num)
{
    return setLegalNum(~num.toLongLong());
}

QString Conversion::calAnd(QString num1, QString num2)
{
    return setLegalNum(num1.toLongLong() & num2.toLongLong());
}

QString Conversion::calOr(QString num1, QString num2)
{
    return setLegalNum(num1.toLongLong() | num2.toLongLong());
}

QString Conversion::calXor(QString num1, QString num2)
{
    return setLegalNum(num1.toLongLong() ^ num2.toLongLong());
}

QString Conversion::calNor(QString num1, QString num2)
{
    return setLegalNum(~(num1.toLongLong() | num2.toLongLong()));
}

QString Conversion::calDiv(QString num1, QString num2)
{
    if (0 == num2.toLongLong()) {
        qWarning() << "Divisor cannot be 0！";
        return QString("0");
    }
    return setLegalNum(num1.toLongLong() / num2.toLongLong());
}

void Conversion::resOtherToDec(QString other, quint64 &dec, int base, int digitNum, int count)
{
    if (digitNum >= 0) {
        // 获取对应的数值
        quint64 num = m_digitList.indexOf(other.at(digitNum));
        if (num >= base) {
            // 该位的值大于该进制下能输入的最大值
            qWarning() << "Hexadecimal number input error!";
            return;
        }

        // 根据所在位，计算对应的十进制数
        dec = dec + num * factorial(base, count);

        // 索引从右向左
        digitNum--;
        // 位数越来越大
        count++;
        // 递归
        resOtherToDec(other, dec, base, digitNum, count);
    }

    return;
}

quint64 Conversion::factorial(int x, int y)
{
    if (0 == y) {
        return 1;
    }
    if (1 == y) {
        return x;
    }
    int num = x;
    quint64 result = x;
    for (int i = 1; i < y; i++) {
        result = result * num;
    }
    return result;
}

QString Conversion::calLsh(QString num1, QString num2)
{
    if (num2.toLongLong() < 0 || m_digit <= num2.toLongLong()) {
        qWarning() << "Right operand error！";
        return QString("0");
    }
    // 转换二进制
    QString bef = decTo(num1, BASE_BIN);

    // 根据位数再前面缺少的位数补0
    int size = bef.size();
    for (int i = m_digit - size; i > 0; i--) {
        bef.push_front(QString("0"));
    }

    // 进行左移
    size = bef.size();
    // 需要左移的数
    QString str = bef.mid(num2.toLongLong(), size - num2.toLongLong());
    // 补位
    for (int i = 0; i < num2.toLongLong(); i++) {
        str.append(QString("0"));
    }

    // 将左移后的二进制转换为十进制
    QString result = otherToDec(str, BASE_BIN);

    return result;
}

QString Conversion::calRsh(QString num1, QString num2)
{
    if (num2.toLongLong() < 0 || m_digit <= num2.toLongLong()) {
        qWarning() << "Right operand error！";
        return QString("0");
    }
    // 根据正负判断补位的数
    QString symbol = QString("0");
    if (num1.toLongLong() < 0) {
        symbol = QString("1");
    }

    // 转换二进制
    QString bef = decTo(num1, BASE_BIN);

    // 根据位数再前面缺少的位数补0
    int size = bef.size();
    for (int i = m_digit - size; i > 0; i--) {
        bef.push_front(QString("0"));
    }

    // 进行右移
    size = bef.size();
    // 需要右移的数
    QString str = bef.mid(0, size - num2.toLongLong());
    // 补位
    for (int i = 0; i < num2.toLongLong(); i++) {
        str.push_front(symbol);
    }

    // 将右移后的二进制转换为十进制
    QString result = otherToDec(str, BASE_BIN);

    return result;
}

QString Conversion::calRoL(QString num)
{
    // 转换二进制
    QString bef = decTo(num, BASE_BIN);

    // 根据位数再前面缺少的位数补0
    int size = bef.size();
    for (int i = m_digit - size; i > 0; i--) {
        bef.push_front(QString("0"));
    }

    // 开始循环左移
    size = bef.size();
    // 需要左移的数
    QString str = bef.mid(1, size - 1);
    // 将高位移出的数补到低位
    str.append(bef.at(0));

    // 将得到的二进制转换为十进制
    QString result = otherToDec(str, BASE_BIN);

    return result;
}

QString Conversion::calRoR(QString num)
{
    // 转换二进制
    QString bef = decTo(num, BASE_BIN);
    // 根据位数再前面缺少的位数补0
    int size = bef.size();
    for (int i = m_digit - size; i > 0; i--) {
        bef.push_front(QString("0"));
    }

    // 开始循环右移
    size = bef.size();
    // 需要右移的数
    QString str = bef.mid(0, size - 1);
    // 将低位移出的数补到高位
    str.push_front(bef.at(size - 1));

    // 将得到的二进制转换为十进制
    QString result = otherToDec(str, BASE_BIN);

    return result;
}

int Conversion::getDigit() const
{
    return m_digit;
}

QString Conversion::setCode(QString value, QString code, int base)
{
    if ("" == code) {
        // 没有编码格式
        return "";
    }
    // 转换十进制
    QString decValue = otherToDec(value, base);
    if (decValue.toLongLong() == 127) {
        return "";
    }

    if ("ASCII" == code && decValue.toLongLong() > 127) {
        // 数值大于127，超出ASCII范围
        return "";
    }

    // 判断传入的value数值是否为十六进制
    if (BASE_HEX != base) {
        // 不是十六进制，转换为十六进制
        value = decTo(decValue, BASE_HEX);
    }

    int temp[400];
    QChar qchar[100];
    QString strOut = "";
    bool ok;

    int count = value.count();
    if (0 != count % DIGIT_8) {
        // 不能被8整除，补0
        for (int i = count % DIGIT_8; i < DIGIT_8; i++) {
            value.push_front(QString("0"));
        }
    }
    count = value.count();

    int len = count / DIGIT_8;

    for (int i = 0; i < count; i += DIGIT_8) {
        temp[i / DIGIT_8] = value.mid(i, DIGIT_8).toInt(&ok, BASE_HEX); //每四位转化为16进制整型
        qchar[i / DIGIT_8] = temp[i / DIGIT_8];
        QString str0(qchar, len);
        strOut = str0;
    }

    return strOut;
}
