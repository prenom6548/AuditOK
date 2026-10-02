#include <iostream>
#include "daemon.h"

namespace process_manager {

Daemon::Daemon(std::vector<ProcessConf> processConfs) {
    for (auto &item: processConfs)
        m_processes.emplace(item.name(), std::make_shared<Process>(item));
}

Daemon::~Daemon() {
    // nothing to do
}

std::pair<bool, std::string> Daemon::startProcess(const std::string &name) {
    std::string errorMessage;
    auto iter = m_processes.find(name);
    if (iter == m_processes.end()) {
        errorMessage.clear();
        errorMessage.append(name);
        errorMessage.append(" ");
        errorMessage.append("does not exist");
        std::cout << errorMessage << std::endl;
        return std::make_pair(false,errorMessage);
    }

    return iter->second->start();
}

std::pair<bool, std::string> Daemon::stopProcess(const std::string &name) {
    std::string errorMessage;
    auto iter = m_processes.find(name);
    if (iter == m_processes.end()) {
        errorMessage.clear();
        errorMessage.append(name);
        errorMessage.append(" ");
        errorMessage.append("does not exist");
        std::cout << errorMessage << std::endl;
        return std::make_pair(false,errorMessage);
    }

    return iter->second->stop();
}

std::pair<bool, std::string> Daemon::restart(const std::string &name) {
    std::string errorMessage;
    auto iter = m_processes.find(name);
    if (iter == m_processes.end()) {
        errorMessage.clear();
        errorMessage.append(name);
        errorMessage.append(" ");
        errorMessage.append("does not exist");
        std::cout << errorMessage << std::endl;
        return std::make_pair(false, errorMessage);
    }

    iter->second->stop();
    return iter->second->start();
}

const std::map<std::string, std::shared_ptr<Process>> &Daemon::processes() {
    return m_processes;
}

void Daemon::run() {
    for (auto &[name, process]: m_processes)
        if (process->isAutostart())
            process->start();
}

}
