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

#ifndef MULTFILEENUMERATOR_H
#define MULTFILEENUMERATOR_H

#include <gio/gio.h>

#include <QQueue>

G_BEGIN_DECLS

#define VFS_TYPE_MULT_FILE_ENUMERATOR vfs_mult_file_enumerator_get_type()

G_DECLARE_FINAL_TYPE(MultVFSFileEnumerator, vfs_mult_file_enumerator, VFS, MULT_FILE_ENUMERATOR, GFileEnumerator)

MultVFSFileEnumerator *vfs_mult_file_enumerator_new(void);

typedef struct _MultVFSFileEnumeratorPrivate MultVFSFileEnumeratorPrivate;

struct _MultVFSFileEnumeratorPrivate
{
    QQueue<QString> *enumerate_queue;
    QString *enumerator_uri;
};

struct _MultVFSFileEnumerator
{
    GFileEnumerator parent_instance;
    MultVFSFileEnumeratorPrivate *priv;
};

G_END_DECLS

#endif // MULTFILEENUMERATOR_H
