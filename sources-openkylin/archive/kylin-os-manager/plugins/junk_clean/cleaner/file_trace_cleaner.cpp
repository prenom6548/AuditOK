#include "file_trace_cleaner.h"
#include <unistd.h>
#include <pwd.h>
#include <QDebug>
#include <QDomDocument>
#include <QFile>

namespace junk_clean {

FileTraceCleaner::FileTraceCleaner(QObject *parent)
    : Cleaner(parent),
      recently_file_("")
{
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        if (pw->pw_dir) {
            recently_file_ = QString("%1/.local/share/recently-used.xbel").arg(pw->pw_dir);
        } else {
            qCritical() << "File trace cleaner get user home dir fail.";
        }
    } else {
        qCritical() << "File trace cleaner get user pw info fail: " << strerror(errno);
    }
}

QString FileTraceCleaner::Mark() const {
    return "file_trace_cleaner";
}

enum Type FileTraceCleaner::Type() const {
    return Type::kUsageTraces;
}

enum Level FileTraceCleaner::Level() const {
    return Level::kNoEffect;
}

QString FileTraceCleaner::Name() const {
    return tr("File recording");
}

QString FileTraceCleaner::Description() const {
    return tr("Recently opened file records");
}

void FileTraceCleaner::Scan() {
    file_traces_.clear();

    quint64 junk_mark{0};
    quint64 total_size{0};
    do {
        if (recently_file_.isEmpty()) {
            qCritical() << "File trace scan get record file path fail.";
            break;
        }

        QFile file(recently_file_);
        if (!file.exists()) {
            qCritical() << "File trace scan record file is not exist.";
            break;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "File trace scan open record file fail.";
            break;
        }

        QDomDocument doc;
        if (!doc.setContent(&file)) {
            qCritical() << "File trace scan analysis record file fail.";
            file.close();
            break;
        }
        file.close();

        auto root = doc.documentElement();
        auto node = root.firstChild();
        while (!node.isNull()) {
            if (node.isElement()) {
                auto element = node.toElement();
                if (element.tagName() == "bookmark" && element.hasAttribute("href")) {
                    auto href = element.attribute("href");
                    JunkItem junk_item;
                    junk_item.mark = ++junk_mark;
                    junk_item.path = href;
                    junk_item.size = 1;
                    file_traces_.insert(junk_mark, href);
                    total_size++;
                    Q_EMIT sig_ScanForJunk(Mark(), junk_item);
                }
            }
            node = node.nextSibling();
        }
    } while (0);

    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void FileTraceCleaner::Clean(QList<quint64> junk_marks) {
    do {
        QFile file(recently_file_);
        if (!file.exists()) {
            qCritical() << "File trace clean record file is not exist.";
            break;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "File trace clean open record file fail.";
            break;
        }

        QDomDocument doc;
        if (!doc.setContent(&file)) {
            qCritical() << "File trace clean analysis record file fail.";
            file.close();
            break;
        }
        file.close();

        for (const auto &junk_mark: junk_marks) {
            auto iter = file_traces_.find(junk_mark);
            if (iter == file_traces_.end()) {
                qWarning() << "File trace clean junk mark [" << junk_mark << "] is not exist.";
                Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
                continue;
            }
            auto root = doc.documentElement();
            auto node_list = doc.elementsByTagName("bookmark");
            for (int i = 0; i < node_list.size(); i++) {
                auto element = node_list.at(i).toElement();
                if (element.attribute("href") == iter.value()) {
                    root.removeChild(node_list.at(i));
                    file_traces_.erase(iter);
                    Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
                    break;
                }
            }
        }

        auto data = doc.toByteArray(2);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qCritical() << "File trace clean write open record file fail.";
            break;
        }
        while (!data.isEmpty()) {
            auto written = file.write(data);
            data.remove(0, written);
        }
        file.close();
    } while (0);

    Q_EMIT sig_CleanFinish(Mark());
}

}
