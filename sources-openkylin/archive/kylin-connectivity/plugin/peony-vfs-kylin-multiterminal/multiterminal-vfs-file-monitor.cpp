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

#include "multiterminal-vfs-file-monitor.h"
#include "multiterminal-vfs-file.h"
#include "multiterminal-vfs-dbus-interface.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <sys/syslog.h>

G_DEFINE_TYPE(MultVFSFileMonitor, vfs_mult_file_monitor, G_TYPE_FILE_MONITOR)

static void vfs_mult_file_monitor_constructed(GObject *obj);
static void vfs_mult_file_monitor_dispose(GObject *obj);
static void vfs_mult_file_monitor_finalize(GObject *obj);

static void vfs_mult_file_monitor_init(MultVFSFileMonitor *monitor) {}

static void vfs_mult_file_monitor_class_init(MultVFSFileMonitorClass *kclass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(kclass);

    gobject_class->constructed = vfs_mult_file_monitor_constructed;
    gobject_class->dispose = vfs_mult_file_monitor_dispose;
    gobject_class->finalize = vfs_mult_file_monitor_finalize;
}

static void vfs_mult_file_monitor_constructed(GObject *obj)
{
    MultVFSFileMonitor *monitor = VFS_MULT_FILE_MONITOR(obj);

    Q_UNUSED(monitor)

    if (G_OBJECT_CLASS(vfs_mult_file_monitor_parent_class)->constructed != NULL) {
        G_OBJECT_CLASS(vfs_mult_file_monitor_parent_class)->constructed(obj);
    }
}

static void vfs_mult_file_monitor_dispose(GObject *obj)
{
    MultVFSFileMonitor *monitor = VFS_MULT_FILE_MONITOR(obj);
}

static void vfs_mult_file_monitor_finalize(GObject *obj)
{
    MultVFSFileMonitor *monitor = VFS_MULT_FILE_MONITOR(obj);

    Q_UNUSED(monitor)

    if (G_OBJECT_CLASS(vfs_mult_file_monitor_parent_class)->finalize != NULL) {
        G_OBJECT_CLASS(vfs_mult_file_monitor_parent_class)->finalize(obj);
    }
}

static MultVFSFileMonitor *vfs_mult_file_monitor_new()
{
    return VFS_MULT_FILE_MONITOR(g_object_new(VFS_TYPE_MULT_FILE_MONITOR, nullptr));
}

void vfs_mult_file_monitor_dir(MultVFSFileMonitor *obj, const QString &uri)
{
    MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
    obj->post = QObject::connect(interface, &MultVfsDbusInterface::sigDeviceUpdate, [=](QString device) {
        if (device.isEmpty()) {
            QString pathUri = uri;
            GFile *file = G_FILE(vfs_mult_file_new_for_uri(pathUri.toUtf8().constData()));
            if (file && G_IS_FILE(file) && G_IS_FILE_MONITOR(obj)) {
                g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_UNMOUNTED);
            }
            g_clear_object(&file);
            return;
        }
        QString path = device;
        QString pathUri = uri + path;
        GFile *file = G_FILE(vfs_mult_file_new_for_uri(pathUri.toUtf8().constData()));
        if (file && G_IS_FILE(file) && G_IS_FILE_MONITOR(obj)) {
            g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_CREATED);
        }
        g_clear_object(&file);
    });
}
