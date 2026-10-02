#include "qax_browser_cookies_cleaner.h"
#include <unistd.h>
#include <pwd.h>
#include <QDebug>
#include <QFileInfo>
#include <QFile>

namespace junk_clean {

QaxBrowserCookiesCleaner::QaxBrowserCookiesCleaner(QObject *parent)
    : Cleaner(parent),
      qax_cookies_path_("")
{
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        if (pw->pw_dir) {
            qax_cookies_path_ = QString("%1/.config/qaxbrowser/Default/Cookies").arg(pw->pw_dir);
        } else {
            qCritical() << "Qax browser cleaner get home path fail.";
        }
    } else {
        qCritical() << "Qax browser cleaner get pw info fail.";
    }
}

QString QaxBrowserCookiesCleaner::Mark() const {
    return "qax_browser_cookies_cleaner";
}

enum Type QaxBrowserCookiesCleaner::Type() const {
    return Type::kInternetJunk;
}

enum Level QaxBrowserCookiesCleaner::Level() const {
    return Level::kCareful;
}

QString QaxBrowserCookiesCleaner::Name() const {
    return tr("Cookies information");
}

QString QaxBrowserCookiesCleaner::Description() const {
    return tr("Browser saved login information");
}

void QaxBrowserCookiesCleaner::Scan() {
    cookies_.clear();
    quint64 junk_mark{0};
    quint64 total_size{0};

    QFileInfo file_info(qax_cookies_path_);
    if (file_info.exists()) {
        JunkItem junk_item;
        junk_item.mark = ++junk_mark;
        junk_item.path = qax_cookies_path_;
        junk_item.size = file_info.size();

        total_size += file_info.size();
        cookies_.insert(junk_mark, qax_cookies_path_);
        Q_EMIT sig_ScanForJunk(Mark(), junk_item);
    }

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void QaxBrowserCookiesCleaner::Clean(QList<quint64> junk_marks) {
    for (const auto &junk_mark: junk_marks) {
        auto iter = cookies_.find(junk_mark);
        if (iter == cookies_.end()) {
            qWarning() << "Qax browser cookies cleaner junk mark [" << junk_mark << "] is not exist.";
            Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
            continue;
        }
        if (QFile::exists(iter.value())) {
            if (!QFile::remove(iter.value())) {
                qCritical() << "Qax browser cookies cleaner clean [" << iter.value() << "] fail.";
            }
        }
        cookies_.erase(iter);
        Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
    }

    Q_EMIT sig_CleanFinish(Mark());
}

}
