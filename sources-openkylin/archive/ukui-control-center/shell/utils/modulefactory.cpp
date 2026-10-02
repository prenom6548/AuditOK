/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "modulefactory.h"
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QJsonParseError>
#include <QJsonObject>
#include "interface.h"

int ModulesFactory::totalModule;
QList<int> ModulesFactory::moduleTypeList;
QList<QPair<QString, QString>> ModulesFactory::pluginToModuleNameList;
QList<QPair<QString, QString>> ModulesFactory::pluginToModuleLocaleNameList;
QVector<ModuleInfo *> ModulesFactory::moduleInfoVec;
QMap<int, QMap<QString, QPointer<QObject>>> ModulesFactory::moduleTypeToPluginMap;

const QString MODULEPATH = "/usr/share/ukui-control-center/data/ukui-control-center-config.json";

ModulesFactory::ModulesFactory() {
    clear();
}

ModulesFactory::~ModulesFactory() {
    clear();
}

void ModulesFactory::clear() {
    totalModule = 0;
    moduleInfoVec.clear();
    moduleTypeList.clear();
    moduleTypeToPluginMap.clear();
    pluginToModuleNameList.clear();
    pluginToModuleLocaleNameList.clear();
}

int ModulesFactory::size() {
    return totalModule;
}

void ModulesFactory::loadConfig() {
    QFile file(MODULEPATH);
    moduleInfoVec.clear();
    if (!file.exists()) {
        qDebug() << "file not exist:" << MODULEPATH;
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "file open failed:" << MODULEPATH;
        return;
    }
    QByteArray readBy = file.readAll();
    file.close();
    QJsonParseError error;
    QJsonDocument moduleDoc = QJsonDocument::fromJson(readBy, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "ModuleFactory parseJson failed:" << error.errorString();
        return;
    }
    if (!moduleDoc.isObject()) {
        qDebug() << "ModuleFactory parseJson failed: document is not an object";
        return;
    }
    QJsonObject modulesObj = moduleDoc.object();
    if (!modulesObj.contains("ukcc")) {
        qDebug() << "ModuleFactory has not ukcc item";
        return;
    }
    QJsonValue ukccValue = modulesObj.value("ukcc");
    if (!ukccValue.isArray()) {
        qDebug() << "ModuleFactory ukcc item is not an array";
        return;
    }
    QJsonArray moduleArr = ukccValue.toArray();
    totalModule = moduleArr.size();
    for (int order = 0; order < totalModule; order++) {
        QJsonValue moduleValue = moduleArr[order];
        if (!moduleValue.isObject()) {
            qDebug() << "ModuleFactory module at index" << order << "is not an object";
            continue;
        }
        QJsonObject moduleObj = moduleValue.toObject();
        QString moduleName = moduleObj["name"].toString().toLower();
        QString localIconName = moduleObj["local_icon_name"].toString();
        QString themeIconName = moduleObj["theme_icon_name"].toString();
        int moduleType = moduleObj["module_type"].toInt();
        QJsonValue localeValue = moduleObj.value("name_locale");
        QJsonObject localeObj;
        if (localeValue.isObject()) {
            localeObj = localeValue.toObject();
        }
        int moduleOrder = order;
        ModuleInfo* curModuleInfo = new ModuleInfo(moduleName, moduleOrder, moduleType, themeIconName, localIconName);
        if (curModuleInfo == nullptr) {
            continue;
        }
        int totalLocale = localeObj.size();
        for (int localeIdx = 0; localeIdx < totalLocale; ++localeIdx) {
            QString key = localeObj.keys().at(localeIdx);
            QString value = localeObj[key].toString();
            curModuleInfo->appendNameLocale(key, value);
        }
        QJsonValue childnodeValue = moduleObj.value("childnode");
        if (!childnodeValue.isArray()) {
            qDebug() << "ModuleFactory childnode is not an array for module" << moduleName;
            moduleInfoVec.append(curModuleInfo);
            continue;
        }
        QJsonArray pluginArr = childnodeValue.toArray();
        int totalPlugin = pluginArr.size();
        for (int pluginIdx = 0; pluginIdx < totalPlugin; ++pluginIdx) {
            QJsonValue pluginValue = pluginArr[pluginIdx];
            if (!pluginValue.isObject()) {
                qDebug() << "ModuleFactory plugin at index" << pluginIdx << "is not an object";
                continue;
            }
            QJsonObject pluginObj = pluginValue.toObject();
            if (!pluginObj.keys().contains("name")) {
                continue;
            }
            QString pluginname = pluginObj["name"].toString().toLower();
            curModuleInfo->appendPluginNameList(pluginIdx, pluginname);
        }
        moduleInfoVec.append(curModuleInfo);
    }
    std::sort(moduleInfoVec.begin(), moduleInfoVec.end(),
              [&](const ModuleInfo* lModuleInfo, const ModuleInfo* rModuleInfo) {
        if (lModuleInfo->moduleOrder != rModuleInfo->moduleOrder) {
            return lModuleInfo->moduleOrder < rModuleInfo->moduleOrder;
        }
        return false;
    });
    foreach (ModuleInfo* moduleInfo, moduleInfoVec) {
        QMap<QString, QPointer<QObject>> pluginsMaps;
        int moduleType = moduleInfo->moduleType;
        moduleTypeToPluginMap.insert(moduleType, pluginsMaps);
        qDebug() << "modulename: " <<moduleInfo->moduleName << moduleInfo->moduleOrder;
    }
    totalModule = moduleInfoVec.size();
}

void ModulesFactory::loadPluginInfo(QObject* pluginObj)
{
    if (!pluginObj) {
        return;
    }
    CommonInterface* pluginInstance = qobject_cast<CommonInterface*>(pluginObj);
    if (!pluginInstance) {
        return;
    }
    QString name = pluginInstance->name().toLower();
    QString i18nName = pluginInstance->plugini18nName();
    int moduleType = pluginInstance->pluginTypes();
    bool isShow = pluginInstance->isShowOnHomePage();
    bool isEnable = pluginInstance->isEnable();
    ModuleInfo* moduleInfo = getModuleInfoByType(moduleType);
    if (moduleInfo == nullptr) {
        return;
    }
    QList<QString>& pluginName = moduleInfo->pluginNameList;
    QList<PluginInfo>& pluginInfoList = moduleInfo->pluginInfoList;

    // 查找当前加载的插件序号i
    int pluginIdx = 0;
    for (; pluginIdx < pluginName.size(); pluginIdx++) {
        if (name.compare(pluginName.at(pluginIdx), Qt::CaseInsensitive) == 0) {
            break;
        }
    }
    PluginInfo pluginInfo;
    pluginInfo.oriNameString = pluginInstance->name();
    pluginInfo.nameString = name;
    pluginInfo.namei18nString = i18nName;
    pluginInfo.type = moduleType;
    pluginInfo.mainShow = isShow;
    pluginInfo.isEnable = isEnable;
    QPair<QString, QString> data;
    data.first = pluginInfo.nameString;
    data.second = moduleInfo->moduleName;
    pluginToModuleNameList.append(data);
    QPair<QString, QString> data1;
    data1.first = pluginInfo.nameString;
    const QString locale = QLocale::system().name();
    data1.second = moduleInfo->getModuleNameLocale(locale);
    pluginToModuleLocaleNameList.append(data1);
    if (!moduleTypeList.contains(pluginInfo.type)) {
        moduleTypeList.append(pluginInfo.type);
    }
    int pluginInfoListSize = pluginInfoList.size();
    if (pluginInfoListSize == 0) {
        pluginInfoList.append(pluginInfo);
    } else {
        bool isInsert = false;
        // 遍历原有list，对比pluginInfo的序号与list中每个元素序号的关系
        for (int preItemIndex = pluginInfoListSize - 1; preItemIndex >= 0; preItemIndex--) {
            int nItemIndex = 0;
            for (nItemIndex = 0; nItemIndex < pluginName.count(); nItemIndex++) {
                if (pluginInfoList[preItemIndex].nameString.toLower().compare(pluginName.at(nItemIndex).toLower(), Qt::CaseInsensitive) == 0) {
                    break;
                }
            }
            // 原有list元素序号小于新的module序号，将module插入该元素之后
            if (nItemIndex <= pluginIdx) {
                pluginInfoList.insert(preItemIndex + 1, pluginInfo);
                isInsert = true;
                break;
            }
        }
        if (!isInsert) {
            pluginInfoList.insert(0, pluginInfo);
        }
    }
    moduleTypeToPluginMap[moduleType].insert(i18nName, pluginObj);
}

QString ModulesFactory::getModuleNamebyName(QString pluginName)
{
    QString moduleName = "";
    for (auto it : pluginToModuleNameList) {
        if (it.first.toLower() == pluginName.toLower()) {
            moduleName = it.second;
            break;
        }
    }
    return moduleName;
}

QString ModulesFactory::getModuleLocaleNamebyName(QString pluginName)
{
    QString moduleName = "";
    for (auto it : pluginToModuleLocaleNameList) {
        if (it.first.toLower() == pluginName.toLower()) {
            moduleName = it.second;
            break;
        }
    }
    return moduleName;
}

PluginInfo* ModulesFactory::getPluginInfoByName(QString pluginName) {
    for (int moduleIdx = 0; moduleIdx < totalModule; moduleIdx++) {
        for (int pluginIdx = 0; pluginIdx < moduleInfoVec.at(moduleIdx)->pluginInfoList.size(); pluginIdx++) {
            if (!moduleInfoVec.at(moduleIdx)->pluginInfoList[pluginIdx].nameString.compare(pluginName, Qt::CaseInsensitive)) {
                return &moduleInfoVec.at(moduleIdx)->pluginInfoList[pluginIdx];
            }
        }
    }
    return nullptr;
}

QObject* ModulesFactory::getPluginObjectByName(QString pluginil8Name) {
    for (int idx = 0; idx < moduleTypeToPluginMap.size(); ++idx) {
        if (moduleTypeToPluginMap[idx].contains(pluginil8Name)) {
            return moduleTypeToPluginMap[idx][pluginil8Name];
        }
    }
    return nullptr;
}

QObject* ModulesFactory::getPluginObjectByType(int moduleType, QString pluginil8Name) {
    if (moduleTypeToPluginMap.contains(moduleType)) {
        return nullptr;
    }
    QMap<QString, QPointer<QObject>> curPluginMap = moduleTypeToPluginMap[moduleType];
    if (!curPluginMap.empty() && curPluginMap.keys().contains(pluginil8Name)) {
        return curPluginMap.value(pluginil8Name);
    }
    return nullptr;
}

ModuleInfo* ModulesFactory::getModuleInfoByIndex(int index) {
    if (!checkModuleIndex(index)) {
        return nullptr;
    }
    return moduleInfoVec[index];
}

ModuleInfo* ModulesFactory::getModuleInfoByType(int moduleType) {
    for (int moduleIdx = 0; moduleIdx < totalModule; moduleIdx++) {
        int curModuleType = moduleInfoVec[moduleIdx]->moduleType;
        if (moduleType != curModuleType) {
            continue;
        }
        return moduleInfoVec[moduleIdx];
    }
    return nullptr;
}

bool ModulesFactory::checkPluginExist(QString pluginil8Name) {
    for (int idx = 0; idx < moduleTypeToPluginMap.size(); ++idx) {
        if (moduleTypeToPluginMap[idx].contains(pluginil8Name)) {
            return true;
        }
    }
    return false;
}

bool ModulesFactory::checkModuleIndex(int index) {
    int moduleSize = moduleInfoVec.size();
    if (index >= 0 && index < moduleSize) {
        return true;
    }
    return false;
}

bool ModulesFactory::checkModuleType(int moduleType) {
    return moduleTypeList.contains(moduleType);
}

void ModulesFactory::pluginDelete() {
    QMap<QString, QPointer<QObject>> moduleMap;
    for (auto plugin : moduleTypeList) {
        if (!moduleTypeToPluginMap.contains(plugin)) {
            continue;
        }
        moduleMap = moduleTypeToPluginMap[plugin];
        QMap<QString, QPointer<QObject>>::const_iterator it;
        for (it = moduleMap.constBegin(); it != moduleMap.constEnd(); ++it)  {
            if (it.value().isNull()) {
                continue;
            }
            CommonInterface* pluginInstance = qobject_cast<CommonInterface*>(it.value());
            if (pluginInstance) {
                qInfo() << "~exit && delete-plugin:" << pluginInstance->name();
                delete pluginInstance;
            }
        }
    }
}
