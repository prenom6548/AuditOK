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

#include <cstdio>

#include "host.h"

namespace kyplugin
{

Host::Host()
{
    /* nothing to do */
}

Host::~Host()
{
    clearProviders();
}

bool Host::add(Provider *provider)
{
    if (provider == NULL) {
        fprintf(stderr, "Trying to add a null provider.\n");
        return false;
    }

    if (!validateProvider(provider)) {
        delete provider;
        return false;
    }

    m_addRequests[provider->kypluginGetType()].push_back(provider);
    return true;
}

void Host::clearProviders()
{
    providersMap::iterator it;
    for (it = m_knownTypes.begin(); it != m_knownTypes.end(); ++it) {
        std::list<Provider *> &providers = it->second.providers;
        std::list<Provider *>::iterator provIt;
        for (provIt = providers.begin(); provIt != providers.end(); ++provIt) {
            delete *provIt;
        }

        std::list<Provider *>().swap(providers);
    }
}

bool Host::knows(const std::string &type) const
{
    return m_knownTypes.find(type) != m_knownTypes.end();
}

unsigned int Host::getVersion(const std::string &type) const
{
    providersMap::const_iterator it = m_knownTypes.find(type);
    if (it != m_knownTypes.end()) {
        return it->second.version;
    }

    return 0;
}

unsigned int Host::getLowestVersion(const std::string &type) const
{
    providersMap::const_iterator it = m_knownTypes.find(type);
    if (it != m_knownTypes.end()) {
        return it->second.lowestVersion;
    }

    return 0;
}

void Host::registerType(const std::string &type, unsigned int version, unsigned int lowestVersion)
{
    if (!knows(type)) {
        ProviderInfo pi;
        pi.version = version;
        pi.lowestVersion = lowestVersion;
        m_knownTypes[type] = pi;
    }

    return;
}

const std::list<Provider *> *Host::getProviders(const std::string &type) const
{
    providersMap::const_iterator it = m_knownTypes.find(type);
    if (it != m_knownTypes.end()) {
        return &it->second.providers;
    }

    return NULL;
}

bool Host::validateProvider(Provider *provider) const
{
    const std::string &type = provider->kypluginGetType();
    if (!knows(type)) {
        fprintf(stderr, "%s provider type isn't registered.\n", type.c_str());
        return false;
    }

    if (!provider->isCompatible(*this)) {
        fprintf(stderr, "Incompatible %s provider version.\n", type.c_str());
        return false;
    }

    return true;
}

bool Host::registerProvider(Provider *provider)
{
    if (!validateProvider(provider)) {
        delete provider;
        return false;
    }

    m_knownTypes[provider->kypluginGetType()].providers.push_back(provider);
    return true;
}

void Host::cancelAddictions()
{
    tempProvidersMap::iterator it;
    for (it = m_addRequests.begin(); it != m_addRequests.end(); ++it) {
        std::list<Provider *> lst = it->second;
        std::list<Provider *>::iterator providerIt;
        for (providerIt = lst.begin(); providerIt != lst.end(); ++providerIt) {
            delete *providerIt;
        }
    }

    /* 清空 map */
    tempProvidersMap().swap(m_addRequests);

    return;
}

bool Host::confirmAddictions()
{
    if (m_addRequests.empty()) {
        return false;
    }

    tempProvidersMap::iterator it;
    for (it = m_addRequests.begin(); it != m_addRequests.end(); ++it) {
        std::list<Provider *> lst = it->second;
        std::list<Provider *>::iterator providerIt;
        for (providerIt = lst.begin(); providerIt != lst.end(); ++providerIt) {
            m_knownTypes[it->first].providers.push_back(*providerIt);
        }
    }

    /* 清空 map */
    tempProvidersMap().swap(m_addRequests);

    return true;
}

} // namespace kyplugin