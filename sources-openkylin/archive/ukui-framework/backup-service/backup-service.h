/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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

#ifndef UKUI_FRAMEWORK_BACKUP_SERVICE_H
#define UKUI_FRAMEWORK_BACKUP_SERVICE_H
#include <qglobal.h>

namespace UkuiConfigBackup {
class BackupServicePrivate;
class BackupService
{
public:
    static BackupService *self();
    Q_DISABLE_COPY_MOVE(BackupService);

private:
    BackupService();
    BackupServicePrivate *d = nullptr;
};
}

#endif //UKUI_FRAMEWORK_BACKUP_SERVICE_H