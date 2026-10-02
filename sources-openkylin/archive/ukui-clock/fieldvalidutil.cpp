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
#include "fieldvalidutil.h"

FieldValidUtil::FieldValidUtil(QObject *parent) : QObject(parent)
{

}

bool FieldValidUtil::isNull(int value)
{
    if(value==0) return true;
    return false;
}

bool FieldValidUtil::isNull(QString str)
{
    return str.isNull()||str.isEmpty();
}

bool FieldValidUtil::isNull(std::string str)
{
    return str.empty();
}

bool FieldValidUtil::isNull(long value)
{
    if(value==0) return true;
    return false;
}

bool FieldValidUtil::isNull(float value)
{
    if(abs(value) <= 1e-6) return true;
    return false;
}

bool FieldValidUtil::isNull(double value)
{
    if(abs(value) <= 1e-15) return true;
    return false;
}

bool FieldValidUtil::isNotNull(int value)
{
    return !isNull(value);
}
/**
 * @brief 数据介于最大最小之间，取等号
 * @param value
 * @param small
 * @param big
 * @return
 */
bool FieldValidUtil::isValueBetweenRange(int value, int small, int big)
{
    return value>=small&&value<=big;
}

bool FieldValidUtil::isNotNull(QString value)
{
    return !isNull(value);
}

bool FieldValidUtil::isNotNull(long value)
{
    return !isNull(value);
}




#define URL_REGEX_STR "^http://([\\w-]+\\.)+[\\w-]+(/[\\w-./?%&=]*)?$"
/**
 * @brief 匹配URL地址
 */
bool FieldValidUtil::isUrl(QString str)
{
    return match(str, URL_REGEX_STR);
}
#define PWD_REGEX_STR "^[a-zA-Z]\\w{6,12}$"

/**
 * @brief 匹配密码，以字母开头，长度在6-12之间，只能包含字符、数字和下划线。
 */
bool FieldValidUtil::isPwd(QString str)
{
    return match(str, PWD_REGEX_STR);
}
#define STRING_CHECK_REGEX_STR "^[a-zA-Z0-9\u4e00-\u9fa5-_]+$"

/**
 * @brief　验证字符，只能包含中文、英文、数字、下划线等字符。
 */
bool FieldValidUtil::QStringCheck(QString str)
{
    return match(str, STRING_CHECK_REGEX_STR);
}
#define EMAIL_REGEX_STR "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$"

/**
 * @brief　匹配Email地址
 */
bool FieldValidUtil::isEmail(QString str)
{
    return match(str, EMAIL_REGEX_STR);
}
#define INTEGER_REGEX_STR "^[+]?\\d+$"

/**
 * @brief　匹配非负整数（正整数+0）
 */
bool FieldValidUtil::isInteger(QString str)
{
    return match(str, INTEGER_REGEX_STR);
}
/**
 * @brief　判断数值类型，包括整数和浮点数
 */
bool FieldValidUtil::isNumeric(QString str)
{
    if(isFloat(str) || isInteger(str)) return true;
    return false;
}
#define DIGITS_REGEX_STR "^[0-9]*$"

/**
 * @brief　只能输入数字
 */
bool FieldValidUtil::isDigits(QString str)
{
    return match(str, DIGITS_REGEX_STR);
}
#define FLOAT_REGEX_STR "^[-\\+]?\\d+(\\.\\d+)?$"

/**
 * @brief　匹配正浮点数
 */
bool FieldValidUtil::isFloat(QString str)
{
    return match(str, FLOAT_REGEX_STR);
}
/**
 * @brief　联系电话(手机/电话皆可)验证
 */
bool FieldValidUtil::isTel(QString text)
{
    if(isMobile(text)||isPhone(text)) return true;
    return false;
}
#define PHONE_REGEX_STR "^(\\d{3,4}-?)?\\d{7,9}$"

/**
 * @brief　电话号码验证
 */
bool FieldValidUtil::isPhone(QString text)
{
    return match(text, PHONE_REGEX_STR);
}
#define MOBILE_REGEX_STR "^(((13[0-9]{1})|(15[0-9]{1})|(18[0-9]{1}))+\\d{8})$"

/**
 * @brief　手机号码验证
 */
bool FieldValidUtil::isMobile(QString text)
{
    if(text.length()!=11) return false;
    return match(text, MOBILE_REGEX_STR);
}
#define IDCARD_NO_REGEX_STR "^(\\d{6})()?(\\d{4})(\\d{2})(\\d{2})(\\d{3})(\\w)$"

/**
 * @brief　身份证号码验证
 */
bool FieldValidUtil::isIdCardNo(QString text)
{
    return match(text, IDCARD_NO_REGEX_STR);
}
#define ZIPCODE_REGEX_STR "^[0-9]{6}$"

/**
 * @brief　邮政编码验证
 */
bool FieldValidUtil::isZipCode(QString text)
{
    return match(text, ZIPCODE_REGEX_STR);
}
/**
 * @brief　判断整数num是否等于0
 */
bool FieldValidUtil::isIntEqZero(int num)
{
    return num==0;
}
/**
 * @brief　判断整数num是否大于0
 */
bool FieldValidUtil::isIntGtZero(int num)
{
    return num>0;
}
/**
 * @brief　判断整数num是否大于或等于0
 */
bool FieldValidUtil::isIntGteZero(int num)
{
    return num>=0;
}
/**
 * @brief　判断浮点数num是否等于0
 */
bool FieldValidUtil::isFloatEqZero(float num)
{
    return isNull(num);
}
/**
 * @brief 判断浮点数num是否大于0
 */
bool FieldValidUtil::isFloatGtZero(float num)
{
    return num>1e-6;
}
#define RIGHTFUL_STRING_REGEX_STR "^[A-Za-z0-9_-]+$"

/**
 * @brief 判断是否为合法字符(a-zA-Z0-9-_)
 */
bool FieldValidUtil::isRightfulQString(QString text)
{
    return match(text, RIGHTFUL_STRING_REGEX_STR);
}
#define ENGLISH_REGEX_STR "^[A-Za-z]+$"

/**
 * @brief 判断英文字符(a-zA-Z)
 */
bool FieldValidUtil::isEnglish(QString text)
{
    return match(text, ENGLISH_REGEX_STR);
}
#define CHINESE_CHAR_REGEX_STR "^[\u0391-\uFFE5]+$"

/**
 * @brief 判断中文字符(包括汉字和符号)
 */
bool FieldValidUtil::isChineseChar(QString text)
{
    return match(text, CHINESE_CHAR_REGEX_STR);
}
#define CHINESE_REGEX_STR "^[\u4e00-\u9fa5]+$"

/**
 * @brief 匹配汉字
 */
bool FieldValidUtil::isChinese(QString text)
{
    return match(text, CHINESE_REGEX_STR);
}
/**
 * @brief 是否包含中英文特殊字符，除英文"-_"字符外
 */
bool FieldValidUtil::isContainsSpecialChar(QString text)
{
    if(isNull(text)) return false;
    QString chars="[,`,~,!,@,#,$,%,^,&,*,(,),+,=,|,{,},',:,;,',[,],.,<,>,/,?,~,！,@,#,￥,%,…,&,*,（,）,—,+,|,{,},【,】,‘,；,：,”,“,’,。,，,、,？,]";
    QStringList list =chars.split(",");
    list<<","<<"\"";
    for(QString ch : list){
        if(text.contains(ch)) return true;
    }
    return false;
}
#define SPECIAL_STRING_FILTER_REGEX_STR "[`~!@#$%^&*()+=|{}':;',\\[\\].<>/?~！@#￥%……&*（）——+|{}【】‘；：\"\"”“’。，、？]"
/**
 * @brief 过滤中英文特殊字符，除英文"-_"字符外
 */
QString FieldValidUtil::QStringFilter(QString text)
{
    QString regExpr=SPECIAL_STRING_FILTER_REGEX_STR;
    return  replaceAll(text,regExpr);
}
/**
 * @brief 过滤中英文特殊字符，除英文"-_"字符外
 */
QString FieldValidUtil::QStringFilter(std::string text)
{
    QString regExpr=SPECIAL_STRING_FILTER_REGEX_STR;
    return  replaceAll(text,regExpr);
}
#define SCRIPT_REGEX_STR "<[\\s]*?script[^>]*?>[\\s\\S]*?<[\\s]*?\\/[\\s]*?script[\\s]*?>"
#define STYLE_REGEX_STR "<[\\s]*?style[^>]*?>[\\s\\S]*?<[\\s]*?\\/[\\s]*?style[\\s]*?>"
#define HTML_REGEX_STR "<[^>]+>"
#define PATTERN_REGEX_STR "\\s+"
/**
 * @brief 过滤html代码
 */
QString FieldValidUtil::htmlFilter(QString inputQString)
{
    // 含html标签的字符串
    QString htmlStr = inputQString;
    QString textStr = "";
    // 定义script的正则表达式{或<script[^>]*?>[\\s\\S]*?<\\/script>
    QString regEx_script = SCRIPT_REGEX_STR;
    // 定义style的正则表达式{或<style[^>]*?>[\\s\\S]*?<\\/style>
    QString regEx_style = STYLE_REGEX_STR;
    // 定义HTML标签的正则表达式
    QString regEx_html = HTML_REGEX_STR;
    //空白
    QString patternStr = PATTERN_REGEX_STR;
    htmlStr = replaceAll(htmlStr,regEx_script);
    htmlStr = replaceAll(htmlStr,regEx_style);
    htmlStr = replaceAll(htmlStr,regEx_html);
    htmlStr = replaceAll(htmlStr,patternStr);
    textStr = htmlStr;
    return textStr;
}
/**
 * @brief 匹配，方便统一替换正则方法
 */
bool FieldValidUtil::match(QString text, QString reg)
{
    if (isNull(text) || isNull(reg))
                return false;
    QRegExp tempRexExp(reg);
    return tempRexExp.exactMatch(text);
}
/**
 * @brief 替换方法
 */
QString FieldValidUtil::replaceAll(QString text, QString reg)
{
    return  text.replace(QRegExp(reg),"");
}

QString FieldValidUtil::replaceAll(std::string text, QString reg)
{
    return QString::fromStdString(text).replace(QRegExp(reg),"");
}



void FieldValidUtil::createEntityCode(QString fields)
{
    QStringList list = fields.split(";");
    foreach (QString var, list) {
        QStringList tmp = var.split(",");
        QString methodName = makeFirstUpper(tmp.at(1));
        qDebug()<<"\t"<<"Q_PROPERTY ("<<tmp.at(0)<< " "<<tmp.at(1)<< " READ "<< tmp.at(1)<<"WRITE " << "set"<<methodName<<");";
    }
}

QString FieldValidUtil::makeFirstUpper(QString str)
{
    QChar first = str.front().toUpper();
    str = str.remove(0,1);
    str = first+str;
    return str;
}



// 附 ： 常用的正则表达式：
//    匹配特定数字：
//    ^[1-9]d*$　 　 //匹配正整数
//    ^-[1-9]d*$ 　 //匹配负整数
//    ^-?[1-9]d*$　　 //匹配整数
//    ^[1-9]d*|0$　 //匹配非负整数（正整数 + 0）
//    ^-[1-9]d*|0$　　 //匹配非正整数（负整数 + 0）
//    ^[1-9]d*.d*|0.d*[1-9]d*$　　 //匹配正浮点数
//    ^-([1-9]d*.d*|0.d*[1-9]d*)$　 //匹配负浮点数
//    ^-?([1-9]d*.d*|0.d*[1-9]d*|0?.0+|0)$　 //匹配浮点数
//    ^[1-9]d*.d*|0.d*[1-9]d*|0?.0+|0$　　 //匹配非负浮点数（正浮点数 + 0）
//    ^(-([1-9]d*.d*|0.d*[1-9]d*))|0?.0+|0$　　//匹配非正浮点数（负浮点数 + 0）
//    评注：处理大量数据时有用，具体应用时注意修正
//
//    匹配特定字符串：
//    ^[A-Za-z]+$　　//匹配由26个英文字母组成的字符串
//    ^[A-Z]+$　　//匹配由26个英文字母的大写组成的字符串
//    ^[a-z]+$　　//匹配由26个英文字母的小写组成的字符串
//    ^[A-Za-z0-9]+$　　//匹配由数字和26个英文字母组成的字符串
//    ^w+$　　//匹配由数字、26个英文字母或者下划线组成的字符串
//
//    在使用RegularExpressionValidator验证控件时的验证功能及其验证表达式介绍如下:
//
//    只能输入数字：“^[0-9]*$”
//    只能输入n位的数字：“^d{n}$”
//    只能输入至少n位数字：“^d{n,}$”
//    只能输入m-n位的数字：“^d{m,n}$”
//    只能输入零和非零开头的数字：“^(0|[1-9][0-9]*)$”
//    只能输入有两位小数的正实数：“^[0-9]+(.[0-9]{2})?$”
//    只能输入有1-3位小数的正实数：“^[0-9]+(.[0-9]{1,3})?$”
//    只能输入非零的正整数：“^+?[1-9][0-9]*$”
//    只能输入非零的负整数：“^-[1-9][0-9]*$”
//    只能输入长度为3的字符：“^.{3}$”
//    只能输入由26个英文字母组成的字符串：“^[A-Za-z]+$”
//    只能输入由26个大写英文字母组成的字符串：“^[A-Z]+$”
//    只能输入由26个小写英文字母组成的字符串：“^[a-z]+$”
//    只能输入由数字和26个英文字母组成的字符串：“^[A-Za-z0-9]+$”
//    只能输入由数字、26个英文字母或者下划线组成的字符串：“^w+$”
//    验证用户密码:“^[a-zA-Z]\\w{5,17}$”正确格式为：以字母开头，长度在6-18之间，
//
//    只能包含字符、数字和下划线。
//    验证是否含有^%&’,;=?$”等字符：“[^%&’,;=?$x22]+”
//    只能输入汉字：“^[u4e00-u9fa5],{0,}$”
//    验证Email地址：“^w+[-+.]w+)*@w+([-.]w+)*.w+([-.]w+)*$”
//    验证InternetURL：“^http://([\\w-]+\\.)+[\\w-]+(/[\\w-./?%&=]*)?$”
//    验证电话号码：“^((d{3,4})|d{3,4}-)?d{7,8}$”
//
//    正确格式为：“XXXX-XXXXXXX”，“XXXX-XXXXXXXX”，“XXX-XXXXXXX”，
//
//    “XXX-XXXXXXXX”，“XXXXXXX”，“XXXXXXXX”。
//    验证身份证号（15位或18位数字）：“^d{15}|d{}18$”
//    验证一年的12个月：“^(0?[1-9]|1[0-2])$”正确格式为：“01”-“09”和“1”“12”
//    验证一个月的31天：“^((0?[1-9])|((1|2)[0-9])|30|31)$” 正确格式为：“01”“09”和“1”“31”。
//
//    匹配中文字符的正则表达式： [u4e00-u9fa5]
//    匹配双字节字符(包括汉字在内)：[^x00-xff]
//    匹配空行的正则表达式：n[s| ]*r
//    匹配HTML标记的正则表达式：/< (.*)>.*|< (.*) />/
//    匹配首尾空格的正则表达式：(^s*)|(s*$)
//    匹配Email地址的正则表达式：w+([-+.]w+)*@w+([-.]w+)*.w+([-.]w+)*
//    匹配网址URL的正则表达式：^http://([\\w-]+\\.)+[\\w-]+(/[\\w-./?%&=]*)?$
