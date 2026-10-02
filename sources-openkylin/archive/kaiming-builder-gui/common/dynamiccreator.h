/**
 * brief 工具类——反射，主要用作根据类名动态创建类
 * author 赵民勇
 * note :
 *     1、类需要继承自QObject或其派生数的上层节点包含QObject
 *     2、暂时只提供无参构造函数注册
 *     3、需要动态创建的类头文件中引入本头文件，并且在类定义中使用宏DECLARE_DYNAMIC_CLASS(yourClassName); 在类定义文件（.cpp）中使用IMPL_DYNAMIC_CLASS(yourClassName)
 *        注意：这些宏如果不合适，请自行在自己的工厂类中自行定制宏
 *     4、例：
 *     类定义的头文件.h中：
 *     #include <QObject>
 *     #include "../common/dynamiccreator.h"
 *     class MyClass : public QObject
 *     {
 *         Q_OBJECT
 *         DECLARE_DYNCREATE_WITH_ONE_PARAM(MyClass, QObject*, QObject*)
 *     }
 *
 *     .cpp文件中，代码开始前写上：
 *     IMPLEMENT_DYNCREATE_WITH_ONE_PARAM(MyClass, QObject*, QObject*)
 */

#ifndef DYNAMIC_CREATOR_H
#define DYNAMIC_CREATOR_H

#include <QObject>
#include <QHash>
#include <QString>
#include <functional>

template<typename ReturnType, typename... Args>
class DynamicCreator
{
public:
    typedef ReturnType (*Constructor)(Args...);

    static int registerClass(const QString& className, Constructor constructor) {
        QHash<QString, Constructor>& instances = constructors();
        instances.insert(className, constructor);
        return instances.size();
    }

    static ReturnType createInstance(const QString& className, Args... args) {
        QHash<QString, Constructor>& instances = constructors();
        if (instances.contains(className)) {
            Constructor constructor = instances[className];
            if (constructor != nullptr) {
                return (*constructor)(args...);
            }
        }

        return nullptr;
    }

private:
    static QHash<QString, Constructor>& constructors() {
        static QHash<QString, Constructor> _instances;
        return _instances;
    }
};

#define DECLARE_DYNCREATE_WITH_ONE_PARAM(class_name, return_type, param_type) \
public: \
    static return_type create##class_name##Object(param_type arg1);

#define IMPLEMENT_DYNCREATE_WITH_ONE_PARAM(class_name, return_type, param_type) \
return_type class_name::create##class_name##Object(param_type arg1) \
{ \
    return static_cast<return_type>(new class_name(arg1)); \
} \
static int g_icreate##class_name##Object = DynamicCreator<return_type, param_type>::registerClass(#class_name, class_name::create##class_name##Object);

#endif //!DYNAMIC_CREATOR_H

