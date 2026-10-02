#include "wechat_cleaner.h"
#include <unistd.h>
#include <pwd.h>
#include <QDebug>
#include <QDir>
#include "utils.h"

namespace junk_clean {

WechatCleaner::WechatCleaner(QObject *parent)
    : Cleaner(parent)
{
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        if (pw->pw_dir) {
            wechat_users_path_ = QString("%1/.config/weixin/wechat/users").arg(pw->pw_dir);
        } else {
            qCritical() << "Wechat cleaner get home path fail.";
        }
    } else {
        qCritical() << "Wechat cleaner get pw info fail.";
    }
}

QString WechatCleaner::Mark() const {
    return "wechat_cleaner";
}

enum Type WechatCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level WechatCleaner::Level() const {
    return Level::kCareful;
}

QString WechatCleaner::Name() const {
    return tr("Wechat");
}

QString WechatCleaner::Description() const {
    return tr("Clean up files received on wechat");
}

void WechatCleaner::Scan() {
    wechat_junk_.clear();
    quint64 total_size{0};
    quint64 junk_mark{0};

    do {
        QDir dir(wechat_users_path_);
        if (!dir.exists()) {
            qCritical() << "Wechat cleaner sacn wechat users path is not exist.";
            break;
        }
        auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry: entries) {
            // 用户目录名需要大于 32 个字符
            if (entry.size() < 32) {
                continue;
            }
            // 用户目录名全部为数字或字母
            bool is_letter_or_number{true};
            for (const auto &c: entry) {
                if (!c.isLetterOrNumber()) {
                    is_letter_or_number = false;
                    break;
                }
            }
            if (!is_letter_or_number) {
                continue;
            }
            // 拼接用户数据缓存目录
            auto user_cache_dir = QString("%1/%2/message").arg(wechat_users_path_).arg(entry);
            auto user_cache_dir_size = K::Utils::CalculateDirSize(QDir(user_cache_dir));

            JunkItem junk_item;
            junk_item.mark = ++junk_mark;
            junk_item.path = user_cache_dir;
            junk_item.size = user_cache_dir_size;

            total_size += user_cache_dir_size;
            wechat_junk_.insert(junk_mark, user_cache_dir);
            Q_EMIT sig_ScanForJunk(Mark(), junk_item);
        }
    } while (0);

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void WechatCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark: junk_marks) {
        auto iter = wechat_junk_.find(junk_mark);
        if (iter == wechat_junk_.end()) {
            qWarning() << "Wechat cleaner clean junk mark [" << junk_mark << "] is not exits";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        K::Utils::RemoveDir(iter.value());
        wechat_junk_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }

    Q_EMIT sig_CleanFinish(Mark());
}

}
