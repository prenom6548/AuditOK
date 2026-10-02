#include "thumbnail_cleaner.h"
//#include <unistd.h>
//#include <pwd.h>
#include <QDir>
#include <QDebug>
#include "utils.h"
namespace junk_clean {

ThumbnailCleaner::ThumbnailCleaner(QObject *parent)
    : Cleaner(parent),
      thumbnail_cache_path_("")
{
    //    struct passwd *pw = getpwuid(getuid());
    //    if (pw) {
    //        if (pw->pw_dir) {
    //            thumbnail_cache_path_ = QString("%1/.cache/qaxbrowser/Default").arg(pw->pw_dir);
    //        } else {
    //            qCritical() << "Thumbnail cleaner get home path fail.";
    //        }
    //    } else {
    //        qCritical() << "Thumbnail cleaner get pw info fail.";
    //    }

    thumbnail_cache_path_ = "/tmp/.cache/peony/thumbnails";
}

QString ThumbnailCleaner::Mark() const {
    return "thumbnail_cleaner";
}

enum Type ThumbnailCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level ThumbnailCleaner::Level() const {
    return Level::kNoEffect;
}

QString ThumbnailCleaner::Name() const {
    return tr("Thumbnail cache");
}

QString ThumbnailCleaner::Description() const {
    return tr("After cleaning, viewing thumbnails will slow dowm. Please clean as needed");
}

void ThumbnailCleaner::Scan() {
    thumbnail_.clear();

    quint64 total_size {0};
    quint64 junk_mark {0};
    do {
        if (thumbnail_cache_path_.isEmpty()) {
            qCritical() << "Thumbnail cleaner scan get cache path fail.";
            break;
        }
        QDir dir(thumbnail_cache_path_);
        auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry : entries) {
            quint64 junk_size {0};
            auto absolute_file_path = dir.absoluteFilePath(entry);
            QFileInfo file_info(absolute_file_path);
            if (file_info.isDir()) {
                junk_size = K::Utils::CalculateDirSize(QDir(absolute_file_path));
            } else {
                junk_size = file_info.size();
            }

            JunkItem junk_item;
            junk_item.mark = ++junk_mark;
            junk_item.path = absolute_file_path;
            junk_item.size = junk_size;
            thumbnail_.insert(junk_mark, absolute_file_path);
            total_size += junk_size;
            Q_EMIT sig_ScanForJunk(Mark(), junk_item);
        }
    } while (0);

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void ThumbnailCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark : junk_marks) {
        auto iter = thumbnail_.find(junk_mark);
        if (iter == thumbnail_.end()) {
            qCritical() << "Thumbnail cleaner clean junk mark [" << junk_mark << "] is not exist.";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        QFileInfo file_info(iter.value());
        if (file_info.exists()) {
            if (file_info.isDir()) {
                K::Utils::RemoveDir(iter.value());
            } else {
                if (!QFile::remove(iter.value())) {
                    qCritical() << "Thumbnail cleaner clean [" << iter.value() << "] fail.";
                }
            }
        }
        thumbnail_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }

    Q_EMIT sig_CleanFinish(Mark());
}

}
