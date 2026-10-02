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

#ifndef KYPLUGIN_MANAGER_H_
#define KYPLUGIN_MANAGER_H_

#include <string>
#include <map>

#include "host.h"

namespace kyplugin
{

class DLibrary;

class PluginManager
{
public:
    ~PluginManager();

    bool load(const std::string &path);
    bool load(const std::string &folder, const std::string &pluginName);
    int loadFromFolder(const std::string &folder, bool recursive = false);
    bool unload(const std::string &pluginName);
    void unloadAll();
    bool addProvider(Provider *provider);
    void getLoadedPlugins(std::vector<const std::string *> &pluginNames) const;
    bool isLoaded(const std::string &pluginName) const;

protected:
    PluginManager();
    void registerType(const std::string &type, unsigned int version, unsigned int lowestVersion);
    const std::list<Provider *> *getProviders(const std::string &type) const;

private:
    static std::string getPluginName(const std::string &path);
    static std::string resolvePathExtension(const std::string &path);

    typedef bool fnRegisterPlugin(Host &);
    typedef std::map<std::string, DLibrary *> libMap;

    libMap m_libraries;

    /* Host 类为友元类 */
    /* 处理所有的 provider */
    Host m_host;
};

} // namespace kyplugin

#endif