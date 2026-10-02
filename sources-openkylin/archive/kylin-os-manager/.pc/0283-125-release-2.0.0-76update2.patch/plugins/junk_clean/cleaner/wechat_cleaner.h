#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_WECHAT_CLEANER_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_WECHAT_CLEANER_H

#include <vector>
#include <functional>
#include "cleaner.h"

namespace junk_clean {

class WechatCleaner: public Cleaner {
    Q_OBJECT
public:
    WechatCleaner(QObject *parent);
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    void FindJunkFromDirectory(const QString &path);

    quint64 junk_mark_;
    quint64 total_size_;
    std::vector<std::function<void()>> scan_funs_;
    QMap<quint64, QString> wechat_junk_;
};

}

#endif
