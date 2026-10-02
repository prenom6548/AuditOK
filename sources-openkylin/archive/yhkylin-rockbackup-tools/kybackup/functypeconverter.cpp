/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/


#include "functypeconverter.h"

FuncTypeConverter::FuncTypeConverter()
{
    m_metaModule = QMetaEnum::fromType<FuncTypeConverter::FunType>();
    m_mapAssociation.insert(BACKUP_SYSTEM, { tr("System Backup"), "ukui-bf-system-backup-symbolic", "SystemBackup" });
    m_mapAssociation.insert(RESTORE_SYSTEM, {tr("System Recovery"), "ukui-bf-system-restore-symbolic", "SystemRestore" });
    m_mapAssociation.insert(BACKUP_DATA, { tr("Data Backup"), "ukui-bf-data-backup-symbolic", "DataBackup" });
    m_mapAssociation.insert(RESTORE_DATA, { tr("Data Recovery"), "ukui-bf-data-restore-symbolic", "DataRestore" });
    m_mapAssociation.insert(OPERATION_LOG, { tr("Log Records"), "ukui-catalogue-symbolic", "OperationLog" });
    m_mapAssociation.insert(GHOST_IMAGE, { tr("Ghost Image"), "ukui-bf-ghost-mirror-symbolic", "GhostImage" });
}

FuncTypeConverter::~FuncTypeConverter()
{
}

// 枚举值对应的枚举变量名
QString FuncTypeConverter::keyCodeToKeyString(int code){
    // 未匹配到则返回空串
    return m_metaModule.valueToKey(code);
}

// 枚举变量名对应的枚举值
int FuncTypeConverter::keystringTokeycode(QString string){
    return m_metaModule.keyToValue(string.toLocal8Bit().data());
}

/**
 * @brief 功能类型对应的功能名称
 * @param funcType 功能类型
 * @return 功能名称
 */
QString FuncTypeConverter::funcTypeToFuncName(int funcType)
{
    return m_mapAssociation.value(funcType).m_funcName;
}

/**
 * @brief 功能类型对应的主题图标名称
 * @param funcType 功能类型
 * @return 主题图标名称
 */
QString FuncTypeConverter::funcTypeToThemeIconName(int funcType)
{
    return m_mapAssociation.value(funcType).m_themeIconName;
}

/**
 * @brief 功能类型对应的窗口类名
 * @param funcType
 * @return 窗口类名
 */
QString FuncTypeConverter::funcTypeToWidgetClassName(int funcType)
{
    return m_mapAssociation.value(funcType).m_widgetClassName;
}
