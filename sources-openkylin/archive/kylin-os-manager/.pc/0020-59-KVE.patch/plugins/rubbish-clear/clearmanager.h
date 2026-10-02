/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef CLEARMANAGER_H
#define CLEARMANAGER_H

#include <QObject>
#include <QStringList>
namespace KylinRubbishClear
{

class ClearManagerPri : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void sigScanDetailData(const QString status, const QStringList list);
    void sigScanDetailStatus(const QString str);
    void sigCleanStatus(const QString status, const QString belong);
    void sigCleanTrashStatus(const QString &str);

public:
    ClearManagerPri(QObject *parent = nullptr);

public Q_SLOTS:
    void slotScanSystem(QMap<QString, QVariant> itemsMap);
    void slotCleanSystm(QMap<QString, QVariant> itemsMap);

private:
    QStringList mCookieList;
    QStringList mTrashList;
};


class ClearManager : public QObject
{
    Q_OBJECT

public:
    static ClearManager *getInstance();

private:
    ClearManager(QObject *obj = nullptr);
    static ClearManagerPri *m_pri;
    static ClearManager *m_instance;
    static QThread *m_thread;

Q_SIGNALS:
    void sigScanDetailData(const QString status, const QStringList list);
    void sigScanDetailStatus(const QString str);
    void sigCleanStatus(const QString status, const QString belong);
    void sigCleanTrashStatus(const QString &str);
    void slotScanSystem(QMap<QString, QVariant> itemsMap);
    void slotCleanSystm(QMap<QString, QVariant> itemsMap);
};

} // namespace KylinRubbishClear

#endif // CLEARMANAGER_H
