/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: huheng <huheng@kylinos.cn>
 *
 */

#ifndef MULTVFSFILEMONITOR_H
#define MULTVFSFILEMONITOR_H

#include <gio/gio.h>

#include <QObject>

G_BEGIN_DECLS

#define VFS_MULT_FILE_MONITOR_NAME ("vfs-mult-file-monitor")

#define VFS_TYPE_MULT_FILE_MONITOR (vfs_mult_file_monitor_get_type())
#define VFS_MULT_FILE_MONITOR(o) (G_TYPE_CHECK_INSTANCE_CAST((o), VFS_TYPE_MULT_FILE_MONITOR, MultVFSFileMonitor))
#define VFS_MULT_FILE_MONITOR_CLASS(k) (G_TYPE_CLASS_CAST((k), VFS_TYPE_MULT_FILE_MONITOR, MultVFSFileMonitorClass))
#define VFS_IS_MULT_FILE_MONITOR(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), VFS_TYPE_MULT_FILE_MONITOR))
#define VFS_IS_MULT_FILE_MONITOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), VFS_TYPE_MULT_FILE_MONITOR))
#define VFS_MULT_FILE_MONITOR_GET_CLASS(o)                                                                             \
    (G_TYPE_INSTANCE_GET_CLASS((o), VFS_TYPE_MULT_FILE_MONITOR, MultVFSFileMonitorClass))

typedef struct _MultVFSFileMonitor MultVFSFileMonitor;
typedef struct _MultVFSFileMonitorClass MultVFSFileMonitorClass;

struct _MultVFSFileMonitor
{
    GFileMonitor parent_monitor;
    QMetaObject::Connection post;
};

struct _MultVFSFileMonitorClass
{
    GFileMonitorClass parent_class;
};

GType vfs_mult_file_monitor_get_type(void);
void vfs_mult_file_monitor_dir(MultVFSFileMonitor *obj, const QString &uri);

G_END_DECLS

#endif // MULTVFSFILEMONITOR_H
