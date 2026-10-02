#include "trash_cleaner.h"
#include <gio/gio.h>
#include <QDebug>

namespace junk_clean {

TrashCleaner::TrashCleaner(QObject *parent)
    : Cleaner(parent)
{

}

QString TrashCleaner::Mark() const {
    return "trash_cleaner";
}

enum Type TrashCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level TrashCleaner::Level() const {
    return Level::kGeneral;
}

QString TrashCleaner::Name() const {
    return tr("Trash");
}

QString TrashCleaner::Description() const {
    return tr("Before cleaning, please confirm that all files in the recycle bin are removable");
}

void TrashCleaner::Scan() {
    trash_junk_.clear();
    quint64 total_size{0};
    quint64 junk_mark{0};

    do {
        GError *error = NULL;
        GFile *file = g_file_new_for_uri("trash:");
        GFileEnumerator *enumerator = g_file_enumerate_children(file,
                                                                G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                                NULL,
                                                                &error);
        if (!enumerator) {
            qCritical() << "Trash cleaner scan create file enumerator fail: " << (error == NULL? "": error->message);
            if (error) {
                g_error_free(error);
            }
            break;
        }

        GFileInfo *info = NULL;
        while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
            const char *name = g_file_info_get_name(info);
            GFile *child = g_file_get_child(file, name);
            char *uri = g_file_get_uri(child);
            quint64 size = GetTrashItemSize(child);

            JunkItem junk_item;
            junk_item.mark = ++junk_mark;
            junk_item.path = uri;
            junk_item.size = size;

            total_size += size;
            trash_junk_.insert(junk_mark, uri);
            Q_EMIT sig_ScanForJunk(Mark(), junk_item);

            g_free(uri);
            g_object_unref(child);
            g_object_unref(info);
        }
        g_file_enumerator_close(enumerator, NULL, NULL);
        g_object_unref(enumerator);
        g_object_unref(file);
    } while (0);

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void TrashCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark: junk_marks) {
        auto iter = trash_junk_.find(junk_mark);
        if (iter == trash_junk_.end()) {
            qWarning() << "Trash cleaner clean junk mark [" << junk_mark << "] is not exist.";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        GFile *file = g_file_new_for_uri(iter.value().toStdString().c_str());
        if (file) {
            GError *error = NULL;
            // g_file_delete 接口描述如果是目录的情况下，只有目录为空时才会被删除
            // 但实际测试回收站中的非空目录仍可以直接删除，猜测回收站中的目录会被看成是一个文件
            if (!g_file_delete(file, NULL, &error)) {
                qCritical() << "Trash cleaner clean fail: " << (error == NULL? "": error->message);
                if (error) {
                    g_error_free(error);
                }
            }
            g_object_unref(file);
        }
        trash_junk_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }

    Q_EMIT sig_CleanFinish(Mark());
}

quint64 TrashCleaner::GetTrashItemSize(GFile *file) const {
    quint64 total_size{0};

    GFileType file_type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);
    if (file_type == G_FILE_TYPE_REGULAR) {
        // 文件
        GFileInfo *file_info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (file_info) {
            total_size += g_file_info_get_size(file_info);
            g_object_unref(file_info);
        }
    } else if (file_type == G_FILE_TYPE_DIRECTORY) {
        // 目录
        GFileEnumerator *enumerator = g_file_enumerate_children(file,
                                                                "*",
                                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                                NULL,
                                                                NULL);
        if (enumerator) {
            GFileInfo *file_info;
            while ((file_info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
                GFile *entry_file = g_file_enumerator_get_child(enumerator, file_info);
                if (entry_file) {
                    total_size += GetTrashItemSize(entry_file);
                    g_object_unref(entry_file);
                }
                g_object_unref(file_info);
            }
            g_file_enumerator_close(enumerator, NULL, NULL);
            g_object_unref(enumerator);
        }
    }

    return total_size;
}

}
