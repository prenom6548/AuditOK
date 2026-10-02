#include <iostream>

#include "kom_tray_interface.h"
#include "daemon.h"

namespace tray {

Daemon::Daemon()
    : m_pluginManager(new kyplugin::KyPlugin)
{
    m_loadPath.emplace_back("/opt/kylin-os-manager/tray-plugins/");
    m_loadPath.emplace_back("/home/snowsi/snowsi/clion-code/kylin-os-manager/cmake-build-v101-2304/src/tray_service/example/");

    loadPlugin();
}

Daemon::~Daemon() {
    /* nothing to do */
}

std::map<std::string, KomTrayProvider *> Daemon::trayProviders() {
    return m_trayProviders;
}

std::map<std::string, std::shared_ptr<Tray>> Daemon::trays() {
    return m_trays;
}

std::pair<bool, std::string> Daemon::startTray(std::string name) {
    auto iter = m_trayProviders.find(name);
    if (iter == m_trayProviders.end()) {
        std::cout << "ERR: " << name << " not found." << std::endl;
        return std::make_pair(false, name + " not found.");
    }

    if (m_trays.find(name) != m_trays.end())
        return std::make_pair(true, "");

    auto tray = std::make_shared<Tray>(iter->second->create());
    tray->enroll();
    m_trays.emplace(std::make_pair(tray->name(), tray));

    return std::make_pair(true, "");
}

std::pair<bool, std::string> Daemon::stopTray(std::string name) {
    auto providerIter = m_trayProviders.find(name);
    if (providerIter == m_trayProviders.end()) {
        std::cout << "ERR:" << name << " not found." << std::endl;
        return std::make_pair(false, name + " not found.");
    }

    auto trayIter = m_trays.find(name);
    if (trayIter != m_trays.end())
        m_trays.erase(trayIter);

    return std::make_pair(true, "");
}

void Daemon::loadPlugin() {
    m_pluginManager->acceptProviderType<KomTrayProvider>();
    for (const auto &item: m_loadPath) {
        m_pluginManager->loadFromFolder(item);
    }

    std::vector<KomTrayProvider *> providers;
    m_pluginManager->getProviders(providers);
    for (const auto &item: providers) {
        std::shared_ptr<Tray> tray = std::make_shared<Tray>(item->create());
        m_trayProviders.emplace(std::make_pair(tray->name(), item));
        m_trays.emplace(std::make_pair(tray->name(), tray));
    }
}

void Daemon::enrollTrays() {
    for (const auto &[name, tray]: m_trays)
        tray->enroll();
}

}
