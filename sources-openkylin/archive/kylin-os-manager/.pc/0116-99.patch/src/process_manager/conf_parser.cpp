#include <algorithm>
#include <iostream>
#include "conf_parser.h"

#define PROGRAM_PREFIX "program:"
#define COMMAND_KEY "command"
#define AUTO_RESTART_KEY "autorestart"
#define AUTO_START_KEY "autostart"

namespace process_manager {

ConfParser::ConfParser(std::string path)
    : m_keyFile(g_key_file_new())
    , m_error(NULL)
{
    if (g_key_file_load_from_file(m_keyFile, path.c_str(), G_KEY_FILE_NONE, &m_error) == FALSE) {
        std::cout << "parser configure file fail. " << m_error->message << std::endl;
    }
}

ConfParser::~ConfParser() {
    if (m_keyFile)
        g_key_file_free(m_keyFile);
    if (m_error)
        g_error_free(m_error);
}

bool ConfParser::parseError() {
    return m_error != NULL;
}

std::set<std::string> ConfParser::processNames() {
    gsize length;
    gchar **groups;
    std::set<std::string> res;
    std::string prefix {PROGRAM_PREFIX};

    groups = g_key_file_get_groups(m_keyFile, &length);
    for (unsigned long int i = 0; i < length; i++) {
        std::string item {groups[i]};
        if (std::equal(std::begin(prefix), std::end(prefix), std::begin(item))
            && (item.size() > prefix.size()))
        {
            item.erase(0, prefix.size());
            res.emplace(item);
        }
    }
    g_strfreev(groups);

    return res;
}

std::vector<ProcessConf> ConfParser::processConfs() {
    std::vector<ProcessConf> res;

    std::set<std::string> names = processNames();
    for (const auto &name: names) {
        std::string group = PROGRAM_PREFIX + name;
        gchar *command = g_key_file_get_value(m_keyFile, group.c_str(), COMMAND_KEY, NULL);
        if (command == NULL) {
            command = g_strdup("");
        }
        gchar *autostart = g_key_file_get_value(m_keyFile, group.c_str(), AUTO_START_KEY, NULL);
        if (autostart == NULL) {
            autostart = g_strdup("true");
        }
        gchar *autorestart = g_key_file_get_value(m_keyFile, group.c_str(), AUTO_RESTART_KEY, NULL);
        if (autorestart == NULL) {
            autorestart = g_strdup("true");
        }

        std::vector<std::string> commands;
        gchar **commandVector = g_strsplit(command, " ", -1);
        for (int i = 0; commandVector[i] != NULL; i++) {
            std::string item {commandVector[i]};
            if (!item.empty()) {
                commands.emplace_back(item);
            }
        }
        res.emplace_back(name, commands, !g_strcmp0(autostart, "true"), !g_strcmp0(autorestart, "true"));
        g_strfreev(commandVector);
        g_free(command);
        g_free(autostart);
        g_free(autorestart);
    }

    return res;
}

}
