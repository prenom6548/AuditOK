#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_JUNK_CLEAN_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_JUNK_CLEAN_H

#include "kom_application_interface.h"

namespace junk_clean {

class JunkClean: public QObject, public KomApplicationInterface {
    Q_OBJECT
public:
    JunkClean();
    virtual ~JunkClean();
    virtual void init(void (*frame_callback)(const char *function_name, ...)) override;
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;
    virtual void communication(enum CommunicationType communication_type, QVariant data) override;

Q_SIGNALS:
    void sig_StartScan(QVariant data);

private:
    enum StackWidget {
        kHomepage,
        kMainWindow,
        kCleanFinish
    };
};

class JunkCleanProvider: public KomApplicationProvider {
public:
    KomApplicationInterface *create() const override;
};

}

#endif
