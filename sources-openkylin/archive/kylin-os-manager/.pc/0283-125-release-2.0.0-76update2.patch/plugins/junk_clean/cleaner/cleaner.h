#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_CLEANER_H

#include <QObject>
#include <QString>
#include "defines.h"

namespace junk_clean {

class Cleaner: public QObject {
    Q_OBJECT
public:
    Cleaner(QObject *parent = nullptr): QObject(parent) {}
    virtual QString Mark() const = 0;
    virtual enum Type Type() const = 0;
    virtual enum Level Level() const = 0;
    virtual QString Name() const = 0;
    virtual QString Description() const = 0;
    virtual void Scan() = 0;
    virtual void Clean(QList<quint64> junk_marks) = 0;
    void SetCancelPtr(bool *cancel) {
        cancelPtr = cancel;
    }

protected:
    bool *cancelPtr = nullptr;

Q_SIGNALS:
    void sig_ScanForJunk(QString mark, junk_clean::JunkItem junk_item);
    void sig_ScanFinish(QString mark, quint64 total_size);
    void sig_CleanForJunk(QString cleaner_mark, quint64 junk_mark);
    void sig_CleanFinish(QString cleaner_mark);
};

}

#endif
