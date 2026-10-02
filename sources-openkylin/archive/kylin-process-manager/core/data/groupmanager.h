/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include "groupmanagementunit.h"
#include "resourcemanagerinterface.h"
#include <jsoncpp/json/json.h>

class GroupManager
{
public:
    GroupManager(const Json::Value &configs);
    bool isInit();
    void initUserProps();
    void createGroups();
    void removeGroups();
    void executePolicy(const std::string &policyId);

private:
    void initGroupUnits();

private:
    std::unique_ptr<GroupManagementUnitCreator> m_unitCreator;
    std::map<sched_policy::GroupType, std::unique_ptr<GroupManagementUnit>> m_groupUnits;
    std::shared_ptr<ResourceManagerInterface> m_resourceManagerInterface;
    Json::Value m_groupConfigs;
    bool m_isInit;
};

#endif // GROUPMANAGER_H
