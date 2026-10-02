/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOBJECT_H__
#define __KMOBJECT_H__

#include <string>

class KMObject
{
public:
    KMObject() = default;
    virtual ~KMObject();

    std::string className() const
    {
        return m_className;
    }
    
    void setClassName(const std::string &className)
    {
        m_className = className;
    }

private:
    std::string m_className{ "KMObject" };
};

#endif // !__KMOBJECT_H__
