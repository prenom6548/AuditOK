#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSIS_CORE_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSIS_CORE_H

#include <QObject>
#include "defines.h"
#include "diagnosis_manage.h"
#include "database.h"

namespace fault_diagnosis {

class DiagnosisCore: public QObject {
    Q_OBJECT
public:
    DiagnosisCore();

Q_SIGNALS:
    void sig_DiagnosticsListFinished(fault_diagnosis::CategoryDiagnosticEntry category_diagnostic_entry);
    void sig_CheckFinished(fault_diagnosis::CheckEntryResult check_entry_result);
    void sig_RepairFinished(fault_diagnosis::RepairEntryResult repair_entry_result);
    void sig_CheckProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    void sig_RepairProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    void sig_Fixed(QString diagnosis_mark, qint64 error_code, bool ok, QString reason);

public Q_SLOTS:
    void on_DiagnosticsList();
    void on_Check(fault_diagnosis::CheckEntry check_entry);
    void on_CheckFinish(bool ok, fault_diagnosis::DiagnosticResult diagnostic_result);
    void on_Repair(fault_diagnosis::RepairEntry repair_entry);
    void on_RepairFinish(bool ok);

private:
    DiagnosisManage *diagnosis_manager_;
    CheckEntry current_check_entry_;
    RepairEntry current_repair_entry_;
};

}

#endif
