/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMREF_H__
#define __KMREF_H__

#include <string>

class KMRef
{
public:
    // bool local = false;
    std::string m_id;
    std::string m_name;
    std::string m_kind;
    std::string m_version;
    std::string m_channel;
    std::string m_module;
    std::string m_arch;
    std::string m_description;
    std::string m_base;
    std::string m_runtime;
    std::string m_repo;  //kaiming-repo local

public:
    void loadFromInfoJson(const std::string &path);
    void cleanup();
    void parseInfo(const std::string & baseFormat);
    void parseVersion();
    bool check(KMRef &ref);

public:
    int m_major = -1;

public:

    bool operator < (const KMRef &ref)
    {
        if (m_channel != ref.m_channel)
        {
            return m_channel < ref.m_channel;
        }
        else if (m_id != ref.m_id)
        {
            return m_id < ref.m_id;
        }
        else
        {
            return m_version < ref.m_version;
        }
    };

    bool operator == (const KMRef &ref) const
    {
        return (m_channel == ref.m_channel) && (m_id == ref.m_id) && (m_version == ref.m_version);
    };
};

#endif // KMREF_H
