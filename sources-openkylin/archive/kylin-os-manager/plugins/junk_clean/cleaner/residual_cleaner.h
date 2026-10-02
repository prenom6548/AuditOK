#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_RESIDUAL_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_RESIDUAL_CLEANER_H

#include <QMap>
#include "cleaner.h"

namespace junk_clean {

class ResidualCleaner: public Cleaner {
    Q_OBJECT
public:
    ResidualCleaner(QObject *parent = nullptr);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private Q_SLOTS:
    void on_ScanForJunk(QString clean_mark, quint64 junk_mark, QString path, quint64 size);
    void on_ScanFinish(QString clean_mark, quint64 total_size);
    void on_CleanForJunk(QString clean_mark,quint64 junk_mark);
    void on_CleanFinish(QString clean_mark);

private:
    QString dbus_uninstall_residual_cleaner_mark_;
    QMap<quint64, QString> packages_;
};

}

#endif
