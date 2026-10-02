/* common/KMDynamicCreator.h
 * 
 * brief 主要用作根据id动态创建类
 * note :
 *     1、类需要继承自KMObject或其派生数的上层节点包含KMObject
 *     2、暂时只提供无参构造函数注册
 *     3、需要动态创建的类头文件中引入本头文件，并且在类定义中使用宏DECLARE_DYNAMIC_CLASS(class_type); 在类定义文件（.cpp）中使用REGISTER_DYNCREATE(id, class_type)
 *     4、例：
 *     类定义的头文件.h中：
 *     #include "src/common/KMDynamicCreator.h"
 *     class MyClass : public KMObject
 *     {
 *         DECLARE_DYNCREATE(MyClass)
 *     }
 *
 *     .cpp文件中写上：
 *     REGISTER_DYNCREATE(MyClassID, MyClass)
 * 
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMDYNAMIC_CREATOR_H__
#define __KMDYNAMIC_CREATOR_H__

#include <string>
#include <map>

#include "KMObject.h"

typedef KMObject* (*Constructor)();

/**
 * @brief 主要用作根据id动态创建类
 */
class KMDynamicCreator
{
public:
    static int registerClass(const std::string &id, const std::string &className, Constructor constructor);

    static KMObject* createObject(const std::string &className);

    static KMObject* createObjectById(const std::string &id);

private:
    static std::map<std::string, Constructor>& constructors();
    static std::map<std::string, std::string>& idTable();
};

#define DECLARE_DYNCREATE(class_type) \
public: \
    static KMObject * create##class_type##Object();

#define REGISTER_DYNCREATE(id, class_type) \
KMObject * class_type::create##class_type##Object() \
{ \
    KMObject* pObject = (KMObject *)(new class_type()); \
    pObject->setClassName(#class_type); \
    return pObject; \
} \
static int g_icreate##class_type##Object = KMDynamicCreator::registerClass(#id, #class_type, class_type::create##class_type##Object);

#endif // !__KMDYNAMIC_CREATOR_H__
