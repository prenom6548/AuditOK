#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_SYSTEM_MEMORY_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_SYSTEM_MEMORY_CLEANER_H

#include "cleaner.h"

namespace junk_clean {

class SystemMemoryCleaner: public Cleaner {
    Q_OBJECT
public:
    SystemMemoryCleaner(QObject *parent = nullptr);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private Q_SLOTS:
    void on_CleanFinish(QString clean_mark);

private:
    QString dbus_system_memory_cleaner_mark_;
};

}

#endif
