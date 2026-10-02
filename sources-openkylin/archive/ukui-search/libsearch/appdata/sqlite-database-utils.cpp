/*
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "sqlite-database-utils.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

constexpr int kDefaultSqliteBusyTimeoutMs = 5000;

bool execPragma(QSqlDatabase &database, const QString &statement, QString *errorMessage)
{
    QSqlQuery query(database);
    if (!query.exec(statement)) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

} // namespace

namespace UkuiSearch {

int defaultSqliteBusyTimeoutMs()
{
    return kDefaultSqliteBusyTimeoutMs;
}

bool configureSqliteDatabase(QSqlDatabase &database,
                             SqliteDatabaseMode mode,
                             QString *errorMessage)
{
    // QSQLITE_BUSY_TIMEOUT is applied by the Qt SQLite driver when opening the connection.
    database.setConnectOptions(QStringLiteral("QSQLITE_BUSY_TIMEOUT=%1")
                                   .arg(defaultSqliteBusyTimeoutMs()));
    if (!database.open()) {
        if (errorMessage) {
            *errorMessage = database.lastError().text();
        }
        return false;
    }

    if (mode == SqliteDatabaseMode::FileBacked) {
        QSqlQuery journalModeQuery(database);
        if (!journalModeQuery.exec(QStringLiteral("PRAGMA journal_mode=WAL"))) {
            if (errorMessage) {
                *errorMessage = journalModeQuery.lastError().text();
            }
            return false;
        }
        if (!journalModeQuery.next()
            || journalModeQuery.value(0).toString().compare(QStringLiteral("wal"),
                                                            Qt::CaseInsensitive)
                   != 0) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to enable WAL mode.");
            }
            return false;
        }

        if (!execPragma(database, QStringLiteral("PRAGMA synchronous=NORMAL"), errorMessage)) {
            return false;
        }
    }

    return true;
}

} // namespace UkuiSearch
