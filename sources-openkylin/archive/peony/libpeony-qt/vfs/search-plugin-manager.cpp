#include <QMutexLocker>
#include <QDebug>
#include "search-plugin-manager.h"
#include "ukui-search-plugin.h"

using namespace Peony;

static SearchPluginManager *globalInstance = nullptr;

SearchPluginManager *SearchPluginManager::getInstance()
{
    if (!globalInstance) {
        globalInstance = new SearchPluginManager;
    }
    return globalInstance;
}

void SearchPluginManager::registerSearchPlugin(SearchPluginIface *plugin)
{
    qDebug() << "==========" << plugin->name();
    if (m_pluginsMap.value(plugin->name())) {
        return;
    }
    m_pluginsMap.insert(plugin->name(), plugin);
}

void SearchPluginManager::unregisterSearchPlugin(SearchPluginIface *plugin)
{
    if (!m_pluginsMap.contains(plugin->name())) {
        return;
    }

    auto value = m_pluginsMap.take(plugin->name());
    delete value;
}

SearchPluginIface *SearchPluginManager::getDefaultSearchPlugin(const QString &uri)
{
    SearchPluginIface *defaultPlugin = nullptr;
    int priority = 0;
    auto plugins = m_pluginsMap.values();
    for (auto plugin : plugins) {
        if (!plugin->isSupportUri(uri))
            continue;
        if (plugin->priority() >= priority) {
            priority = plugin->priority();
            defaultPlugin = plugin;
        }
    }

    return defaultPlugin;
}

SearchPluginIface *SearchPluginManager::getSearchPluginForName(const QString &pluginName)
{
    if (m_pluginsMap.value(pluginName) == nullptr) {
        return nullptr;
    }
    return m_pluginsMap.value(pluginName);
}

SearchPluginManager::SearchPluginManager(QObject *parent)
{
    auto plugin = new UkuiSearchPlugin;
    registerSearchPlugin(plugin);
}

SearchPluginManager::~SearchPluginManager()
{

}
