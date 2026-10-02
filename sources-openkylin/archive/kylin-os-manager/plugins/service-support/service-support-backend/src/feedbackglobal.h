#ifndef FEEDBACKGLOBAL_H
#define FEEDBACKGLOBAL_H
#include <QSet>
#include "../constant.h"
struct InformationUserStruct
{
    int module = OpenKylin;  //模式
    QString mail;            //邮箱
    QString priviteType;     //问题类型
    QString title;           //标题
    QString originalDetails; //问题详情部分未经过处理的文本内容
    QString details;         //描述
    QStringList accessory;   //附件
    QString giteecode;       //用户code
    QString phoneNumber;     //手机号
    QString name;            //称谓
};

typedef QSet<QString> ModuleTypeSet;

#endif // FEEDBACKGLOBAL_H
