#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_FAULT_DIAGNOSIS_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_FAULT_DIAGNOSIS_H

#include <string>
#include <QWidget>
#include <QScopedPointer>
#include <QStackedWidget>
#include "kom_application_interface.h"

class QThread;

namespace fault_diagnosis {

class FaultDiagnosis: public KomApplicationInterface {
public:
    FaultDiagnosis();
    virtual ~FaultDiagnosis();
    virtual void init(void (*frame_callback)(const char * function_name, ...)) override;
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;

private:
    enum StackedWidget {
        kHomePage,
        kMainWindow
    };

    QScopedPointer<QThread> thread_;
};

class FaultDiagnosisProvider: public KomApplicationProvider {
public:
    KomApplicationInterface *create() const override;
};

}

#endif
