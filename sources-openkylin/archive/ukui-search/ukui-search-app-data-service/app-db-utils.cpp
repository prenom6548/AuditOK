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

#include "app-db-utils.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QVector>

#include "application-property-helper.h"

namespace {

struct DesktopPathRow
{
    QString desktopFilePath;
    int launchTimes = 0;
    int favorites = 0;
    int launched = 0;
    int top = 0;
    int lock = 0;
};

int mergePosition(int lhs, int rhs)
{
    if (lhs <= 0) {
        return rhs;
    }
    if (rhs <= 0) {
        return lhs;
    }
    return qMin(lhs, rhs);
}

QVector<DesktopPathRow> loadMalformedRows(QSqlDatabase &database, bool *ok)
{
    QVector<DesktopPathRow> rows;
    QSqlQuery query(database);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "SELECT DESKTOP_FILE_PATH,LAUNCH_TIMES,FAVORITES,LAUNCHED,TOP,LOCK "
        "FROM APPINFO WHERE DESKTOP_FILE_PATH LIKE '%//%'"));
    if (!query.exec()) {
        qWarning() << "Fail to exec cmd" << query.lastQuery() << query.lastError();
        *ok = false;
        return rows;
    }

    while (query.next()) {
        DesktopPathRow row;
        row.desktopFilePath = query.value(QStringLiteral("DESKTOP_FILE_PATH")).toString();
        row.launchTimes = query.value(QStringLiteral("LAUNCH_TIMES")).toInt();
        row.favorites = query.value(QStringLiteral("FAVORITES")).toInt();
        row.launched = query.value(QStringLiteral("LAUNCHED")).toInt();
        row.top = query.value(QStringLiteral("TOP")).toInt();
        row.lock = query.value(QStringLiteral("LOCK")).toInt();
        rows.append(row);
    }

    *ok = true;
    return rows;
}

bool loadCurrentRow(QSqlDatabase &database, const QString &desktopFilePath, DesktopPathRow *row)
{
    QSqlQuery query(database);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "SELECT DESKTOP_FILE_PATH,LAUNCH_TIMES,FAVORITES,LAUNCHED,TOP,LOCK "
        "FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFilePath"));
    query.bindValue(QStringLiteral(":desktopFilePath"), desktopFilePath);
    if (!query.exec()) {
        qWarning() << "Fail to exec cmd" << query.lastQuery() << query.lastError();
        return false;
    }
    if (!query.next()) {
        return false;
    }

    row->desktopFilePath = query.value(QStringLiteral("DESKTOP_FILE_PATH")).toString();
    row->launchTimes = query.value(QStringLiteral("LAUNCH_TIMES")).toInt();
    row->favorites = query.value(QStringLiteral("FAVORITES")).toInt();
    row->launched = query.value(QStringLiteral("LAUNCHED")).toInt();
    row->top = query.value(QStringLiteral("TOP")).toInt();
    row->lock = query.value(QStringLiteral("LOCK")).toInt();
    return true;
}

bool updateExistingRow(QSqlDatabase &database,
                       const QString &desktopFilePath,
                       const DesktopPathRow &row)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "UPDATE APPINFO SET "
        "DESKTOP_FILE_PATH=:desktopFilePath,"
        "LAUNCH_TIMES=:launchTimes,"
        "FAVORITES=:favorites,"
        "LAUNCHED=:launched,"
        "TOP=:top,"
        "LOCK=:lock "
        "WHERE DESKTOP_FILE_PATH=:desktopFilePath"));
    query.bindValue(QStringLiteral(":desktopFilePath"), desktopFilePath);
    query.bindValue(QStringLiteral(":launchTimes"), row.launchTimes);
    query.bindValue(QStringLiteral(":favorites"), row.favorites);
    query.bindValue(QStringLiteral(":launched"), row.launched);
    query.bindValue(QStringLiteral(":top"), row.top);
    query.bindValue(QStringLiteral(":lock"), row.lock);
    if (!query.exec()) {
        qWarning() << desktopFilePath << "update failed!" << query.lastError();
        return false;
    }
    return true;
}

bool deleteMalformedRow(QSqlDatabase &database, const QString &desktopFilePath)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral("DELETE FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFilePath"));
    query.bindValue(QStringLiteral(":desktopFilePath"), desktopFilePath);
    if (!query.exec()) {
        qWarning() << desktopFilePath << "remove failed!" << query.lastError();
        return false;
    }
    return true;
}

bool renameMalformedRow(QSqlDatabase &database, const QString &fromPath, const QString &toPath)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "UPDATE APPINFO SET DESKTOP_FILE_PATH=:truePath WHERE DESKTOP_FILE_PATH=:errorPath"));
    query.bindValue(QStringLiteral(":truePath"), toPath);
    query.bindValue(QStringLiteral(":errorPath"), fromPath);
    if (!query.exec()) {
        qWarning() << fromPath << "repair failed!" << query.lastError();
        return false;
    }
    return true;
}

bool loadTopPosition(QSqlDatabase &database, const QString &desktopFilePath, uint *topPosition)
{
    QSqlQuery query(database);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral("SELECT TOP FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFilePath"));
    query.bindValue(QStringLiteral(":desktopFilePath"), desktopFilePath);
    if (!query.exec()) {
        qWarning() << query.lastQuery() << query.lastError();
        return false;
    }
    if (!query.next()) {
        qWarning() << "No app row found for top update:" << desktopFilePath;
        return false;
    }

    *topPosition = query.value(QStringLiteral("TOP")).toUInt();
    return true;
}

bool loadMaxTopPosition(QSqlDatabase &database, uint *maxTopPosition)
{
    QSqlQuery query(database);
    query.setForwardOnly(true);
    if (!query.exec(QStringLiteral("SELECT MAX(TOP) AS maxTop FROM APPINFO"))) {
        qWarning() << query.lastError() << query.lastQuery();
        return false;
    }
    if (!query.next()) {
        return false;
    }

    *maxTopPosition = query.value(QStringLiteral("maxTop")).toUInt();
    return true;
}

bool shiftTopRange(QSqlDatabase &database,
                   const QString &modifyTime,
                   uint lowerBound,
                   uint upperBound,
                   bool moveForward)
{
    const QString updateExpression = moveForward
                                     ? QStringLiteral("TOP=TOP+1")
                                     : QStringLiteral("TOP=TOP-1");
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
                      "UPDATE APPINFO SET MODIFYED_TIME=:modifyTime, %1 "
                      "WHERE TOP BETWEEN :lowerBound AND :upperBound")
                      .arg(updateExpression));
    query.bindValue(QStringLiteral(":modifyTime"), modifyTime);
    query.bindValue(QStringLiteral(":lowerBound"), qMin(lowerBound, upperBound));
    query.bindValue(QStringLiteral(":upperBound"), qMax(lowerBound, upperBound));
    if (!query.exec()) {
        qWarning() << "Fail to shift top range:" << query.lastError() << query.lastQuery();
        return false;
    }
    return true;
}

bool shiftTopAfterPosition(QSqlDatabase &database, const QString &modifyTime, uint topPosition)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "UPDATE APPINFO SET MODIFYED_TIME=:modifyTime, TOP=TOP-1 WHERE TOP > :topPosition"));
    query.bindValue(QStringLiteral(":modifyTime"), modifyTime);
    query.bindValue(QStringLiteral(":topPosition"), topPosition);
    if (!query.exec()) {
        qWarning() << query.lastQuery() << query.lastError();
        return false;
    }
    return true;
}

bool shiftTopFromPosition(QSqlDatabase &database, const QString &modifyTime, uint topPosition)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "UPDATE APPINFO SET MODIFYED_TIME=:modifyTime, TOP=TOP+1 WHERE TOP >= :topPosition"));
    query.bindValue(QStringLiteral(":modifyTime"), modifyTime);
    query.bindValue(QStringLiteral(":topPosition"), topPosition);
    if (!query.exec()) {
        qWarning() << query.lastQuery() << query.lastError();
        return false;
    }
    return true;
}

bool hasTopOccupant(QSqlDatabase &database, uint topPosition, bool *occupied)
{
    QSqlQuery query(database);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral("SELECT 1 FROM APPINFO WHERE TOP=:topPosition LIMIT 1"));
    query.bindValue(QStringLiteral(":topPosition"), topPosition);
    if (!query.exec()) {
        qWarning() << query.lastError() << query.lastQuery();
        return false;
    }

    *occupied = query.next();
    return true;
}

bool collectTopUpdates(QSqlDatabase &database,
                       uint lowerBound,
                       uint upperBound,
                       UkuiSearch::ApplicationInfoMap *updatedInfo)
{
    if (!updatedInfo) {
        return true;
    }

    QSqlQuery query(database);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "SELECT DESKTOP_FILE_PATH, TOP FROM APPINFO WHERE TOP BETWEEN :lowerBound AND :upperBound"));
    query.bindValue(QStringLiteral(":lowerBound"), qMin(lowerBound, upperBound));
    query.bindValue(QStringLiteral(":upperBound"), qMax(lowerBound, upperBound));
    if (!query.exec()) {
        qWarning() << query.lastError() << query.lastQuery();
        return false;
    }

    while (query.next()) {
        (*updatedInfo)[query.value(QStringLiteral("DESKTOP_FILE_PATH")).toString()]
            .insert(UkuiSearch::ApplicationProperty::Top, query.value(QStringLiteral("TOP")));
    }
    return true;
}

void recordTopUpdate(UkuiSearch::ApplicationInfoMap *updatedInfo,
                     const QString &desktopFilePath,
                     uint topPosition)
{
    if (!updatedInfo) {
        return;
    }
    (*updatedInfo)[desktopFilePath].insert(UkuiSearch::ApplicationProperty::Top,
                                           QVariant(topPosition));
}

bool writeTopPosition(QSqlDatabase &database,
                      const QString &desktopFilePath,
                      const QString &modifyTime,
                      uint topPosition)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "UPDATE APPINFO SET MODIFYED_TIME=:modifyTime, TOP=:topPosition "
        "WHERE DESKTOP_FILE_PATH=:desktopFilePath"));
    query.bindValue(QStringLiteral(":modifyTime"), modifyTime);
    query.bindValue(QStringLiteral(":topPosition"), topPosition);
    query.bindValue(QStringLiteral(":desktopFilePath"), desktopFilePath);
    if (!query.exec()) {
        qWarning() << "Set app top state failed!" << query.lastError();
        return false;
    }
    return true;
}

} // namespace

namespace UkuiSearch {

bool repairMalformedDesktopPaths(QSqlDatabase &database, bool *repaired)
{
    if (repaired) {
        *repaired = false;
    }

    bool ok = false;
    const QVector<DesktopPathRow> malformedRows = loadMalformedRows(database, &ok);
    if (!ok) {
        return false;
    }

    bool changed = false;
    bool success = true;
    for (const auto &malformedRow : malformedRows) {
        const QString cleanPath = QDir::cleanPath(malformedRow.desktopFilePath);
        if (cleanPath == malformedRow.desktopFilePath) {
            qDebug() << "Skip malformed path that cleanPath does not change:" << malformedRow.desktopFilePath;
            continue;
        }

        DesktopPathRow existingRow;
        const bool hasExistingRow = loadCurrentRow(database, cleanPath, &existingRow);

        if (!database.transaction()) {
            qWarning() << "Failed to start transaction mode!!!" << database.lastError();
            success = false;
            continue;
        }

        bool rowOk = false;
        if (hasExistingRow) {
            existingRow.launchTimes += malformedRow.launchTimes;
            existingRow.favorites = mergePosition(existingRow.favorites, malformedRow.favorites);
            existingRow.launched = existingRow.launched || malformedRow.launched;
            existingRow.top = mergePosition(existingRow.top, malformedRow.top);
            existingRow.lock = existingRow.lock || malformedRow.lock;
            rowOk = updateExistingRow(database, cleanPath, existingRow)
                    && deleteMalformedRow(database, malformedRow.desktopFilePath);
        } else {
            rowOk = renameMalformedRow(database, malformedRow.desktopFilePath, cleanPath);
        }

        if (!rowOk) {
            database.rollback();
            success = false;
            continue;
        }
        if (!database.commit()) {
            qWarning() << "Failed to commit!" << database.lastError();
            database.rollback();
            success = false;
            continue;
        }

        changed = true;
    }

    if (repaired) {
        *repaired = changed;
    }
    return success;
}

bool applyTopStateUpdate(QSqlDatabase &database,
                         const QString &desktopFilePath,
                         uint top,
                         ApplicationInfoMap *updatedInfo)
{
    if (updatedInfo) {
        updatedInfo->clear();
    }

    uint previousTop = 0;
    if (!loadTopPosition(database, desktopFilePath, &previousTop)) {
        return false;
    }
    if (previousTop == top) {
        qWarning() << "Top state has no changes, I quit!";
        return false;
    }

    uint maxTop = 0;
    if (!loadMaxTopPosition(database, &maxTop)) {
        return false;
    }

    if (top == 0) {
        if (previousTop < 1) {
            return false;
        }
    } else if (previousTop > 0) {
        if (maxTop < top) {
            qWarning() << QString("Max top pos is %0.To be moved to a invalid pos, I quit!!").arg(maxTop);
            return false;
        }
    } else if (maxTop + 1 < top) {
        qWarning() << QString("Max top pos is %0.To be moved to a invalid pos, I quit!!").arg(maxTop);
        return false;
    }

    if (!database.transaction()) {
        qWarning() << "Failed to start transaction mode!!!" << database.lastError();
        return false;
    }

    const QString modifyTime = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    bool ok = true;

    if (top == 0) {
        ok = shiftTopAfterPosition(database, modifyTime, previousTop)
             && collectTopUpdates(database, previousTop, maxTop, updatedInfo);
    } else if (previousTop == 0) {
        bool occupied = false;
        ok = hasTopOccupant(database, top, &occupied);
        if (ok && occupied) {
            ok = shiftTopFromPosition(database, modifyTime, top)
                 && collectTopUpdates(database, top, maxTop + 1, updatedInfo);
        }
    } else {
        ok = shiftTopRange(database, modifyTime, previousTop, top, previousTop > top)
             && collectTopUpdates(database, previousTop, top, updatedInfo);
    }

    if (!ok || !writeTopPosition(database, desktopFilePath, modifyTime, top)) {
        database.rollback();
        return false;
    }

    if (!database.commit()) {
        qWarning() << "Failed to commit !" << database.lastError();
        database.rollback();
        return false;
    }

    recordTopUpdate(updatedInfo, desktopFilePath, top);
    return true;
}

bool applyValueSetUpdates(QSqlDatabase &database,
                          const ApplicationInfoMap &appInfoMap,
                          ApplicationInfoMap *appliedUpdates)
{
    if (appliedUpdates) {
        appliedUpdates->clear();
    }

    bool success = true;
    for (auto iter = appInfoMap.constBegin(); iter != appInfoMap.constEnd(); ++iter) {
        const QString &desktopFilePath = iter.key();
        const ApplicationPropertyMap &propMap = iter.value();

        if (!database.transaction()) {
            qWarning() << "Failed to start transaction mode!!!" << database.lastError();
            return false;
        }

        bool desktopOk = true;
        QSqlQuery query(database);
        for (auto propIter = propMap.constBegin(); propIter != propMap.constEnd(); ++propIter) {
            const QString field = ApplicationPropertyHelper(propIter.key()).dataBaseField();
            if (field.isEmpty()) {
                qWarning() << "Set state failed! Invalid property field mapping:" << propIter.key();
                desktopOk = false;
                success = false;
                database.rollback();
                break;
            }
            query.prepare(QString(
                              // field comes from ApplicationPropertyHelper's fixed enum-to-column mapping.
                              "UPDATE appInfo SET MODIFYED_TIME=:modifyTime, %1=:value "
                              "WHERE DESKTOP_FILE_PATH=:desktopFilePath")
                              .arg(field));
            query.bindValue(QStringLiteral(":modifyTime"),
                            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            query.bindValue(QStringLiteral(":value"), propIter.value());
            query.bindValue(QStringLiteral(":desktopFilePath"), desktopFilePath);
            if (!query.exec()) {
                qWarning() << "Set state failed!" << query.lastError();
                desktopOk = false;
                success = false;
                database.rollback();
                break;
            }
        }

        if (!desktopOk) {
            continue;
        }

        if (!database.commit()) {
            qWarning() << "Failed to commit !" << database.lastError();
            database.rollback();
            success = false;
            continue;
        }

        if (appliedUpdates) {
            appliedUpdates->insert(desktopFilePath, propMap);
        }
    }

    return success;
}

} // namespace UkuiSearch
