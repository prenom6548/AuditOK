/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#ifndef UKUISEARCHDIRWATCHERDBUS_H
#define UKUISEARCHDIRWATCHERDBUS_H

#include "qtsinglecoreapplication.h"

class UkuiSearchDirManagerDbus : public QtSingleCoreApplication
{
    Q_OBJECT
public:
    UkuiSearchDirManagerDbus(int &argc, char *argv[], const QString &applicationName = "ukui-search-service-dir-manager");
    bool initializationSucceeded() const;
protected Q_SLOTS:
    void parseCmd(const QStringList &args, bool isPrimary);
private:
    bool m_initializationSucceeded = true;
};

#endif // UKUISEARCHDIRWATCHERDBUS_H
