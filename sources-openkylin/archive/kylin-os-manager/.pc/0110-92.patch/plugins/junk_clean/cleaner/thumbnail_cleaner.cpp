#include "thumbnail_cleaner.h"

namespace junk_clean {

ThumbnailCleaner::ThumbnailCleaner(QObject *parent)
    : Cleaner(parent)
{

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
    Q_EMIT sig_ScanFinish(Mark(), 0);
}

void ThumbnailCleaner::Clean(QList<quint64> junk_marks) {
    Q_EMIT sig_CleanFinish(Mark());
}

}
