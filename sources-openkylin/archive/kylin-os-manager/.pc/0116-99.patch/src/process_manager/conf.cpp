#include "conf.h"

namespace process_manager {

ProcessConf::ProcessConf(std::string name, std::vector<std::string> commands, bool autostart, bool autorestart)
    : m_name(name)
    , m_commands(commands)
    , m_autostart(autostart)
    , m_autorestart(autorestart)
{

}

ProcessConf::~ProcessConf() {

}

std::string ProcessConf::name() {
    return m_name;
}

std::vector<std::string> ProcessConf::commands() {
    return m_commands;
}

bool ProcessConf::autostart() {
    return m_autostart;
}

bool ProcessConf::autorestart() {
    return m_autorestart;
}

}
