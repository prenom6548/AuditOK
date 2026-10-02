#include "enterprise_wechat_cleaner.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "utils.h"

namespace junk_clean {

EnterpriseWechatCleaner::EnterpriseWechatCleaner(QObject *parent)
    : Cleaner(parent)
{

}

bool EnterpriseWechatCleaner::Enable() const {
    auto home_document_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);

    auto enterprise_wechat_check_path = QString("%1/WXWork").arg(home_document_path);
    QDir enterprise_wechat(enterprise_wechat_check_path);
    if (!enterprise_wechat.exists()) {
        return false;
    }

    return true;
}

QString EnterpriseWechatCleaner::Mark() const {
    return "wechat_enterprise_cleaner";
}

enum Type EnterpriseWechatCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level EnterpriseWechatCleaner::Level() const {
    return Level::kCareful;
}

QString EnterpriseWechatCleaner::Name() const {
    return tr("Enterprise Wechat");
}

QString EnterpriseWechatCleaner::Description() const {
    return tr("Clean up files received on enterprise wechat");
}

/*
 * ~/文档/WXWork/1688855228378992/Cache/
 */
void EnterpriseWechatCleaner::Scan() {
    quint64 junk_mark{0};
    quint64 total_size{0};
    enterprise_wechat_junk_.clear();

    do {
        auto home_document_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
        auto cache_path = QString("%1/WXWork").arg(home_document_path);
        QDir dir(cache_path);
        if (!dir.exists()) {
            break;
        }

        auto entries = dir.entryList(QDir::Filter::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const auto &entry: entries) {
            if (entry.size() < 16) {
                continue;
            }

            bool is_number{true};
            for (const auto &c: entry) {
                if (!c.isNumber()) {
                    is_number = false;
                    break;
                }
            }
            if (!is_number) {
                continue;
            }

            // 拼接 cache 目录
            auto cache = QString("%1/%2/Cache").arg(cache_path, entry);
            QDir d(cache);
            if (!d.exists()) {
                break;
            }

            auto size = K::Utils::CalculateDirSize(d);

            JunkItem junk_item;
            junk_item.mark = ++junk_mark;
            junk_item.size = size;
            junk_item.path = cache;

            total_size += size;
            enterprise_wechat_junk_.insert(junk_mark, cache);
            Q_EMIT sig_ScanForJunk(Mark(), junk_item);
        }
    } while(0);

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void EnterpriseWechatCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark: junk_marks) {
        auto iter = enterprise_wechat_junk_.find(junk_mark);
        if (iter == enterprise_wechat_junk_.end()) {
            qCritical() << "Enterprise wechat cleaner clean junk mark [" << junk_mark << "] is not exits";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        QFileInfo file_info(iter.value());
        if (file_info.isDir()) {
            K::Utils::RemoveDir(iter.value());
        } else {
            // 文件
            if (!QFile::remove(iter.value())) {
                qCritical() << "Enterprise wechat cleaner clean [" << iter.value() << "] file fail";
            }
        }
        enterprise_wechat_junk_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }
    Q_EMIT sig_CleanFinish(Mark());
}

}
