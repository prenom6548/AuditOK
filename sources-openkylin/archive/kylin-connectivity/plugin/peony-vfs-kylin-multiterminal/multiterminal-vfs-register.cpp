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

#include "multiterminal-vfs-register.h"
#include "multiterminal-vfs-file.h"
#include <gio/gio.h>

using namespace Peony;

bool mult_is_registed = false;

static GFile *test_vfs_parse_name(GVfs *vfs, const char *parseName, gpointer udata)
{
    QString tmp = parseName;
    return vfs_mult_file_new_for_uri(parseName);
}

static GFile *test_vfs_lookup(GVfs *vfs, const char *uri, gpointer udata)
{
    return test_vfs_parse_name(vfs, uri, udata);
}

void Peony::MultVFSRegister::registMultVFS()
{
    if (mult_is_registed) {
        return;
    }

    GVfs *vfs = nullptr;
    gboolean res = false;
    const gchar *const *schemes;

    vfs = g_vfs_get_default();
    schemes = g_vfs_get_supported_uri_schemes(vfs);

    const gchar *const *p = schemes;
    while (*p) {
        p++;
    }

#if GLIB_CHECK_VERSION(2, 50, 0)
    // 从"mult"字段开始进行解析，比如输入 mult:/ 后就开始自动解析了
    res = g_vfs_register_uri_scheme(vfs, "mult", test_vfs_lookup, NULL, NULL, test_vfs_parse_name, NULL, NULL);
#else
    // FIXME: how to implement search operation in old glib?
#endif
}

Peony::MultVFSRegister::MultVFSRegister() {}

void MultVFSInternalPlugin::initVFS()
{
    MultVFSRegister::registMultVFS();
}

void *MultVFSInternalPlugin::parseUriToVFSFile(const QString &uri)
{
    return vfs_mult_file_new_for_uri(uri.toUtf8().constData()); // return nullptr;
}
