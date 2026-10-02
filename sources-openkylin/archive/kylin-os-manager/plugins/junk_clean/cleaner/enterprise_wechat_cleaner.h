#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_ENTERPRISE_WECHAT_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEANER_ENTERPRISE_WECHAT_H

#include "cleaner.h"

namespace junk_clean {

class EnterpriseWechatCleaner: public Cleaner {
    Q_OBJECT
public:
    EnterpriseWechatCleaner(QObject *parent);
    bool Enable() const override;
    QString Mark() const override;
    enum Type Type() const override;
    enum Level Level() const override;
    QString Name() const override;
    QString Description() const override;
    void Scan() override;
    void Clean(QList<quint64> junk_marks) override;

private:
    QMap<quint64, QString> enterprise_wechat_junk_;
};

}

#endif
