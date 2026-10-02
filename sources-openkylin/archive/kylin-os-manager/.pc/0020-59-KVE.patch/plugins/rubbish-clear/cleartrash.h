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

#ifndef CLEARTRASH_H
#define CLEARTRASH_H

#include <vector>
#include <string>
#include <gio/gio.h>
#include <QObject>
#include <QStringList>

namespace KylinRubbishClear
{

class ClearTrash : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void trashStatus(QString str);

public:
    ClearTrash(QObject *parent = nullptr);
    ~ClearTrash() = default;

    QStringList scanTrashFiles();
    void cleanup();

private:
    void list(std::vector<std::string> &uris);
    gint64 getTrashItemSize(GFile *file);
};

} // namespace KylinRubbishClear

#endif
