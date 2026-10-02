#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_TRASH_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_TRASH_CLEANER_H

#include <gio/gio.h>
#include "cleaner.h"

namespace junk_clean {

class TrashCleaner: public Cleaner {
    Q_OBJECT
public:
    TrashCleaner(QObject *parent = nullptr);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    quint64 GetTrashItemSize(GFile *file) const;

    QMap<quint64, QString> trash_junk_;
};

}

#endif
