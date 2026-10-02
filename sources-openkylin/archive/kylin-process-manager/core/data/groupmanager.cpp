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

#include "groupmanager.h"
#include "loghelper.h"
#include "processinfohelper.h"
#include "resourcemanagerinterface.h"
#include "systemddbusinterface.h"
#include <QDebug>

GroupManager::GroupManager(const Json::Value &configs)
    : m_groupConfigs(configs)
    , m_resourceManagerInterface(std::make_shared<ResourceManagerInterface>())
    , m_isInit(false)
{

}

bool GroupManager::isInit()
{
    return m_isInit;
}

void GroupManager::initUserProps()
{
    if (m_isInit)
        return;
    if (m_resourceManagerInterface->setCurrentUserProperty()) {
        initGroupUnits();
        m_isInit = true;
    }
}

void GroupManager::createGroups()
{
    // 创建cgroup路径
    for (const auto &cgroupUnit : m_groupUnits) {
        cgroupUnit.second->createGroupPath();
    }
    m_resourceManagerInterface->setGroupEnable(true);
}

void GroupManager::removeGroups()
{
    // 删除cgroup路径
    m_resourceManagerInterface->setGroupEnable(false);
    for (const auto &cgroupUnit : m_groupUnits) {
        cgroupUnit.second->removeGroupPath();
    }
}

void GroupManager::executePolicy(const std::string &policyId)
{
    // 设置cgroup限制值
    for (const auto &cgroupUnit : m_groupUnits) {
        cgroupUnit.second->executeResourceLimit(policyId);
    }
}

void GroupManager::initGroupUnits()
{
    auto controllers = m_groupConfigs["ControllerList"];
    auto attributes = m_groupConfigs["Attributes"];
    auto groups = m_groupConfigs["AggregateProfiles"];

    m_unitCreator.reset(new GroupManagementUnitCreator(
        m_resourceManagerInterface, controllers, attributes));

    for (Json::Value::ArrayIndex i = 0; i < groups.size(); ++i) {
        auto unit = m_unitCreator->createGroupManagementUnit(groups[i]);
        m_resourceManagerInterface->setGroupPath(unit->groupName(), unit->groupPath());
        m_groupUnits[unit->groupType()] = std::move(unit);
    }
}