#include "qq_cleaner.h"
#include <unistd.h>
#include <pwd.h>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include "utils.h"

namespace junk_clean {

QQCleaner::QQCleaner(QObject *parent)
    : Cleaner(parent),
      junk_mark_(0),
      total_size_(0)
{
    // 获取 QQ 缓存目录
    QString home_path;
    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        home_path = pw->pw_dir;
    }
    cache_path_ = QString("%1/.config/QQ").arg(home_path);

    // 扫描 QQ emoji 缓存
    auto scan_emoji_cache = [this]() {
        QDir dir(cache_path_);
        if (!dir.exists()) {
            return;
        }

        auto dir_list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const auto &d: dir_list) {
            // nt_qq_733699988b24ca70bcb9c107b157f4c6
            if (d.size() < 38) {
                continue;
            }
            if (!d.startsWith("nt_qq_")) {
                continue;
            }

            // 拼接 Emoji 缓存路径
            QString emoji_cache_path = QString("%1/%2/nt_data/Emoji/marketface").arg(cache_path_, d);
            QDir emoji_cache_dir(emoji_cache_path);
            if (!emoji_cache_dir.exists()) {
                continue;
            }

            auto emoji_cache_size = K::Utils::CalculateDirSize(emoji_cache_dir);

            JunkItem junk_item;
            junk_item.mark = ++junk_mark_;
            junk_item.path = emoji_cache_path;
            junk_item.size = emoji_cache_size;
            qq_junk_.insert(junk_mark_, emoji_cache_path);
            total_size_ += emoji_cache_size;

            Q_EMIT sig_ScanForJunk(Mark(), junk_item);
        }
    };
    scan_funcs_.push_back(std::move(scan_emoji_cache));
}

bool QQCleaner::Enable() const {
    auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);

    auto QQ_check_path = QString("%1/.config/QQ").arg(home_path);
    QDir QQ(QQ_check_path);
    if (!QQ.exists()) {
        return false;
    }

    return true;
}

QString QQCleaner::Mark() const {
    return "qq_cleaner";
}

enum Type QQCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level QQCleaner::Level() const {
    return Level::kCareful;
}

QString QQCleaner::Name() const {
    return tr("QQ");
}

QString QQCleaner::Description() const {
    return tr("Temporary data generated using QQ");
}

void QQCleaner::Scan() {
    qq_junk_.clear();
    junk_mark_ = 0;
    total_size_ = 0;

    for (const auto &func: scan_funcs_) {
        func();
    }
    Q_EMIT sig_ScanFinish(Mark(), total_size_);
}

void QQCleaner::Clean(QList<quint64> junk_marks) {
    for (auto junk_mark: junk_marks) {
        const auto iter = qq_junk_.find(junk_mark);
        if (iter == qq_junk_.end()) {
            qWarning() << "QQ cleaner clean junk mark [" << junk_mark << "] is not exist.";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        QFileInfo file_info(iter.value());
        if (file_info.isDir()) {
            K::Utils::RemoveDir(iter.value());
        } else {
            // 文件
            if (!QFile::remove(iter.value())) {
                qCritical() << "QQ cleaner remove [" << iter.value() << "] file fail.";
            }
        }
        qq_junk_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }
    Q_EMIT sig_CleanFinish(Mark());
}

}
