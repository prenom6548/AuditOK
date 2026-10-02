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

#include "multiterminal-vfs-file-enumerator.h"
#include "multiterminal-vfs-dbus-interface.h"

#include <QFile>
#include <sys/syslog.h>

G_DEFINE_TYPE_WITH_PRIVATE(MultVFSFileEnumerator, vfs_mult_file_enumerator, G_TYPE_FILE_ENUMERATOR);

static void next_async_op_free(GList *files);
void vfs_mult_file_enumerator_dispose(GObject *object);
static gboolean enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error);
static GFileInfo *enumerate_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error);
static GList *vfs_mult_file_enumerator_next_files_finished(GFileEnumerator *enumerator, GAsyncResult *result,
                                                           GError **error);
static void next_files_thread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable);
static void vfs_mult_file_enumerator_next_files_async(GFileEnumerator *enumerator, int num_files, int io_priority,
                                                      GCancellable *cancellable, GAsyncReadyCallback callback,
                                                      gpointer user_data);

static void vfs_mult_file_enumerator_init(MultVFSFileEnumerator *self)
{
    MultVFSFileEnumeratorPrivate *priv =
        (MultVFSFileEnumeratorPrivate *)vfs_mult_file_enumerator_get_instance_private(self);

    self->priv = priv;
    self->priv->enumerate_queue = new QQueue<QString>;
    self->priv->enumerator_uri = new QString;
}

static void vfs_mult_file_enumerator_class_init(MultVFSFileEnumeratorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GFileEnumeratorClass *enumerator_class = G_FILE_ENUMERATOR_CLASS(klass);

    gobject_class->dispose = vfs_mult_file_enumerator_dispose;
    enumerator_class->next_file = enumerate_next_file;
    enumerator_class->close_fn = enumerator_close;

    // async
    enumerator_class->next_files_async = vfs_mult_file_enumerator_next_files_async;
    enumerator_class->next_files_finish = vfs_mult_file_enumerator_next_files_finished;
}

void vfs_mult_file_enumerator_dispose(GObject *object)
{
    MultVFSFileEnumerator *self = VFS_MULT_FILE_ENUMERATOR(object);

    delete self->priv->enumerator_uri;
    self->priv->enumerate_queue->clear();
    delete self->priv->enumerate_queue;
}

static GFileInfo *enumerate_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error)
{
    QString errorStr;

    if (cancellable && g_cancellable_is_cancelled(cancellable)) {
        *error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_CANCELLED, "cancelled");
        return nullptr;
    }
    auto ve = VFS_MULT_FILE_ENUMERATOR(enumerator);
    auto eq = ve->priv->enumerate_queue;
    if (QString(*ve->priv->enumerator_uri) == ADDRESS_PREFIX) {
        while (!eq->isEmpty()) {
            auto uri = eq->dequeue();
            GFileInfo *tmp = g_file_info_new();
            QString realUriSuffix = uri;
            g_file_info_set_name(tmp, realUriSuffix.toUtf8().constData());
            g_file_info_set_display_name(tmp, realUriSuffix.toUtf8().constData());
            return tmp;
        }
    } else {
        if (eq) {
            if (eq->isEmpty()) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
        while (!eq->isEmpty()) {
            auto uri = eq->dequeue();
            GFileInfo *tmp = g_file_info_new();
            QString realUriSuffix = uri;
            g_file_info_set_name(tmp, realUriSuffix.toUtf8().constData());
            g_file_info_set_display_name(tmp, realUriSuffix.toUtf8().constData());
            return tmp;
        }
    }

    return nullptr;
}

static gboolean enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error)
{
    MultVFSFileEnumerator *self = VFS_MULT_FILE_ENUMERATOR(enumerator);
    Q_UNUSED(self)

    return true;
}

static void vfs_mult_file_enumerator_next_files_async(GFileEnumerator *enumerator, int num_files, int io_priority,
                                                      GCancellable *cancellable, GAsyncReadyCallback callback,
                                                      gpointer user_data)
{
    GTask *task = g_task_new(enumerator, cancellable, callback, user_data);
    g_task_set_source_tag(task, (gpointer)vfs_mult_file_enumerator_next_files_async);
    g_task_set_task_data(task, GINT_TO_POINTER(num_files), NULL);
    g_task_set_priority(task, io_priority);

    g_task_run_in_thread(task, next_files_thread);

    g_object_unref(task);
}

static GList *vfs_mult_file_enumerator_next_files_finished(GFileEnumerator *enumerator, GAsyncResult *result,
                                                           GError **error)
{
    g_return_val_if_fail(g_task_is_valid(result, enumerator), NULL);

    return (GList *)g_task_propagate_pointer(G_TASK(result), error);
}

static void next_files_thread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable)
{
    auto enumerator = G_FILE_ENUMERATOR(source_object);
    int num_files = GPOINTER_TO_INT(task_data);

    int i = 0;
    GList *files = nullptr;
    GError *error = nullptr;
    GFileInfo *info = nullptr;
    GFileEnumeratorClass *c = G_FILE_ENUMERATOR_GET_CLASS(enumerator);

    //遍历所有文件，每次返回一个fileinfo
    for (i = 0; i < num_files; ++i) {
        if (g_cancellable_set_error_if_cancelled(cancellable, &error)) {
            info = NULL;
        } else {
            info = c->next_file(enumerator, cancellable, &error); //返回fileinfo
        }

        if (nullptr == info) {
            break;
        } else {
            files = g_list_prepend(files, info);
        }
    }

    if (error) {
        g_task_return_error(task, error);
    } else {
        g_task_return_pointer(task, files, (GDestroyNotify)next_async_op_free);
    }
}

static void next_async_op_free(GList *files)
{
    g_list_free_full(files, g_object_unref);
}
