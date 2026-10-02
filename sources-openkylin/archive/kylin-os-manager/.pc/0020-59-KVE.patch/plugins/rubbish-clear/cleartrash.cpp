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

#include <QDebug>
#include <QUrl>
#include "cleartrash.h"

namespace KylinRubbishClear
{

ClearTrash::ClearTrash(QObject *parent) : QObject(parent)
{
    // nothing to do
}

QStringList ClearTrash::scanTrashFiles()
{
    QStringList res;

    std::vector<std::string> uris;
    list(uris);
    if (uris.empty()) {
        qInfo() << "Trash is empty.";
        return res;
    }

    gint64 totalSize = 0;
    for (const auto &uri : uris) {
        GFile *file;
        file = g_file_new_for_uri(uri.c_str());
        if (file) {
            totalSize += getTrashItemSize(file);
            g_object_unref(file);
        }
    }

    res.append(QString("Trash%1%2:%3").arg(getenv("HOME")).arg("/.local/share/Trash/files").arg(totalSize));
    return res;
}

void ClearTrash::list(std::vector<std::string> &uris)
{
    GFile *file;
    GFileEnumerator *enumerator;
    GFileInfo *info;
    GError *error = NULL;

    file = g_file_new_for_uri("trash:");
    enumerator = g_file_enumerate_children(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                           NULL, &error);
    if (!enumerator) {
        qCritical() << "get GFile enumerator fail. " << (error == NULL ? "" : error->message);
        if (error)
            g_error_free(error);
        return;
    }

    while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
        const char *name;
        char *uri;
        GFile *child;

        name = g_file_info_get_name(info);
        child = g_file_get_child(file, name);
        uri = g_file_get_uri(child);

        std::string item {uri};
        if (!item.empty())
            uris.emplace_back(item);

        g_object_unref(info);
        g_object_unref(child);
        g_free(uri);
    }

    g_file_enumerator_close(enumerator, NULL, NULL);
    g_object_unref(enumerator);
    g_object_unref(file);
}

gint64 ClearTrash::getTrashItemSize(GFile *file)
{
    GFileType fileType;

    Q_EMIT trashStatus(QUrl::fromPercentEncoding(g_file_get_uri(file)));

    fileType = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);
    if (fileType == G_FILE_TYPE_REGULAR) {
        GFileInfo *info;
        gint64 size = 0;

        info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info) {
            size = g_file_info_get_size(info);
            g_object_unref(info);
        }
        return size;
    } else if (fileType == G_FILE_TYPE_DIRECTORY) {
        GFileEnumerator *enumerator;
        GFileInfo *info;
        gint64 size = 0;

        enumerator = g_file_enumerate_children(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, NULL);
        if (enumerator) {
            while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
                GFile *child = g_file_enumerator_get_child(enumerator, info);
                if (child) {
                    size += getTrashItemSize(child);
                    g_object_unref(child);
                }
                g_object_unref(info);
            }
            g_file_enumerator_close(enumerator, NULL, NULL);
            g_object_unref(enumerator);
        }
        return size;
    }

    return 0;
}

void ClearTrash::cleanup()
{
    GFile *file;
    GFile *child;
    GFileInfo *info;
    GFileEnumerator *enumerator;

    file = g_file_new_for_uri("trash:");
    enumerator = g_file_enumerate_children(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                           NULL, NULL);
    if (enumerator) {
        while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
            child = g_file_enumerator_get_child(enumerator, info);
            if (child) {
                g_file_delete(child, NULL, NULL);
                g_object_unref(child);
            }
            g_object_unref(info);
        }
        g_file_enumerator_close(enumerator, NULL, NULL);
        g_object_unref(enumerator);
    }
}

} // namespace KylinRubbishClear
