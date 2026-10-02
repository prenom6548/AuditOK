#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_SERVICE_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_SERVICE_H

#include <QObject>
#include "defines.h"
#include "cleaner_manager.h"

namespace junk_clean {

class CleanUpService: public QObject {
    Q_OBJECT
public:
    CleanUpService(bool *cancelPtr,QObject *parent = nullptr);

Q_SIGNALS:
    void sig_CleanUpItemsFinish(junk_clean::TypeCleanUpItems type_clean_up_items);
    void sig_ScanForJunk(QString mark, junk_clean::JunkItem junk_item);
    void sig_ScanFinish(QString mark, quint64 total_size);
    void sig_CleanForJunk(QString cleaner_mark, quint64 junk_mark);
    void sig_CleanFinish(QString cleaner_mark);

public Q_SLOTS:
    void on_CleanUpItems();
    void on_Scan(QString mark);
    void on_Clean(junk_clean::CleanTask task);

private:
    CleanerManager *cleaner_manager_;
};

}

#endif
