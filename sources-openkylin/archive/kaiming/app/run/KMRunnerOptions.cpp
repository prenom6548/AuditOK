/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMRunnerOptions.h"

#include <sstream>
#include <iomanip>
#include <unistd.h>

#include "common/KMLogger.h"
#include "common/kmtranslation.h"

class KMRunnerOptions::Private
{
public:
    bool isHelp;
    bool isDevel;
    bool isFileWarding;
    std::string detach;
    std::string command;
    std::string ref;
    std::string m_splitOfArgs;
    std::vector<std::string> args;
};

KMRunnerOptions::KMRunnerOptions()
{
    d_ptr = std::make_shared<Private>();
}

void KMRunnerOptions::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMRunnerOptions::postParseHook()
{
    if (d_ptr->isHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    // 为兼容之前部分不设置--同样可以执行的场景，此处重设参数
    d_ptr->args.clear();
    // kaiming run [OPTION…] ref [[--] args]
    int posRef = getPositionalOptionIndex("ref");
    int steps = d_ptr->m_splitOfArgs.empty() ? 1 : 2;
    if (posRef > 0 && m_originalArgs.size() > posRef + steps)
    {
        for (int i = posRef + steps; i < m_originalArgs.size(); i++)
        {
            d_ptr->args.emplace_back(m_originalArgs.at(i));
        }
    }
}

/**
 * @brief 命令行参数添加描述选项
 */
void KMRunnerOptions::addOptions()
{
    setDescription(_("\nDescribe:\n \tkaiming run [OPTION…] ref [[--] args] - run the installed application\n"));

    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&d_ptr->isHelp)->defaultValue(false), _("display Help Options"));
    addOption("devel", "d", KMOption::value<bool>(&d_ptr->isDevel)->defaultValue(false), _("using the development runtime"));
    addOption("file-forwarding", "f", KMOption::value<bool>(&d_ptr->isFileWarding)->defaultValue(false), _("enable file forwarding"));
    addOption("detach", "s", KMOption::value<std::string>(&d_ptr->detach), _("detach the main process\n"
                                                                             "true: detach the main process\n"
                                                                             "false: do not detach the main process\n"
                                                                             "usage: "));
    addOption("command", "c", KMOption::value<std::string>(&d_ptr->command), _("display running command example: --command=bash"));

    //位置选项
    std::stringstream ss;
    ss << _("ref support types:") << "\n"
       << std::setw(34) << " "
       << "id/version\n"
       << std::setw(34) << " "
       << "id/version/module\n"
       << std::setw(34) << " "
       << "id/version/module/channel";
    addPositionOption("ref", KMOption::value<std::string>(&d_ptr->ref), 1, ss.str());
    addPositionOption("--", KMOption::value(&d_ptr->m_splitOfArgs), 1, _("The start flag of command's arguments(not kaiming's options)"));
    addPositionOption("args", KMOption::value<std::vector<std::string>>(&d_ptr->args), -1, _("command parameter\n"));
}

/**
 * @brief 获取命令行command参数
 *  @return 应用要求
 */
std::string KMRunnerOptions::getCommand() const
{
    return d_ptr->command;
}

/**
 * @brief 获取命令行要运行的应用程序及相关参数的内容
 */
std::string KMRunnerOptions::getAppId() const
{
    return d_ptr->ref;
}

/**
 * @brief 获取启动参数
 * @return vector<string> 返回启动参数
 */
const std::vector<std::string> &KMRunnerOptions::getArgs() const
{
    return d_ptr->args;
}

/**
 * @brief 获取devel开发运行时
 * @return bool true 开发运行时, false 非开发运行时
 */
bool KMRunnerOptions::isDevel() const
{
    return d_ptr->isDevel;
}

/**
 * @brief 获取是否启动文件转发
 * @return bool true 启动, false 不启动
 */
bool KMRunnerOptions::isFileWarding() const
{
    return d_ptr->isFileWarding;
}

/**
 * @brief 获取是否分离参数
 * @return bool true 分离, false 不分离
 */
bool KMRunnerOptions::isDetach() const
{
    // 终端运行不能分离
    if (isatty(fileno(stdin)) == 1)
    {
        return false;
    }
    if (d_ptr->detach == "true")
    {
        return true;
    }
    else if (d_ptr->detach == "false")
    {
        return false;
    }
    // 其他情况执行默认策略,终端运行则不分离,非终端运行则分离
    return (isatty(fileno(stdin)) == 0);
}
