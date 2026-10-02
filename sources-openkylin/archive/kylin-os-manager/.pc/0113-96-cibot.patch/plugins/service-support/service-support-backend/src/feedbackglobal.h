#ifndef FEEDBACKGLOBAL_H
#define FEEDBACKGLOBAL_H
#include <QSet>
#include <QMap>
#include "../constant.h"
struct InformationUserStruct
{
    int module = DefaultMode; //模式
    QString mail;             //邮箱
    QString priviteType;      //问题类型
    QString title;            //标题
    QString originalDetails;  //问题详情部分未经过处理的文本内容
    QString details;          //描述
    QStringList accessory;    //附件
    QString debugFilePath;    //调试文件目录

    QString user;           //称谓
    QString phoneNumber;    //手机号
    QString serviceNumber;  //服务序列号
    QString priviteTypeStr; //问题类型文本

    QString jobNumber; //工号
    QString userName;  //用户名
    QString userGroup; //组织

    QString execution;    //执行
    QString severity;     //严重程度
    QString pri;          //优先级
    QString bugType;      // bug类型
    QString task;         // 任务
    QString story;        // 需求
    QString deadline;     //截止日期
    QString buildId;      //版本ID
    QString productId;    //产品ID
    QString projectId;    //项目ID
    QString assignedTo;   //指派给
    QString usecase;      //用例ID
    QString zentaoName;   //禅道用户名
    QString zentaoPwd;    //禅道密码
    QString zentaoModule; //模块ID
    QString zentaoBranch; //分支ID
    QString CPUArch;      //体系架构

    //旧接口，工行1.2版本旧界面使用
    int type = 0;
    int level = 0;

    QString uniqueType;//唯一标识
};

typedef QSet<QString> ModuleTypeSet;

#endif // FEEDBACKGLOBAL_H
