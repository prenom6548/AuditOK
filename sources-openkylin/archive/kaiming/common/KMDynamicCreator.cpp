/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMDynamicCreator.h"

int KMDynamicCreator::registerClass(const std::string &id, const std::string &className, Constructor constructor)
{
    idTable().emplace(id, className);
    std::map<std::string, Constructor> &instances = constructors();
    instances.emplace(className, constructor);
    return instances.size();
}

KMObject *KMDynamicCreator::createObject(const std::string &className)
{
    std::map<std::string, Constructor> &instances = constructors();
    if (instances.end() != instances.find(className))
    {
        Constructor constructor = instances[className];
        if (constructor != nullptr)
        {
            return (*constructor)();
        }
    }

    return nullptr;
}

KMObject *KMDynamicCreator::createObjectById(const std::string &id)
{
    const std::string &className = idTable()[id];
    if (className.empty())
    {
        return nullptr;
    }
    return createObject(className);
}

std::map<std::string, std::string> &KMDynamicCreator::idTable()
{
    static std::map<std::string, std::string> _idTable;
    return _idTable;
}

std::map<std::string, Constructor> &KMDynamicCreator::constructors()
{
    static std::map<std::string, Constructor> _instances;
    return _instances;
}
