#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_CLEANER_MANAGER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_CLEANER_MANAGER_H

#include <memory>
#include <QObject>
#include <QMap>
#include "cleaner.h"

namespace junk_clean {

class CleanerManager: public QObject {
    Q_OBJECT
public:
    CleanerManager(bool *cancelPtr,QObject *parent = nullptr);
    const QMap<QString, Cleaner *> &Cleaners() const;
    Cleaner *CleanerWithMark(const QString &mark) const;

Q_SIGNALS:
    void sig_ScanForJunk(QString mark, junk_clean::JunkItem junk_item);
    void sig_ScanFinish(QString mark, quint64 total_size);
    void sig_CleanForJunk(QString cleaner_mark, quint64 junk_mark);
    void sig_CleanFinish(QString cleaner_mark);

private:
    void Register(Cleaner *cleaner);
    QMap<QString, Cleaner *> cleaners_;
    bool *m_cancelPtr = nullptr;
};

}

#endif
