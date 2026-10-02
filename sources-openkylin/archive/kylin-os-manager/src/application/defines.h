#ifndef KYLIN_OS_MANAGER_SRC_DEFINES_H
#define KYLIN_OS_MANAGER_SRC_DEFINES_H

#include <QString>

// 错误码
#define ERROR_COMMAND_LINE_PARSING_FAILED 1    // 命令行参数解析失败
#define ERROR_COMMAND_LINE_VERIFY_FAILED  2    // 命令行参数验证失败

enum class CommandType {
    HELP,
    VERSION,
    NONE
};

struct CommandLineArgument {
    CommandLineArgument(): type(CommandType::NONE), repair(false) {}

    CommandType type;
    // 启动时跳转到指定的插件
    QString     jumpTabName;
    QString     jumpSecondaryName;
    // 故障检测
    bool        repair;       // 修复指定错误
    QString     error_code;
    QString     error_data;
    QString     module;
    QString     path;
    bool        diagnosis;    // 检测指定类型
    QString     diagnosis_type;
};

#endif
