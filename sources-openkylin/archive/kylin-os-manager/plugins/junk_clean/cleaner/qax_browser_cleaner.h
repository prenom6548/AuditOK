#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_QAX_BROWSER_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_QAX_BROWSER_CLEANER_H

#include "cleaner.h"
#include <QMap>

namespace junk_clean {

class QaxBrowserCleaner: public Cleaner {
    Q_OBJECT
public:
    QaxBrowserCleaner(QObject *parent = nullptr);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    QString qax_browser_cache_path_;
    QMap<quint64, QString> qax_browser_junk_;
};

}

#endif
