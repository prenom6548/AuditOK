#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_QAX_BROWSER_COOKIES_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_QAX_BROWSER_COOKIES_CLEANER_H

#include <QMap>
#include "cleaner.h"

namespace junk_clean {

class QaxBrowserCookiesCleaner: public Cleaner {
    Q_OBJECT
public:
    QaxBrowserCookiesCleaner(QObject *parent = nullptr);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    QString qax_cookies_path_;
    QMap<quint64, QString> cookies_;
};

}

#endif
