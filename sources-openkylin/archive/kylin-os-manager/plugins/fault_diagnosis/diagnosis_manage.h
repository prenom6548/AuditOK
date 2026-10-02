#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_REGISTER_CENTER_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_REGISTER_CENTER_H

#include <QMap>
#include <QSharedPointer>
#include "diagnosis.h"

namespace fault_diagnosis {

class DiagnosisManage: public QObject {
    Q_OBJECT
public:
    DiagnosisManage(QObject *parent = nullptr);
    const QMap<QString, QSharedPointer<Diagnosis>> &Diagnoses();
    QSharedPointer<Diagnosis> DiagnosisWithMark(const QString &mark);

Q_SIGNALS:
    void sig_CheckProgress(QString mark, qint64 base_diagnostic_mark, qint32 progress);
    void sig_RepairProgress(QString mark, qint64 base_diagnostic_mark, qint32 progress);
    void sig_Checked(QString mark, fault_diagnosis::DiagnosticResult diagnostic_result);
    void sig_CheckFinish(QString mark, QString base_diagnostic_mark, bool ok);
    void sig_Fixed(QString mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);
    void sig_RepairFinish(QString mark, QString base_diagnostic_mark, bool ok);

private:
    void Register(const QSharedPointer<Diagnosis> &diagnosis);

    QMap<QString, QSharedPointer<Diagnosis>> manager_;
};

}

#endif
