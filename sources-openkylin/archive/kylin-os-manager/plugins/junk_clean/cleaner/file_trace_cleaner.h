#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_FILE_TRACE_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_FILE_TRACE_CLEANER_H

#include "cleaner.h"
#include <QMap>

namespace junk_clean {

class FileTraceCleaner: public Cleaner {
    Q_OBJECT
public:
    FileTraceCleaner(QObject *parent = nullptr);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    QString recently_file_;
    QMap<quint64, QString> file_traces_;
};

}

#endif
