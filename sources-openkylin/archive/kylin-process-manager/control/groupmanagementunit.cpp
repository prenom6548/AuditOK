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

#include "groupmanagementunit.h"
#include "loghelper.h"
#include "misc.h"
#include "profileaction.h"
#include "schedpolicy.h"
#include <QDebug>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{

const char *name_key = "Name";
const char *value_key = "Value";
const char *path_key = "Path";
const char *profiles_key = "Profiles";
const char *controller_key = "Controller";
const char *controllers_key = "Controllers";
const char *actions_key = "Actions";
const char *param_key = "Param";
const char *file_key = "File";
const char *set_attribute_action_name = "SetAttribute";
}

GroupManagementUnitCreator::GroupManagementUnitCreator(
    const std::shared_ptr<ResourceManagerInterface> &resourceInterface,
    const Json::Value &controllerList, const Json::Value &attributes)
    : m_controllerList(controllerList)
    , m_attributes(attributes)
    , m_resourceInterface(resourceInterface)
{
}

std::unique_ptr<GroupManagementUnit> GroupManagementUnitCreator::createGroupManagementUnit(const Json::Value &groupInfo)
{
    auto controllers = createControllers(groupInfo);
    auto groupUnit = std::unique_ptr<GroupManagementUnit>(
        new GroupManagementUnit(createProcessGroupInfo(groupInfo), std::move(controllers), m_resourceInterface));
    return groupUnit;
}

std::unique_ptr<ProcessGroupInfo> GroupManagementUnitCreator::createProcessGroupInfo(const Json::Value &groupInfo)
{
    auto groupName = groupInfo[name_key].asString();
    auto path = groupInfo[path_key].asString();
    auto type = sched_policy::groupTypeFromString(groupName);
    return std::unique_ptr<ProcessGroupInfo>(new ProcessGroupInfo{groupName, path, type});
}

GroupManagementUnit::PolicyControllerMap GroupManagementUnitCreator::createControllers(const Json::Value &groupInfo)
{
    GroupManagementUnit::PolicyControllerMap policyControllers;
    auto profiles = groupInfo[profiles_key];
    auto deviceModeNames = profiles.getMemberNames();
    for (const auto &deviceModeName : deviceModeNames) {
        auto deviceModeProfiles = profiles[deviceModeName];
        auto powerModeNames = deviceModeProfiles.getMemberNames();
        for (const auto &powerModeName : powerModeNames) {
            auto controllers = createResouceControllers(deviceModeName, powerModeName, deviceModeProfiles);
            auto policyId = sched_policy::generatePolicyId(deviceModeName, powerModeName);
            policyControllers[policyId] = std::move(controllers);
        }
    }

    return policyControllers;
}

std::vector<std::unique_ptr<ResourceController>> GroupManagementUnitCreator::createResouceControllers(
    const std::string &deviceMode, const std::string powerMode, const Json::Value &profiles)
{
    std::vector<std::unique_ptr<ResourceController>> resourceControllers;
    auto controllers = profiles[powerMode][controllers_key];
    if (controllers.empty()) {
        resourceControllers.emplace_back(std::unique_ptr<ResourceController>(new ResourceController("")));
        return resourceControllers;
    }
    for (Json::Value::ArrayIndex j = 0; j < controllers.size(); ++j) {
        if (controllers[j].empty()) {
            continue;
        }
        auto controllerName = controllers[j][name_key].asString();
        auto controllerValuesArray = controllers[j][value_key];
        std::vector<std::string> controllerValues;
        for (Json::Value::ArrayIndex h = 0; h < controllerValuesArray.size(); ++h) {
            controllerValues.emplace_back(controllerValuesArray[h].asString());
        }
        auto resourceController = createResourceController(controllerName, controllerValues);
        if (!resourceController) {
            PRINT_NOTICE("Resource controller '%s' is empty.\n", controllerName.c_str());
            continue;
        }
        resourceControllers.emplace_back(std::move(resourceController));
    }
    return resourceControllers;
}

std::unique_ptr<ResourceController> GroupManagementUnitCreator::createResourceController(
    const std::string &controllerName, const std::vector<std::string> &values)
{
    for (Json::Value::ArrayIndex i = 0; i < m_controllerList.size(); ++i) {
        auto name = m_controllerList[i][name_key].asString();
        if (name != controllerName) {
            continue;
        }

        auto actions = m_controllerList[i][actions_key];
        std::unique_ptr<ResourceController> resourceController(new ResourceController(name));
        if (values.size() != actions.size()) {
            PRINT_NOTICE("Can not find the controller '%s' info.\n", controllerName.c_str());
            return nullptr;
        }
        for (Json::Value::ArrayIndex j = 0; j < actions.size(); ++j) {
            auto actionName = actions[j][name_key].asString();
            auto actionParam = actions[j][param_key].asString();
            if (actionName == set_attribute_action_name) {
                auto attribute = createProfileAttribute(actionParam);
                std::unique_ptr<ProfileAction> action(
                    new SetAttributeAction(std::move(attribute), values[j], m_resourceInterface));
                resourceController->addAction(std::move(action));
            }
        }
        return resourceController;
    }
    return nullptr;
}

std::unique_ptr<ProfileAttribute> GroupManagementUnitCreator::createProfileAttribute(const std::string &attrName)
{
    for (Json::Value::ArrayIndex i = 0; i < m_attributes.size(); ++i) {
        auto name = m_attributes[i][name_key].asString();
        if (attrName != name) {
            continue;
        }
        auto controllerName = m_attributes[i][controller_key].asString();
        auto fileAttr = m_attributes[i][file_key].asString();
        return std::unique_ptr<ProfileAttribute>(new CgroupProfileAttribute{name, controllerName, fileAttr});
    }
    return nullptr;
}

GroupManagementUnit::GroupManagementUnit(
    std::shared_ptr<ProcessGroupInfo> groupInfo, PolicyControllerMap policyControllers,
    const std::shared_ptr<ResourceManagerInterface> &resourceInterface)
    : m_groupInfo(groupInfo)
    , m_policyControllers(std::move(policyControllers))
    , m_resourceInterface(resourceInterface)
{
}

void GroupManagementUnit::createGroupPath()
{
    m_resourceInterface->createGroupPath(m_groupInfo->name);
}

void GroupManagementUnit::removeGroupPath()
{
    m_resourceInterface->removeGroupPath(m_groupInfo->name);
}

void GroupManagementUnit::executeResourceLimit(const std::string &policyId)
{
    if (m_policyControllers.find(policyId) == m_policyControllers.end()) {
        PRINT_DBG("Exceute resource limit failed, can not find the policy id %s.\n", policyId.c_str());
        return;
    }

    for (const auto &controller : m_policyControllers[policyId]) {
        controller->executeForCgroup(m_groupInfo->name);
    }
}

std::vector<std::string> GroupManagementUnit::cgroupControllerNames() const
{
    std::vector<std::string> controllerNames;

    auto mergeControllerNames = [&controllerNames](const std::vector<std::string> &newNames) {
        for (const auto &name : newNames) {
            auto it = std::find(controllerNames.begin(), controllerNames.end(), name);
            if (it == controllerNames.end()) {
                controllerNames.emplace_back(name);
            }
        }
    };

    for (const auto &controllers : m_policyControllers) {
        for (const auto &controller : controllers.second) {
            mergeControllerNames(controller->cgroupControllerNames());
        }
    }

    return controllerNames;
}
