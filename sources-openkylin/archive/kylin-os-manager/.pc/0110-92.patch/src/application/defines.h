#ifndef KYLIN_OS_MANAGER_SRC_DEFINES_H
#define KYLIN_OS_MANAGER_SRC_DEFINES_H

#include <QString>

enum class CommandType {
    HELP,
    VERSION,
    NONE
};

struct CommandLineArgument {
    CommandLineArgument()
        : type(CommandType::NONE)
        , jumpTabName("") {}

    CommandType type;
    QString jumpTabName;
};

#endif
