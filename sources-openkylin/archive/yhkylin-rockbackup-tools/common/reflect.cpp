/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

/**
 * brief 工具类——反射，主要用作根据类名动态创建类
 * author 赵民勇
 * note :
 *     1、类需要继承自QObject或其派生数的上层节点包含QObject
 *     2、暂时只提供无参构造函数注册
 *     3、需要动态创建的类头文件中引入本头文件，并且在类定义中使用宏DECLARE_DYNAMIC_CLASS(yourClassName); 在类定义文件（.cpp）中使用IMPL_DYNAMIC_CLASS(yourClassName)
 *     4、例：
 *     类定义的头文件.h中：
 *     #include <QObject>
 *     #include "../common/reflect.h"
 *     class MyClass : public QObject
 *     {
 *         Q_OBJECT
 *         DECLARE_DYNCREATE(MyClass)
 *     }
 *
 *     .cpp文件中，代码开始前写上：
 *     IMPLEMENT_DYNCREATE(MyClass)
 */

#include "reflect.h"

int Reflect::registerClass(const QString& className, Constructor constructor)
{
    QHash<QString, Constructor>& instances = constructors();
    instances.insert(className, constructor);
    return instances.size();
}

QObject * Reflect::createObject(const QString& className)
{
    QHash<QString, Constructor>& instances = constructors();
    if (instances.contains(className)) {
        Constructor constructor = instances[className];
        if (constructor != nullptr) {
            return (*constructor)();
        }
    }

    return nullptr;
}

QHash<QString, Constructor>& Reflect::constructors()
{
    static QHash<QString, Constructor> _instances;
    return _instances;
}

