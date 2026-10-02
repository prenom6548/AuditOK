#ifndef KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_DAEMON_H
#define KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_DAEMON_H

#include <vector>
#include <map>
#include <memory>
#include <utility>
#include "process.h"
#include "conf.h"

namespace process_manager {

class Daemon {
public:
    Daemon(std::vector<ProcessConf> processConfs);
    ~Daemon();

    void run();
    std::pair<bool, std::string> startProcess(const std::string &name);
    std::pair<bool, std::string> stopProcess(const std::string &name);
    std::pair<bool, std::string> restart(const std::string &name);
    const std::map<std::string, std::shared_ptr<Process>> &processes();

private:
    std::map<std::string, std::shared_ptr<Process>> m_processes;
};

}

#endif
