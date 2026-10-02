/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMSubCommand.h"

KMSubCommand::KMSubCommand()
{
}

KMSubCommand::~KMSubCommand()
{
}

std::vector<SubCommandDescription> KMSubCommandFactory::m_subCommands;

void KMSubCommandFactory::setSubCommandDescriptions(const std::vector<SubCommandDescription> &subCommands)
{
    m_subCommands = subCommands;
}

const std::vector<SubCommandDescription>& KMSubCommandFactory::subCommandDescriptions()
{
    return m_subCommands;
}

std::unique_ptr<KMSubCommand> KMSubCommandFactory::createKMSubCommand(const std::string &subCommand)
{
    KMSubCommand *pObjetct = dynamic_cast<KMSubCommand *>(KMDynamicCreator::createObjectById(subCommand));
    return std::unique_ptr<KMSubCommand>(pObjetct);
}
