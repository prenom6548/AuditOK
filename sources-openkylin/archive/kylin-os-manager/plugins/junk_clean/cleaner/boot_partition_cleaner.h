#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_BOOT_PARTITION_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_BOOT_PARTITION_CLEANER_H

#include "cleaner.h"

namespace junk_clean {

class BootPartitionCleaner: public Cleaner {
    Q_OBJECT
public:
    BootPartitionCleaner(QObject *parent);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private Q_SLOTS:
    void on_ScanForJunk(QString cleaner_mark, quint64 junk_mark, QString path, quint64 size);
    void on_ScanFinish(QString cleaner_mark, quint64 total_size);
    void on_CleanForJunk(QString cleaner_mark, quint64 junk_mark);
    void on_CleanFinish(QString cleaner_mark);
};

}

#endif
