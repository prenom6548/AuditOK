#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_LOG_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_LOG_CLEANER_H

#include "cleaner.h"
#include <QMap>

namespace junk_clean {

class LogCleaner: public Cleaner {
    Q_OBJECT
public:
    LogCleaner(QObject *parent);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    void RecursiveScanInDir(const QString &dir_path);

    QString home_log_path_;
    quint64 total_size_;
    quint64 junk_mark_;
    QMap<quint64, QString> log_junks_;
};

}

#endif
