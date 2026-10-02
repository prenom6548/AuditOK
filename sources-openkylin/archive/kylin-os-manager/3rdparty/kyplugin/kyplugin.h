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

#ifndef KYPLUGIN_KYPLUGIN_H_
#define KYPLUGIN_KYPLUGIN_H_

#include "provider.h"
#include "plugin_manager.h"

namespace kyplugin
{

class KyPlugin : public PluginManager
{
public:
    KyPlugin();

    template <typename ProviderType> void acceptProviderType();
    template <typename ProviderType> void getProviders(std::vector<ProviderType *> &providers);

    static void generateReport(std::string name, std::string data);
    static std::string exportReport(void);

private:
    static std::map<std::string, std::string> m_report;

    static std::string generateReportHead(std::string name);
};

template <typename ProviderType> void KyPlugin::acceptProviderType()
{
    PluginManager::registerType(ProviderType::providerType, ProviderType::providerVersion,
                                ProviderType::providerLowestVersion);
}

template <typename ProviderType> void KyPlugin::getProviders(std::vector<ProviderType *> &providers)
{
    const std::list<Provider *> *lst = PluginManager::getProviders(ProviderType::providerType);
    if (!lst) {
        return;
    }

    providers.reserve(providers.size() + lst->size());
    std::list<Provider *>::const_iterator it;
    for (it = lst->begin(); it != lst->end(); ++it) {
        providers.push_back(static_cast<ProviderType *>(*it));
    }

    return;
}

} // namespace kyplugin

#endif