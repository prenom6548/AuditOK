/*
 * Pluma - Plug-in Management Framework
 * Copyright (C) 2010-2012 Gil Costa (gsaurus@gmail.com)
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented;
 *    you must not claim that you wrote the original software.
 *    If you use this software in a product, an acknowledgment
 *    in the product documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such,
 *    and must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef KYPLUGIN_HOST_H_
#define KYPLUGIN_HOST_H_

#include <vector>
#include <list>
#include <map>

#include "provider.h"

namespace kyplugin
{

class Host
{
    friend class PluginManager;
    friend class Provider;

public:
    bool add(Provider *provider);

private:
    Host();
    ~Host();

    bool knows(const std::string &type) const;
    unsigned int getVersion(const std::string &type) const;
    unsigned int getLowestVersion(const std::string &type) const;
    void registerType(const std::string &type, unsigned int version, unsigned int lowestVersion);
    const std::list<Provider *> *getProviders(const std::string &type) const;
    void clearProviders();
    bool validateProvider(Provider *provider) const;
    bool registerProvider(Provider *provider);
    void cancelAddictions();
    bool confirmAddictions();

private:
    struct ProviderInfo
    {
        unsigned int version;               // 主机方版本
        unsigned int lowestVersion;         // 主机方最低兼容版本
        std::list<Provider *> providers;    // 注册进来的插件工厂类指针
    };

    typedef std::map<std::string, ProviderInfo> providersMap;
    typedef std::map<std::string, std::list<Provider *>> tempProvidersMap;

    /*
     * 核心数据结构
     * 维护着主机方支持的类型及主机方该类型对应的版本号
     * 以及通过版本号兼容判断注册进来的插件工厂类指针
     */
    providersMap m_knownTypes;

    // 临时存放一下加载进来的 providers
    tempProvidersMap m_addRequests;
};

} // namespace kyplugin

#endif
