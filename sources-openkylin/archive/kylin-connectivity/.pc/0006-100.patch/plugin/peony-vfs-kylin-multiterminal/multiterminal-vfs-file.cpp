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

#include "multiterminal-vfs-file.h"
#include "multiterminal-vfs-file-enumerator.h"
#include "multiterminal-vfs-file-monitor.h"
#include "multiterminal-vfs-dbus-interface.h"

#include <QObject>
#include <QDebug>
#include <QUrl>
#include <QThread>
#include <QDir>
#include <QIcon>
#include <sys/syslog.h>

static void vfs_mult_file_g_file_iface_init(GFileIface *iface);

char *vfs_mult_file_get_uri(GFile *file);
char *vfs_mult_file_get_path(GFile *file);
char *vfs_mult_file_get_schema(GFile *file);
GFile *vfs_mult_file_get_parent(GFile *file);
static GFile *vfs_mult_file_dup(GFile *file);
char *vfs_mult_file_get_basename(GFile *file);
GFile *vfs_mult_file_new_for_uri(const char *uri);
static void vfs_mult_file_dispose(GObject *obj);
static gboolean vfs_mult_file_is_native(GFile *file);
gboolean vfs_mult_file_copy(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable,
                            GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error);
GFile *vfs_mult_file_resolve_relative_path(GFile *file, const char *relative_path);
GFileMonitor *vfs_mult_file_monitor_directory(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable,
                                              GError **error);
GFileInfo *vfs_mult_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags,
                                    GCancellable *cancellable, GError **error);
GFileEnumerator *vfs_mult_file_enumerate_children(GFile *file, const char *attribute, GFileQueryInfoFlags flags,
                                                  GCancellable *cancellable, GError **error);
GFileEnumerator *vfs_mult_file_enumerate_children_internal(GFile *file, const char *attribute,
                                                           GFileQueryInfoFlags flags, GCancellable *cancellable,
                                                           GError **error);
gboolean vfs_mult_file_equal(GFile *file1, GFile *file2);
GFileInputStream *vfs_mult_read(GFile *file, GCancellable *cancellable, GError **error);
GFileOutputStream *vfs_mult_file_create(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
gboolean vfs_mult_file_move(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable,
                            GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error);
gboolean vfs_mult_file_delete(GFile *file, GCancellable *cancellable, GError **error);

G_DEFINE_TYPE_EXTENDED(MultVFSFile, vfs_mult_file, G_TYPE_OBJECT, 0,
                       G_ADD_PRIVATE(MultVFSFile) G_IMPLEMENT_INTERFACE(G_TYPE_FILE, vfs_mult_file_g_file_iface_init));

static void init_dbus_module()
{
    MultVfsDbusInterface::getInstance()->initDbusData();
}

// 类结构体初始化函数，只在第一次创建对象时调用
static void vfs_mult_file_class_init(MultVFSFileClass *kclass)
{
    init_dbus_module();
    GObjectClass *gobject_class = G_OBJECT_CLASS(kclass);

    gobject_class->dispose = vfs_mult_file_dispose;
}

// 实例结构体初始化函数，每次创建对象时都会调用
static void vfs_mult_file_init(MultVFSFile *self)
{
    MultVFSFilePrivate *priv = (MultVFSFilePrivate *)vfs_mult_file_get_instance_private(self);
    self->priv = priv;
    self->priv->path_uri = nullptr;
}

static void vfs_mult_file_dispose(GObject *object)
{
    auto vfsfile = VFS_MULT_FILE(object);
    if (vfsfile->priv->path_uri) {
        g_free(vfsfile->priv->path_uri);
        vfsfile->priv->path_uri = nullptr;
    }
}

static void vfs_mult_file_g_file_iface_init(GFileIface *iface)
{
    iface->dup = vfs_mult_file_dup;
    iface->get_uri = vfs_mult_file_get_uri;
    iface->get_path = vfs_mult_file_get_path;
    iface->is_native = vfs_mult_file_is_native;
    iface->copy = vfs_mult_file_copy;
    iface->get_parent = vfs_mult_file_get_parent;
    iface->query_info = vfs_mult_file_query_info;
    iface->get_basename = vfs_mult_file_get_basename;
    iface->get_uri_scheme = vfs_mult_file_get_schema;
    iface->monitor_dir = vfs_mult_file_monitor_directory;
    iface->enumerate_children = vfs_mult_file_enumerate_children;
    iface->resolve_relative_path = vfs_mult_file_resolve_relative_path;
    iface->equal = vfs_mult_file_equal;
    iface->read_fn = vfs_mult_read;
    iface->create = vfs_mult_file_create;
    iface->move = vfs_mult_file_move;
    iface->delete_file = vfs_mult_file_delete;
}

char *vfs_mult_file_get_uri(GFile *file)
{
    auto vfs_file = VFS_MULT_FILE(file);
    return g_strdup(vfs_file->priv->path_uri);
}

gboolean vfs_mult_file_is_native(GFile *file)
{
    Q_UNUSED(file);

    return FALSE;
}

GFile *vfs_mult_file_dup(GFile *file)
{
    if (!VFS_MULT_FILE(file)) {
        return g_file_new_for_uri(ADDRESS_PREFIX);
    }
    auto vfs_file = VFS_MULT_FILE(file);
    auto dup = VFS_MULT_FILE(g_object_new(VFS_TYPE_MULT_FILE, nullptr));
    dup->priv->path_uri = g_strdup(vfs_file->priv->path_uri);

    return G_FILE(dup);
}

void mult_vfs_file_enumerator_parse_uri(MultVFSFileEnumerator *enumerator, const char *uri)
{
    *enumerator->priv->enumerator_uri = uri;
    MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
    QString device = interface->getDeviceName();
    if (device.isEmpty()) {
        return;
    }
    QString url = QString(uri);
    QString path = url;
    if (url == ADDRESS_PREFIX) {
        enumerator->priv->enumerate_queue->append(device);
        return;
    }
    QStringList list;
    if (url == ADDRESS_PREFIX + device) {
        list = interface->getFileList("/");
        for (int i = 0; i < list.size(); i++) {
            enumerator->priv->enumerate_queue->append(list.value(i));
        }
        return;
    }
    url.remove(ADDRESS_PREFIX + device);
    if (!url.isEmpty()) {
        list = interface->getFileList(url);
        for (int i = 0; i < list.size(); i++) {
            enumerator->priv->enumerate_queue->append(list.value(i));
        }
        return;
    }
    return;
}

GFileEnumerator *vfs_mult_file_enumerate_children_internal(GFile *file, const char *attribute,
                                                           GFileQueryInfoFlags flags, GCancellable *cancellable,
                                                           GError **error)
{
    auto vfs_file = VFS_MULT_FILE(file);
    g_autofree char *uri = g_file_get_uri(file);
    auto enumerator = VFS_MULT_FILE_ENUMERATOR(g_object_new(VFS_TYPE_MULT_FILE_ENUMERATOR, "container", file, nullptr));
    mult_vfs_file_enumerator_parse_uri(enumerator, uri);

    Q_UNUSED(flags)
    Q_UNUSED(error)
    Q_UNUSED(attribute)
    Q_UNUSED(cancellable)

    return G_FILE_ENUMERATOR(enumerator);
}

GFile *vfs_mult_file_new_for_uri(const char *uri)
{
    auto vfs_file = VFS_MULT_FILE(g_object_new(VFS_TYPE_MULT_FILE, nullptr));
    vfs_file->priv->path_uri = g_strdup(uri);

    return G_FILE(vfs_file);
}

char *vfs_mult_file_get_path(GFile *file)
{
    g_autofree char *uri = vfs_mult_file_get_uri(file);
    QString trueUri = QString(uri);
    MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
    trueUri.replace(QString(ADDRESS_PREFIX + interface->getDeviceName()), interface->getMountDir());
    return g_strdup(trueUri.toUtf8().constData());
}

gboolean vfs_mult_file_copy(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable,
                            GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error)
{
    QString errorStr;

    Q_UNUSED(flags)
    Q_UNUSED(cancellable)
    Q_UNUSED(progress_callback)
    Q_UNUSED(progress_callback_data)
    g_return_val_if_fail(nullptr != source, FALSE);
    g_return_val_if_fail(nullptr != destination, FALSE);

    // source是 MULT 虚拟文件, destination是file真实文件，即暂时只支持从kyroid vfs里面将文件拷贝到Linux真实路径下
    if (VFS_IS_MULT_FILE(source) && !VFS_IS_MULT_FILE(destination)) {
        char *source_uri = g_file_get_uri(source);
        char *destination_uri = g_file_get_uri(destination);
        if (source_uri && destination_uri && QString(destination_uri).startsWith("file://")) {
            QString truePath = QString(source_uri);
            MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
            truePath.replace(QString(ADDRESS_PREFIX + interface->getDeviceName()), interface->getMountDir());
            GFile *src_file = NULL;
            src_file = g_file_new_for_path(truePath.toUtf8().constData());
            GError *err = NULL;
            GFileCopyFlags m_default_copy_flag = GFileCopyFlags(
                G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA | G_FILE_COPY_NO_FALLBACK_FOR_MOVE);
            g_file_copy(src_file, destination,
                        GFileCopyFlags(m_default_copy_flag | G_FILE_COPY_BACKUP | G_FILE_COPY_OVERWRITE), NULL, NULL,
                        NULL, &err);
            g_clear_object(&src_file);
            if (err) {
                errorStr = QObject::tr("Failed to copy. Reason: %1").arg(QString(err->message));
                g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
                if (err->code == G_IO_ERROR_EXISTS) {
                }
                g_error_free(err);
                return FALSE;
            }

            return TRUE;
        }
    } else if (!VFS_IS_MULT_FILE(source) && VFS_IS_MULT_FILE(destination)) {
        char *source_uri = g_file_get_uri(source);
        char *destination_uri = g_file_get_uri(destination);

        int lastPos = QString(destination_uri).lastIndexOf("/");
        QString destPath = QString(destination_uri).left(lastPos);
        MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
        destPath.replace(QString(ADDRESS_PREFIX + interface->getDeviceName()), interface->getMountDir());
        QUrl sourceUrl = QString(source_uri);
        g_free(source_uri);
        g_free(destination_uri);
        if (!sourceUrl.isLocalFile()) {
            errorStr = QObject::tr("This is not a local file");
            g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
            return FALSE;
        }
        const QString sourcePath = sourceUrl.toLocalFile();
        const QFileInfo info(sourcePath);
        if (!info.exists()) {
            errorStr = QObject::tr("File does not exist");
            g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
            return FALSE;
        } else {
            if (!info.isFile()) {
                errorStr = QObject::tr("This is not a file");
                g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
                return FALSE;
            } else {
                char *basename;
                basename = g_file_get_basename(source);

                GError *err = NULL;
                GFileCopyFlags m_default_copy_flag = GFileCopyFlags(
                    G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA | G_FILE_COPY_NO_FALLBACK_FOR_MOVE);
                GFile *dest_file = NULL;
                g_free(basename);
                dest_file = g_file_new_for_path(destPath.toUtf8().constData());
                g_file_copy(source, dest_file,
                            GFileCopyFlags(m_default_copy_flag | G_FILE_COPY_BACKUP | G_FILE_COPY_OVERWRITE), NULL,
                            NULL, NULL, &err);
                g_clear_object(&dest_file);
                if (err) {
                    g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
                    if (err->code == G_IO_ERROR_EXISTS) {
                    }
                    g_error_free(err);
                    return FALSE;
                }

                return TRUE;
            }
        }
    } else {
        errorStr = QObject::tr("Operation not supported");
        g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
        return FALSE;
    }

    errorStr = QObject::tr("MULT may not have started");
    g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
    return FALSE;
}

GFile *vfs_mult_file_resolve_relative_path(GFile *file, const char *relative_path)
{
    if (!VFS_MULT_FILE(file)) {
        return vfs_mult_file_new_for_uri(ADDRESS_PREFIX);
    }
    auto first_vfs_file = VFS_MULT_FILE(file);

    g_autofree char *uri = g_file_get_uri(file);
    MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
    if (interface->getDeviceName().isEmpty()) {
        return vfs_mult_file_new_for_uri(ADDRESS_PREFIX);
    }

    QString parentUri = uri;
    // 处理第一级目录
    // mult:///,如果输入栏输入的是第一级目录的名字"mult:///"
    if (parentUri == ADDRESS_PREFIX) {
        // 当输入栏输入"mult:///"时，first_vfs_file->priv->path_uri的值为"mult:///"，relative_path的值为picture、video、audio、document
        auto value = vfs_mult_file_new_for_uri(QString("mult:///%1").arg(relative_path).toUtf8());
        return value;
    }

    QString tmp = relative_path;
    auto second_vfs_file = VFS_MULT_FILE(vfs_mult_file_new_for_uri(QString("%1/%2").arg(uri).arg(tmp).toUtf8()));
    return G_FILE(second_vfs_file);
}

// 返回每个文件的fileinfo
GFileInfo *vfs_mult_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags,
                                    GCancellable *cancellable, GError **error)
{
    auto vfsfile = VFS_MULT_FILE(file);
    g_return_val_if_fail(nullptr != file, nullptr);

    GFileInfo *info = nullptr;
    QString trueUri = nullptr;
    g_autofree char *uri = g_file_get_uri(file);
    QString url(uri);
    // 第一级目录
    // 当输入栏输入"mult:///"时，vfsfile->priv->path_uri的值为"mult:///"
    if (url == ADDRESS_PREFIX) {
        info = g_file_info_new();
        QString name = QObject::tr("Multiterminal");
        g_file_info_set_display_name(info, name.toUtf8().constData());
        GIcon *icon = NULL;
        QIcon icn = QIcon::fromTheme("folder-phone-symbolic");
        if (icn.isNull()) {
            icon = g_themed_icon_new("folder-phone");
        } else {
            icon = g_themed_icon_new("folder-phone");
        }
        g_file_info_set_icon(info, icon);
        g_object_unref(icon);
        g_file_info_set_content_type(info, "inode/directory");
        g_file_info_set_size(info, 0);

        return info;
    }

    MultVfsDbusInterface *interface = MultVfsDbusInterface::getInstance();
    if (url == QString(ADDRESS_PREFIX + interface->getDeviceName())) {
        info = g_file_info_new();
        g_file_info_set_display_name(info, interface->getDeviceName().toUtf8().constData());
        GIcon *icon = g_themed_icon_new("folder-phone");
        g_file_info_set_icon(info, icon);
        g_object_unref(icon);
        g_file_info_set_content_type(info, "inode/directory");
        return info;
    }

    if (url.startsWith(QString(ADDRESS_PREFIX + interface->getDeviceName()))) {
        trueUri = url;
        QString fileName = trueUri.mid(trueUri.lastIndexOf("/") + 1, trueUri.size());

        FileInfoList list = interface->getInfoList();
        FileInfo fileInfo;
        fileInfo = list.value(fileName);
        info = g_file_info_new();
        g_file_info_set_display_name(info, fileName.toUtf8().constData());
        g_file_info_set_size(info, fileInfo.size());
        QDateTime dateTime = QDateTime::fromString(fileInfo.dateTime(), Qt::ISODate);
        int year = dateTime.date().year();    // 年
        int month = dateTime.date().month();  // 月
        int day = dateTime.date().day();      // 日
        int hour = dateTime.time().hour();    // 时
        int sec = dateTime.time().second();   // 分
        int msec = dateTime.time().msec();    // 秒
        GTimeZone *zone = g_time_zone_new_local();
        GDateTime *time = g_date_time_new(zone, year, month, day, hour, sec, msec);
        g_file_info_set_modification_date_time(info, time);
        g_time_zone_unref(zone);
        g_date_time_unref(time);
        QString contentType;
        QString iconName;
        if (fileInfo.type() == FileInfo::FileType::Dir) {
            contentType = "inode/directory";
            iconName = FileInfo::iconName(contentType);
            GIcon *icon = g_themed_icon_new(iconName.toUtf8().constData());
            g_file_info_set_icon(info, icon);
            g_object_unref(icon);
            g_file_info_set_content_type(info, contentType.toUtf8().constData());
            return info;
        }
        contentType = fileInfo.getContentType();
        if (contentType.contains(FILEINFO_CONTENT_TYPE_IMAGE)) {
            iconName = FileInfo::iconName(FILEINFO_CONTENT_TYPE_IMAGE);
        } else if (contentType.contains(FILEINFO_CONTENT_TYPE_AUDIO)) {
            iconName = FileInfo::iconName(FILEINFO_CONTENT_TYPE_AUDIO);
        } else if (contentType.contains(FILEINFO_CONTENT_TYPE_VIDEO)) {
            iconName = FileInfo::iconName(FILEINFO_CONTENT_TYPE_VIDEO);
        } else if (contentType.contains(FILEINFO_CONTENT_TYPE_TEXT)) {
            iconName = FileInfo::iconName(FILEINFO_CONTENT_TYPE_TEXT);
        } else {
            iconName = FileInfo::iconName(contentType);
        }
        GIcon *icon = g_themed_icon_new(iconName.toUtf8().constData());
        g_file_info_set_icon(info, icon);
        g_object_unref(icon);
        g_file_info_set_content_type(info, contentType.toUtf8().constData());
        return info;
    }

    Q_UNUSED(flags)
    // Q_UNUSED(error)
    Q_UNUSED(attributes)
    Q_UNUSED(cancellable)

    QString errorStr = QObject::tr("Incorrect path");
    g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());

    return info;
}

GFileEnumerator *vfs_mult_file_enumerate_children(GFile *file, const char *attribute, GFileQueryInfoFlags flags,
                                                  GCancellable *cancellable, GError **error)
{
    auto vf = VFS_MULT_FILE(file);
    Q_UNUSED(vf)

    return vfs_mult_file_enumerate_children_internal(file, attribute, flags, cancellable, error);
}

/*
 * Attention: 去掉 vfs_mult_file_monitor_directory ，或者 vfs_mult_file_monitor_directory
 * 没有实现GFileMonitor，或者返回nullptr，则报如下警告 (peony:48675): GLib-GObject-WARNING **: 19:41:53.687: invalid
 * (NULL) pointer instance (peony:48675): GLib-GObject-CRITICAL **: 19:41:53.687: g_signal_connect_data: assertion
 * 'G_TYPE_CHECK_INSTANCE (instance)' failed
 *
 */
GFileMonitor *vfs_mult_file_monitor_directory(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable,
                                              GError **error)
{
    GFileMonitor *fmonitor = (GFileMonitor *)g_object_new(VFS_TYPE_MULT_FILE_MONITOR, nullptr);

    char *uri = g_file_get_uri(file);
    QString fileUri = QString(uri);
    g_free(uri);

    vfs_mult_file_monitor_dir(VFS_MULT_FILE_MONITOR(fmonitor), fileUri);

    Q_UNUSED(flags)
    Q_UNUSED(error)
    Q_UNUSED(cancellable)

    return fmonitor;  // return nullptr;
}

char *vfs_mult_file_get_schema(GFile *file)
{
    Q_UNUSED(file);
    return g_strdup("mult");
}

GFile *vfs_mult_file_get_parent(GFile *file)
{
    Q_UNUSED(file);
    return g_file_new_for_uri(ADDRESS_PREFIX);
}

char *vfs_mult_file_get_basename(GFile *file)
{
    char *uri = vfs_mult_file_get_uri(file);
    QUrl url = QString(uri);
    g_free(uri);

    return g_strdup(url.path().toUtf8().constData());
}

gboolean vfs_mult_file_equal(GFile *file1, GFile *file2)
{
    char *uri1 = vfs_mult_file_get_uri(file1);
    char *uri2 = vfs_mult_file_get_uri(file2);
    QUrl url1 = QString(uri1);
    QUrl url2 = QString(uri2);
    g_free(uri1);
    g_free(uri2);

    return url1 == url2;
}

GFileOutputStream *vfs_mult_file_create(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error)
{
    QString errorStr;

    Q_UNUSED(file)
    Q_UNUSED(flags)
    Q_UNUSED(cancellable)

    errorStr = QObject::tr("Operation not supported");
    g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());

    return nullptr;
}

GFileInputStream *vfs_mult_read(GFile *file, GCancellable *cancellable, GError **error)
{
    g_return_val_if_fail(VFS_MULT_FILE(file), nullptr);

    g_autofree char *path = vfs_mult_file_get_path(file);

    g_return_val_if_fail(path, nullptr);

    g_autoptr(GFile) tf = g_file_new_for_path(path);

    g_return_val_if_fail(tf, nullptr);

    return g_file_read(tf, cancellable, error);
}

gboolean vfs_mult_file_move(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable,
                            GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error)
{
    QString errorStr;

    Q_UNUSED(flags)
    Q_UNUSED(cancellable)
    Q_UNUSED(progress_callback)
    Q_UNUSED(progress_callback_data)
    Q_UNUSED(error)

    if (VFS_IS_MULT_FILE(source) && !VFS_IS_MULT_FILE(destination)) {
        return TRUE;
    } else {
        errorStr = QObject::tr("Operation not supported");
        g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
        return FALSE;
    }
}

gboolean vfs_mult_file_delete(GFile *file, GCancellable *cancellable, GError **error)
{
    Q_UNUSED(file)
    Q_UNUSED(cancellable)
    Q_UNUSED(error)

    return TRUE;
}
