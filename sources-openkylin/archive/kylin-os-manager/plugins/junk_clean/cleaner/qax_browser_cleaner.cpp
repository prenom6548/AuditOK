#include "qax_browser_cleaner.h"
#include <unistd.h>
#include <pwd.h>
#include <QDebug>
#include <QDir>
#include "utils.h"

namespace junk_clean  {

QaxBrowserCleaner::QaxBrowserCleaner(QObject *parent)
    : Cleaner(parent),
      qax_browser_cache_path_("")
{
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        if (pw->pw_dir) {
            qax_browser_cache_path_ = QString("%1/.cache/qaxbrowser/Default").arg(pw->pw_dir);
        } else {
            qCritical() << "Qax browser cleaner get home path fail.";
        }
    } else {
        qCritical() << "Qax browser cleaner get pw info fail.";
    }
}

QString QaxBrowserCleaner::Mark() const {
    return "qax_browser_cleaner";
}

enum Type QaxBrowserCleaner::Type() const {
    return Type::kInternetJunk;
}

enum Level QaxBrowserCleaner::Level() const {
    return Level::kNoEffect;
}

QString QaxBrowserCleaner::Name() const {
    return tr("QaxBrowser");
}

QString QaxBrowserCleaner::Description() const {
    return tr("Some web pages or resources cached by the browser when loading pages");
}

void QaxBrowserCleaner::Scan() {
    qax_browser_junk_.clear();

    quint64 total_size{0};
    quint64 junk_mark{0};
    do {
        if (qax_browser_cache_path_.isEmpty()) {
            qCritical() << "Qax browser cleaner scan get cache path fail.";
            break;
        }
        QDir dir(qax_browser_cache_path_);
        auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry: entries) {
            quint64 junk_size{0};
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
            qax_browser_junk_.insert(junk_mark, absolute_file_path);
            total_size += junk_size;
            Q_EMIT sig_ScanForJunk(Mark(), junk_item);
        }
    } while (0);

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void QaxBrowserCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark: junk_marks) {
        auto iter = qax_browser_junk_.find(junk_mark);
        if (iter == qax_browser_junk_.end()) {
            qCritical() << "Qax browser cleaner clean junk mark [" << junk_mark << "] is not exist.";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        QFileInfo file_info(iter.value());
        if (file_info.exists()) {
            if (file_info.isDir()) {
                K::Utils::RemoveDir(iter.value());
            } else {
                if (!QFile::remove(iter.value())) {
                    qCritical() << "Qax browser cleaner clean [" << iter.value() << "] fail.";
                }
            }
        }
        qax_browser_junk_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }

    Q_EMIT sig_CleanFinish(Mark());
}

}
