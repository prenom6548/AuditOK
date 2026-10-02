#ifndef KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_CONF_H
#define KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_CONF_H

#include <string>
#include <vector>

namespace process_manager {

class ProcessConf {
public:
    ProcessConf(std::string name, std::vector<std::string> commands, bool autostart, bool autorestart);
    ~ProcessConf();

    std::string name();
    std::vector<std::string> commands();
    bool autostart();
    bool autorestart();

private:
    std::string m_name;
    std::vector<std::string> m_commands;
    bool m_autostart;
    bool m_autorestart;
};

}

#endif
