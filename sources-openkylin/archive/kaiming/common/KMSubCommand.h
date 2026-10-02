/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMSUBCOMMAND_H__
#define __KMSUBCOMMAND_H__

#include <memory>
#include <vector>

#include "common/KMDynamicCreator.h"
#include "common/KMObject.h"

/**
 * @brief : 子命令基类
 */
class KMSubCommand : public KMObject
{
public:
    explicit KMSubCommand();
    virtual ~KMSubCommand();

    virtual int dispose(int argc, char **argv) = 0;
};

struct SubCommandDescription
{
    std::string m_name;
    std::string m_description;
};

class KMSubCommandFactory
{
public:
    static void setSubCommandDescriptions(const std::vector<SubCommandDescription> &subCommands);
    static const std::vector<SubCommandDescription> &subCommandDescriptions();
    static std::unique_ptr<KMSubCommand> createKMSubCommand(const std::string &subCommand);

private:
    KMSubCommandFactory() = delete;
    ~KMSubCommandFactory() = delete;

    static std::vector<SubCommandDescription> m_subCommands;
};

#define REGISTER_SUBCOMMAND_DYNCREATE(subcommand, class_type) REGISTER_DYNCREATE(subcommand, class_type)

#endif // !__KMSUBCOMMAND_H__
