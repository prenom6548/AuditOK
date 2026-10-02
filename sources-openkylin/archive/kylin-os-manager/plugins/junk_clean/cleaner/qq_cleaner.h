#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_QQ_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_QQ_CLEANER_H

#include <functional>
#include <vector>
#include <QMap>
#include "cleaner.h"

namespace junk_clean {

class QQCleaner: public Cleaner {
    Q_OBJECT
public:
    QQCleaner(QObject *parent);
    bool Enable() const override;
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    quint64 junk_mark_;
    quint64 total_size_;
    QString cache_path_;
    std::vector<std::function<void()>> scan_funcs_;
    QMap<quint64, QString> qq_junk_;
};

}

#endif
