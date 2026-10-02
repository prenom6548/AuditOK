/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "CacheCreate.h"

CacheCreate::CacheCreate(QObject *parent):CacheBase(parent)
{
}

void CacheCreate::createTable()
{
    const int curVersion = m_dataBaseInterface->userVersion();

    if (curVersion < s_dbVersion) {
        if (curVersion < 2 && !m_migration_2_0_Done) {
            qInfo() << "CacheCreate::createTable upgrading database from" << curVersion
                    << "to" << s_dbVersion;
            migrate_2_0();
            m_migration_2_0_Done = true;
        }
        createAllTables();
        if (!m_dataBaseInterface->setUserVersion(s_dbVersion)) {
            qWarning() << "CacheCreate::createTable failed to set user_version";
        }
    } else if (curVersion > s_dbVersion) {
        qWarning() << "CacheCreate::createTable database user_version" << curVersion
                   << "is newer than code version" << s_dbVersion
                   << "; skipping migration";
        createAllTables();
    } else {
        createAllTables();
    }
}

void CacheCreate::createAllTables() const
{
    // 全部 7 张表使用 IF NOT EXISTS，支持自愈补建
    m_dataBaseInterface->createTable(s_userInfoTableCreateCmd);
    m_dataBaseInterface->createTable(s_commonInfoTableCreateCmd);
    m_dataBaseInterface->createTable(s_reuploadTableCreateCmd);
    qDebug() << "create Tid table:" << m_dataBaseInterface->createTable(s_tidInfoTableCreateCmd);
    m_dataBaseInterface->createTable(s_stateInfoTableCreateCmd);
    m_dataBaseInterface->createTable(s_eventMessageTableCreateCmd);
    m_dataBaseInterface->createTable(s_eventReuploadMessageTableCreateCmd);
}

void CacheCreate::migrate_2_0() const
{
    bool ok = true;
    ok = m_dataBaseInterface->createTable("DROP TABLE IF EXISTS Data4") && ok;
    ok = m_dataBaseInterface->createTable("DROP TABLE IF EXISTS Data5") && ok;
    if (!ok) {
        qWarning() << "CacheCreate::migrate_2_0 failed to drop old Data4/Data5";
    }
}
