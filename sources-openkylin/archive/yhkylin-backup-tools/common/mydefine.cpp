/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "mydefine.h"
#include <QDBusMetaType>

// Marshall the MyStructure data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const BackupWrapper &backupWrapper)
{
    argument.beginStructure();
    argument << backupWrapper.m_type << backupWrapper.m_iPosition << backupWrapper.m_User << backupWrapper.m_comment << backupWrapper.m_backupName
             << backupWrapper.m_uuid << backupWrapper.m_backupPaths << backupWrapper.m_backupExcludePaths << backupWrapper.m_prefixDestPath
             << backupWrapper.m_note << backupWrapper.m_frontUserName << backupWrapper.m_frontUid
             << backupWrapper.m_gid << backupWrapper.m_isOtherMachine << backupWrapper.m_backupType << backupWrapper.m_size;
    argument.endStructure();
    return argument;
}

// Retrieve the MyStructure data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, BackupWrapper &backupWrapper)
{
    argument.beginStructure();
    argument >> backupWrapper.m_type >> backupWrapper.m_iPosition >> backupWrapper.m_User >> backupWrapper.m_comment >> backupWrapper.m_backupName
             >> backupWrapper.m_uuid >> backupWrapper.m_backupPaths >> backupWrapper.m_backupExcludePaths >> backupWrapper.m_prefixDestPath
             >> backupWrapper.m_note >> backupWrapper.m_frontUserName >> backupWrapper.m_frontUid
             >> backupWrapper.m_gid >> backupWrapper.m_isOtherMachine >> backupWrapper.m_backupType >> backupWrapper.m_size;
    argument.endStructure();
    return argument;
}

void BackupWrapper::registerMetaType()
{
    qRegisterMetaType<BackupWrapper>("BackupWrapper");
    qDBusRegisterMetaType<BackupWrapper>();
}
