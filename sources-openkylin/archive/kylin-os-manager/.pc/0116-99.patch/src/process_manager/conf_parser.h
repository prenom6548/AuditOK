#ifndef KYLIN_OS_MANAGER_SRC_PROCESS_MANAGE_CONF_PARSER_H
#define KYLIN_OS_MANAGER_SRC_PROCESS_MANAGE_CONF_PARSER_H

#include <string>
#include <vector>
#include <set>
#include <glib.h>

#include "conf.h"

namespace process_manager {

class ConfParser {
public:
    ConfParser(std::string path);
    ~ConfParser();

    std::vector<ProcessConf> processConfs();
    std::set<std::string> processNames();
    bool parseError();

private:
    GKeyFile *m_keyFile;
    GError *m_error;
};

}

#endif
