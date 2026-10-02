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

#ifndef MULTVFSFILE_H
#define MULTVFSFILE_H

#include <glib-object.h>
#include <gio/gio.h>

#include <QString>

G_BEGIN_DECLS

#define VFS_TYPE_MULT_FILE vfs_mult_file_get_type()

G_DECLARE_FINAL_TYPE(MultVFSFile, vfs_mult_file, VFS, MULT_FILE, GObject)

MultVFSFile *vfs_mult_file_new(void);

typedef struct _MultVFSFilePrivate MultVFSFilePrivate;

struct _MultVFSFilePrivate
{
    gchar *path_uri;
};

struct _MultVFSFile
{
    GObject parent_instance;
    MultVFSFilePrivate *priv;
};

G_END_DECLS

extern "C"
{
    GFile *vfs_mult_file_new_for_uri(const char *uri);
    static GFileEnumerator *vfs_mult_file_enumerate_children_internal(GFile *file, const char *attribute,
                                                                      GFileQueryInfoFlags flags,
                                                                      GCancellable *cancellable, GError **error);
}

#endif // MULTVFSFILE_H
