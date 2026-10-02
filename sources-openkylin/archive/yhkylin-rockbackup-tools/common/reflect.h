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

#ifndef REFLECT_H
#define REFLECT_H

#include <QObject>
#include <QHash>
#include <QString>

typedef QObject * (*Constructor)();

class Reflect
{
public:
    static int registerClass(const QString& className, Constructor constructor);

    static QObject * createObject(const QString& className);

private:
    static QHash<QString, Constructor>& constructors();
};

#define DECLARE_DYNCREATE(class_name) \
public: \
    static QObject * create##class_name##Object();

#define IMPLEMENT_DYNCREATE(class_name) \
QObject * class_name::create##class_name##Object() \
{ \
    return (QObject *)(new class_name()); \
} \
static int g_icreate##class_name##Object = Reflect::registerClass(#class_name, class_name::create##class_name##Object);


#endif //!REFLECT_H

