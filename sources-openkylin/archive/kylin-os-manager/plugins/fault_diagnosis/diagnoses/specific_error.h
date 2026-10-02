#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSES_SPECIFIC_ERROR_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_DIAGNOSES_SPECIFIC_ERROR_H

#include "diagnosis.h"

namespace fault_diagnosis {

class SpecificError: public Diagnosis {
    Q_OBJECT
public:
    SpecificError(QObject *parent = nullptr);
    ~SpecificError();

    QString Mark() override;
    bool DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories, QVariant data = QVariant()) override;
    bool Check(const QString &base_diagnostic_mark, QVariant data = QVariant()) override;
    bool Repair(const QString &base_diagnostic_mark, const QList<RepairItem> &repair_item) override;

private Q_SLOTS:
    void on_Checked(QString result);
    void on_CheckFinish(bool ok);

private:
    QString current_diagnostic_mark_;
};

}

#endif
