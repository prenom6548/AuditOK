/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMSubCommand.h"
#include "common/kmtranslation.h"

KMSubCommand::KMSubCommand()
{
}

KMSubCommand::~KMSubCommand()
{
}

const std::vector<SubCommandEntry>& KMSubCommandFactory::subCommandEntrys()
{
    /**
     * brief: name为空串时，认为是一个子命令分组的开始
     */
    static std::vector<SubCommandEntry> subCommands = {

        { "create", _("Create an new project, include ${appid}/${appid}.km.yaml template") },
        { "build", _("Run a build command inside the build dir") },
        { "enter", _("Enter the build environment to execute commands, usually used for debugging") },
        // { "query", _("Query the kaiming packages installed locally or in the repository source.") },
        { "create-from-debsource", _("Create the kaiming project from the deb source.") },
        { "kch", _("Maintain changelog for kaiming packages.") },
        // { "hostmode-build", _("Run a build for hostapp") }
    };

    return subCommands;
}

std::unique_ptr<KMSubCommand> KMSubCommandFactory::createKMSubCommand(const std::string &subCommand)
{
    KMSubCommand *pObjetct = dynamic_cast<KMSubCommand *>(KMDynamicCreator::createObjectById("SubCommand_" + subCommand));
    return std::unique_ptr<KMSubCommand>(pObjetct);
}
