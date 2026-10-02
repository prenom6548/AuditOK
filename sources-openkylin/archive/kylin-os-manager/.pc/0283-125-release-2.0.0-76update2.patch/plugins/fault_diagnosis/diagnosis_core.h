#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSIS_CORE_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSIS_CORE_H

#include <QObject>
#include <QMap>
#include "defines.h"
#include "diagnosis_manage.h"
#include "database.h"

namespace fault_diagnosis {

class DiagnosisCore: public QObject {
    Q_OBJECT
public:
    DiagnosisCore();
    static const QMap<enum DiagnosisType, QString> &DiagnosisTypeI18n();

Q_SIGNALS:
    void sig_DiagnosticsListFinished(fault_diagnosis::CategoryDiagnosticEntry category_diagnostic_entry);
    void sig_Checked(QString mark, fault_diagnosis::DiagnosticResult diagnostic_result);
    void sig_CheckFinished(QString mark, QString base_diagnostic_mark, bool ok);
    void sig_RepairFinished(fault_diagnosis::RepairEntryResult repair_entry_result);
    void sig_CheckProgress(QString mark, qint64 base_diagnostic_mark, qint32 progress);
    void sig_RepairProgress(QString mark, qint64 base_diagnostic_mark, qint32 progress);
    void sig_Fixed(QString mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);

public Q_SLOTS:
    void on_DiagnosticsList(fault_diagnosis::Mode mode, QVariant data);
    void on_Check(fault_diagnosis::CheckEntry check_entry);
    void on_Repair(fault_diagnosis::RepairEntry repair_entry);
    void on_Fixed(QString mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);
    void on_RepairFinish(QString mark, QString base_diagnostic_mark, bool ok);

private:
    void Init();
    bool IsEnableInternalNetworkDiagnosis() const;

    static QMap<enum DiagnosisType, QString> diagnosis_type_i18n_;
    DiagnosisManage *diagnosis_manager_;
};

}

#endif
