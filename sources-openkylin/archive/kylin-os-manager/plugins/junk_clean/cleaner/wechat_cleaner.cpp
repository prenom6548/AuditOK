#include "wechat_cleaner.h"
#include <unistd.h>
#include <pwd.h>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include "utils.h"

namespace junk_clean {

WechatCleaner::WechatCleaner(QObject *parent)
    : Cleaner(parent),
      junk_mark_(0),
      total_size_(0)
{
    // 获取家目录
    QString home_path;
    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        home_path = pw->pw_dir;
    }

    // 扫描网页版微信
    auto scan_web_wechat = [this, home_path]() {
        QString cache_path = QString("%1/.config/weixin/wechat/users").arg(home_path);
        QDir dir(cache_path);
        if (!dir.exists()) {
            return;
        }

        auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
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
            auto user_cache_path = QString("%1/%2/message").arg(cache_path, entry);
            FindJunkFromDirectory(user_cache_path);
        }
    };
    scan_funs_.push_back(std::move(scan_web_wechat));

    /*
     * 扫描 Universal 版本微信
     * ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/cache
     * ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/temp
     * ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/msg/file/
     * ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/msg/video/
     */
    auto scan_universal_wechat = [this]() {
        auto home_document_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);

        QString cache_path = QString("%1/xwechat_files").arg(home_document_path);
        QDir dir(cache_path);
        if (!dir.exists()) {
            return;
        }

        auto entries = dir.entryList(QDir::Filter::NoDotAndDotDot | QDir::Filter::Dirs | QDir::Filter::NoSymLinks);
        for (const auto &entry: entries) {
            // 拼接 cache 目录
            QString cache = QString("%1/%2/cache").arg(cache_path, entry);
            FindJunkFromDirectory(cache);

            // 拼接 temp 目录
            QString temp = QString("%1/%2/temp").arg(cache_path, entry);
            FindJunkFromDirectory(temp);

            // 拼接 file 目录
            QString file = QString("%1/%2/msg/file").arg(cache_path, entry);
            FindJunkFromDirectory(file);

            // 拼接 video 目录
            QString video = QString("%1/%2/msg/video").arg(cache_path, entry);
            FindJunkFromDirectory(video);
        }
    };
    scan_funs_.push_back(std::move(scan_universal_wechat));

    /*
     * 扫描 kmre 版本微信
     * ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Cache/
     * ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Temp
     * ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Sns/Cache/
     * ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/File
     * ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Video
     */
    auto scan_kmre_wechat = [this, home_path]() {
        auto cache_path = QString("%1/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files").arg(home_path);
        QDir dir(cache_path);
        if (!dir.exists()) {
            return;
        }

        auto entries = dir.entryList(QDir::Filter::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const auto &entry: entries) {
            if (entry.size() < 18) {
                continue;
            }
            if (!entry.startsWith("wxid_")) {
                continue;
            }

            // 拼接 cache 目录
            auto cache = QString("%1/%2/FileStorage/Cache").arg(cache_path, entry);
            FindJunkFromDirectory(cache);

            // 拼接 temp 目录
            auto temp = QString("%1/%2/FileStorage/Temp").arg(cache_path, entry);
            FindJunkFromDirectory(temp);

            // 拼接 Sns/Cache 目录
            auto sns_cache = QString("%1/%2/FileStorage/Sns/Cache").arg(cache_path, entry);
            FindJunkFromDirectory(sns_cache);

            // 拼接 File 目录
            auto file = QString("%1/%2/FileStorage/File").arg(cache_path, entry);
            FindJunkFromDirectory(file);

            // 拼接 Video 目录
            auto video = QString("%1/%2/FileStorage/Video").arg(cache_path, entry);
            FindJunkFromDirectory(video);
        }
    };
    scan_funs_.push_back(std::move(scan_kmre_wechat));
}

bool WechatCleaner::Enable() const {
    auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);
    auto home_document_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);

    auto web_wechat_check_path = QString("%1/.config/weixin/wechat/users").arg(home_path);
    auto universal_wechat_check_path = QString("%1/xwechat_files").arg(home_document_path);
    auto kmre_wechat_check_path = QString("%1/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files").arg(home_path);

    QDir web_wechat(web_wechat_check_path);
    QDir universal_wechat(universal_wechat_check_path);
    QDir kmre_wechat(kmre_wechat_check_path);
    if (!web_wechat.exists() && !universal_wechat.exists() && !kmre_wechat.exists()) {
        return false;
    }

    return true;
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
    junk_mark_ = 0;
    total_size_ = 0;

    for (const auto &func: scan_funs_) {
        func();
    }

    Q_EMIT sig_ScanFinish(Mark(), total_size_);
}

void WechatCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark: junk_marks) {
        auto iter = wechat_junk_.find(junk_mark);
        if (iter == wechat_junk_.end()) {
            qWarning() << "Wechat cleaner clean junk mark [" << junk_mark << "] is not exits";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        QFileInfo file_info(iter.value());
        if (file_info.isDir()) {
            K::Utils::RemoveDir(iter.value());
        } else {
            // 文件
            if (!QFile::remove(iter.value())) {
                qCritical() << "wechat cleaner clean [" << iter.value() << "] file fail";
            }
        }
        wechat_junk_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }
    Q_EMIT sig_CleanFinish(Mark());
}

void WechatCleaner::FindJunkFromDirectory(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    auto size = K::Utils::CalculateDirSize(dir);

    JunkItem junk_item;
    junk_item.mark = ++junk_mark_;
    junk_item.path = path;
    junk_item.size = size;

    total_size_ += size;
    wechat_junk_.insert(junk_mark_, path);
    Q_EMIT sig_ScanForJunk(Mark(), junk_item);
}

}
